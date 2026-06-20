# SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
# SPDX-License-Identifier: MIT
# This file is licensed under the terms of the LICENSE.md file.

import gdb # type: ignore
import gdb.printing # type: ignore
import traceback

# hxflat_map uses two parallel hxallocator arrays:
#
#	template<typename T_, hxsize_t fixed_capacity_>
#	class hxallocator {
#		// ...
#		char m_data_[fixed_capacity_ * sizeof(T_)];  // static
#	};
#	template<typename T_>
#	class hxallocator<T_, 0> {
#		// ...
#		hxsize_t m_capacity_;
#		T_* m_data_;
#	};
#	template<typename key_t_, typename mapped_t_, typename compare_t_,
#	         bool multi_t_, hxsize_t capacity_>
#	class hxflat_map {
#		hxsize_t m_size_;
#		hxallocator<key_t_, capacity_> m_keys_;
#		hxallocator<mapped_t_, capacity_> m_values_;
#	};
#

class HxFlatMapPrinter:
	"""
	Pretty printer for hxflat_map<key_t, mapped_t, compare_t, multi_t, capacity>.
	Supports both static (capacity > 0) and dynamic (capacity == 0) storage.
	"""

	def __init__(self, val):
		self.val = val

	def to_string(self):
		try:
			size = int(self.val['m_size_'])
			keys_alloc = self.val['m_keys_']
			vals_alloc = self.val['m_values_']

			key_type = self.val.type.template_argument(0)
			mapped_type = self.val.type.template_argument(1)
			cap_arg = self.val.type.template_argument(4)

			if cap_arg == 0:
				capacity = int(keys_alloc['m_capacity_'])
				if capacity == 0:
					return '<unallocated>'
				raw_keys = int(keys_alloc['m_data_'])
				raw_vals = int(vals_alloc['m_data_'])
			else:
				capacity = int(cap_arg)
				raw_keys = int(keys_alloc['m_data_'].address)
				raw_vals = int(vals_alloc['m_data_'].address)

			self._key_type = key_type
			self._mapped_type = mapped_type
			self._size = size
			self._raw_keys = raw_keys
			self._raw_vals = raw_vals

			key_name = f'{key_type}'.split(':')[-1]
			val_name = f'{mapped_type}'.split(':')[-1]
			return '[{}/{}] {}->{}'.format(size, capacity, key_name, val_name)
		except Exception as e:
			error = f'{traceback.format_exc()}'
			return error.split('\n', 1)[1]

	def children(self):
		try:
			if not hasattr(self, '_size'):
				return
			for i in range(self._size):
				key_ptr = gdb.Value(self._raw_keys + i * self._key_type.sizeof).cast(
					self._key_type.pointer())
				val_ptr = gdb.Value(self._raw_vals + i * self._mapped_type.sizeof).cast(
					self._mapped_type.pointer())
				yield (f'[{i}].key', key_ptr.dereference())
				yield (f'[{i}].value', val_ptr.dereference())
		except Exception:
			return

	def display_hint(self):
		return 'map'

def build_pretty_printer():
	pp = gdb.printing.RegexpCollectionPrettyPrinter('hxflat_map_printer')
	pp.add_printer('hxflat_map', r'hxflat_map<', HxFlatMapPrinter)
	return pp

gdb.printing.register_pretty_printer(gdb.current_objfile(), build_pretty_printer(), replace=True)
