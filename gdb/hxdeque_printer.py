# SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
# SPDX-License-Identifier: MIT
# This file is licensed under the terms of the LICENSE.md file.

import gdb
import gdb.printing
import re
import traceback
from typing import Iterator, Optional, Tuple

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
#		size_t m_head_;  // free running offset of front element
#		size_t m_tail_;  // free running offset one past back element
#	};
#
# The offsets wrap modulo the size_t range. Elements are stored at indices
# (m_head_ + i) & (capacity - 1) for i in [0, m_tail_ - m_head_).
#

class hxdeque_printer:
	def __init__(self, val: gdb.Value) -> None:
		self.val: gdb.Value = val
		self._summary: Optional[str] = None
		self._ok: bool = False
		self._count: int
		self._capacity: int
		self._mask: int
		self._head: int
		self._data_addr: int
		self._elem_type: gdb.Type

	def _parse(self) -> str:
		if self._summary is not None:
			return self._summary

		head_val: gdb.Value = self.val['m_head_']
		tail_val: gdb.Value = self.val['m_tail_']
		if head_val.is_optimized_out or tail_val.is_optimized_out:
			self._summary = '<optimized out>'
			return self._summary

		head: int = int(head_val)
		tail: int = int(tail_val)
		wrap: int = 1 << (8 * head_val.type.sizeof)
		count: int = (tail - head) % wrap

		targ1: gdb.Value = self.val.type.template_argument(1)
		capacity: int
		data_addr: int
		if targ1 == 0:
			capacity = int(self.val['m_capacity_'])
			if capacity <= 0:
				self._summary = '<unallocated>'
				return self._summary
			data_addr = int(self.val['m_data_'])
		else:
			capacity = int(targ1)
			data_addr = int(self.val['m_data_'].address)

		elem_type: gdb.Type = self.val.type.template_argument(0)

		self._count = count
		self._capacity = capacity
		self._mask = capacity - 1
		self._head = head
		self._data_addr = data_addr
		self._elem_type = elem_type
		self._ok = True

		basename: str = re.sub(r'^((\w+|\(anonymous namespace\))::)+', '', f'{elem_type}')
		self._summary = f'[{count}/{capacity}] {basename}'
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
	pp.add_printer('hxdeque', r'^(\w+::)*hxdeque<.*>$', hxdeque_printer)
	return pp

gdb.printing.register_pretty_printer(gdb.current_objfile(), build_pretty_printer(), replace=True)
