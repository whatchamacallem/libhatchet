#!/bin/sh
# SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
# SPDX-License-Identifier: MIT
# This file is licensed under the terms of the LICENSE.md file.
#
# The entrypoint of the libhatchet-cache-* images built by test_msvc_wslc.ps1.
# It runs the container's command as t, a user without privileges. Root is only
# used first to set up what t can't:
#
# - ThreadSanitizer requires the session VM's ASLR entropy to be lowered, which
#   requires --cap-add SYS_ADMIN.
# - With --cap-add SYS_MODULE, the --sdks image lets the plugdev group attach
#   USB devices with usbip.
#
# SYS_ADMIN and SYS_MODULE are dropped along with root.

set -eu

# -- ThreadSanitizer -----------------------------------------------------------

# The setting belongs to the session VM, so only its first container lowers it.
# /proc/sys is read-only in a container.
lower_address_randomization() {
	[ "$(cat /proc/sys/vm/mmap_rnd_bits)" -gt 28 ] || return 0

	export LIBMOUNT_FORCE_MOUNT2=always
	if ! (mount -o remount,bind,rw,nosuid,nodev,noexec /proc/sys &&
			echo 28 >/proc/sys/vm/mmap_rnd_bits); then
		echo "error: ThreadSanitizer requires vm.mmap_rnd_bits <= 28. Run with --cap-add SYS_ADMIN." >&2
		exit 1
	fi
	mount -o remount,bind,ro,nosuid,nodev,noexec /proc/sys
}

# -- USB/IP --------------------------------------------------------------------

# usbip is installed by install_deb.sh --sdks. Its kernel module requires
# SYS_MODULE, which is bit 16 of the effective capabilities.
can_attach_devices() {
	command -v usbip >/dev/null &&
		[ $((0x$(awk '/^CapEff:/ {print $2}' /proc/self/status) >> 16 & 1)) = 1 ]
}

# usbipd-win is reached as "windows".
name_windows_address() {
	if [ -n "${HX_WINDOWS_IP:-}" ] && ! grep -q '[[:space:]]windows$' /etc/hosts; then
		printf '%s\twindows\n' "$HX_WINDOWS_IP" >>/etc/hosts
	fi
}

# Docker only allows a container a few devices, even as root, using BPF
# programs attached to its cgroup. wslc has no option to allow more, so the
# programs are detached with bpf(2).
remove_device_filter() {
	python3 - <<'EOF'
import ctypes, os, struct

BPF_PROG_DETACH, BPF_PROG_GET_FD_BY_ID, BPF_PROG_QUERY = 9, 13, 16
BPF_CGROUP_DEVICE = 6
SYS_BPF = {'x86_64': 321, 'aarch64': 280}[os.uname().machine]

libc = ctypes.CDLL(None, use_errno=True)

# Calls bpf(2) with the leading fields of a bpf_attr. Returns the result and the
# fields as the kernel left them.
def call_packet_filter(command, layout, *fields):
	attr = ctypes.create_string_buffer(struct.pack(layout, *fields), 64)
	result = libc.syscall(SYS_BPF, command, attr, len(attr))
	if result < 0:
		raise OSError(ctypes.get_errno(), os.strerror(ctypes.get_errno()))
	return result, struct.unpack_from(layout, attr)

# The query is target_fd, attach_type, query_flags, attach_flags, prog_ids and
# prog_cnt, which returns the number of programs.
cgroup = os.open('/sys/fs/cgroup', os.O_RDONLY | os.O_DIRECTORY)
programs = (ctypes.c_uint32 * 64)()
_, (*_, count) = call_packet_filter(BPF_PROG_QUERY, 'IIIIQI', cgroup,
	BPF_CGROUP_DEVICE, 0, 0, ctypes.addressof(programs), len(programs))

for program in programs[:count]:
	descriptor, _ = call_packet_filter(BPF_PROG_GET_FD_BY_ID, 'I', program)
	call_packet_filter(BPF_PROG_DETACH, 'IIII', cgroup, descriptor, BPF_CGROUP_DEVICE, 0)
EOF
}

