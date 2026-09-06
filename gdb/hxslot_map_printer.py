# SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
# SPDX-License-Identifier: MIT
# This file is licensed under the terms of the LICENSE.md file.

import gdb
import gdb.printing  # pyright: ignore[reportMissingModuleSource]
import re
import traceback
from typing import Iterator, Optional, Tuple

# hxslot_map uses this layout:
#
#	class slot_t_ {
#		uint64_t m_handle_;
#		uint32_t m_index_;
#		uint32_t m_backref_;
#	};
#	template<hxslot_map_concept_ T_, uint32_t table_size_bits_=0>
#	class hxslot_map {
#		// ...
#		uint32_t m_size_;
#		hxslot_map_mask_<...> m_mask_;
#		hxallocator<slot_t_, s_capacity_> m_slots_;
#		hxallocator<T_, s_value_capacity_> m_values_;
#	};
#
# data()[i] is looked up by handle slots[slots[i].m_backref_].m_handle_, the
# same indirection emplace() uses to hand back a fresh value's handle.
#

def _allocator_data(allocator: gdb.Value) -> int:
	allocator_type: gdb.Type = allocator.type
	fixed_capacity: gdb.Value = allocator_type.template_argument(1)
	if fixed_capacity == 0:
		return int(allocator['m_data_'])
	return int(allocator['m_data_'].address)

class hxslot_map_printer:
	def __init__(self, val: gdb.Value) -> None:
		self.val: gdb.Value = val
		self._summary: Optional[str] = None
		self._ok: bool = False
		self._value_type: gdb.Type
		self._slot_type: gdb.Type
		self._size: int
		self._values_addr: int
		self._slots_addr: int

	def _parse(self) -> str:
		if self._summary is not None:
			return self._summary

		size: gdb.Value = self.val['m_size_']
		if size.is_optimized_out:
			self._summary = '<optimized out>'
			return self._summary

		values: gdb.Value = self.val['m_values_']
		slots: gdb.Value = self.val['m_slots_']
		value_type: gdb.Type = self.val.type.template_argument(0)
		slot_type: gdb.Type = slots.type.template_argument(0)
		type_name: str = re.sub(r'(\w+|\(anonymous namespace\))::', '', f'{self.val.type.strip_typedefs()}')

		if values.type.template_argument(1) == 0 and int(values['m_capacity_']) <= 0:
			self._summary = f'[0] {type_name}'
			return self._summary

		self._value_type = value_type
		self._slot_type = slot_type
		self._size = int(size)
		self._values_addr = _allocator_data(values)
		self._slots_addr = _allocator_data(slots)
		self._ok = True

		self._summary = f'[{self._size}] {type_name}'
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
			value_ptr_type: gdb.Type = self._value_type.pointer()
			slot_ptr_type: gdb.Type = self._slot_type.pointer()
			slots: gdb.Value = gdb.Value(self._slots_addr).cast(slot_ptr_type)
			for i in range(self._size):
				value_ptr: gdb.Value = gdb.Value(
					self._values_addr + i * self._value_type.sizeof).cast(value_ptr_type)
				backref: int = int(slots[i]['m_backref_'])
				handle: int = int(slots[backref]['m_handle_'])
				yield (f'[{handle:x}]', value_ptr.dereference())
		except Exception:
			return

	def display_hint(self) -> str:
		return 'array'

def build_pretty_printer() -> gdb.printing.RegexpCollectionPrettyPrinter:
	pp = gdb.printing.RegexpCollectionPrettyPrinter('hxslot_map')
	pp.add_printer('hxslot_map', r'^(\w+::)*hxslot_map<.*>$', hxslot_map_printer)
	return pp

gdb.printing.register_pretty_printer(gdb.current_objfile(), build_pretty_printer(), replace=True)
