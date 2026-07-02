#pragma once
// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

#ifndef LIBHATCHET_VER
#error Internal. Do not include this file directly.
#endif

#ifndef HX_DOXYGEN_PARSER

namespace hxdetail_ {

template<typename node_t_, hxhash_t table_size_bits_>
class hxhash_table_internal_allocator_ : public hxallocator<node_t_*, 1 << table_size_bits_> {
public:
	hxhash_table_internal_allocator_(void) {
		::memset(this->data(), 0x00, sizeof(node_t_*) * static_cast<size_t>(this->capacity()));
	}
	constexpr hxhash_t get_hash_shift(void) const { return hxhash_bits - table_size_bits_; }
	void set_table_size_bits(hxhash_t bits) {
		hxassertmsg(bits == table_size_bits_, "fixed_capacity"); (void)bits;
	}
};

template<typename node_t_>
class hxhash_table_internal_allocator_<node_t_, hxallocator_dynamic_capacity>
	: public hxallocator<node_t_*, hxallocator_dynamic_capacity> {
public:
	hxhash_table_internal_allocator_(void) : m_hash_shift_(0) { }

	hxhash_t get_hash_shift(void) const {
		hxassertmsg(m_hash_shift_ != 0, "container_unallocated");
		return m_hash_shift_;
	}

	void set_table_size_bits(hxhash_t bits_) {
		hxassertmsg(m_hash_shift_ == 0 || bits_ == hxhash_bits - m_hash_shift_,
			"reallocation_disallowed");
		if(m_hash_shift_ == 0) {
			// bits_ < hxhash_bits: shifting a hxhash_t by its own width is UB.
			hxassertmsg(bits_ > 0 && bits_ < hxhash_bits, "bad_hash_bits %d", static_cast<int>(bits_));
			m_hash_shift_ = hxhash_bits - bits_;
			this->reserve_storage(static_cast<hxsize_t>(1) << bits_);
			::memset(this->data(), 0x00, sizeof(node_t_*) * static_cast<size_t>(this->capacity()));
		}
	}

private:
	hxhash_t m_hash_shift_;
};

} // hxdetail_
#endif // HX_DOXYGEN_PARSER
