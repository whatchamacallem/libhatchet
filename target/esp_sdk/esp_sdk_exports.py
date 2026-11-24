# SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
# SPDX-License-Identifier: MIT
# This file is licensed under the terms of the LICENSE.md file.
#
# exports writes the linker script that links a client against esp_sdk.elf.
# check fails a client that needs a symbol esp_sdk does not export or that
# relies on a weak symbol across the two links, and prints where the client's
# C++ runtime resolved.

import argparse
import struct
import sys
from typing import Dict, Final, List, NamedTuple, Sequence

# SHT_SYMTAB, SHN_UNDEF and SHN_ABS of the ELF format.
c_symbol_table_type: Final = 2
c_undefined_section: Final = 0
c_absolute_section: Final = 0xfff1

# STB_GLOBAL, STB_WEAK and STT_TLS of the ELF format.
c_global_binding: Final = 1
c_weak_binding: Final = 2
c_thread_local_type: Final = 6

# The C++ runtime, which may resolve in the client or in esp_sdk.
c_runtime_prefixes: Final = ("_Znw", "_Zna", "_Zdl", "_Zda", "__cxa_", "__dso_handle")

# Each family must resolve in one link. esp_sdk's delete cannot free what a
# client's new allocated.
c_runtime_families: Final = (("_Znw", "_Zna", "_Zdl", "_Zda"), ("__cxa_guard_",))


class esp_sdk_symbol(NamedTuple):
	symbol_value: int
	symbol_size: int
	symbol_type: int
	weak_binding: bool
	section_index: int


def esp_sdk_symbols_from_elf(elf_file_path: str) -> Dict[str, esp_sdk_symbol]:
	with open(elf_file_path, "rb") as elf_stream:
		elf_bytes = elf_stream.read()
	if elf_bytes[0:6] != b"\x7fELF\x01\x01":
		raise ValueError(f"{elf_file_path} is not a 32 bit little endian ELF file")
	(table_offset,) = struct.unpack_from("<I", elf_bytes, 32)
	entry_size, entry_count = struct.unpack_from("<HH", elf_bytes, 46)
	# sh_type, sh_offset, sh_size and sh_link.
	section_headers = [struct.unpack_from("<4xI8xIII", elf_bytes,
		table_offset + entry_index * entry_size) for entry_index in range(entry_count)]
	global_symbols: Dict[str, esp_sdk_symbol] = {}
	for section_type, symbols_offset, symbols_size, strings_index in section_headers:
		if section_type != c_symbol_table_type:
			continue
		strings_offset = section_headers[strings_index][1]
		for symbol_offset in range(symbols_offset + 16, symbols_offset + symbols_size, 16):
			name_offset, symbol_value, symbol_size, symbol_information, section_index = (
				struct.unpack_from("<IIIBxH", elf_bytes, symbol_offset))
			symbol_binding = symbol_information >> 4
			if symbol_binding != c_global_binding and symbol_binding != c_weak_binding:
				continue
			name_begin = strings_offset + name_offset
			symbol_name = elf_bytes[name_begin:elf_bytes.index(b"\0", name_begin)].decode("ascii")
			global_symbols[symbol_name] = esp_sdk_symbol(symbol_value, symbol_size,
				symbol_information & 0xf, symbol_binding == c_weak_binding, section_index)
	return global_symbols


def esp_sdk_exports_script(esp_sdk_file_path: str, boot_size: int) -> str:
	esp_sdk_symbols = esp_sdk_symbols_from_elf(esp_sdk_file_path)
	client_region = esp_sdk_symbols.get("esp_sdk_client_region")
	if client_region is None or client_region.section_index == c_undefined_section:
		raise ValueError(f"{esp_sdk_file_path} does not define esp_sdk_client_region")
	if not 0 < boot_size < client_region.symbol_size:
		raise ValueError(f"a boot size of {boot_size:#x} does not fit esp_sdk_client_region")
	image_size = client_region.symbol_size - boot_size
	# Without attributes ld puts no unplaced section into esp_sdk_image.
	script_lines = [
		"/* Written by esp_sdk_exports.py from esp_sdk.elf. */",
		"MEMORY",
		"{",
		f"\tesp_sdk_image : ORIGIN = {client_region.symbol_value:#x}, LENGTH = {image_size:#x}",
		f"\tesp_sdk_boot : ORIGIN = {client_region.symbol_value + image_size:#x}, "
			f"LENGTH = {boot_size:#x}",
		"}"]
	# gp holds esp_sdk's __global_pointer$. A client does not relax against it.
	for symbol_name, esp_sdk_symbol in sorted(esp_sdk_symbols.items()):
		if (esp_sdk_symbol.section_index != c_undefined_section
				and esp_sdk_symbol.symbol_type != c_thread_local_type
				and symbol_name != "__global_pointer$"):
			script_lines.append(f"PROVIDE(\"{symbol_name}\" = {esp_sdk_symbol.symbol_value:#x});")
	return "\n".join(script_lines) + "\n"


