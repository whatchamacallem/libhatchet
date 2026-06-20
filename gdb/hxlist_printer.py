# SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
# SPDX-License-Identifier: MIT
# This file is licensed under the terms of the LICENSE.md file.

import gdb # type: ignore
import gdb.printing # type: ignore
import traceback

# hxlist uses this layout:
#
#	class hxlist_node {
#		hxlist_node* m_list_prev_;
#		hxlist_node* m_list_next_;
#	};
#
#	template<typename node_t_, typename deleter_t_>
#	class hxlist {
#		size_t     m_size_;
#		hxlist_node m_head_;   // sentinel: m_head_.m_list_next_ is front,
#	};                         //           m_head_.m_list_prev_ is back.
#

class HxListPrinter:
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
			# Sentinel address — m_head_ is a hxlist_node embedded in the list.
			self._sentinel_addr = int(self.val['m_head_'].address)

			basename = f'{node_type}'.split(':')[-1]
			return f'[{size}] {basename}'
		except Exception:
			error = f'{traceback.format_exc()}'
			return error.split('\n', 1)[1]

	def _node_fields(self, node):
		"""Yield (name, value) for fields that are not part of hxlist_node."""
		hxlist_node_type = gdb.lookup_type('hxlist_node')
		base_field_names = {f.name for f in hxlist_node_type.fields()}
		for field in self._node_type.fields():
			# Skip the hxlist_node base class pseudo-field and its members.
			if field.is_base_class:
				continue
			if field.name in base_field_names:
				continue
			yield (field.name, node[field.name])

	def children(self):
		try:
			if not hasattr(self, '_size'):
				return
			hxlist_node_type = gdb.lookup_type('hxlist_node')
			node_ptr_type = hxlist_node_type.pointer()
			current_addr = int(self.val['m_head_']['m_list_next_'])
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
				current_addr = int(base_ptr['m_list_next_'])
		except Exception:
			return

	def display_hint(self):
		return 'array'

def build_pretty_printer():
	pp = gdb.printing.RegexpCollectionPrettyPrinter('hxlist_printer')
	pp.add_printer('hxlist', r'^hxlist<.*>$', HxListPrinter)
	return pp

gdb.printing.register_pretty_printer(gdb.current_objfile(), build_pretty_printer())
