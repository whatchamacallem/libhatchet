# SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
# SPDX-License-Identifier: MIT
# This file is licensed under the terms of the LICENSE.md file.

import gdb # type: ignore
import gdb.printing # type: ignore
import traceback

# hxbitset uses this layout:
#
#	template<size_t bit_count_>
#	class hxbitset {
#		// ...
#		size_t m_data_[s_words_];  // s_words_ = ceil(bit_count_ / (sizeof(size_t)*8))
#	};
#

_MAX_BITS = 128

class HxBitsetPrinter:
	def __init__(self, val):
		self.val = val

	def to_string(self):
		try:
			bit_count = int(self.val.type.template_argument(0))
			size_t_type = gdb.lookup_type('size_t')
			word_bits = size_t_type.sizeof * 8

			data_field = self.val['m_data_']
			if data_field.is_optimized_out:
				return '<optimized out>'

			displayed_bits = min(bit_count, _MAX_BITS)
			chars = []
			# std::bitset::to_string emits bit (bit_count-1) first down to bit 0.
			for bit in range(bit_count - 1, bit_count - 1 - displayed_bits, -1):
				word_idx = bit // word_bits
				bit_idx = bit % word_bits
				word = int(data_field[word_idx])
				chars.append('1' if (word >> bit_idx) & 1 else '0')

			result = ''.join(chars)
			if bit_count > _MAX_BITS:
				result += '...'
			return result
		except Exception:
			error = f'{traceback.format_exc()}'
			return error.split('\n', 1)[1]

	def children(self):
		try:
			size_t_type = gdb.lookup_type('size_t')
			word_bits = size_t_type.sizeof * 8
			bit_count = int(self.val.type.template_argument(0))
			word_count = (bit_count + word_bits - 1) // word_bits
			data_field = self.val['m_data_']
			for i in range(word_count):
				yield (f'm_data_[{i}]', data_field[i])
		except Exception:
			return

	def display_hint(self):
		return None

def build_pretty_printer():
	pp = gdb.printing.RegexpCollectionPrettyPrinter('hxbitset_printer')
	pp.add_printer('hxbitset', r'hxbitset<', HxBitsetPrinter)
	return pp

gdb.printing.register_pretty_printer(gdb.current_objfile(), build_pretty_printer(), replace=True)
