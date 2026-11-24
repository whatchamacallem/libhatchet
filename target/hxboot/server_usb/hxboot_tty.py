# SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
# SPDX-License-Identifier: MIT
# This file is licensed under the terms of the LICENSE.md file.
#
# The only module that uses pyserial.

import time
from dataclasses import dataclass, field
from typing import NamedTuple, Optional, Tuple

# pyserial is in the ESP-IDF Python environment.
import serial
from serial.tools import list_ports


class board(NamedTuple):
	# The board directory name and the name in an image header.
	target_name: str
	usb_vendor_identifier: str
	# True when the running program implements USB, so a launch drops the port.
	drops_port_at_launch: bool


# The boards there is hardware for.
c_board_table: Tuple[board, ...] = (
	board("esp32c6", "303a", False),
	board("pico2", "2e8a", True),
)


def find_board(target_name: str) -> board:
	for board_record in c_board_table:
		if board_record.target_name == target_name:
			return board_record
	raise KeyError(target_name)


@dataclass
class tty:
	board_record: board
	device_path: Optional[str] = None
	serial_port: Optional[serial.Serial] = None
	partial_line: bytearray = field(default_factory=bytearray)

	def close(self) -> None:
		if self.serial_port is not None:
			self.serial_port.close()
			self.serial_port = None
		self.partial_line.clear()

	def open(self) -> None:
		assert self.serial_port is None
		opened_device_path = self.device_path
		if opened_device_path is None:
			vendor_number = int(self.board_record.usb_vendor_identifier, 16)
			device_paths = [port_information.device for port_information in list_ports.comports()
				if port_information.vid == vendor_number]
			if not device_paths:
				raise serial.SerialException("No serial port has USB vendor ID "
					+ self.board_record.usb_vendor_identifier + ".")
			opened_device_path = device_paths[0]
		# A read returns within a tenth of a second so that read_line keeps to
		# its timeout.
		self.serial_port = serial.Serial(opened_device_path, timeout=0.1)

	def read_line(self, timeout_seconds: float) -> Optional[str]:
		deadline_seconds = time.monotonic() + timeout_seconds
		newline_offset = self.partial_line.find(b"\n")
		while newline_offset < 0:
			if time.monotonic() >= deadline_seconds:
				return None
			try:
				if self.serial_port is None:
					self.open()
				assert self.serial_port is not None
				received_bytes = self.serial_port.read(max(1, self.serial_port.in_waiting))
			except OSError:
				if not self.board_record.drops_port_at_launch:
					raise
				self.close()
				time.sleep(0.1)
				continue
			self.partial_line.extend(received_bytes)
			newline_offset = self.partial_line.find(b"\n")
		line_bytes = self.partial_line[:newline_offset]
		del self.partial_line[:newline_offset + 1]
		return line_bytes.decode("utf-8", "replace").rstrip("\r")

	def reset(self) -> None:
		if self.board_record.drops_port_at_launch:
			# A port that the running program implements cannot reset the board.
			# Opening it is what starts the program, which waits for the host.
			return
		# The reset idf.py monitor uses. RTS is lowered first because RTS high
		# with DTR going low is how esptool enters the bootloader on chips with
		# USB.
		assert self.serial_port is not None
		self.serial_port.rts = False
		self.serial_port.dtr = False
		self.serial_port.rts = True
		time.sleep(0.2)
		self.serial_port.rts = False

	def write(self, reply_frame: bytes) -> None:
		assert self.serial_port is not None
		self.serial_port.write(reply_frame)
