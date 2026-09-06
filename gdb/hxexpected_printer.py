# SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
# SPDX-License-Identifier: MIT
# This file is licensed under the terms of the LICENSE.md file.

import gdb
import gdb.printing  # pyright: ignore[reportMissingModuleSource]
import re
import traceback
from typing import Iterator, Tuple

# hxexpected uses this layout:
#
#	template<typename T_, hxsize_t fixed_capacity_>
#	class hxallocator {
#		// ...
#		alignas(T_) char m_data_[fixed_capacity_ * hxsizeof<T_>()];  // static, capacity==1
#	};
#	template<hxexpected_concept_ T_, hxunexpected_concept_ E_=bool>
#	class hxexpected : private hxallocator<T_, 1> {
#		// ...
#		E_ m_error_;  // false == holds a value, true == holds no value
#	};
#

class hxexpected_printer:
	def __init__(self, val: gdb.Value) -> None:
		self.val: gdb.Value = val

	def _has_value(self) -> bool:
		return not bool(self.val['m_error_'])

	def to_string(self) -> str:
		try:
			error: gdb.Value = self.val['m_error_']
			if error.is_optimized_out:
				return '<optimized out>'
			type_name: str = re.sub(r'(\w+|\(anonymous namespace\))::', '', f'{self.val.type.strip_typedefs()}')
			count: int = 0 if bool(error) else 1
			return f'[{count}] {type_name}'
		except Exception:
			error_text: str = f'{traceback.format_exc()}'
			return error_text.split('\n', 1)[1]

	def children(self) -> Iterator[Tuple[str, gdb.Value]]:
		try:
			error: gdb.Value = self.val['m_error_']
			if error.is_optimized_out:
				return
			allocator_type: gdb.Type = self.val.type.fields()[0].type
			yield ('hxallocator', self.val.cast(allocator_type))
			if self._has_value():
				value_type: gdb.Type = self.val.type.template_argument(0)
				data: gdb.Value = self.val.cast(allocator_type)['m_data_']
				yield ('value', data.address.cast(value_type.pointer()).dereference())
			yield ('error', error)
		except Exception:
			return

def build_pretty_printer() -> gdb.printing.RegexpCollectionPrettyPrinter:
	pp = gdb.printing.RegexpCollectionPrettyPrinter('hxexpected')
	pp.add_printer('hxexpected', r'^(\w+::)*hxexpected<.*>$', hxexpected_printer)
	return pp

gdb.printing.register_pretty_printer(gdb.current_objfile(), build_pretty_printer(), replace=True)
