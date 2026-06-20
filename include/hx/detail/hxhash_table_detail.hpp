#pragma once
// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

#ifndef LIBHATCHET_VER
#error Internal. Do not include this file directly.
#endif

#ifndef HX_DOXYGEN_PARSER

// hxhash_table internals. See hxhash_table.hpp instead. This is an
// hxhash_table-specific subclass of hxallocator. C++98 requires it to be
// declared outside `hxhash_table`. The table has a size of 2^table_size_bits_.

namespace hxdetail_ {

template<typename node_t_, hxhash_t table_size_bits_>
class hxhash_table_internal_allocator_ : public hxallocator<node_t_*, 1u << table_size_bits_> {
public:
	hxhash_table_internal_allocator_(void) {
		::memset(this->data(), 0x00, sizeof(node_t_*) * this->capacity());
	}
	hxhash_t get_table_size_bits(void) const { return table_size_bits_; }
	void set_table_size_bits(hxhash_t bits) {
		hxassertmsg(bits == table_size_bits_, "fixed_capacity"); (void)bits;
	}
};

template<typename node_t_>
class hxhash_table_internal_allocator_<node_t_, hxallocator_dynamic_capacity>
	: public hxallocator<node_t_*, hxallocator_dynamic_capacity> {
public:
	hxhash_table_internal_allocator_() : m_table_size_bits_(0u) { }

	hxhash_t get_table_size_bits(void) const {
		hxassertmsg(m_table_size_bits_ != 0u, "container_unallocated");
		return m_table_size_bits_;
	}

	void set_table_size_bits(hxhash_t bits_) {
		hxassertmsg(m_table_size_bits_ == 0u || bits_ == m_table_size_bits_, "reallocation_disallowed");
		if(m_table_size_bits_ == 0u) {
			// bits_ < hxhash_bits: shifting a hxhash_t by its own width is UB.
			hxassertmsg(bits_ > 0u && bits_ < hxhash_bits, "bad_hash_bits %d", static_cast<int>(bits_));
			m_table_size_bits_ = bits_;
			this->reserve_storage_(static_cast<size_t>(1u) << bits_);
			::memset(this->data(), 0x00, sizeof(node_t_*) * this->capacity());
		}
	}

private:
	hxhash_t m_table_size_bits_;
};

} // hxdetail_

#endif // HX_DOXYGEN_PARSER
