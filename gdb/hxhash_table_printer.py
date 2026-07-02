# SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
# SPDX-License-Identifier: MIT
# This file is licensed under the terms of the LICENSE.md file.

import gdb
import gdb.printing
import traceback
from typing import Iterator, Tuple, Union

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
#	class hxhash_table {
#		// ...
#		deleter_t_ m_deleter_;
#		hxsize_t m_size_;
#		hxhash_table_internal_allocator_<node_t_, table_size_bits_> m_table_;
#	};
#
# hxhash_table_internal_allocator_ derives from hxallocator<node_t_*, ...> and
# holds the bucket array. The fourth template argument table_size_bits_ selects
# between two layouts, and the bucket count is always 2^table_size_bits_. Static
# layout is selected when table_size_bits_ == 0. Each bucket slot is a node_t_*.
#
# WARNING: These fields are not guaranteed by the contract. If the client code
# does not use the provided base classes then the pretty printer may not be
# pretty.
#
# hxhash_table_set_node layout:
#   hxhash_table_set_node* m_hash_next_;
#   key_t_                 m_key_;
#   hxhash_t               m_hash_;
#
# hxhash_table_map_node additionally has:
#   value_t_               m_value_;
#

class hxhash_table_printer:
	def __init__(self, val: gdb.Value) -> None:
		self.val: gdb.Value = val
		self._bucket_addr: int
		self._bucket_count: int
		self._node_ptr_type: gdb.Type
		self._size: int

	def to_string(self) -> str:
		try:
			size: int = int(self.val['m_size_'])
			table: gdb.Value = self.val['m_table_']

			if self.val['m_size_'].is_optimized_out:
				return '<optimized out>'

			bits_arg: gdb.Value = self.val.type.template_argument(3)
			bucket_addr: int
			bucket_count: int
			if bits_arg != 0:
				bucket_addr = int(table['m_data_'].address)
				bucket_count = 1 << int(bits_arg)
			else:
				bucket_addr = int(table['m_data_'])
				if bucket_addr == 0:
					return '<unallocated>'
				bucket_count = int(table['m_capacity_'])

			node_type: gdb.Type = self.val.type.template_argument(0)
			ptr_type: gdb.Type = node_type.pointer()

			self._bucket_addr = bucket_addr
			self._bucket_count = bucket_count
			self._node_ptr_type = ptr_type
			self._size = size

			return f'[{size}/{bucket_count} buckets]'
		except Exception:
			error: str = f'{traceback.format_exc()}'
			return error.split('\n', 1)[1]

	def _iter_nodes(self) -> Iterator[Tuple[int, gdb.Value]]:
		ptr_size: int = self._node_ptr_type.sizeof
		idx: int = 0
		for b in range(self._bucket_count):
			slot_addr: int = self._bucket_addr + b * ptr_size
			node_ptr_ptr: gdb.Value = gdb.Value(slot_addr).cast(self._node_ptr_type.pointer())
			node_ptr: gdb.Value = node_ptr_ptr.dereference()
			while int(node_ptr) != 0:
				yield (idx, node_ptr)
				idx += 1
				node_ptr = node_ptr['m_hash_next_']

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

	def children(self) -> Iterator[Tuple[str, Union[gdb.Value, str]]]:
		try:
			if not hasattr(self, '_size'):
				return
			for idx, node_ptr in self._iter_nodes():
				result: Tuple[gdb.Value, ...] = self._node_value(node_ptr)
				if len(result) == 2:
					yield (f'[{idx}] key', result[0])
					yield (f'[{idx}] value', result[1])
				else:
					yield (f'[{idx}]', result[0])
		except Exception as e:
			yield ('<error>', str(e))

	def display_hint(self) -> str:
		return 'array'

def build_pretty_printer() -> gdb.printing.RegexpCollectionPrettyPrinter:
	pp = gdb.printing.RegexpCollectionPrettyPrinter('hxhash_table')
	pp.add_printer('hxhash_table', r'hxhash_table<', hxhash_table_printer)
	return pp

gdb.printing.register_pretty_printer(gdb.current_objfile(), build_pretty_printer(), replace=True)
