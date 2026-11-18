#!/usr/bin/env bash
# SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
# SPDX-License-Identifier: MIT
# This file is licensed under the terms of the LICENSE.md file.
#
# Builds test_target.cpp for the Pico with the top level CMakeLists.txt, flashes
# it and then streams the report it sends over USB serial. It also runs in a
# test_msvc_wslc.ps1 --clone-sdks container, where it attaches the Pico with
# usbip. See README.md.
# Run with: ./libhatchet_flash_pico.sh [--port /dev/ttyACM0] [--board pico2]
set -euo pipefail

cd "$(dirname "$(readlink -f "$0")")/.."

# The Pico SDK is at ~/pico-sdk where "install_deb.sh --sdks" installs it, with
# picotool next to it. A different PICO_SDK_PATH in the environment is not used.
export PICO_SDK_PATH="$HOME/pico-sdk"
PATH="$PATH:$(dirname "$PICO_SDK_PATH")/picotool"
BUILD_DIR="build/pico"

BOARD="${PICO_BOARD:-pico2}"
BAUD="${BAUD:-115200}"
PORT="${PORT:-}"

# -- Argument Parsing -----------------------------------------------------------
while [[ $# -gt 0 ]]; do
	case "$1" in
		--port)  PORT="$2";  shift 2 ;;
		--board) BOARD="$2"; shift 2 ;;
		--baud)  BAUD="$2";  shift 2 ;;
		*) echo "Unknown option: $1"; exit 1 ;;
	esac
done

# -- Helpers --------------------------------------------------------------------
info()  { echo -e "\033[1;34m[INFO]\033[0m  $*"; }
ok()    { echo -e "\033[1;32m[OK]\033[0m    $*"; }
warn()  { echo -e "\033[1;33m[WARN]\033[0m  $*"; }
die()   { echo -e "\033[1;31m[ERROR]\033[0m $*" >&2; exit 1; }

require() { command -v "$1" &>/dev/null || die "Required tool not found: $1"; }

# -- USB/IP ---------------------------------------------------------------------
# In a test_msvc_wslc.ps1 --clone-sdks container the Pico is shared by
# usbipd-win, which the container reaches as "windows". 2e8a is the vendor ID
# for Raspberry Pi. The Pico re-enumerates when it switches between BOOTSEL and
# the program, which detaches it, so it is attached again each time. Nothing is
# done without usbip and windows, or while a Pico is attached.
attach_pico() {
	command -v usbip &>/dev/null && getent hosts windows &>/dev/null || return 0
	lsusb -d 2e8a: &>/dev/null && return 0
	local busid
	busid=$(usbip list -r windows 2>/dev/null |
		sed -n 's/^[[:space:]]*\([0-9][0-9.-]*\): .*(2e8a:[0-9a-f]*)$/\1/p' | head -n 1)
	if [[ -n "$busid" ]] && usbip attach -r windows -b "$busid" &>/dev/null; then
		# udev gives the device to plugdev and adds its serial port.
		udevadm settle 2>/dev/null || sleep 1
	fi
}

# Prints the serial port of the Pico running the program. Another board's
# serial port may be attached too.
find_pico_port() {
	local tty
	for tty in /sys/class/tty/ttyACM*; do
		if [[ "$(cat "$tty/device/../idVendor" 2>/dev/null)" == 2e8a ]]; then
			echo "/dev/${tty##*/}"
			return 0
		fi
	done
	return 1
}

# -- Pre-Flight Checks ----------------------------------------------------------
require cmake
require arm-none-eabi-gcc
require picotool

# -- 1. Build -------------------------------------------------------------------
info "Cleaning build directory ..."
rm -rf "$(readlink -f build)" build

info "Configuring CMake for board '$BOARD' ..."
cmake -S . -B "$BUILD_DIR" \
	-DCMAKE_BUILD_TYPE=Release \
	-DPICO_BOARD="$BOARD" \
	-DPICO_SDK_PATH="$PICO_SDK_PATH" \
	-G Ninja \
	-Wno-dev \
	2>&1

info "Building hxtest_target ..."
cmake --build "$BUILD_DIR" --target hxtest_target -j"$(nproc)"

UF2="$BUILD_DIR/hxtest_target.uf2"
[[ -f "$UF2" ]] || die "Build succeeded but $UF2 not found."
ok "Build complete: $UF2"

# -- 2. Flash -------------------------------------------------------------------
info "Looking for Pico in BOOTSEL mode ..."

TIMEOUT=30
ELAPSED=0
attach_pico
while ! picotool info &>/dev/null; do
	if [[ $ELAPSED -eq 0 ]]; then
		warn "No Pico found in BOOTSEL mode."
		echo " - Hold the BOOTSEL button on the Pico, plug in USB, then release."
		echo " - In a container, share it from Windows. See target/README.md."
		echo " - Waiting up to ${TIMEOUT}s ..."
	fi
	sleep 1
	attach_pico
	ELAPSED=$((ELAPSED + 1))
	if [[ $ELAPSED -ge $TIMEOUT ]]; then
		die "Timed out waiting for Pico in BOOTSEL mode."
	fi
done

ok "Pico detected. Flashing ..."
picotool load -f "$UF2"
picotool reboot

ok "Flash complete. Pico is rebooting."

# -- 3. Find Serial Port --------------------------------------------------------
# The program waits for the port to be opened before it reports. The running
# Pico is a different USB device from the one in BOOTSEL, which usbipd-win has
# to share too.
if [[ -z "$PORT" ]]; then
	info "Waiting for serial port to appear ..."
	for i in $(seq 1 60); do
		attach_pico
		PORT=$(find_pico_port) && break
		sleep 0.5
	done
fi

# Let the driver stabilize so there is something to cat.
sleep 0.5

if [[ -z "$PORT" ]] || [[ ! -e "$PORT" ]]; then
	die "No serial port found. In a container, share the running Pico from Windows. Or try: ./libhatchet_flash_pico.sh --port /dev/ttyACM0"
fi

ok "Serial port: $PORT at ${BAUD} baud."
echo ""

# -- 4. Stream the Report -------------------------------------------------------
# The port goes away when the Pico reboots, which ends cat with an error.
stty -F "$PORT" "$BAUD" raw -echo -hupcl
cat "$PORT" || true
