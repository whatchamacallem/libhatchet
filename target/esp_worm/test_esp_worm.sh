#!/usr/bin/env bash
# SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
# SPDX-License-Identifier: MIT
# This file is licensed under the terms of the LICENSE.md file.
#
# The test of esp_worm on an ESP32-C6-LCD-1.47. Runs the rest of its arguments,
# a command in which hxboot launches esp_worm served over USB. esp_worm joins
# the Wi-Fi network it was built with and reports its address. A run that exits
# with 77 because no board is connected ends the test with 77.
#
# The test then plays the host in a user and network namespace, where it may
# make a WireGuard interface. The namespace has only loopback, so a relay
# passes the tunnel's packets to a UDP socket to esp_worm opened outside it.
# Files of several sizes are put with TFTP, got back and compared.
#
# test_esp_worm.sh <session dir> <work dir> <run command>...
#
# The session directory holds the test keys. The report and the files are
# written to the work directory.
set -euo pipefail

SCRIPT_PATH=$(readlink -f "$0")

TRANSFER_SIZES=(1 511 512 513 1536 4096 70000)

# The tunnel's addresses and port are esp_worm's defaults. The relay's port is
# the host's to choose.
HOST_TUNNEL_ADDRESS="10.9.0.1"
DEVICE_TUNNEL_ADDRESS="10.9.0.2"
TUNNEL_PORT=51820
RELAY_PORT=51821

SESSION_DIRECTORY=""
WORK_DIRECTORY=""
REPORT_FILE=""
RUN_COMMAND=()

DEVICE_ADDRESS=""

RELAY_PROCESS=""

# The namespace runs this script again with only the two directories.
parse_arguments() {
	SESSION_DIRECTORY="$1"
	WORK_DIRECTORY="$2"
	REPORT_FILE="$WORK_DIRECTORY/report.txt"
	shift 2
	RUN_COMMAND=("$@")
}

run_program() {
	"${RUN_COMMAND[@]}" --report "$REPORT_FILE"
}

find_device_address() {
	DEVICE_ADDRESS=$(sed -n 's/^# \([0-9.]*\) authmode .*/\1/p' "$REPORT_FILE")
	if [[ -z "$DEVICE_ADDRESS" ]]; then
		echo "error: esp_worm did not report its address." >&2
		exit 1
	fi
	echo "esp_worm is at $DEVICE_ADDRESS."
}

make_transfer_files() {
	local TRANSFER_SIZE
	for TRANSFER_SIZE in "${TRANSFER_SIZES[@]}"; do
		head -c "$TRANSFER_SIZE" /dev/urandom > "$WORK_DIRECTORY/source_$TRANSFER_SIZE.bin"
	done
}

# The socket stays in the container's network, which reaches the Wi-Fi network
# through Docker, WSL and Windows. esp_worm answers whatever address and port
# the host's handshake came from, which their address translation maps back
# here.
enter_namespace() {
	exec 3<>"/dev/udp/$DEVICE_ADDRESS/$TUNNEL_PORT"
	INSIDE_NAMESPACE=1 unshare --user --map-root-user --net "$SCRIPT_PATH" \
		"$SESSION_DIRECTORY" "$WORK_DIRECTORY"
	echo "Every file came back the same."
}

session_down() {
	local EXIT_STATUS=$?
	if ((EXIT_STATUS != 0)); then
		echo "--- wg"
		wg show esp_worm0 2>/dev/null || true
		echo "--- ip"
		ip -s link show 2>/dev/null || true
	fi
	kill "$RELAY_PROCESS" 2>/dev/null || true
	wait "$RELAY_PROCESS" 2>/dev/null || true
}

# Runs in the namespace. The interface sends to the relay on loopback, which
# passes each datagram between it and fd 3, the socket to esp_worm. The relay
# reports once it is bound.
session_up() {
	ip link set lo up
	exec 4< <(python3 -I -c '
import select, socket, sys

device_socket = socket.socket(fileno=3)
tunnel_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
tunnel_socket.bind(("127.0.0.1", int(sys.argv[1])))
tunnel_socket.connect(("127.0.0.1", int(sys.argv[2])))
print("bound", flush=True)
socket_pairs = {device_socket: tunnel_socket, tunnel_socket: device_socket}
while True:
	ready_sockets, _, _ = select.select(list(socket_pairs), [], [])
	for ready_socket in ready_sockets:
		socket_pairs[ready_socket].send(ready_socket.recv(65535))
' "$RELAY_PORT" "$TUNNEL_PORT")
	RELAY_PROCESS=$!
	if ! read -r -u 4; then
		echo "error: The relay did not start." >&2
		exit 1
	fi
	ip link add esp_worm0 type wireguard
	wg set esp_worm0 listen-port "$TUNNEL_PORT" private-key "$SESSION_DIRECTORY/host.key" \
		peer "$(< "$SESSION_DIRECTORY/device.pub")" \
		allowed-ips "$DEVICE_TUNNEL_ADDRESS/32" \
		endpoint "127.0.0.1:$RELAY_PORT"
	ip address add "$HOST_TUNNEL_ADDRESS/24" dev esp_worm0
	ip link set esp_worm0 up
}

# Runs in the namespace. A name is written once and lwIP takes 20 characters,
# so each file goes to a new short name. tftp-hpa can exit with 0 after a
# failed transfer, so cmp decides.
session_transfer() {
	local NAME_STEM="$EPOCHSECONDS"
	local FILE_INDEX=0
	local TRANSFER_SIZE
	for TRANSFER_SIZE in "${TRANSFER_SIZES[@]}"; do
		local SOURCE_FILE="$WORK_DIRECTORY/source_$TRANSFER_SIZE.bin"
		local REMOTE_NAME="${NAME_STEM}_$FILE_INDEX.bin"
		local RETURNED_FILE="$WORK_DIRECTORY/returned_$FILE_INDEX.bin"
		FILE_INDEX=$((FILE_INDEX + 1))
		echo "Putting $SOURCE_FILE ($TRANSFER_SIZE bytes) as $REMOTE_NAME..."
		if ! tftp -m binary "$DEVICE_TUNNEL_ADDRESS" -c put "$SOURCE_FILE" "$REMOTE_NAME"; then
			echo "error: The put of $REMOTE_NAME failed." >&2
			exit 1
		fi
		rm -f "$RETURNED_FILE"
		if ! tftp -m binary "$DEVICE_TUNNEL_ADDRESS" -c get "$REMOTE_NAME" "$RETURNED_FILE"; then
			echo "error: The get of $REMOTE_NAME failed." >&2
			exit 1
		fi
		if ! cmp "$SOURCE_FILE" "$RETURNED_FILE"; then
			echo "error: $REMOTE_NAME did not come back the same." >&2
			exit 1
		fi
		echo "$REMOTE_NAME came back the same."
	done
	wg show esp_worm0
}

main() {
	parse_arguments "$@"
	if [[ -n "${INSIDE_NAMESPACE:-}" ]]; then
		trap session_down EXIT
		session_up
		session_transfer
		return
	fi
	run_program
	find_device_address
	make_transfer_files
	enter_namespace
}

main "$@"
