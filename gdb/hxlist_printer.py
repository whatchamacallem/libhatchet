# SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
# SPDX-License-Identifier: MIT
# This file is licensed under the terms of the LICENSE.md file.

import gdb # type: ignore
import gdb.printing # type: ignore
import traceback

# hxlist uses this layout:
#
#	class hxlist_node {
#		intptr_t m_list_link_;  // prev XOR next
#	};
#
#	template<typename node_t_, typename deleter_t_>
#	class hxlist {
#		hxsize_t     m_size_;
#		hxlist_node  m_sentinel_;  // m_sentinel_.m_list_link_ == tail XOR front
#		hxlist_node* m_tail_;      // points to last node, or &m_sentinel_ when empty
#	};
#

class hxlistPrinter:
	def __init__(self, val):
		self.val = val

	def to_string(self):
		try:
			if self.val['m_size_'].is_optimized_out:
				return '<optimized out>'

			size = int(self.val['m_size_'])
			node_type = self.val.type.template_argument(0)

			self._size = size
			self._node_type = node_type
			self._sentinel_addr = int(self.val['m_sentinel_'].address)
			self._tail_addr = int(self.val['m_tail_'])
			self._sentinel_link = int(self.val['m_sentinel_']['m_list_link_'])

			# Get the base class type and its field names directly from the node
			# type, avoiding gdb.lookup_type() which is unreliable when the same
			# anonymous-namespace name exists in multiple TUs.
			self._base_type = None
			self._base_field_names = set()
			for field in node_type.fields():
				if field.is_base_class:
					self._base_type = field.type
					for bf in field.type.fields():
						self._base_field_names.add(bf.name)
					break

			basename = f'{node_type}'.split(':')[-1]
			return f'[{size}] {basename}'
		except Exception:
			error = f'{traceback.format_exc()}'
			return error.split('\n', 1)[1]

	def _node_fields(self, node):
		for field in self._node_type.fields():
			if field.is_base_class:
				continue
			if field.name in self._base_field_names:
				continue
			yield (field.name, node[field.name])

	def children(self):
		try:
			if not hasattr(self, '_size') or self._base_type is None:
				return
			node_ptr_type = self._base_type.pointer()
			# front = tail XOR m_sentinel_.m_list_link_
			current_addr = self._tail_addr ^ self._sentinel_link
			prev_addr = self._sentinel_addr
			for idx in range(self._size):
				if current_addr == self._sentinel_addr:
					break
				node_ptr = gdb.Value(current_addr).cast(self._node_type.pointer())
				node = node_ptr.dereference()
				fields = list(self._node_fields(node))
				if len(fields) == 1:
					yield (f'[{idx}]', fields[0][1])
				else:
					for name, val in fields:
						yield (f'[{idx}] {name}', val)
				base_ptr = gdb.Value(current_addr).cast(node_ptr_type)
				link = int(base_ptr['m_list_link_'])
				next_addr = prev_addr ^ link
				prev_addr = current_addr
				current_addr = next_addr
		except Exception:
			return

	def display_hint(self):
		return 'array'

def build_pretty_printer():
	pp = gdb.printing.RegexpCollectionPrettyPrinter('hxlist_printer')
	pp.add_printer('hxlist', r'hxlist<', hxlistPrinter)
	return pp

gdb.printing.register_pretty_printer(gdb.current_objfile(), build_pretty_printer(), replace=True)