# devtmpfs has the VM's device nodes and adds them as devices attach. It
# replaces Docker's /dev and takes over its mounts and standard symlinks.
mount_device_nodes() {
	local HX_MOUNT_ HX_TARGET_
	mkdir -p /run/dev
	mount -t devtmpfs devtmpfs /run/dev

	for HX_MOUNT_ in $(awk '$5 ~ "^/dev/" {print $5}' /proc/self/mountinfo); do
		HX_TARGET_="/run/dev${HX_MOUNT_#/dev}"
		[ -e "$HX_TARGET_" ] || mkdir -p "$HX_TARGET_"
		mount --move "$HX_MOUNT_" "$HX_TARGET_"
	done

	ln -sfn /proc/self/fd /run/dev/fd
	ln -sfn /proc/self/fd/0 /run/dev/stdin
	ln -sfn /proc/self/fd/1 /run/dev/stdout
	ln -sfn /proc/self/fd/2 /run/dev/stderr

	mount --move /run/dev /dev
}

# The VM's modules are on one of its read-only disks. modprobe and udev load
# them from /lib/modules.
mount_kernel_modules() {
	local HX_DISK_ HX_VERSION_
	HX_VERSION_=$(uname -r)
	mkdir -p /run/vm-modules "/lib/modules/$HX_VERSION_"

	for HX_DISK_ in /sys/block/*; do
		[ "$(cat "$HX_DISK_/ro")" = 1 ] || continue
		mount -o ro "/dev/${HX_DISK_##*/}" /run/vm-modules 2>/dev/null || continue
		if [ -f "/run/vm-modules/$HX_VERSION_/modules/modules.dep" ]; then
			mount --bind "/run/vm-modules/$HX_VERSION_/modules" "/lib/modules/$HX_VERSION_"
			return
		fi
		umount /run/vm-modules
	done

	echo "error: The modules for kernel $HX_VERSION_ were not found on the VM's disks." >&2
	exit 1
}

# vhci-hcd is the host controller that usbip attaches devices to. The plugdev
# group can attach and detach. usbip records the attached devices in
# /var/run/vhci_hcd.
load_host_controller() {
	local HX_CONTROLLER_
	modprobe vhci-hcd

	mount -o remount,rw /sys
	for HX_CONTROLLER_ in /sys/devices/platform/vhci_hcd.*; do
		chgrp plugdev "$HX_CONTROLLER_/attach" "$HX_CONTROLLER_/detach"
		chmod g+w "$HX_CONTROLLER_/attach" "$HX_CONTROLLER_/detach"
	done
	install -d -g plugdev -m 2775 /var/run/vhci_hcd
}

# udev applies install_deb.sh's rule, which gives the attached devices to
# plugdev, and loads their drivers. The trigger is for devices that attached
# before udev started.
start_device_manager() {
	local HX_CONTROLLER_
	SYSTEMD_LOG_LEVEL=notice /usr/lib/systemd/systemd-udevd --daemon

	for HX_CONTROLLER_ in /sys/devices/platform/vhci_hcd.*; do
		udevadm trigger --action=add --parent-match="$HX_CONTROLLER_"
	done
	udevadm settle
}

# -- Main ----------------------------------------------------------------------

# Replaces this script with the command, run as t with t's groups and a clean
# environment. Nothing t runs can get SYS_ADMIN or SYS_MODULE back.
run_without_privileges() {
	exec setpriv --reuid=t --regid=t --init-groups --reset-env \
		--inh-caps=-sys_admin,-sys_module --bounding-set=-sys_admin,-sys_module "$@"
}

main() {
	lower_address_randomization

	if can_attach_devices; then
		name_windows_address
		remove_device_filter
		mount_device_nodes
		mount_kernel_modules
		load_host_controller
		start_device_manager
	fi

	run_without_privileges "$@"
}

main "$@"
