# SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
# SPDX-License-Identifier: MIT
# This file is licensed under the terms of the LICENSE.md file.

import gdb
import gdb.printing  # pyright: ignore[reportMissingModuleSource]
import traceback

class hxtest_object_printer:
	def __init__(self, val: gdb.Value) -> None:
		self.val: gdb.Value = val

	def to_string(self) -> str:
		try:
			state: str = str(self.val['m_state']).rsplit('::', 1)[-1]
			value: int = int(self.val['m_value'])
			return f'{{ state={state}, value={value} }}'
		except Exception:
			error: str = f'{traceback.format_exc()}'
			return error.split('\n', 1)[1]

def build_pretty_printer() -> gdb.printing.RegexpCollectionPrettyPrinter:
	printer = gdb.printing.RegexpCollectionPrettyPrinter('hxtest_object')
	printer.add_printer('hxtest_object', r'^(\w+::)*hxtest_object$', hxtest_object_printer)
	return printer

gdb.printing.register_pretty_printer(
	gdb.current_objfile(), build_pretty_printer(), replace=True)
