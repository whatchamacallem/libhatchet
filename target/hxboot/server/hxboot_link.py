# SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
# SPDX-License-Identifier: MIT
# This file is licensed under the terms of the LICENSE.md file.
#
# The host half of the link. Text and bytes only, no I/O.

import enum
import re
import struct
import zlib
from typing import Final, NamedTuple, Optional, Tuple

# Matches HX_BOOT_LINK_CHUNK.
chunk_size: Final = 1024

# The longest file name in characters. Matches HX_BOOT_LINK_NAME_MAX.
name_max: Final = 63


class status(enum.IntEnum):
	"""Matches hxboot_link_status."""

	ok = 0
	not_found = 1
	bad_request = 2
	io_error = 3


class request(NamedTuple):
	sequence_number: int
	# size or read.
	operation_name: str
	# Hex numbers and names, as sent.
	operation_arguments: Tuple[str, ...]


class reply(NamedTuple):
	reply_status: status
	reply_payload: bytes


def parse(request_line: str) -> Optional[request]:
	line_match = re.fullmatch(r"# hxlink ([0-9a-fA-F]{4}(?: [!-~]+)+) ([0-9a-fA-F]{8})",
		request_line)
	if line_match is None:
		return None
	checked_text: str = line_match.group(1)
	if zlib.crc32(checked_text.encode("ascii")) != int(line_match.group(2), 16):
		return None
	request_words = checked_text.split(" ")
	return request(int(request_words[0], 16), request_words[1], tuple(request_words[2:]))


def frame(link_request: request, link_reply: reply) -> bytes:
	assert len(link_reply.reply_payload) <= chunk_size
	frame_bytes = struct.pack("<HHHH", 0x4c48, link_request.sequence_number,
		link_reply.reply_status, len(link_reply.reply_payload)) + link_reply.reply_payload
	return frame_bytes + struct.pack("<I", zlib.crc32(frame_bytes))
