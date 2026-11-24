# SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
# SPDX-License-Identifier: MIT
# This file is licensed under the terms of the LICENSE.md file.
#
# Writes hxfont.raw from X11's misc-fixed 8x13, a public domain font. hxfont.raw
# is a 128 byte map from each ASCII code to the index of its glyph, then the
# glyphs of the 95 codes from space to tilde. A code that is not printable in
# the C locale maps to the glyph of '?'. A glyph is 12 bytes, rows 1 to 12 of
# its 8x13 cell, one bit per pixel with the leftmost in the high bit. Row 0 is
# blank in every glyph and is not stored.

import argparse
import pathlib
import sys
import urllib.request
from typing import Dict, Final, List, Sequence

c_font_url: Final = ("https://gitlab.freedesktop.org/xorg/font/misc-misc/-/raw/"
	"font-misc-misc-1.1.3/8x13.bdf")
c_cache_file: Final = pathlib.Path.home() / ".cache" / "hxfont" / "8x13.bdf"

c_cell_height: Final = 13
c_cell_box: Final = "8 13 0 -2"
c_map_size: Final = 128
c_printable_codes: Final = range(0x20, 0x7f)


def hxfont_bdf_text() -> str:
	if not c_cache_file.is_file():
		c_cache_file.parent.mkdir(parents=True, exist_ok=True)
		with urllib.request.urlopen(c_font_url, timeout=60) as font_response:
			font_bytes: bytes = font_response.read()
		partial_file = c_cache_file.with_suffix(".part")
		partial_file.write_bytes(font_bytes)
		partial_file.replace(c_cache_file)
	return c_cache_file.read_text(encoding="ascii")


def hxfont_glyph_rows(font_text: str) -> Dict[int, List[int]]:
	glyph_rows: Dict[int, List[int]] = {}
	code_point = -1
	cell_box = ""
	bitmap_rows: List[int] = []
	in_bitmap = False
	for font_line in font_text.splitlines():
		if font_line.startswith("ENCODING "):
			code_point = int(font_line.split()[1])
		elif font_line.startswith("BBX "):
			cell_box = font_line[4:].strip()
		elif font_line == "BITMAP":
			bitmap_rows = []
			in_bitmap = True
		elif font_line == "ENDCHAR":
			in_bitmap = False
			if code_point in c_printable_codes:
				if cell_box != c_cell_box or len(bitmap_rows) != c_cell_height:
					raise SystemExit(f"hxfont.py: Glyph {code_point:#x} does not fill its cell.")
				glyph_rows[code_point] = bitmap_rows
		elif in_bitmap:
			bitmap_rows.append(int(font_line, 16))
	if len(glyph_rows) != len(c_printable_codes):
		raise SystemExit("hxfont.py: A printable glyph is missing.")
	return glyph_rows


def hxfont_raw(glyph_rows: Dict[int, List[int]]) -> bytes:
	question_index = ord("?") - c_printable_codes.start
	glyph_map = bytes(code_point - c_printable_codes.start if code_point in c_printable_codes
		else question_index for code_point in range(c_map_size))
	glyph_bytes = bytearray()
	for code_point in c_printable_codes:
		cell_rows = glyph_rows[code_point]
		if cell_rows[0] != 0:
			raise SystemExit(f"hxfont.py: Row 0 of glyph {code_point:#x} is not blank.")
		glyph_bytes += bytes(cell_rows[1:])
	return glyph_map + bytes(glyph_bytes)


def hxfont_main(command_arguments: Sequence[str]) -> int:
	argument_parser = argparse.ArgumentParser()
	argument_parser.add_argument("output_file", nargs="?", default="hxfont.raw",
		metavar="<output file>")
	parsed_arguments = argument_parser.parse_args(command_arguments)
	raw_bytes = hxfont_raw(hxfont_glyph_rows(hxfont_bdf_text()))
	pathlib.Path(parsed_arguments.output_file).write_bytes(raw_bytes)
	return 0


if __name__ == "__main__":
	sys.exit(hxfont_main(sys.argv[1:]))
