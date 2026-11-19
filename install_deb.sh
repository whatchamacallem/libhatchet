#!/bin/bash
# SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
# SPDX-License-Identifier: MIT
# This file is licensed under the terms of the LICENSE.md file.
#
# Building and running all the tests on a Debian derivative requires these
# packages.
#
# usage: install_deb.sh [--sdks]
#
# It should be safe to just run this script. --sdks also installs the emsdk,
# which test_wasm.sh requires, the Pico SDK with picotool, which the target
# directory requires, and ESP-IDF for the ESP32-C6. They go in ~/emsdk,
# ~/pico-sdk, ~/picotool, ~/esp-idf and ~/.espressif. PATH is not modified.
# Use ". ~/esp-idf/export.sh" to enter the ESP-IDF environment.
#
# --sdks is only tested on Ubuntu and requires it. It also installs the usbip
# client for attaching USB devices shared by usbipd-win, and a udev rule that
# gives the devices it attaches to the plugdev group. The owner of ~ is added
# to the dialout and plugdev groups, which takes effect at the next login.
# Attaching still requires root unless the system allows plugdev to, as the
# test_msvc_wslc.ps1 containers do.

trap 'trap "" INT PIPE TERM; pkill -P $$; wait; exit 1' INT PIPE TERM
set -euo pipefail

export DEBIAN_FRONTEND=noninteractive

# -- Packages ------------------------------------------------------------------

HX_PACKAGES_="
	ccache       clang        clang-tidy      cmake    doxygen    g++
	g++-multilib gcc-multilib gcovr           gdb      git        llvm
	make         meson        musl            musl-dev musl-tools ninja-build
	procps       python3      libclang-rt-dev"

# --sdks. Ubuntu's usbip is in linux-tools-generic.
HX_SDK_PACKAGES_="
	bison             libffi-dev                     python3-pip
	ca-certificates   libnewlib-arm-none-eabi        python3-venv
	dfu-util          libssl-dev                     udev
	flex              libstdc++-arm-none-eabi-newlib usbutils
	gcc-arm-none-eabi libusb-1.0-0                   wget
	gperf             libusb-1.0-0-dev               xz-utils
	hwdata            linux-tools-generic
	kmod              pkg-config"

HX_EMSDK_="$HOME/emsdk"

HX_PICO_SDK_="$HOME/pico-sdk"
HX_PICO_SDK_VERSION_=2.3.1
HX_PICOTOOL_DIR_="$HOME/picotool"

# ESP-IDF installs its tools and Python venv in ~/.espressif.
HX_ESP_IDF_="$HOME/esp-idf"
HX_ESP_IDF_VERSION_=v6.1.1

# Ubuntu's /usr/bin/usbip only runs on Ubuntu kernels. This runs it on others,
# such as WSL's.
HX_USBIP_="/usr/local/bin/usbip"
HX_USBIP_SCRIPT_='#!/bin/sh
exec "$(ls -d /usr/lib/linux-tools/*-generic | sort -V | tail -n 1)/usbip" "$@"'

# Serial ports are in the dialout group by default.
HX_USBIP_RULES_="/etc/udev/rules.d/70-libhatchet-usbip.rules"
HX_USBIP_RULES_TEXT_='# Gives the plugdev group the USB devices attached by usbip.
SUBSYSTEM=="usb", ENV{DEVTYPE}=="usb_device", DRIVERS=="vhci_hcd", GROUP="plugdev", MODE="0660"
SUBSYSTEM=="hidraw", DRIVERS=="vhci_hcd", GROUP="plugdev", MODE="0660"'

# Set by main. 1 when --sdks was passed.
HX_SDKS_=0

# Set by main.
HX_SUDO_=""

# -- Separator -----------------------------------------------------------------

HX_COLORS_=(208 214 220 226 190 154 118 82 83 84 85 86 87 45 39 33
			27 33 39 45 87 86 85 84 83 82 118 154 190 226 220 214)
