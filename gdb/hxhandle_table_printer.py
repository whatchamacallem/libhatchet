# SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
# SPDX-License-Identifier: MIT
# This file is licensed under the terms of the LICENSE.md file.

import gdb
import gdb.printing  # pyright: ignore[reportMissingModuleSource]
import re
import traceback
from typing import Iterator, Optional, Tuple

# hxhandle_table uses this layout:
#
#	class slot_t_ {
#		uint64_t m_key_;
#		union {
#			T_* m_ptr_;
#			slot_t_* m_next_;
#		};
#	};
#	template<typename T_, typename deleter_t_=hxdefault_delete, uint32_t table_size_bits_=0>
#	class hxhandle_table : private deleter_t_ {
#		// ...
#		hxpow2_allocator_<slot_t_, table_size_bits_, false> m_table_;  // : hxallocator<slot_t_, N>
#		uint32_t m_size_;
#		slot_t_* m_free_head_;
#	};
#
# Slot 0 is a permanent sentinel. A slot is occupied when m_next_ does not
# point within the table's own storage, matching the check in the .inl file.
#

class hxhandle_table_printer:
	def __init__(self, val: gdb.Value) -> None:
		self.val: gdb.Value = val
		self._summary: Optional[str] = None
		self._ok: bool = False
		self._value_type: gdb.Type
		self._slot_type: gdb.Type
		self._capacity: int
		self._data_addr: int
		self._addr_mask: int

	def _parse(self) -> str:
		if self._summary is not None:
			return self._summary

		size: gdb.Value = self.val['m_size_']
		if size.is_optimized_out:
			self._summary = '<optimized out>'
			return self._summary

		table: gdb.Value = self.val['m_table_']
		allocator_type: gdb.Type = table.type.fields()[0].type
		allocator: gdb.Value = table.cast(allocator_type)

		value_type: gdb.Type = self.val.type.template_argument(0)
		slot_type: gdb.Type = allocator_type.template_argument(0)
		fixed_capacity: gdb.Value = allocator_type.template_argument(1)
		type_name: str = re.sub(r'(\w+|\(anonymous namespace\))::', '', f'{self.val.type.strip_typedefs()}')

		capacity: int
		data_addr: int
		if fixed_capacity == 0:
			capacity = int(allocator['m_capacity_'])
			if capacity <= 0:
				self._summary = f'[0/0] {type_name}'
				return self._summary
			data_addr = int(allocator['m_data_'])
		else:
			capacity = int(fixed_capacity)
			data_addr = int(allocator['m_data_'].address)

		self._value_type = value_type
		self._slot_type = slot_type
		self._capacity = capacity
		self._data_addr = data_addr
		self._addr_mask = (1 << (8 * slot_type.pointer().sizeof)) - 1
		self._ok = True

		self._summary = f'[{int(size)}/{capacity - 1}] {type_name}'
		return self._summary

	def to_string(self) -> str:
		try:
			return self._parse()
		except Exception:
			error: str = f'{traceback.format_exc()}'
			return error.split('\n', 1)[1]

	def children(self) -> Iterator[Tuple[str, gdb.Value]]:
		try:
			self._parse()
			if not self._ok:
				return
			slot_ptr_type: gdb.Type = self._slot_type.pointer()
			value_ptr_type: gdb.Type = self._value_type.pointer()
			bytes_span: int = self._capacity * self._slot_type.sizeof
			for i in range(1, self._capacity):
				slot: gdb.Value = gdb.Value(self._data_addr + i * self._slot_type.sizeof).cast(
					slot_ptr_type).dereference()
				next_addr: int = int(slot['m_next_']) & self._addr_mask
				if ((next_addr - self._data_addr) & self._addr_mask) < bytes_span:
					continue  # Free slot.
				key: gdb.Value = slot['m_key_']
				ptr: gdb.Value = gdb.Value(int(slot['m_ptr_'])).cast(value_ptr_type)
				yield (f'[{int(key):x}]', ptr.dereference())
		except Exception:
			return

def build_pretty_printer() -> gdb.printing.RegexpCollectionPrettyPrinter:
	pp = gdb.printing.RegexpCollectionPrettyPrinter('hxhandle_table')
	pp.add_printer('hxhandle_table', r'^(\w+::)*hxhandle_table<.*>$', hxhandle_table_printer)
	return pp

gdb.printing.register_pretty_printer(gdb.current_objfile(), build_pretty_printer(), replace=True)
