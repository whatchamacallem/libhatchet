#!/usr/bin/env bash
# SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
# SPDX-License-Identifier: MIT
# This file is licensed under the terms of the LICENSE.md file.
#
# Builds test_target.cpp for the Raspberry Pi Pico 2 with the top level
# CMakeLists.txt, flashes it and then prints the report it sends over USB
# serial. Exits with an error when a test fails or the report does not finish.
# It also runs in a test_msvc_wslc.ps1 --clone-sdks container, where it attaches
# the Pico with usbip. test_target.sh uses --connected and --no-build. See
# README.md and iot.md.
# Run with: ./flash_pico.sh [--port /dev/ttyACM0] [--target pico2]
#   [--no-build] [--connected]
set -euo pipefail

cd "$(dirname "$(readlink -f "$0")")/.."

. target/util.usb.sh

# The Pico SDK is at ~/pico-sdk where "install_deb.sh --sdks" installs it, with
# picotool next to it. A different PICO_SDK_PATH in the environment is not used.
export PICO_SDK_PATH="$HOME/pico-sdk"
PATH="$PATH:$(dirname "$PICO_SDK_PATH")/picotool"

TARGET="pico2"
VENDOR="2e8a"
BAUD="${BAUD:-115200}"
PORT="${PORT:-}"
TIMEOUT="${TIMEOUT:-60}"
BUILD=1
CONNECTED=0

while [[ $# -gt 0 ]]; do
	case "$1" in
		--port)      PORT="$2";    shift 2 ;;
		--target)    TARGET="$2";  shift 2 ;;
		--baud)      BAUD="$2";    shift 2 ;;
		--timeout)   TIMEOUT="$2"; shift 2 ;;
		--no-build)  BUILD=0;      shift ;;
		--connected) CONNECTED=1;  shift ;;
		*) echo "Unknown option: $1"; exit 1 ;;
	esac
done

BUILD_DIR="build/$TARGET"
UF2="$BUILD_DIR/hxtest_target.uf2"

info()  { echo -e "\033[1;34m[INFO]\033[0m  $*"; }
ok()    { echo -e "\033[1;32m[OK]\033[0m    $*"; }
warn()  { echo -e "\033[1;33m[WARN]\033[0m  $*"; }
die()   { echo -e "\033[1;31m[ERROR]\033[0m $*" >&2; exit 1; }

require() { command -v "$1" &>/dev/null || die "Required tool not found: $1"; }

# -- Connection -----------------------------------------------------------------
# --connected exits with 0 when a Pico is connected and 1 when not.
usb_attach "$VENDOR"
if [[ "$CONNECTED" == 1 ]]; then
	usb_connected "$VENDOR" || exit 1
	exit 0
fi

# -- Pre-Flight Checks ----------------------------------------------------------
require picotool

# -- 1. Build -------------------------------------------------------------------
# --no-build flashes what test_target.sh built.
if [[ "$BUILD" == 1 ]]; then
	require cmake
	require arm-none-eabi-gcc

	info "Cleaning build directory ..."
	rm -rf "$(readlink -f build)" build

	info "Configuring CMake for board '$TARGET' ..."
	cmake -S . -B "$BUILD_DIR" \
		-DCMAKE_BUILD_TYPE=Release \
		-DPICO_BOARD="$TARGET" \
		-DPICO_SDK_PATH="$PICO_SDK_PATH" \
		-G Ninja \
		-Wno-dev \
		2>&1

	info "Building hxtest_target ..."
	cmake --build "$BUILD_DIR" --target hxtest_target -j"$(nproc)"
fi

[[ -f "$UF2" ]] || die "$UF2 not found."
ok "Build: $UF2"

# -- 2. Flash -------------------------------------------------------------------
# The program reboots into BOOTSEL when it finishes, so the button is only
# needed the first time or after a crash.
info "Looking for Pico in BOOTSEL mode ..."

BOOTSEL_TIMEOUT=30
ELAPSED=0
while ! picotool info &>/dev/null; do
	if [[ $ELAPSED -eq 0 ]]; then
		warn "No Pico found in BOOTSEL mode."
		echo " - Hold the BOOTSEL button on the Pico, plug in USB, then release."
		echo " - In a container, share it from Windows. See target/README.md."
		echo " - Waiting up to ${BOOTSEL_TIMEOUT}s ..."
	fi
	sleep 1
	usb_attach "$VENDOR"
	ELAPSED=$((ELAPSED + 1))
	if [[ $ELAPSED -ge $BOOTSEL_TIMEOUT ]]; then
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
		usb_attach "$VENDOR"
		PORT=$(usb_serial_port "$VENDOR") && break
		sleep 0.5
	done
fi

# Let the driver stabilize so there is something to cat.
sleep 0.5

if [[ -z "$PORT" ]] || [[ ! -e "$PORT" ]]; then
	die "No serial port found. In a container, share the running Pico from Windows. Or try: ./flash_pico.sh --port /dev/ttyACM0"
fi

ok "Serial port: $PORT at ${BAUD} baud."
echo ""

# -- 4. Check the Report --------------------------------------------------------
# The port goes away when the Pico reboots into BOOTSEL, which ends cat with an
# error. The report is kept in the build directory.
REPORT="$BUILD_DIR/report.txt"
stty -F "$PORT" "$BAUD" raw -echo -hupcl
timeout "$TIMEOUT" cat "$PORT" | tee "$REPORT" || true

grep -q '^# libhatchet' "$REPORT" || die "The report did not finish."
if grep -q '^not ok' "$REPORT"; then
	die "A test failed."
fi
ok "All tests passed."
