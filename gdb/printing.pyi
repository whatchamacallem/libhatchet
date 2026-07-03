# SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
# SPDX-License-Identifier: MIT
# This file is licensed under the terms of the LICENSE.md file.

# Type stubs for the parts of gdb.printing used by the pretty printers.

from typing import Any, Optional

class RegexpCollectionPrettyPrinter:
	def __init__(self, name: str) -> None: ...
	def add_printer(self, name: str, regexp: str, gen_printer: Any) -> None: ...

def register_pretty_printer(
	obj: Optional[Any], printer: Any, replace: bool = ...
) -> None: ...
