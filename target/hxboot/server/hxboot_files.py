# SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
# SPDX-License-Identifier: MIT
# This file is licensed under the terms of the LICENSE.md file.

import os
import re
import struct
from dataclasses import dataclass, field
from typing import Dict

import hxboot_link


@dataclass
class file_service:
	root_directory: str
	file_snapshots: Dict[str, bytes] = field(default_factory=dict)

	def reset(self) -> None:
		"""Called at each "TAP version" line."""
		self.file_snapshots.clear()

	def serve(self, link_request: hxboot_link.request) -> hxboot_link.reply:
		operation_name = link_request.operation_name
		request_arguments = link_request.operation_arguments
		number_pattern = "[0-9a-fA-F]+"
		if operation_name == "size":
			well_formed = len(request_arguments) == 1
		elif operation_name == "read":
			well_formed = (len(request_arguments) == 3
				and re.fullmatch(number_pattern, request_arguments[1]) is not None
				and re.fullmatch(number_pattern, request_arguments[2]) is not None
				and int(request_arguments[2], 16) <= hxboot_link.chunk_size)
		else:
			well_formed = False
		if not well_formed or len(request_arguments[0]) > hxboot_link.name_max:
			return hxboot_link.reply(hxboot_link.status.bad_request, b"")
		file_name = request_arguments[0]
		file_snapshot = self.file_snapshots.get(file_name)
		if file_snapshot is None:
			root_path = os.path.realpath(self.root_directory)
			file_path = os.path.realpath(os.path.join(root_path, file_name))
			if (not file_path.startswith(os.path.join(root_path, ""))
					or not os.path.isfile(file_path)):
				return hxboot_link.reply(hxboot_link.status.not_found, b"")
			try:
				with open(file_path, "rb") as file_stream:
					file_snapshot = file_stream.read()
			except OSError:
				return hxboot_link.reply(hxboot_link.status.io_error, b"")
			self.file_snapshots[file_name] = file_snapshot
		if operation_name == "size":
			return hxboot_link.reply(hxboot_link.status.ok, struct.pack("<I", len(file_snapshot)))
		read_offset = int(request_arguments[1], 16)
		read_end = read_offset + int(request_arguments[2], 16)
		return hxboot_link.reply(hxboot_link.status.ok, file_snapshot[read_offset:read_end])
