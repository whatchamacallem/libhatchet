# SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
# SPDX-License-Identifier: MIT
# This file is licensed under the terms of the LICENSE.md file.

import gdb
import gdb.printing  # pyright: ignore[reportMissingModuleSource]
import re
import traceback
from typing import Iterator, Optional, Tuple

HXHASH_TABLE_MAX_SORTED_NODES = 1024

# hxhash_table uses this layout:
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
#	template<hxhash_table_concept_ node_t_,
#		typename deleter_t_=hxdefault_delete,
#		bool multi_t_ = false,
#		hxhash_t table_size_bits_=hxallocator_dynamic_capacity>
#	class hxhash_table : private deleter_t_, private hxhash_table_base_ {
#		// ...
#		hxsize_t m_size_;
#		hxpow2_allocator_<hxhash_node_base*, table_size_bits_> m_table_;
#	};
#
# hxpow2_allocator_ derives from hxallocator<hxhash_node_base*, ...> and holds
# the bucket array. The fourth template argument table_size_bits_ selects
# between two layouts. Dynamic layout is selected when table_size_bits_ == 0
# and the bucket count is m_capacity_. Otherwise the layout is static and the
# bucket count is 2^table_size_bits_. Each bucket slot is a hxhash_node_base*.
#
# WARNING: These fields are not guaranteed by the contract. If the client code
# does not use the provided base classes then the pretty printer may not be
# pretty.
#
# hxhash_node_base layout, inherited by every node_t_:
#   hxhash_node_base* hash_next;
#   hxhash_t          hash_val;
#
# hxhash_table_set_node additionally has:
#   key_t_                 m_key_;
#
# hxhash_table_map_node additionally has:
#   value_t_               m_value_;

def _hxhash_table_has_value_field(node_type: gdb.Type) -> bool:
	for field in node_type.fields():
		if field.is_base_class:
			if _hxhash_table_has_value_field(field.type):
				return True
		elif field.name == 'm_value_':
			return True
	return False

class hxhash_table_printer:
	def __init__(self, val: gdb.Value) -> None:
		self.val: gdb.Value = val
		self._summary: Optional[str] = None
		self._ok: bool = False
		self._bucket_addr: int
		self._bucket_count: int
		self._node_ptr_type: gdb.Type
		self._size: int

	def _parse(self) -> str:
		if self._summary is not None:
			return self._summary

		if self.val['m_size_'].is_optimized_out:
			self._summary = '<optimized out>'
			return self._summary

		size: int = int(self.val['m_size_'])
		table: gdb.Value = self.val['m_table_']

		bits_arg: gdb.Value = self.val.type.template_argument(3)
		type_name: str = re.sub(r'(\w+|\(anonymous namespace\))::', '', f'{self.val.type.strip_typedefs()}')
		bucket_addr: int
		bucket_count: int
		if bits_arg != 0:
			bucket_addr = int(table['m_data_'].address)
			bucket_count = 1 << int(bits_arg)
		else:
			bucket_addr = int(table['m_data_'])
			if bucket_addr == 0:
				self._summary = f'[0/0 buckets] {type_name}'
				return self._summary
			bucket_count = int(table['m_capacity_'])

		node_type: gdb.Type = self.val.type.template_argument(0)

		self._bucket_addr = bucket_addr
		self._bucket_count = bucket_count
		self._node_ptr_type = node_type.pointer()
		self._size = size
		self._ok = True

		self._summary = f'[{size}/{bucket_count} buckets] {type_name}'
		return self._summary

	def to_string(self) -> str:
		try:
			return self._parse()
		except Exception:
			error: str = f'{traceback.format_exc()}'
			return error.split('\n', 1)[1]

	def _iterator_nodes(self) -> Iterator[Tuple[int, gdb.Value]]:
		ptr_size: int = self._node_ptr_type.sizeof
		for b in range(self._bucket_count):
			slot_addr: int = self._bucket_addr + b * ptr_size
			node_ptr_ptr: gdb.Value = gdb.Value(slot_addr).cast(self._node_ptr_type.pointer())
			node_ptr: gdb.Value = node_ptr_ptr.dereference()
			while int(node_ptr) != 0:
				yield (int(node_ptr['hash_val']), node_ptr)
				# hash_next is declared with the hxhash_node_base type.
				node_ptr = node_ptr['hash_next'].cast(self._node_ptr_type)

	def _node_value(self, node_ptr: gdb.Value) -> Tuple[gdb.Value, ...]:
		node: gdb.Value = node_ptr.dereference()
		try:
			key: gdb.Value = node['m_key_']
			try:
				value: gdb.Value = node['m_value_']
				return key, value
			except Exception:
				return (key,)
		except Exception:
			return (node,)

	def children(self) -> Iterator[Tuple[str, gdb.Value]]:
		try:
			self._parse()
			if not self._ok:
				return
			table: gdb.Value = self.val['m_table_']
			allocator_type: gdb.Type = table.type.fields()[0].type
			if _hxhash_table_has_value_field(self.val.type.template_argument(0)):
				yield ('hxallocator label', gdb.Value('hxallocator'))
			yield ('hxallocator', table.cast(allocator_type))
			nodes: Iterator[Tuple[int, gdb.Value]] = self._iterator_nodes()
			if self._size <= HXHASH_TABLE_MAX_SORTED_NODES:
				nodes = sorted(nodes, key=lambda pair: pair[0])
			for hash_val, node_ptr in nodes:
				result: Tuple[gdb.Value, ...] = self._node_value(node_ptr)
				label: str = f'[{hash_val:08x}]'
				if len(result) == 2:
					yield (f'{label}.key', result[0])
					yield (f'{label}.value', result[1])
				else:
					yield (label, result[0])
		except Exception:
			return

	def display_hint(self) -> str:
		try:
			if _hxhash_table_has_value_field(self.val.type.template_argument(0)):
				return 'map'
		except Exception:
			pass
		return 'array'

def build_pretty_printer() -> gdb.printing.RegexpCollectionPrettyPrinter:
	pp = gdb.printing.RegexpCollectionPrettyPrinter('hxhash_table')
	pp.add_printer('hxhash_table', r'^(\w+::)*hxhash_table<.*>$', hxhash_table_printer)
	return pp

gdb.printing.register_pretty_printer(gdb.current_objfile(), build_pretty_printer(), replace=True)