HX_TOTAL_COLORS_=${#HX_COLORS_[@]}
HX_COLOR_OFFSET_=0

separator() {
	local HX_I_
	for((HX_I_=40; HX_I_--;)); do
		local HX_COLOR_INDEX_=$(((HX_COLOR_OFFSET_ + HX_I_) % HX_TOTAL_COLORS_))
		echo -ne "\033[38;5;${HX_COLORS_[HX_COLOR_INDEX_]}m--"
	done
	echo -e "\033[0m"
	((HX_COLOR_OFFSET_ += 3))
}

# -- Checks --------------------------------------------------------------------

check_usbip_installed() {
	local HX_GROUPS_
	HX_GROUPS_=" $(id -nG "$(stat -c %U "$HOME")") "
	cmp -s "$HX_USBIP_" <(echo "$HX_USBIP_SCRIPT_") &&
		cmp -s "$HX_USBIP_RULES_" <(echo "$HX_USBIP_RULES_TEXT_") &&
		[[ "$HX_GROUPS_" == *" dialout "* && "$HX_GROUPS_" == *" plugdev "* ]]
}

# Succeeds when every package is installed, and with --sdks every SDK.
check_packages_installed() {
	if [ "$HX_SDKS_" = 1 ]; then
		[ -f "$HX_EMSDK_/.emscripten" ] || return 1
		[ -x "$HX_PICOTOOL_DIR_/picotool" ] || return 1
		# Filling the Python venv is the last step of the ESP-IDF install.sh. This
		# is the same check that export.sh makes.
		python3 "$HX_ESP_IDF_/tools/idf_tools.py" check-python-dependencies \
			> /dev/null 2>&1 || return 1
		check_usbip_installed || return 1
	fi
	! grep -qv '^ii' < <(dpkg-query -W -f='${db:Status-Abbrev}\n' \
		$HX_PACKAGES_ 2>&1)
}

# -- Debian Shared -------------------------------------------------------------

install_apt_packages() {
	separator
	$HX_SUDO_ apt-get update

	separator
	local HX_CMD_
	HX_CMD_=$(echo "$HX_SUDO_ apt-get install -y --no-install-recommends \
		--no-upgrade $HX_PACKAGES_" | tr -s '[:space:]' ' ')
	echo "$HX_CMD_" | fold -s

	separator
	$HX_CMD_
}

print_tool_versions() {
	local HX_CMD_
	for HX_CMD_ in clang cmake doxygen gcc gcovr meson musl-gcc ninja pgrep \
		python3
	do
		separator
		echo "$HX_CMD_ --version"
		$HX_CMD_ --version
	done
}

# -- Emscripten ----------------------------------------------------------------

install_emscripten_sdk() {
	if [ ! -f "$HX_EMSDK_/.emscripten" ]; then
		separator
		git clone --depth 1 https://github.com/emscripten-core/emsdk.git \
			"$HX_EMSDK_"
		"$HX_EMSDK_/emsdk" install latest
		"$HX_EMSDK_/emsdk" activate latest
		EMSDK_QUIET=1 . "$HX_EMSDK_/emsdk_env.sh"
		emcc --version
	fi
}

# -- Pico ----------------------------------------------------------------------

install_pico_sdk() {
	if [ ! -x "$HX_PICOTOOL_DIR_/picotool" ]; then
		separator
		[ -d "$HX_PICO_SDK_" ] || git -c advice.detachedHead=false clone \
			--depth 1 --branch "$HX_PICO_SDK_VERSION_"                   \
			--recurse-submodules --shallow-submodules                    \
			https://github.com/raspberrypi/pico-sdk.git "$HX_PICO_SDK_"
		local HX_TMP_DIR_
		HX_TMP_DIR_=$(mktemp -d)
		git -c advice.detachedHead=false clone --depth 1                    \
			--branch "$HX_PICO_SDK_VERSION_"                                \
			https://github.com/raspberrypi/picotool.git "$HX_TMP_DIR_/picotool"
		cmake -S "$HX_TMP_DIR_/picotool" -B "$HX_TMP_DIR_/build" -G Ninja \
			-DCMAKE_BUILD_TYPE=Release -DPICO_SDK_PATH="$HX_PICO_SDK_"    \
			-DPICOTOOL_FLAT_INSTALL=1                                     \
			-DCMAKE_INSTALL_PREFIX="$(dirname "$HX_PICOTOOL_DIR_")"
		cmake --build "$HX_TMP_DIR_/build"
		cmake --install "$HX_TMP_DIR_/build"
		rm -rf "$HX_TMP_DIR_"
	fi
}

# -- ESP-IDF -------------------------------------------------------------------

# install.sh is safe to re-run after a failure.
install_esp_idf() {
	if ! python3 "$HX_ESP_IDF_/tools/idf_tools.py" check-python-dependencies \
			> /dev/null 2>&1; then
		separator
		[ -d "$HX_ESP_IDF_" ] || git -c advice.detachedHead=false clone \
			--depth 1 --branch "$HX_ESP_IDF_VERSION_"                   \
			--recurse-submodules --shallow-submodules                   \
			https://github.com/espressif/esp-idf.git "$HX_ESP_IDF_"
		"$HX_ESP_IDF_/install.sh" esp32c6
	fi
}

# -- USB/IP --------------------------------------------------------------------

install_usbip() {
	if ! check_usbip_installed; then
		separator
		echo "$HX_USBIP_SCRIPT_" | $HX_SUDO_ tee "$HX_USBIP_"
		$HX_SUDO_ chmod 755 "$HX_USBIP_"
		echo "$HX_USBIP_RULES_TEXT_" | $HX_SUDO_ tee "$HX_USBIP_RULES_"
		$HX_SUDO_ usermod -aG dialout,plugdev "$(stat -c %U "$HOME")"
	fi
}

# -- Main ----------------------------------------------------------------------

main() {
	if [ "${1:-}" = "--sdks" ]; then
		if ! grep -qx 'ID=ubuntu' /etc/os-release; then
			echo "error: --sdks requires Ubuntu." 1>&2
			exit 1
		fi
		HX_SDKS_=1
		HX_PACKAGES_="$HX_PACKAGES_ $HX_SDK_PACKAGES_"
	fi

	[ "$(id -u)" -ne 0 ] && HX_SUDO_="sudo"
	if check_packages_installed; then
		echo "packages_already_installed"
	else
		install_apt_packages
		print_tool_versions
	fi

	if [ "$HX_SDKS_" = 0 ]; then
		echo "warning: Some SDKs omitted as --sdks was not used." 1>&2
		exit 0
	fi

	install_emscripten_sdk
	install_pico_sdk
	install_esp_idf
	install_usbip

	echo 🪓🪓🪓
}

main "$@"
