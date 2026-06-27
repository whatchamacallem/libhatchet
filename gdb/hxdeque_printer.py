# SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
# SPDX-License-Identifier: MIT
# This file is licensed under the terms of the LICENSE.md file.

import gdb
import gdb.printing
import traceback
from typing import Iterator, Tuple

# hxdeque uses this layout:
#
#	template<typename T_, hxsize_t fixed_capacity_>
#	class hxallocator {
#		// ...
#		alignas(T_) char m_data_[fixed_capacity_ * hxsizeof<T_>()];  // static
#	};
#	template<typename T_>
#	class hxallocator<T_, hxallocator_dynamic_capacity> {
#		// ...
#		hxsize_t m_capacity_;
#		T_* m_data_;
#	};
#	template<typename T_, hxsize_t capacity_=hxallocator_dynamic_capacity>
#	class hxdeque : private hxallocator<T_, capacity_> {
#		// ...
#		hxsize_t m_mask_;   // capacity - 1
#		hxsize_t m_head_;   // index of front element
#		hxsize_t m_tail_;   // index one past back element
#		hxsize_t m_count_;  // number of elements
#	};
#
# Elements are stored at indices (m_head_ + i) & m_mask_ for i in [0, m_count_).
#

class hxdeque_printer:
	def __init__(self, val: gdb.Value) -> None:
		self.val: gdb.Value = val
		self._count: int
		self._capacity: int
		self._mask: int
		self._head: int
		self._data_addr: int
		self._elem_type: gdb.Type

	def to_string(self) -> str:
		try:
			count: int = int(self.val['m_count_'])
			mask: int = int(self.val['m_mask_'])
			capacity: int = mask + 1 if mask != 0 else 0

			if self.val['m_count_'].is_optimized_out:
				return '<optimized out>'

			if capacity == 0:
				return '<unallocated>'

			elem_type: gdb.Type = self.val.type.template_argument(0)

			data_addr: int
			if int(self.val.type.template_argument(1)) == 0:
				data_addr = int(self.val['m_data_'])
			else:
				data_addr = int(self.val['m_data_'].address)

			self._count = count
			self._capacity = capacity
			self._mask = mask
			self._head = int(self.val['m_head_'])
			self._data_addr = data_addr
			self._elem_type = elem_type

			basename: str = f'{elem_type}'.split(':')[-1]
			return f'[{count}/{capacity}] {basename}'
		except Exception:
			error: str = f'{traceback.format_exc()}'
			return error.split('\n', 1)[1]

	def children(self) -> Iterator[Tuple[str, gdb.Value]]:
		try:
			if not hasattr(self, '_count'):
				return
			for i in range(self._count):
				slot: int = (self._head + i) & self._mask
				addr: int = self._data_addr + slot * self._elem_type.sizeof
				ptr: gdb.Value = gdb.Value(addr).cast(self._elem_type.pointer())
				yield (f'[{i}]', ptr.dereference())
		except Exception:
			return

	def display_hint(self) -> str:
		return 'array'

def build_pretty_printer() -> gdb.printing.RegexpCollectionPrettyPrinter:
	pp = gdb.printing.RegexpCollectionPrettyPrinter('hxdeque')
	pp.add_printer('hxdeque', r'hxdeque<', hxdeque_printer)
	return pp

gdb.printing.register_pretty_printer(gdb.current_objfile(), build_pretty_printer(), replace=True)
