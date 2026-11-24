#!/usr/bin/env bash
# SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
# SPDX-License-Identifier: MIT
# This file is licensed under the terms of the LICENSE.md file.
#
# The test command of an esp_sdk program. Flashes what changed of flash_args
# and of the slots. The server then resets the board, serves the image and
# judges the TAP document that hxboot starts and the launched image ends. Exits
# with 77 when no board is connected unless HX_REQUIRE_BOARD=1.
#
# run_esp_sdk.sh --python <python> --chip <chip> --vendor <USB vendor ID>
#	--build <dir> --report <file> [--flash <offset> <file>]... [--serve <file>]
#	[--gold-run]
#
# Files are in the build directory. --serve serves the file as hxboot.hxb.
# --gold-run refuses it so hxboot launches its gold slot, and passes when test
# suite 1 is the only failure.
set -euo pipefail

TARGET_DIRECTORY=$(readlink -f "$(dirname "$0")/..")

. "$TARGET_DIRECTORY/util_usb.sh"

PYTHON=""
CHIP=""
VENDOR=""
BUILD_DIRECTORY=""
REPORT_FILE=""
FLASH_PAIRS=()
SERVE_FILE=""
GOLD_RUN=0

PORT=""

parse_arguments() {
	while [[ $# -gt 0 ]]; do
		case "$1" in
			--python) PYTHON="$2"; shift 2 ;;
			--chip) CHIP="$2"; shift 2 ;;
			--vendor) VENDOR="$2"; shift 2 ;;
			--build) BUILD_DIRECTORY="$2"; shift 2 ;;
			--report) REPORT_FILE="$2"; shift 2 ;;
			--flash) FLASH_PAIRS+=("$2" "$3"); shift 3 ;;
			--serve) SERVE_FILE="$2"; shift 2 ;;
			--gold-run) GOLD_RUN=1; shift ;;
			*)
				echo "error: Unknown argument: $1" >&2
				exit 1
				;;
		esac
	done
}

check_connection() {
	usb_attach "$VENDOR"
	if usb_connected "$VENDOR"; then
		return
	fi
	if [[ "${HX_REQUIRE_BOARD:-0}" == 1 ]]; then
		echo "error: No $CHIP is connected." >&2
		exit 1
	fi
	echo "warning: No $CHIP is connected. Not tested."
	exit 77
}

find_serial_port() {
	if ! PORT=$(usb_serial_port "$VENDOR"); then
		echo "error: No $CHIP tty." >&2
		exit 1
	fi
	echo "Serial port: $PORT"
}

flash_changes() {
	local HEADER_SIZE CHIP_SIZE
	HEADER_SIZE=$(sed -n '1s/.*--flash-size \([^ ]*\).*/\1/p' "$BUILD_DIRECTORY/flash_args")
	CHIP_SIZE=$("$PYTHON" -m esptool --chip "$CHIP" --port "$PORT" --after no-reset flash-id |
		sed -n 's/^Detected flash size: //p')
	if [[ "$HEADER_SIZE" != "$CHIP_SIZE" ]]; then
		echo "error: The image header says ${HEADER_SIZE:-nothing} of flash and the chip has" \
			"${CHIP_SIZE:-an unknown size}." >&2
		exit 1
	fi
	echo "Flashing what changed..."
	(cd "$BUILD_DIRECTORY" && "$PYTHON" -m esptool --chip "$CHIP" --port "$PORT" \
		--after no-reset write-flash --skip-flashed "@flash_args" "${FLASH_PAIRS[@]}")
}

# hxboot asks the server for hxboot.hxb. The server opens the port before it
# resets the chip because chips with USB drop what is sent while nothing is
# reading.
serve_image() {
	local ROOT_DIRECTORY="$BUILD_DIRECTORY/hxboot_root"
	local SERVER_PATH="$TARGET_DIRECTORY/../tap_protocol:$TARGET_DIRECTORY/hxboot/server"
	SERVER_PATH+=":$TARGET_DIRECTORY/hxboot/server_usb"
	local SERVER_ARGUMENTS=()
	rm -rf "$ROOT_DIRECTORY"
	mkdir "$ROOT_DIRECTORY"
	if [[ -n "$SERVE_FILE" ]]; then
		cp "$BUILD_DIRECTORY/$SERVE_FILE" "$ROOT_DIRECTORY/hxboot.hxb"
	fi
	if ((GOLD_RUN == 1)); then
		SERVER_ARGUMENTS+=(--gold-run --expected-failures 1)
	fi
	echo "Resetting into hxboot..."
	if ! PYTHONPATH="$SERVER_PATH" "$PYTHON" -P -m hxboot_server --target "$CHIP" \
			--port "$PORT" --root "$ROOT_DIRECTORY" "${SERVER_ARGUMENTS[@]}" |
			tee "$REPORT_FILE"; then
		echo "error: A test failed, the program crashed or the report did not finish." >&2
		exit 1
	fi
}

check_report() {
	if ((GOLD_RUN == 0)); then
		echo "All tests passed."
	elif [[ "$(grep "^not ok" "$REPORT_FILE")" == "not ok 1 - hxboot" ]]; then
		echo "The gold slot ran and its report failed only test suite 1."
	else
		echo "error: The gold slot's report failed more than test suite 1." >&2
		exit 1
	fi
}

main() {
	parse_arguments "$@"
	check_connection
	find_serial_port
	flash_changes
	serve_image
	check_report
}

main "$@"
