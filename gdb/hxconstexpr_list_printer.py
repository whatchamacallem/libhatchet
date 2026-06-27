# SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
# SPDX-License-Identifier: MIT
# This file is licensed under the terms of the LICENSE.md file.

import gdb
import gdb.printing
import traceback
from typing import Iterator, Optional, Set, Tuple

# hxconstexpr_list uses this layout:
#
#	class hxconstexpr_list_node {
#		// ...
#		hxconstexpr_list_node* m_list_prev_;
#		hxconstexpr_list_node* m_list_next_;
#	};
#
#	template<typename node_t_, typename deleter_t_=hxdefault_delete>
#	class hxconstexpr_list {
#		// ...
#		deleter_t_        m_deleter_;
#		hxsize_t          m_size_;
#		hxconstexpr_list_node m_sentinel_;  // sentinel: m_sentinel_.m_list_next_ is front,
#	};                                      //           m_sentinel_.m_list_prev_ is back.
#

class hxconstexpr_list_printer:
	def __init__(self, val: gdb.Value) -> None:
		self.val: gdb.Value = val
		self._size: int
		self._node_type: gdb.Type
		self._sentinel_addr: int
		self._front_addr: int
		self._base_type: Optional[gdb.Type]
		self._base_field_names: Set[Optional[str]]

	def to_string(self) -> str:
		try:
			if self.val['m_size_'].is_optimized_out:
				return '<optimized out>'

			size: int = int(self.val['m_size_'])
			node_type: gdb.Type = self.val.type.template_argument(0)

			self._size = size
			self._node_type = node_type
			self._sentinel_addr = int(self.val['m_sentinel_'].address)
			self._front_addr = int(self.val['m_sentinel_']['m_list_next_'])

			self._base_type = None
			self._base_field_names = set()
			for field in node_type.fields():
				if field.is_base_class:
					self._base_type = field.type
					for bf in field.type.fields():
						self._base_field_names.add(bf.name)
					break

			basename: str = f'{node_type}'.split(':')[-1]
			return f'[{size}] {basename}'
		except Exception:
			error: str = f'{traceback.format_exc()}'
			return error.split('\n', 1)[1]

	def _node_fields(self, node: gdb.Value) -> Iterator[Tuple[Optional[str], gdb.Value]]:
		for field in self._node_type.fields():
			if field.is_base_class:
				continue
			if field.name in self._base_field_names:
				continue
			yield (field.name, node[field.name])

	def children(self) -> Iterator[Tuple[str, gdb.Value]]:
		try:
			if not hasattr(self, '_size') or self._base_type is None:
				return
			node_ptr_type: gdb.Type = self._base_type.pointer()
			current_addr: int = self._front_addr
			for idx in range(self._size):
				if current_addr == self._sentinel_addr:
					break
				node_ptr: gdb.Value = gdb.Value(current_addr).cast(self._node_type.pointer())
				node: gdb.Value = node_ptr.dereference()
				fields: list[Tuple[Optional[str], gdb.Value]] = list(self._node_fields(node))
				if len(fields) == 1:
					yield (f'[{idx}]', fields[0][1])
				else:
					for name, val in fields:
						yield (f'[{idx}] {name}', val)
				base_ptr: gdb.Value = gdb.Value(current_addr).cast(node_ptr_type)
				current_addr = int(base_ptr['m_list_next_'])
		except Exception:
			return

	def display_hint(self) -> str:
		return 'array'

def build_pretty_printer() -> gdb.printing.RegexpCollectionPrettyPrinter:
	pp = gdb.printing.RegexpCollectionPrettyPrinter('hxconstexpr_list')
	pp.add_printer('hxconstexpr_list', r'hxconstexpr_list<', hxconstexpr_list_printer)
	return pp

gdb.printing.register_pretty_printer(gdb.current_objfile(), build_pretty_printer(), replace=True)
