# SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
# SPDX-License-Identifier: MIT
# This file is licensed under the terms of the LICENSE.md file.

import gdb # type: ignore
import gdb.printing # type: ignore
import traceback

# hxvector uses this allocation strategy:
#
#	template<typename T_, hxsize_t fixed_capacity_>
#	class hxallocator {
#		// ...
#		T_ m_data_[fixed_capacity_];
#	};
#	template<typename T_>
#	class hxallocator<T_, 0> {
#		// ...
#		int m_capacity_;
#		T_* m_data_;
#	};
#	template<typename T_, hxsize_t capacity_=0>
#	class hxvector : public hxallocator<T_, capacity_> {
#		// ...
#		T_* m_end_;
#	}
#

class HxVectorPrinter:
	"""
	Pretty printer for hxvector<T, capacity>. There are two different underlying
	implementations and this logic works for both of them.
	"""

	def __init__(self, val):
		self.val = val

	def to_string(self):
		try:
			data = self.val['m_data_']
			end = self.val['m_end_']

			if data.is_optimized_out or end.is_optimized_out:
				return '<optimized out>'

			if self.val.type.template_argument(1) == 0:
				capacity = int(self.val['m_capacity_'])
			else:
				capacity = int(self.val.type.template_argument(1))
			if capacity == 0:
				return '<unallocated>'
			if capacity < 0:
				return '<negative capacity>'

			# There are two different underlying implementations and this logic
			# works for both of them. This is Python, so we have int instead of
			# uintptr_t to calculate addresses with.
			if self.val.type.template_argument(1) != 0:
				data = int(data.address)
			else:
				data = int(data)
			end = int(end)

			elem_type = self.val.type.template_argument(0)
			size = int((end - data) / elem_type.sizeof)
			if size < 0:
				return '<negative size>'

			# Cache these for calculating children.
			self._elem_type = elem_type
			self._size = size
			self._data = data

			basename = f'{self._elem_type}'.split(':')[-1]
			return '[{}/{}] {}'.format(self._size, capacity, basename)
		except Exception as e:
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
		return 'vector'

def build_pretty_printer():
	pp = gdb.printing.RegexpCollectionPrettyPrinter('hxvector_printer')
	pp.add_printer('hxvector', r'^hxvector<', HxVectorPrinter)
	return pp

gdb.printing.register_pretty_printer(gdb.current_objfile(), build_pretty_printer(), replace=True)
