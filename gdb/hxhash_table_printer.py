# SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
# SPDX-License-Identifier: MIT
# This file is licensed under the terms of the LICENSE.md file.

import gdb # type: ignore
import gdb.printing # type: ignore
import traceback

# hxhash_table uses this layout:
#
#	template<node_t_, hxhash_t table_size_bits_, bool multi_t_, typename deleter_t_>
#	class hxhash_table {
#		size_t m_size_;
#		hxhash_table_internal_allocator_<node_t_, table_size_bits_> m_table_;
#	};
#
# hxhash_table_internal_allocator_ is itself an hxallocator<node_t_*, 1 << bits>.
# Each bucket slot is a node_t_* (singly-linked list head via hash_next()).
# Dynamic tables additionally have an m_table_size_bits_ field.
#
# NOTA BENE: These fields are not guaranteed by the contract. If the client code
# does not use the provided base classes then the pretty printer will not work.
#
# hxhash_table_set_node layout:
#   node_t_* m_hash_next_;
#   key_t_   m_key_;
#   hxhash_t m_hash_;
#
# hxhash_table_map_node additionally has:
#   value_t_ m_value_;
#

class HxHashTablePrinter:
	def __init__(self, val):
		self.val = val

	def to_string(self):
		try:
			size = int(self.val['m_size_'])
			table = self.val['m_table_']

			if self.val['m_size_'].is_optimized_out:
				return '<optimized out>'

			# Resolve bucket array pointer. hxhash_table_internal_allocator_
			# inherits from hxallocator. For static capacity: m_data_ is an
			# array. For dynamic: m_data_ is a pointer.
			bits_arg = self.val.type.template_argument(1)
			if bits_arg != 0:
				bucket_addr = int(table['m_data_'].address)
				bucket_count = 1 << int(bits_arg)
			else:
				# Dynamic: read pointer value and m_table_size_bits_.
				bucket_addr = int(table['m_data_'])
				if bucket_addr == 0:
					return '<unallocated>'
				bits = int(table['m_table_size_bits_'])
				bucket_count = 1 << bits

			node_type = self.val.type.template_argument(0)
			ptr_type = node_type.pointer()

			self._bucket_addr = bucket_addr
			self._bucket_count = bucket_count
			self._node_ptr_type = ptr_type
			self._size = size

			return f'[{size}/{bucket_count} buckets]'
		except Exception:
			error = f'{traceback.format_exc()}'
			return error.split('\n', 1)[1]

	def _iter_nodes(self):
		"""Yield (index, node_ptr GDB value) for every node across all buckets."""
		ptr_size = gdb.lookup_type('void').pointer().sizeof
		idx = 0
		for b in range(self._bucket_count):
			slot_addr = self._bucket_addr + b * ptr_size
			node_ptr_ptr = gdb.Value(slot_addr).cast(self._node_ptr_type.pointer())
			node_ptr = node_ptr_ptr.dereference()
			while int(node_ptr) != 0:
				yield (idx, node_ptr)
				idx += 1
				next_field = node_ptr['m_hash_next_']
				node_ptr = next_field

	def _node_value(self, node_ptr):
		"""Return display items for a node: key for set nodes, key+value for map
		nodes, or the full dereferenced node as fallback."""
		node = node_ptr.dereference()
		try:
			key = node['m_key_']
			try:
				value = node['m_value_']
				return key, value
			except Exception:
				return (key,)
		except Exception:
			return (node,)

	def children(self):
		try:
			if not hasattr(self, '_size'):
				return
			for idx, node_ptr in self._iter_nodes():
				result = self._node_value(node_ptr)
				if len(result) == 2:
					yield (f'[{idx}] key', result[0])
					yield (f'[{idx}] value', result[1])
				else:
					yield (f'[{idx}]', result[0])
		except Exception as e:
			yield ('<error>', str(e))

	def display_hint(self):
		return 'array'

def build_pretty_printer():
	pp = gdb.printing.RegexpCollectionPrettyPrinter('hxhash_table_printer')
	pp.add_printer('hxhash_table', r'^hxhash_table<.*>$', HxHashTablePrinter)
	return pp

gdb.printing.register_pretty_printer(gdb.current_objfile(), build_pretty_printer())
