# SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
# SPDX-License-Identifier: MIT
# This file is licensed under the terms of the LICENSE.md file.

import gdb # type: ignore
import gdb.printing # type: ignore
import traceback

# hxdeque uses this layout:
#
#	template<typename T_, hxsize_t capacity_>
#	class hxdeque : public hxallocator<T_, capacity_> {
#		// ...
#		hxsize_t m_mask_;   // capacity - 1
#		hxsize_t m_head_;   // index of front element
#		hxsize_t m_tail_;   // index one past back element
#		hxsize_t m_count_;  // number of elements
#	};
#
# hxallocator<T_, capacity_> stores elements as char[capacity_ * hxsizeof(T_)],
# so m_data_.address is always the base of the element array.
#
# Elements are stored at indices (m_head_ + i) & m_mask_ for i in [0, m_count_).
#

class HxDequePrinter:
	def __init__(self, val):
		self.val = val

	def to_string(self):
		try:
			count = int(self.val['m_count_'])
			mask = int(self.val['m_mask_'])
			capacity = mask + 1 if mask != 0 else 0

			if self.val['m_count_'].is_optimized_out:
				return '<optimized out>'

			if capacity == 0:
				return '<unallocated>'

			elem_type = self.val.type.template_argument(0)

			self._count = count
			self._capacity = capacity
			self._mask = mask
			self._head = int(self.val['m_head_'])
			self._data_addr = int(self.val['m_data_'].address)
			self._elem_type = elem_type

			basename = f'{elem_type}'.split(':')[-1]
			return f'[{count}/{capacity}] {basename}'
		except Exception:
			error = f'{traceback.format_exc()}'
			return error.split('\n', 1)[1]

	def children(self):
		try:
			if not hasattr(self, '_count'):
				return
			for i in range(self._count):
				slot = (self._head + i) & self._mask
				addr = self._data_addr + slot * self._elem_type.sizeof
				ptr = gdb.Value(addr).cast(self._elem_type.pointer())
				yield (f'[{i}]', ptr.dereference())
		except Exception:
			return

	def display_hint(self):
		return 'array'

def build_pretty_printer():
	pp = gdb.printing.RegexpCollectionPrettyPrinter('hxdeque_printer')
	pp.add_printer('hxdeque', r'hxdeque<', HxDequePrinter)
	return pp

gdb.printing.register_pretty_printer(gdb.current_objfile(), build_pretty_printer(), replace=True)
