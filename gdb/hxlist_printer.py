# SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
# SPDX-License-Identifier: MIT
# This file is licensed under the terms of the LICENSE.md file.

import gdb
import gdb.printing  # pyright: ignore[reportMissingModuleSource]
import re
import traceback
from typing import Iterator, Optional, Set, Tuple

# hxlist uses this layout:
#
#	class hxlist_node {
#		// ...
#		intptr_t m_list_link_;  // prev XOR next
#	};
#
#	template<typename node_t_, typename deleter_t_=hxdefault_delete>
#	class hxlist {
#		// ...
#		deleter_t_   m_deleter_;
#		hxsize_t     m_size_;
#		hxlist_node  m_sentinel_;  // m_sentinel_.m_list_link_ == tail XOR front
#		hxlist_node* m_tail_;      // points to last node, or &m_sentinel_ when empty
#	};
#
# The links store addresses of the hxlist_node base subobject of each node.
#
# GDB sources every printer script into one shared Python namespace, so all
# module level symbols here must be unique across the gdb directory.

def _hxlist_find_link_base(node_type: gdb.Type) -> Optional[gdb.Type]:
	for field in node_type.fields():
		if not field.is_base_class:
			continue
		for base_field in field.type.fields():
			if base_field.name == 'm_list_link_':
				return field.type
		deeper: Optional[gdb.Type] = _hxlist_find_link_base(field.type)
		if deeper is not None:
			return deeper
	return None

class hxlist_printer:
	def __init__(self, val: gdb.Value) -> None:
		self.val: gdb.Value = val
		self._summary: Optional[str] = None
		self._ok: bool = False
		self._size: int
		self._node_type: gdb.Type
		self._addr_mask: int
		self._sentinel_addr: int
		self._tail_addr: int
		self._sentinel_link: int
		self._base_type: Optional[gdb.Type]
		self._base_field_names: Set[Optional[str]]

	def _parse(self) -> str:
		if self._summary is not None:
			return self._summary

		if self.val['m_size_'].is_optimized_out:
			self._summary = '<optimized out>'
			return self._summary

		size: int = int(self.val['m_size_'])
		node_type: gdb.Type = self.val.type.template_argument(0)

		# m_list_link_ is a signed intptr_t. Mask all link arithmetic to the
		# pointer width so signed values compare equal to unsigned addresses.
		addr_mask: int = (1 << (8 * self.val['m_tail_'].type.sizeof)) - 1

		self._size = size
		self._node_type = node_type
		self._addr_mask = addr_mask
		self._sentinel_addr = int(self.val['m_sentinel_'].address) & addr_mask
		self._tail_addr = int(self.val['m_tail_']) & addr_mask
		self._sentinel_link = int(self.val['m_sentinel_']['m_list_link_']) & addr_mask

		self._base_type = _hxlist_find_link_base(node_type)
		self._base_field_names = set()
		if self._base_type is not None:
			for base_field in self._base_type.fields():
				self._base_field_names.add(base_field.name)
		self._ok = True

		basename: str = re.sub(r'^((\w+|\(anonymous namespace\))::)+', '', f'{node_type}')
		self._summary = f'[{size}] {basename}'
		return self._summary

	def to_string(self) -> str:
		try:
			return self._parse()
		except Exception:
			error: str = f'{traceback.format_exc()}'
			return error.split('\n', 1)[1]

	def _node_fields(self, node: gdb.Value) -> Iterator[Tuple[Optional[str], gdb.Value]]:
		for field in self._node_type.fields():
			if field.is_base_class:
				continue
			if field.name in self._base_field_names:
				continue
			assert field.name is not None
			yield (field.name, node[field.name])

	def children(self) -> Iterator[Tuple[str, gdb.Value]]:
		try:
			self._parse()
			if not self._ok or self._base_type is None:
				return
			base_ptr_type: gdb.Type = self._base_type.pointer()
			node_ptr_type: gdb.Type = self._node_type.pointer()
			current_addr: int = (self._tail_addr ^ self._sentinel_link) & self._addr_mask
			prev_addr: int = self._sentinel_addr
			for idx in range(self._size):
				if current_addr == self._sentinel_addr:
					break
				base_ptr: gdb.Value = gdb.Value(current_addr).cast(base_ptr_type)
				node: gdb.Value = base_ptr.cast(node_ptr_type).dereference()
				fields: list[Tuple[Optional[str], gdb.Value]] = list(self._node_fields(node))
				if len(fields) == 1:
					yield (f'[{idx}]', fields[0][1])
				else:
					for name, val in fields:
						yield (f'[{idx}] {name}', val)
				link: int = int(base_ptr['m_list_link_']) & self._addr_mask
				next_addr: int = prev_addr ^ link
				prev_addr = current_addr
				current_addr = next_addr
		except Exception:
			return

	def display_hint(self) -> str:
		return 'array'

def build_pretty_printer() -> gdb.printing.RegexpCollectionPrettyPrinter:
	pp = gdb.printing.RegexpCollectionPrettyPrinter('hxlist')
	pp.add_printer('hxlist', r'^(\w+::)*hxlist<.*>$', hxlist_printer)
	return pp

gdb.printing.register_pretty_printer(gdb.current_objfile(), build_pretty_printer(), replace=True)
