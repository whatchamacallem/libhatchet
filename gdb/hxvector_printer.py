# SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
# SPDX-License-Identifier: MIT
# This file is licensed under the terms of the LICENSE.md file.

import gdb
import gdb.printing
import re
import traceback
from typing import Iterator, Optional, Tuple

# hxvector uses this layout:
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
#	template<hxvector_concept_ T_, hxsize_t capacity_=hxallocator_dynamic_capacity>
#	class hxvector : private hxallocator<T_, capacity_> {
#		// ...
#		T_* m_end_;
#	}
#

class hxvector_printer:
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

		data_val: gdb.Value = self.val['m_data_']
		end_val: gdb.Value = self.val['m_end_']

		if data_val.is_optimized_out or end_val.is_optimized_out:
			self._summary = '<optimized out>'
			return self._summary

		targ1: gdb.Value = self.val.type.template_argument(1)

		capacity: int
		data: int
		if targ1 == 0:
			capacity = int(self.val['m_capacity_'])
			if capacity <= 0:
				self._summary = '<unallocated>'
				return self._summary
			data = int(data_val)
		else:
			capacity = int(targ1)
			data = int(data_val.address)

		end: int = int(end_val)

		elem_type: gdb.Type = self.val.type.template_argument(0)
		size: int = (end - data) // elem_type.sizeof
		if size < 0:
			self._summary = '<negative size>'
			return self._summary

		self._elem_type = elem_type
		self._size = size
		self._data = data
		self._ok = True

		basename: str = re.sub(r'^((\w+|\(anonymous namespace\))::)+', '', f'{elem_type}')
		self._summary = '[{}/{}] {}'.format(size, capacity, basename)
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
	pp = gdb.printing.RegexpCollectionPrettyPrinter('hxvector')
	pp.add_printer('hxvector', r'^(\w+::)*hxvector<.*>$', hxvector_printer)
	return pp

gdb.printing.register_pretty_printer(gdb.current_objfile(), build_pretty_printer(), replace=True)
