# SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
# SPDX-License-Identifier: MIT
# This file is licensed under the terms of the LICENSE.md file.

import gdb
import gdb.printing
import traceback
from typing import Iterator, Tuple

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
		self._elem_type: gdb.Type
		self._size: int
		self._data: int

	def to_string(self) -> str:
		try:
			data_val: gdb.Value = self.val['m_data_']
			end_val: gdb.Value = self.val['m_end_']

			if data_val.is_optimized_out or end_val.is_optimized_out:
				return '<optimized out>'

			capacity: int
			if self.val.type.template_argument(1) == 0:
				capacity = int(self.val['m_capacity_'])
			else:
				capacity = int(self.val.type.template_argument(1))
			if capacity == 0:
				return '<unallocated>'
			if capacity < 0:
				return '<negative capacity>'

			data: int
			if self.val.type.template_argument(1) != 0:
				data = int(data_val.address)
			else:
				data = int(data_val)
			end: int = int(end_val)

			elem_type: gdb.Type = self.val.type.template_argument(0)
			size: int = int((end - data) / elem_type.sizeof)
			if size < 0:
				return '<negative size>'

			self._elem_type = elem_type
			self._size = size
			self._data = data

			basename: str = f'{self._elem_type}'.split(':')[-1]
			return '[{}/{}] {}'.format(self._size, capacity, basename)
		except Exception as e:
			error: str = f'{traceback.format_exc()}'
			return error.split('\n', 1)[1]

	def children(self) -> Iterator[Tuple[str, gdb.Value]]:
		try:
			if not hasattr(self, '_size'):
				return
			for i in range(self._size):
				int_ptr: int = self._data + i * self._elem_type.sizeof
				elem_ptr: gdb.Value = gdb.Value(int_ptr).cast(self._elem_type.pointer())
				yield (f'[{i}]', elem_ptr.dereference())
		except Exception:
			return

	def display_hint(self) -> str:
		return 'vector'

def build_pretty_printer() -> gdb.printing.RegexpCollectionPrettyPrinter:
	pp = gdb.printing.RegexpCollectionPrettyPrinter('hxvector')
	pp.add_printer('hxvector', r'^hxvector<', hxvector_printer)
	return pp

gdb.printing.register_pretty_printer(gdb.current_objfile(), build_pretty_printer(), replace=True)
