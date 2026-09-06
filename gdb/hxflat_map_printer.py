# SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
# SPDX-License-Identifier: MIT
# This file is licensed under the terms of the LICENSE.md file.

import gdb
import gdb.printing  # pyright: ignore[reportMissingModuleSource]
import re
import traceback
from typing import Iterator, Optional, Tuple

# hxflat_map uses two parallel hxallocator arrays:
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
#	template<typename key_t_,
#		typename mapped_t_,
#		typename compare_t_=hxkey_less_t<key_t_>,
#		bool multi_t_=true,
#		hxsize_t capacity_=hxallocator_dynamic_capacity>
#	class hxflat_map {
#		// ...
#		hxsize_t m_size_;
#		hxallocator<key_t_, capacity_> m_keys_;
#		hxallocator<mapped_t_, capacity_> m_values_;
#	};
#

class hxflat_map_printer:
	def __init__(self, val: gdb.Value) -> None:
		self.val: gdb.Value = val
		self._summary: Optional[str] = None
		self._ok: bool = False
		self._key_type: gdb.Type
		self._mapped_type: gdb.Type
		self._size: int
		self._raw_keys: int
		self._raw_vals: int

	def _parse(self) -> str:
		if self._summary is not None:
			return self._summary

		if self.val['m_size_'].is_optimized_out:
			self._summary = '<optimized out>'
			return self._summary

		size: int = int(self.val['m_size_'])
		keys_alloc: gdb.Value = self.val['m_keys_']
		vals_alloc: gdb.Value = self.val['m_values_']

		key_type: gdb.Type = self.val.type.template_argument(0)
		mapped_type: gdb.Type = self.val.type.template_argument(1)
		cap_arg: gdb.Value = self.val.type.template_argument(4)
		type_name: str = re.sub(r'(\w+|\(anonymous namespace\))::', '', f'{self.val.type.strip_typedefs()}')

		capacity: int
		raw_keys: int
		raw_vals: int
		if cap_arg == 0:
			capacity = int(keys_alloc['m_capacity_'])
			if capacity <= 0:
				self._summary = f'[0/0] {type_name}'
				return self._summary
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
		self._ok = True

		self._summary = f'[{size}/{capacity}] {type_name}'
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
			yield ('keys', self.val['m_keys_'])
			yield ('values', self.val['m_values_'])
			for i in range(self._size):
				key_ptr: gdb.Value = gdb.Value(self._raw_keys + i * self._key_type.sizeof).cast(
					self._key_type.pointer())
				val_ptr: gdb.Value = gdb.Value(self._raw_vals + i * self._mapped_type.sizeof).cast(
					self._mapped_type.pointer())
				yield (f'[{key_ptr.dereference()}]', val_ptr.dereference())
		except Exception:
			return

	def display_hint(self) -> str:
		return 'array'

def build_pretty_printer() -> gdb.printing.RegexpCollectionPrettyPrinter:
	pp = gdb.printing.RegexpCollectionPrettyPrinter('hxflat_map')
	pp.add_printer('hxflat_map', r'^(\w+::)*hxflat_map<.*>$', hxflat_map_printer)
	return pp

gdb.printing.register_pretty_printer(gdb.current_objfile(), build_pretty_printer(), replace=True)
