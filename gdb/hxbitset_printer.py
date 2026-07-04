# SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
# SPDX-License-Identifier: MIT
# This file is licensed under the terms of the LICENSE.md file.

import gdb
import gdb.printing  # pyright: ignore[reportMissingModuleSource]
import traceback
from typing import Iterator, Optional, Tuple

# hxbitset uses this layout:
#
#	template<size_t bit_count_>
#	class hxbitset {
#		// ...
#		size_t m_data_[s_words_];  // s_words_ = ceil(bit_count_ / (sizeof(size_t)*8))
#	};
#

_MAX_BITS: int = 128

class hxbitset_printer:
	def __init__(self, val: gdb.Value) -> None:
		self.val: gdb.Value = val

	def to_string(self) -> str:
		try:
			bit_count: int = int(self.val.type.template_argument(0))

			data_field: gdb.Value = self.val['m_data_']
			if data_field.is_optimized_out:
				return '<optimized out>'

			word_bits: int = data_field.type.target().sizeof * 8

			displayed_bits: int = min(bit_count, _MAX_BITS)
			chars: list[str] = []
			for bit in range(bit_count - 1, bit_count - 1 - displayed_bits, -1):
				word_idx: int = bit // word_bits
				bit_idx: int = bit % word_bits
				word: int = int(data_field[word_idx])
				chars.append('1' if (word >> bit_idx) & 1 else '0')

			result: str = ''.join(chars)
			if bit_count > _MAX_BITS:
				result += '...'
			return result
		except Exception:
			error: str = f'{traceback.format_exc()}'
			return error.split('\n', 1)[1]

	def children(self) -> Iterator[Tuple[str, gdb.Value]]:
		try:
			bit_count: int = int(self.val.type.template_argument(0))
			data_field: gdb.Value = self.val['m_data_']
			word_bits: int = data_field.type.target().sizeof * 8
			word_count: int = (bit_count + word_bits - 1) // word_bits
			for i in range(word_count):
				yield (f'm_data_[{i}]', data_field[i])
		except Exception:
			return

	def display_hint(self) -> Optional[str]:
		return None

def build_pretty_printer() -> gdb.printing.RegexpCollectionPrettyPrinter:
	pp = gdb.printing.RegexpCollectionPrettyPrinter('hxbitset')
	pp.add_printer('hxbitset', r'^(\w+::)*hxbitset<.*>$', hxbitset_printer)
	return pp

gdb.printing.register_pretty_printer(gdb.current_objfile(), build_pretty_printer(), replace=True)
