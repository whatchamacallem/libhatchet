# SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
# SPDX-License-Identifier: MIT
# This file is licensed under the terms of the LICENSE.md file.

import gdb
import gdb.printing  # pyright: ignore[reportMissingModuleSource]
import traceback
from typing import Iterator, Optional, Tuple

class hxallocator_columns_parameter(gdb.Parameter):
	def __init__(self) -> None:
		super().__init__('hxallocator-columns', gdb.COMMAND_DATA, gdb.PARAM_ZUINTEGER)
		self.value: int = 4

hxallocator_columns = hxallocator_columns_parameter()
hxallocator_hex_row_prefix = '__hxallocator_hex_row__:'

class hxallocator_hex_row_printer:
	def __init__(self, row: str) -> None:
		self.row: str = row

	def to_string(self) -> str:
		return self.row

class hxallocator_hex_row_lookup:
	enabled = True
	name = 'hxallocator_hex_row'

	def __call__(self, val: gdb.Value) -> Optional[hxallocator_hex_row_printer]:
		if not str(val.type).startswith('char ['):
			return None
		row: str = val.string()
		if not row.startswith(hxallocator_hex_row_prefix):
			return None
		return hxallocator_hex_row_printer(row[len(hxallocator_hex_row_prefix):])

class hxallocator_printer:
	def __init__(self, val: gdb.Value) -> None:
		self.val: gdb.Value = val

	def _parse(self) -> Tuple[str, list[str], int]:
		element_type: gdb.Type = self.val.type.template_argument(0)
		fixed_capacity: gdb.Value = self.val.type.template_argument(1)
		if fixed_capacity == 0:
			capacity: int = int(self.val['m_capacity_'])
			address: int = int(self.val['m_data_'])
			storage: str = 'dynamic'
		else:
			capacity = int(fixed_capacity)
			address = int(self.val['m_data_'].address)
			storage = 'static'

		byte_count: int = capacity * element_type.sizeof
		summary: str = f'[{byte_count}B {storage}]'
		rows: list[str] = []
		column_count: int = hxallocator_columns.value
		bytes_per_row: int = column_count * 4
		if byte_count == 0 or column_count == 0:
			return summary, rows, bytes_per_row

		memory: memoryview = gdb.selected_inferior().read_memory(address, byte_count)
		hex_bytes: str = memory.tobytes().hex()
		for row_offset in range(0, byte_count, bytes_per_row):
			columns: list[str] = []
			row_end: int = min(row_offset + bytes_per_row, byte_count)
			for column_offset in range(row_offset, row_end, 4):
				column_end: int = min(column_offset + 4, byte_count)
				columns.append(hex_bytes[column_offset * 2:column_end * 2])
			rows.append(' '.join(columns))
		return summary, rows, bytes_per_row

	def children(self) -> Iterator[Tuple[str, gdb.Value]]:
		try:
			_, rows, bytes_per_row = self._parse()
			for i, row in enumerate(rows):
				yield (f'[{i * bytes_per_row}B]', gdb.Value(hxallocator_hex_row_prefix + row))
		except Exception:
			return

	def display_hint(self) -> str:
		return 'array'

	def to_string(self) -> str:
		try:
			summary, _, _ = self._parse()
			return summary
		except Exception:
			error: str = f'{traceback.format_exc()}'
			return error.split('\n', 1)[1]

def build_pretty_printer() -> gdb.printing.RegexpCollectionPrettyPrinter:
	printer = gdb.printing.RegexpCollectionPrettyPrinter('hxallocator')
	printer.add_printer('hxallocator', r'^(\w+::)*hxallocator<.*>$', hxallocator_printer)
	return printer

gdb.printing.register_pretty_printer(
	gdb.current_objfile(), build_pretty_printer(), replace=True)
gdb.printing.register_pretty_printer(
	gdb.current_objfile(), hxallocator_hex_row_lookup(), replace=True)
