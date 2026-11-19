#!/usr/bin/env bash
# SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
# SPDX-License-Identifier: MIT
# This file is licensed under the terms of the LICENSE.md file.
#
# Builds test_target.cpp for the ESP32-C6 with ESP-IDF and the top level
# CMakeLists.txt, flashes it and then prints the report it sends to its USB
# serial port. Exits with an error when a test fails, the program crashes or the
# report does not finish. It also runs in a test_msvc_wslc.ps1 --clone-sdks
# container, where it attaches the ESP32-C6 with usbip. test_target.sh uses
# --connected and --no-build. See README.md and iot.md.
# Run with: ./libhatchet_flash_esp32.sh [--port /dev/ttyACM0] [--target esp32c6]
#   [--no-build] [--connected]
set -euo pipefail

cd "$(dirname "$(readlink -f "$0")")/.."

. target/libhatchet_usb.sh

# ESP-IDF is at ~/esp-idf where "install_deb.sh --sdks" installs it. A different
# IDF_PATH in the environment is not used.
HX_ESP_IDF_="$HOME/esp-idf"

# The ESP32-C6 is the chip there is hardware for. 303a is the vendor ID for
# Espressif, which its USB serial port uses.
TARGET="esp32c6"
VENDOR="303a"
BAUD="${BAUD:-115200}"
PORT="${PORT:-}"
TIMEOUT="${TIMEOUT:-60}"
BUILD=1
CONNECTED=0

# -- Argument Parsing -----------------------------------------------------------
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

# -- Helpers --------------------------------------------------------------------
info()  { echo -e "\033[1;34m[INFO]\033[0m  $*"; }
ok()    { echo -e "\033[1;32m[OK]\033[0m    $*"; }
warn()  { echo -e "\033[1;33m[WARN]\033[0m  $*"; }
die()   { echo -e "\033[1;31m[ERROR]\033[0m $*" >&2; exit 1; }

# -- Connection -----------------------------------------------------------------
# --connected exits with 0 when an ESP32 is connected and 1 when not.
usb_attach "$VENDOR"
if [[ "$CONNECTED" == 1 ]]; then
	usb_connected "$VENDOR" || exit 1
	exit 0
fi

# -- Pre-Flight Checks ----------------------------------------------------------
[[ -f "$HX_ESP_IDF_/export.sh" ]] || die "ESP-IDF not found at $HX_ESP_IDF_. Run: ./install_deb.sh --sdks"

info "Entering the ESP-IDF environment ..."
export IDF_PATH="$HX_ESP_IDF_"
. "$HX_ESP_IDF_/export.sh" > /dev/null || die "ESP-IDF export.sh failed. Run: . $HX_ESP_IDF_/export.sh"

# -- 1. Find Serial Port --------------------------------------------------------
if [[ -z "$PORT" ]]; then
	PORT=$(usb_serial_port "$VENDOR") ||
		die "No ESP32 serial port found. Try: ./libhatchet_flash_esp32.sh --port /dev/ttyACM0"
fi
[[ -e "$PORT" ]] || die "$PORT not found."
ok "Serial port: $PORT"

# -- 2. Build -------------------------------------------------------------------
# --no-build flashes what test_target.sh built.
if [[ "$BUILD" == 1 ]]; then
	info "Cleaning build directory ..."
	rm -rf "$(readlink -f build)" build

	info "Building hxtest_target for '$TARGET' ..."
	idf.py -B "$BUILD_DIR" -DIDF_TARGET="$TARGET" build
fi

[[ -f "$BUILD_DIR/flash_args" ]] || die "$BUILD_DIR/flash_args not found."
ok "Build: $BUILD_DIR/hxtest_target.bin"

# -- 3. Flash -------------------------------------------------------------------
# esptool resets the chip into its bootloader over USB. --chip makes it stop
# when the chip is a different one. The chip is left in the bootloader and the
# report is read from its reset.
info "Flashing ..."
(cd "$BUILD_DIR" && esptool --chip "$TARGET" --port "$PORT" --after no-reset write-flash "@flash_args")
ok "Flash complete."

# -- 4. Print the Report --------------------------------------------------------
# The port is opened before the chip is reset into the program because chips
# with USB drop what is sent while nothing is reading. The reset is the one
# idf.py monitor uses: RTS pulses EN low while DTR leaves IO0 high. Opening the
# port raises both, and RTS is lowered first because RTS high with DTR going low
# is how esptool enters the bootloader on chips with USB. The ESP-IDF boot log
# before the report is not printed.
info "Resetting into hxtest_target ..."
echo ""
python3 -I - "$PORT" "$BAUD" "$TIMEOUT" <<'EOF'
import sys, time
import serial

port_name, baud, timeout = sys.argv[1], int(sys.argv[2]), float(sys.argv[3])

port = serial.Serial(port_name, baud, timeout=0.5)
port.rts = False
port.dtr = False
port.rts = True
time.sleep(0.2)
port.rts = False

started = False
failures = 0
deadline = time.monotonic() + timeout
while time.monotonic() < deadline:
	line = port.readline().decode('utf-8', 'replace').rstrip('\r\n')
	if line.startswith('TAP version'):
		if started:
			sys.exit('error: The program restarted.')
		started = True
	if started and line:
		print(line, flush=True)
	if 'Guru Meditation' in line or 'abort() was called' in line:
		# The backtrace follows.
		deadline = min(deadline, time.monotonic() + 1.0)
		failures += 1
	elif started and line.startswith('not ok'):
		failures += 1
	elif started and line.startswith('# libhatchet'):
		sys.exit(1 if failures else 0)

sys.exit('error: The report did not finish in %g seconds.' % timeout)
EOF
