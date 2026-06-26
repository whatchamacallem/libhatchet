# SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
# SPDX-License-Identifier: MIT
# This file is licensed under the terms of the LICENSE.md file.

import gdb # type: ignore
import gdb.printing # type: ignore
import traceback

# hxarray uses this allocation strategy:
#
#	template<typename T_, hxsize_t fixed_capacity_>
#	class hxallocator {
#		// ...
#		alignas(T_) char m_data_[fixed_capacity_ * hxsizeof<T_>()];
#	};
#	template<typename T_>
#	class hxallocator<T_, 0> {
#		// ...
#		hxsize_t m_capacity_;
#		T_* m_data_;
#	};
#	template<typename T_, hxsize_t capacity_=0>
#	class hxarray : private hxallocator<T_, capacity_> {
#		// ...
#		// No m_end_. Size == capacity_ always.
#	}
#

class HxArrayPrinter:
	"""
	Pretty printer for hxarray<T, capacity>. Size always equals capacity.
	Supports both static (capacity > 0) and dynamic (capacity == 0) storage.
	"""

	def __init__(self, val):
		self.val = val

	def to_string(self):
		try:
			data = self.val['m_data_']

			if data.is_optimized_out:
				return '<optimized out>'

			elem_type = self.val.type.template_argument(0)
			targ1 = self.val.type.template_argument(1)

			if targ1 == 0:
				# Dynamic capacity: m_data_ is a pointer, m_capacity_ holds the size.
				capacity = int(self.val['m_capacity_'])
				if capacity == 0:
					return '<unallocated>'
				raw_data = int(data)
			else:
				# Static capacity: m_data_ is an inline char array whose
				# address is the element buffer base.
				capacity = int(targ1)
				if capacity <= 0:
					return '<invalid capacity>'
				raw_data = int(data.address)

			self._elem_type = elem_type
			self._size = capacity
			self._data = raw_data

			basename = f'{self._elem_type}'.split(':')[-1]
			return '[{}] {}'.format(capacity, basename)
		except Exception:
			error = f'{traceback.format_exc()}'
			return error.split('\n', 1)[1]

	def children(self):
		try:
			if not hasattr(self, '_size'):
				return
			for i in range(self._size):
				int_ptr = self._data + i * self._elem_type.sizeof
				elem_ptr = gdb.Value(int_ptr).cast(self._elem_type.pointer())
				yield (f'[{i}]', elem_ptr.dereference())
		except Exception:
			return

	def display_hint(self):
		return 'array'

def build_pretty_printer():
	pp = gdb.printing.RegexpCollectionPrettyPrinter('hxarray_printer')
	pp.add_printer('hxarray', r'^hxarray<', HxArrayPrinter)
	return pp

gdb.printing.register_pretty_printer(gdb.current_objfile(), build_pretty_printer(), replace=True)
