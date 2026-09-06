#pragma once
// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

#ifndef LIBHATCHET_VER
#error Internal. Do not include this file directly.
#endif

#ifndef HX_DOXYGEN_PARSER

namespace hxdetail_ {

template<typename slot_t_, uint32_t c_table_size_bits_, bool zero_init_>
class hxpow2_allocator_
	: public hxallocator<slot_t_, static_cast<hxsize_t>(1) << c_table_size_bits_> {
public:
	hxinline hxpow2_allocator_(void) {
		hxif_constexpr(zero_init_) {
			::memset(this->data(), 0x00, sizeof(slot_t_) * static_cast<size_t>(this->capacity()));
		}
	}
	// GCOVR_EXCL_START
	// Sometimes constexpr inlines even in debug.
	hxinline constexpr uint32_t get_hash_shift_(void) const { return hxhash_bits - c_table_size_bits_; }
	hxinline constexpr uint32_t get_mask_(void) const { return (1u << c_table_size_bits_) - 1u; }
	// GCOVR_EXCL_STOP
	hxinline hxattr_flatten void set_size_bits_(uint32_t bits_, uint32_t cached_value_) {
		this->reserve_storage(static_cast<hxsize_t>(1) << bits_); // Just asserts.
		(void)cached_value_;
	}
};

template<typename slot_t_, bool zero_init_>
class hxpow2_allocator_<slot_t_, hxallocator_dynamic_capacity, zero_init_>
	: public hxallocator<slot_t_, hxallocator_dynamic_capacity> {
public:
	hxinline hxpow2_allocator_(void) : m_cached_value_(0) { }
	hxinline uint32_t get_hash_shift_(void) const { return m_cached_value_; }
	hxinline uint32_t get_mask_(void) const { return m_cached_value_; }
	hxinline hxattr_flatten void set_size_bits_(uint32_t bits_, uint32_t cached_value_) {
		hxassertmsg(bits_ - 1u < 30u, "bad_bits %u", bits_); // bits_ must be > 0.
		m_cached_value_ = cached_value_;
		this->reserve_storage(static_cast<hxsize_t>(1) << bits_);
		hxif_constexpr(zero_init_) {
			::memset(this->data(), 0x00, sizeof(slot_t_) * static_cast<size_t>(this->capacity()));
		}
	}

private:
	uint32_t m_cached_value_;
};

} // hxdetail_
#endif // HX_DOXYGEN_PARSER
