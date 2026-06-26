# SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
# SPDX-License-Identifier: MIT
# This file is licensed under the terms of the LICENSE.md file.

import gdb # type: ignore
import gdb.printing # type: ignore
import traceback

# hxflat_set uses this allocation strategy:
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
#	template<typename key_t_, typename compare_t_, bool multi_t_, hxsize_t capacity_>
#	class hxflat_set : private hxallocator<key_t_, capacity_> {
#		// ...
#		key_t_* m_end_;
#	}
#

class HxFlatSetPrinter:
	"""
	Pretty printer for hxflat_set<key_t, compare_t, multi_t, capacity>.
	Supports both static (capacity > 0) and dynamic (capacity == 0) storage.
	"""

	def __init__(self, val):
		self.val = val

	def to_string(self):
		try:
			data = self.val['m_data_']
			end = self.val['m_end_']

			if data.is_optimized_out or end.is_optimized_out:
				return '<optimized out>'

			elem_type = self.val.type.template_argument(0)
			cap_arg = self.val.type.template_argument(3)

			if cap_arg == 0:
				capacity = int(self.val['m_capacity_'])
				if capacity == 0:
					return '<unallocated>'
				raw_data = int(data)
			else:
				capacity = int(cap_arg)
				raw_data = int(data.address)

			raw_end = int(end)
			size = int((raw_end - raw_data) / elem_type.sizeof)
			if size < 0:
				return '<negative size>'

			self._elem_type = elem_type
			self._size = size
			self._data = raw_data

			basename = f'{elem_type}'.split(':')[-1]
			return '[{}/{}] {}'.format(size, capacity, basename)
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
		return 'array'

def build_pretty_printer():
	pp = gdb.printing.RegexpCollectionPrettyPrinter('hxflat_set_printer')
	pp.add_printer('hxflat_set', r'hxflat_set<', HxFlatSetPrinter)
	return pp

gdb.printing.register_pretty_printer(gdb.current_objfile(), build_pretty_printer(), replace=True)
