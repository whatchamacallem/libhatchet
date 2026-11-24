# SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
# SPDX-License-Identifier: MIT
# This file is licensed under the terms of the LICENSE.md file.

# Containers without access to USB devices still see them in sysfs.
usb_connected() {
	local device
	for device in /sys/bus/usb/devices/*; do
		[[ "$(cat "$device/idVendor" 2>/dev/null)" == "$1" ]] || continue
		[[ -e "$(printf '/dev/bus/usb/%03d/%03d' "$(cat "$device/busnum")" \
			"$(cat "$device/devnum")")" ]] && return 0
	done
	return 1
}

# In a container, boards are shared by usbipd-win on the host "windows". A
# board that re-enumerates, such as a Pico switching between BOOTSEL and the
# program, is detached and has to be attached again.
usb_attach() {
	command -v usbip &>/dev/null && getent hosts windows &>/dev/null || return 0
	usb_connected "$1" && return 0
	local busid
	busid=$(usbip list -r windows 2>/dev/null |
		sed -n "s/^[[:space:]]*\([0-9][0-9.-]*\): .*($1:[0-9a-f]*)\$/\1/p" | head -n 1)
	if [[ -n "$busid" ]] && usbip attach -r windows -b "$busid" &>/dev/null; then
		# The device is added after usbip returns, so it is waited for. udev
		# then gives it to plugdev and adds its serial port.
		local i
		for i in {1..50}; do
			usb_connected "$1" && break
			sleep 0.1
		done
		udevadm settle 2>/dev/null || sleep 1
	fi
}

usb_serial_port() {
	local tty
	for tty in /sys/class/tty/ttyACM*; do
		if [[ "$(cat "$tty/device/../idVendor" 2>/dev/null)" == "$1" ]]; then
			echo "/dev/${tty##*/}"
			return 0
		fi
	done
	return 1
}
