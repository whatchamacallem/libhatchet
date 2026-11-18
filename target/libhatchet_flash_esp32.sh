#!/usr/bin/env bash
# SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
# SPDX-License-Identifier: MIT
# This file is licensed under the terms of the LICENSE.md file.
#
# Builds test_target.cpp for an ESP32 with ESP-IDF and the top level
# CMakeLists.txt, flashes it and then prints the report it sends to the serial
# port. Exits with an error when a test fails, the program crashes or the report
# does not finish. The chip is detected unless --target is given.
# Run with: ./libhatchet_flash_esp32.sh [--port /dev/ttyUSB0] [--target esp32]
set -euo pipefail

cd "$(dirname "$(readlink -f "$0")")/.."

# ESP-IDF is at ~/esp-idf where "install_deb.sh --sdks" installs it. A different
# IDF_PATH in the environment is not used.
HX_ESP_IDF_="$HOME/esp-idf"

TARGET="${IDF_TARGET:-}"
BAUD="${BAUD:-115200}"
PORT="${PORT:-}"
TIMEOUT="${TIMEOUT:-60}"

# -- Argument Parsing -----------------------------------------------------------
while [[ $# -gt 0 ]]; do
	case "$1" in
		--port)    PORT="$2";    shift 2 ;;
		--target)  TARGET="$2";  shift 2 ;;
		--baud)    BAUD="$2";    shift 2 ;;
		--timeout) TIMEOUT="$2"; shift 2 ;;
		*) echo "Unknown option: $1"; exit 1 ;;
	esac
done

# -- Helpers --------------------------------------------------------------------
info()  { echo -e "\033[1;34m[INFO]\033[0m  $*"; }
ok()    { echo -e "\033[1;32m[OK]\033[0m    $*"; }
warn()  { echo -e "\033[1;33m[WARN]\033[0m  $*"; }
die()   { echo -e "\033[1;31m[ERROR]\033[0m $*" >&2; exit 1; }

# -- Pre-Flight Checks ----------------------------------------------------------
[[ -f "$HX_ESP_IDF_/export.sh" ]] || die "ESP-IDF not found at $HX_ESP_IDF_. Run: ./install_deb.sh --sdks"

info "Entering the ESP-IDF environment ..."
export IDF_PATH="$HX_ESP_IDF_"
. "$HX_ESP_IDF_/export.sh" > /dev/null || die "ESP-IDF export.sh failed. Run: . $HX_ESP_IDF_/export.sh"

# -- 1. Find Serial Port --------------------------------------------------------
# Chips with USB, such as the ESP32-C6, are /dev/ttyACM*. Boards with a USB to
# UART bridge, such as the WT32-ETH01 with an adapter, are /dev/ttyUSB*.
if [[ -z "$PORT" ]]; then
	for candidate in /dev/ttyACM0 /dev/ttyACM1 /dev/ttyUSB0 /dev/ttyUSB1; do
		if [[ -e "$candidate" ]]; then
			PORT="$candidate"
			break
		fi
	done
fi
[[ -n "$PORT" && -e "$PORT" ]] || die "No serial port found. Try: ./libhatchet_flash_esp32.sh --port /dev/ttyUSB0"
ok "Serial port: $PORT"

# -- 2. Detect Chip -------------------------------------------------------------
# esptool resets chips with USB and boards that wire DTR and RTS to IO0 and EN
# into the bootloader. The WT32-ETH01 has to be put there by hand: hold IO0 low
# while resetting it.
if [[ -z "$TARGET" ]]; then
	info "Detecting the chip ..."
	DETECTED=$(esptool --port "$PORT" --after no-reset chip-id 2>&1) ||
		die "esptool could not connect. Put the chip in its bootloader.\n$DETECTED"
	# E.g. "Detecting chip type... ESP32-C6" is esp32c6.
	TARGET=$(sed -n 's/^Detecting chip type\.\.\. *//p' <<< "$DETECTED" | head -n 1 |
		tr -d '[:space:]-' | tr '[:upper:]' '[:lower:]')
	[[ -n "$TARGET" ]] || die "The chip type was not reported.\n$DETECTED"
fi
ok "Target: $TARGET"

BUILD_DIR="build/$TARGET"

# -- 3. Build -------------------------------------------------------------------
info "Cleaning build directory ..."
rm -rf "$(readlink -f build)" build

info "Building hxtest_target for '$TARGET' ..."
idf.py -B "$BUILD_DIR" -DIDF_TARGET="$TARGET" build

[[ -f "$BUILD_DIR/flash_args" ]] || die "Build succeeded but $BUILD_DIR/flash_args not found."
ok "Build complete: $BUILD_DIR/hxtest_target.bin"

# -- 4. Flash -------------------------------------------------------------------
# The chip is left in the bootloader. The report is read from its reset.
info "Flashing ..."
(cd "$BUILD_DIR" && esptool --chip "$TARGET" --port "$PORT" --after no-reset write-flash "@flash_args")
ok "Flash complete."

# -- 5. Print the Report --------------------------------------------------------
# The port is opened before the chip is reset into the program because chips
# with USB drop what is sent while nothing is reading. The reset is the one
# idf.py monitor uses: RTS pulses EN low while DTR leaves IO0 high. Opening the
# port raises both, and RTS is lowered first because RTS high with DTR going low
# is how esptool enters the bootloader on chips with USB. Without that wiring
# the board has to be reset by hand. The ESP-IDF boot log before the report is
# not printed.
info "Resetting into hxtest_target. Reset the board by hand if nothing happens."
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
