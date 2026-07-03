# SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
# SPDX-License-Identifier: MIT
# This file is licensed under the terms of the LICENSE.md file.

import gdb
import gdb.printing
import re
import traceback
from typing import Iterator, Optional, Tuple

# hxarray uses this layout:
#
#	template<typename T_, hxsize_t fixed_capacity_>
#	class hxallocator {
#		// ...
#		alignas(T_) char m_data_[fixed_capacity_ * hxsizeof<T_>()];
#	};
#	template<typename T_>
#	class hxallocator<T_, hxallocator_dynamic_capacity> {
#		// ...
#		hxsize_t m_capacity_;
#		T_* m_data_;
#	};
#	template<hxarray_concept_ T_, hxsize_t capacity_=hxallocator_dynamic_capacity>
#	class hxarray : private hxallocator<T_, capacity_> {
#		// ...
#		// No m_end_. Size == capacity_ always.
#	}
#

class hxarray_printer:
	def __init__(self, val: gdb.Value) -> None:
		self.val: gdb.Value = val
		self._summary: Optional[str] = None
		self._ok: bool = False
		self._elem_type: gdb.Type
		self._size: int
		self._data: int

	def _parse(self) -> str:
		if self._summary is not None:
			return self._summary

		data: gdb.Value = self.val['m_data_']

		if data.is_optimized_out:
			self._summary = '<optimized out>'
			return self._summary

		elem_type: gdb.Type = self.val.type.template_argument(0)
		targ1: gdb.Value = self.val.type.template_argument(1)

		capacity: int
		raw_data: int
		if targ1 == 0:
			capacity = int(self.val['m_capacity_'])
			if capacity <= 0:
				self._summary = '<unallocated>'
				return self._summary
			raw_data = int(data)
		else:
			capacity = int(targ1)
			raw_data = int(data.address)

		self._elem_type = elem_type
		self._size = capacity
		self._data = raw_data
		self._ok = True

		basename: str = re.sub(r'^((\w+|\(anonymous namespace\))::)+', '', f'{elem_type}')
		self._summary = '[{}] {}'.format(capacity, basename)
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
			for i in range(self._size):
				int_ptr: int = self._data + i * self._elem_type.sizeof
				elem_ptr: gdb.Value = gdb.Value(int_ptr).cast(self._elem_type.pointer())
				yield (f'[{i}]', elem_ptr.dereference())
		except Exception:
			return

	def display_hint(self) -> str:
		return 'array'

def build_pretty_printer() -> gdb.printing.RegexpCollectionPrettyPrinter:
	pp = gdb.printing.RegexpCollectionPrettyPrinter('hxarray')
	pp.add_printer('hxarray', r'^(\w+::)*hxarray<.*>$', hxarray_printer)
	return pp

gdb.printing.register_pretty_printer(gdb.current_objfile(), build_pretty_printer(), replace=True)
