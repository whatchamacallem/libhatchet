# SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
# SPDX-License-Identifier: MIT
# This file is licensed under the terms of the LICENSE.md file.

import gdb
import gdb.printing  # pyright: ignore[reportMissingModuleSource]
import re
import traceback
from typing import Iterator, Optional, Tuple

hxbitset_columns = 4
hxbitset_hex_row_prefix = '__hxbitset_hex_row__:'

class hxbitset_hex_row_printer:
	def __init__(self, row: str) -> None:
		self.row: str = row

	def to_string(self) -> str:
		return self.row

class hxbitset_hex_row_lookup:
	enabled = True
	name = 'hxbitset_hex_row'

	def __call__(self, val: gdb.Value) -> Optional[hxbitset_hex_row_printer]:
		if not str(val.type).startswith('char ['):
			return None
		row: str = val.string()
		if not row.startswith(hxbitset_hex_row_prefix):
			return None
		return hxbitset_hex_row_printer(row[len(hxbitset_hex_row_prefix):])

class hxbitset_printer:
	def __init__(self, val: gdb.Value) -> None:
		self.val: gdb.Value = val

	def _parse(self) -> Tuple[str, list[str]]:
		data_field: gdb.Value = self.val['m_data_']
		word_count: int = data_field.type.sizeof // data_field.type.target().sizeof
		byte_count: int = word_count * data_field.type.target().sizeof
		address: int = int(data_field.address)
		type_name: str = re.sub(r'(\w+|\(anonymous namespace\))::', '', f'{self.val.type.strip_typedefs()}')

		summary: str = f'[{byte_count}B] {type_name}'
		rows: list[str] = []
		column_count: int = hxbitset_columns
		if byte_count == 0 or column_count == 0:
			return summary, rows

		memory: memoryview = gdb.selected_inferior().read_memory(address, byte_count)
		hex_bytes: str = memory.tobytes().hex()
		bytes_per_row: int = column_count * 4
		for row_offset in range(0, byte_count, bytes_per_row):
			columns: list[str] = []
			row_end: int = min(row_offset + bytes_per_row, byte_count)
			for column_offset in range(row_offset, row_end, 4):
				column_end: int = min(column_offset + 4, byte_count)
				columns.append(hex_bytes[column_offset * 2:column_end * 2])
			rows.append(' '.join(columns))
		return summary, rows

	def children(self) -> Iterator[Tuple[str, gdb.Value]]:
		try:
			_, rows = self._parse()
			bytes_per_row: int = hxbitset_columns * 4
			for i, row in enumerate(rows):
				yield (f'[{i * bytes_per_row}B]', gdb.Value(hxbitset_hex_row_prefix + row))
		except Exception:
			return

	def display_hint(self) -> str:
		return 'array'

	def to_string(self) -> str:
		try:
			summary, _ = self._parse()
			return summary
		except Exception:
			error: str = f'{traceback.format_exc()}'
			return error.split('\n', 1)[1]

def build_pretty_printer() -> gdb.printing.RegexpCollectionPrettyPrinter:
	pp = gdb.printing.RegexpCollectionPrettyPrinter('hxbitset')
	pp.add_printer('hxbitset', r'^(\w+::)*hxbitset<.*>$', hxbitset_printer)
	return pp

gdb.printing.register_pretty_printer(gdb.current_objfile(), build_pretty_printer(), replace=True)
gdb.printing.register_pretty_printer(
	gdb.current_objfile(), hxbitset_hex_row_lookup(), replace=True)