def esp_sdk_check_client(esp_sdk_file_path: str, client_file_path: str) -> List[str]:
	esp_sdk_symbols = esp_sdk_symbols_from_elf(esp_sdk_file_path)
	client_symbols = esp_sdk_symbols_from_elf(client_file_path)
	error_messages: List[str] = []
	runtime_locations: Dict[str, str] = {}
	for symbol_name, client_symbol in sorted(client_symbols.items()):
		if client_symbol.section_index == c_undefined_section:
			symbol_location = "undefined"
			if not client_symbol.weak_binding:
				error_messages.append(f"{symbol_name} is not exported by esp_sdk, add it to "
					"target/esp_sdk/main/esp_sdk_exports.txt")
		elif client_symbol.section_index == c_absolute_section:
			# The exports are absolute PROVIDEs.
			symbol_location = "esp_sdk"
		else:
			symbol_location = "client"
		if symbol_name.startswith(c_runtime_prefixes):
			print(f"{symbol_name} {symbol_location}")
			runtime_locations[symbol_name] = symbol_location
			continue
		esp_sdk_symbol = esp_sdk_symbols.get(symbol_name)
		if (symbol_location == "client" and esp_sdk_symbol is not None
				and (client_symbol.weak_binding or esp_sdk_symbol.weak_binding)):
			error_messages.append(f"{symbol_name} is weak in the client or in esp_sdk, so the "
				"client's definition does not override esp_sdk's")
	for runtime_family in c_runtime_families:
		family_locations = {runtime_location for runtime_name, runtime_location
			in runtime_locations.items()
			if runtime_name.startswith(runtime_family) and runtime_location != "undefined"}
		if len(family_locations) > 1:
			error_messages.append(f"{'*, '.join(runtime_family)}* resolve to both the client "
				"and esp_sdk")
	return error_messages


def esp_sdk_exports_main(command_arguments: Sequence[str]) -> int:
	argument_parser = argparse.ArgumentParser(prog="esp_sdk_exports.py",
		description="Links the clients of esp_sdk against esp_sdk.elf.")
	command_parsers = argument_parser.add_subparsers(dest="command_name", required=True)
	exports_parser = command_parsers.add_parser("exports",
		help="writes the client regions and a PROVIDE for each symbol of esp_sdk")
	exports_parser.add_argument("--boot-size", required=True, metavar="<bytes>",
		type=lambda size_text: int(size_text, 0), help="the size of hxboot's slice")
	exports_parser.add_argument("esp_sdk_file_path", metavar="<esp_sdk.elf>")
	exports_parser.add_argument("script_file_path", metavar="<out.ld>")
	check_parser = command_parsers.add_parser("check",
		help="fails a client that does not link correctly with esp_sdk")
	check_parser.add_argument("esp_sdk_file_path", metavar="<esp_sdk.elf>")
	check_parser.add_argument("client_file_path", metavar="<client.elf>")
	parsed_arguments = argument_parser.parse_args(command_arguments)
	command_name: str = parsed_arguments.command_name
	esp_sdk_file_path: str = parsed_arguments.esp_sdk_file_path
	try:
		if command_name == "exports":
			boot_size: int = parsed_arguments.boot_size
			script_file_path: str = parsed_arguments.script_file_path
			script_text = esp_sdk_exports_script(esp_sdk_file_path, boot_size)
			with open(script_file_path, "w", encoding="ascii") as script_stream:
				script_stream.write(script_text)
			return 0
		client_file_path: str = parsed_arguments.client_file_path
		error_messages = esp_sdk_check_client(esp_sdk_file_path, client_file_path)
	except (OSError, ValueError, struct.error) as elf_error:
		print(f"error: {elf_error}", file=sys.stderr)
		return 1
	for error_message in error_messages:
		print(f"error: {client_file_path}: {error_message}", file=sys.stderr)
	return 1 if error_messages else 0


if __name__ == "__main__":
	sys.exit(esp_sdk_exports_main(sys.argv[1:]))
