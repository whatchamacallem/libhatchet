# SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
# SPDX-License-Identifier: MIT
# This file is licensed under the terms of the LICENSE.md file.

import gdb
import gdb.printing  # pyright: ignore[reportMissingModuleSource]
import re
import traceback
from typing import Iterator, Tuple

# hxptr uses this layout:
#
#	template<typename T_, typename deleter_t_=hxdefault_delete>
#	class hxptr : private deleter_t_ {
#		// ...
#		T_* m_ptr_;
#	};
#

class hxptr_printer:
	def __init__(self, val: gdb.Value) -> None:
		self.val: gdb.Value = val

	def to_string(self) -> str:
		try:
			ptr: gdb.Value = self.val['m_ptr_']
			if ptr.is_optimized_out:
				return '<optimized out>'
			type_name: str = re.sub(r'(\w+|\(anonymous namespace\))::', '', f'{self.val.type.strip_typedefs()}')
			if int(ptr) == 0:
				return f'[0] {type_name} = null'
			return f'[1] {type_name}'
		except Exception:
			error: str = f'{traceback.format_exc()}'
			return error.split('\n', 1)[1]

	def children(self) -> Iterator[Tuple[str, gdb.Value]]:
		try:
			ptr: gdb.Value = self.val['m_ptr_']
			if ptr.is_optimized_out or int(ptr) == 0:
				return
			yield ('value', ptr.dereference())
		except Exception:
			return

def build_pretty_printer() -> gdb.printing.RegexpCollectionPrettyPrinter:
	pp = gdb.printing.RegexpCollectionPrettyPrinter('hxptr')
	pp.add_printer('hxptr', r'^(\w+::)*hxptr<.*>$', hxptr_printer)
	return pp

gdb.printing.register_pretty_printer(gdb.current_objfile(), build_pretty_printer(), replace=True)
