# SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
# SPDX-License-Identifier: MIT
# This file is licensed under the terms of the LICENSE.md file.
#
# A build step that the server does not use.

import argparse
import struct
import sys
import zlib
from typing import List, NamedTuple, Sequence, Tuple


class hxboot_image(NamedTuple):
	"""A RAM image."""

	target_name: str
	load_address: int
	entry_address: int
	payload_bytes: bytes
	# The payload and the bss after it.
	image_size: int
	esp_sdk_hash: bytes


def hxboot_image_from_elf(elf_file_path: str, target_name: str, esp_sdk_hash: bytes) -> hxboot_image:
	with open(elf_file_path, "rb") as elf_stream:
		elf_bytes = elf_stream.read()
	if elf_bytes[0:6] != b"\x7fELF\x01\x01":
		raise ValueError(f"{elf_file_path} is not a 32 bit little endian ELF file")
	entry_address, table_offset = struct.unpack_from("<II", elf_bytes, 24)
	entry_size, entry_count = struct.unpack_from("<HH", elf_bytes, 42)
	loaded_segments: List[Tuple[int, bytes]] = []
	image_end = 0
	for entry_index in range(entry_count):
		segment_type, file_offset, physical_address, file_size, memory_size = struct.unpack_from(
			"<II4xIII", elf_bytes, table_offset + entry_index * entry_size)
		if segment_type != 1 or memory_size == 0:
			continue
		image_end = max(image_end, physical_address + memory_size)
		if file_size == 0:
			continue
		segment_bytes = elf_bytes[file_offset:file_offset + file_size]
		if len(segment_bytes) != file_size:
			raise ValueError(f"{elf_file_path} is truncated")
		loaded_segments.append((physical_address, segment_bytes))
	if not loaded_segments:
		raise ValueError(f"{elf_file_path} has no loadable bytes")
	loaded_segments.sort()
	load_address = loaded_segments[0][0]
	payload_buffer = bytearray()
	for segment_address, segment_bytes in loaded_segments:
		payload_offset = segment_address - load_address
		if payload_offset < len(payload_buffer):
			raise ValueError(f"{elf_file_path} has overlapping segments at {segment_address:#x}")
		payload_buffer.extend(bytes(payload_offset - len(payload_buffer)))
		payload_buffer.extend(segment_bytes)
	return hxboot_image(target_name, load_address, entry_address, bytes(payload_buffer),
		image_end - load_address, esp_sdk_hash)


def hxboot_image_to_bytes(boot_image: hxboot_image) -> bytes:
	target_bytes = boot_image.target_name.encode("ascii")
	if len(target_bytes) > 15:
		raise ValueError(f"{boot_image.target_name} is longer than 15 characters")
	header_fields = struct.pack("<16sIIIII8s", target_bytes, boot_image.load_address,
		boot_image.entry_address, len(boot_image.payload_bytes), boot_image.image_size,
		zlib.crc32(boot_image.payload_bytes), boot_image.esp_sdk_hash)
	return (b"HXB2" + struct.pack("<I", zlib.crc32(header_fields)) + header_fields
		+ boot_image.payload_bytes)


def main(command_arguments: Sequence[str]) -> int:
	argument_parser = argparse.ArgumentParser(prog="hxboot_image.py",
		description="Builds a .hxb image from an ELF.")
	argument_parser.add_argument("--target", required=True, metavar="<board>",
		help="the board name, e.g. esp32c6")
	argument_parser.add_argument("--esp-sdk", required=True, metavar="<esp_sdk.bin>",
		help="the esp_sdk app the ELF links against")
	argument_parser.add_argument("elf_file_path", metavar="<elf>")
	argument_parser.add_argument("image_file_path", metavar="<hxb>")
	parsed_arguments = argument_parser.parse_args(command_arguments)
	target_name: str = parsed_arguments.target
	esp_sdk_file_path: str = parsed_arguments.esp_sdk
	elf_file_path: str = parsed_arguments.elf_file_path
	image_file_path: str = parsed_arguments.image_file_path
	try:
		# esptool writes the SHA256 of the ELF at 0xB0 of an app binary, which is
		# app_elf_sha256 in its app description. hxboot compares the first 8
		# bytes with esp_app_get_description().
		with open(esp_sdk_file_path, "rb") as esp_sdk_stream:
			esp_sdk_stream.seek(0xB0)
			esp_sdk_hash = esp_sdk_stream.read(8)
		if len(esp_sdk_hash) != 8:
			raise ValueError(f"{esp_sdk_file_path} is shorter than 0xB8 bytes")
		image_bytes = hxboot_image_to_bytes(hxboot_image_from_elf(elf_file_path, target_name,
			esp_sdk_hash))
		with open(image_file_path, "wb") as image_stream:
			image_stream.write(image_bytes)
	except (OSError, ValueError, struct.error) as image_error:
		print(f"error: {image_error}", file=sys.stderr)
		return 1
	return 0


if __name__ == "__main__":
	sys.exit(main(sys.argv[1:]))
