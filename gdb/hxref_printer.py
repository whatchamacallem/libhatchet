# SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
# SPDX-License-Identifier: MIT
# This file is licensed under the terms of the LICENSE.md file.

import gdb
import gdb.printing  # pyright: ignore[reportMissingModuleSource]
import re
import traceback
from typing import Iterator, Tuple

# hxref uses this layout:
#
#	template<typename T_>
#	class hxref {
#		// ...
#		T_* m_value_;
#	};
#

class hxref_printer:
	def __init__(self, val: gdb.Value) -> None:
		self.val: gdb.Value = val

	def to_string(self) -> str:
		try:
			value: gdb.Value = self.val['m_value_']
			if value.is_optimized_out:
				return '<optimized out>'
			type_name: str = re.sub(r'(\w+|\(anonymous namespace\))::', '', f'{self.val.type.strip_typedefs()}')
			if int(value) == 0:
				return f'[0] {type_name} = null'
			return f'[1] {type_name}'
		except Exception:
			error: str = f'{traceback.format_exc()}'
			return error.split('\n', 1)[1]

	def children(self) -> Iterator[Tuple[str, gdb.Value]]:
		try:
			value: gdb.Value = self.val['m_value_']
			if value.is_optimized_out or int(value) == 0:
				return
			yield ('value', value.dereference())
		except Exception:
			return

def build_pretty_printer() -> gdb.printing.RegexpCollectionPrettyPrinter:
	pp = gdb.printing.RegexpCollectionPrettyPrinter('hxref')
	pp.add_printer('hxref', r'^(\w+::)*hxref<.*>$', hxref_printer)
	return pp

gdb.printing.register_pretty_printer(gdb.current_objfile(), build_pretty_printer(), replace=True)
