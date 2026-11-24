#!/usr/bin/env bash
# SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
# SPDX-License-Identifier: MIT
# This file is licensed under the terms of the LICENSE.md file.
#
# run_pico_sdk.sh --picotool <picotool> --vendor <USB vendor ID> --uf2 <file>
#	--report <file>
set -euo pipefail

TARGET_DIRECTORY=$(readlink -f "$(dirname "$0")/..")

. "$TARGET_DIRECTORY/util_usb.sh"

PICOTOOL=""
VENDOR=""
UF2_FILE=""
REPORT_FILE=""

PORT=""

parse_arguments() {
	while [[ $# -gt 0 ]]; do
		case "$1" in
			--picotool) PICOTOOL="$2" ;;
			--vendor) VENDOR="$2" ;;
			--uf2) UF2_FILE="$2" ;;
			--report) REPORT_FILE="$2" ;;
			*)
				echo "error: Unknown argument: $1" >&2
				exit 1
				;;
		esac
		shift 2
	done
}

check_connection() {
	usb_attach "$VENDOR"
	if usb_connected "$VENDOR"; then
		return
	fi
	if [[ "${HX_REQUIRE_BOARD:-0}" == 1 ]]; then
		echo "error: No Pico is connected." >&2
		exit 1
	fi
	echo "warning: No Pico is connected. Not tested."
	exit 77
}

# The program reboots into BOOTSEL when it finishes, so the button is only
# needed the first time or after a crash.
flash_program() {
	local ELAPSED_SECONDS=0
	while ! "$PICOTOOL" info &>/dev/null; do
		if ((ELAPSED_SECONDS == 0)); then
			echo "Hold the BOOTSEL button on the Pico, plug in USB, then release. In a"
			echo "container, share it from Windows. Waiting up to 30 seconds..."
		elif ((ELAPSED_SECONDS == 30)); then
			echo "error: No Pico in BOOTSEL mode." >&2
			exit 1
		fi
		sleep 1
		usb_attach "$VENDOR"
		ELAPSED_SECONDS=$((ELAPSED_SECONDS + 1))
	done
	echo "Flashing $UF2_FILE..."
	"$PICOTOOL" load -f "$UF2_FILE"
	"$PICOTOOL" reboot
}

# The program waits for the port to be opened before it reports. The running
# Pico is a different USB device from the one in BOOTSEL, which usbipd-win has
# to share too.
find_serial_port() {
	local ATTEMPT
	for ((ATTEMPT = 0; ATTEMPT < 60; ATTEMPT++)); do
		usb_attach "$VENDOR"
		PORT=$(usb_serial_port "$VENDOR") && break
		sleep 0.5
	done

	# Let the driver stabilize so there is something to cat.
	sleep 0.5

	if [[ -z "$PORT" ]] || [[ ! -e "$PORT" ]]; then
		echo "error: No serial port found. In a container, share the running Pico from" \
			"Windows." >&2
		exit 1
	fi
	echo "Serial port: $PORT"
}

# The port goes away when the Pico reboots into BOOTSEL, which ends cat with an
# error, and cat and tee also end with an error when the TAP consumer is done
# first, so pipefail is turned off and only its status counts.
check_report() {
	stty -F "$PORT" 115200 raw -echo -hupcl
	set +o pipefail
	if ! timeout 60 cat "$PORT" | tee "$REPORT_FILE" |
			python3 -I "$TARGET_DIRECTORY/../tap_protocol/hxtap_consumer.py"; then
		echo "error: A test failed or the report did not finish." >&2
		exit 1
	fi
	echo "All tests passed."
}

main() {
	parse_arguments "$@"
	check_connection
	flash_program
	find_serial_port
	check_report
}

main "$@"
