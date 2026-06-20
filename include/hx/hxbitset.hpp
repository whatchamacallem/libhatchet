#pragma once
// SPDX-FileCopyrightText: © 2025 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

/// \file hxbitset.hpp - A fixed-size bitset stored as an array of `size_t` words
/// with no heap allocation.

#include "libhatchet.h"

// HX_USE_MODULE allows including macros in addition to the hx module.
#if HX_USE_MODULE
#error Header does not provide macros only.
#endif

HX_NS_BEGIN_

/// hxbitset - A fixed-size bitset stored as an array of `size_t` words with no
/// heap allocation.
/// - `bit_count` : The number of bits in the `hxbitset`. Must be greater than zero. Also known as the popcount.
template<size_t bit_count_>
class hxbitset {
public:
	/// Constructs a zero-initialized `hxbitset`.
	hxconstexpr hxbitset(void) { this->reset(); }

	/// Constructs a `hxbitset` from a single `size_t` value. Only valid when
	/// `bit_count` equals the number of bits in `size_t`.
	/// - `val` : The value to initialize the bitset with.
	hxconstexpr explicit hxbitset(size_t val_);

	/// Constructs a `hxbitset` as a copy of `x`.
	/// - `x` : The `hxbitset` to copy from.
	hxconstexpr hxbitset(const hxbitset& x_);

	/// Assigns the bits of `x` to this `hxbitset`. Asserts that `&x` is not `this`.
	/// - `x` : The `hxbitset` to copy from.
	hxconstexpr void operator=(const hxbitset& x_);

	/// Returns the value of the bit at position `pos`. Asserts that `pos` is in
	/// range.
	/// - `pos` : Bit index. Must be less than `bit_count`.
	hxattr_nodiscard hxconstexpr bool operator[](size_t pos_) const;

	/// Applies bitwise AND with `x` in place. Asserts that `&x` is not `this`.
	/// - `x` : The `hxbitset` to AND with.
	hxconstexpr hxbitset& operator&=(const hxbitset& x_);

	/// Applies bitwise OR with `x` in place. Asserts that `&x` is not `this`.
	/// - `x` : The `hxbitset` to OR with.
	hxconstexpr hxbitset& operator|=(const hxbitset& x_);

	/// Applies bitwise XOR with `x` in place. Asserts that `&x` is not `this`.
	/// - `x` : The `hxbitset` to XOR with.
	hxconstexpr hxbitset& operator^=(const hxbitset& x_);

	/// Shifts all bits left by `count` positions, filling vacated bits with 0.
	/// - `count` : Number of positions to shift left.
	hxconstexpr hxbitset& operator<<=(size_t count_);

	/// Shifts all bits right by `count` positions, filling vacated bits with 0.
	/// - `count` : Number of positions to shift right.
	hxconstexpr hxbitset& operator>>=(size_t count_);

	/// Returns `true` if all bits compare equal to those of `x`. Asserts that
	/// `&x` is not `this`.
	/// - `x` : The `hxbitset` to compare with.
	hxattr_nodiscard hxconstexpr bool operator==(const hxbitset& x_) const;

#if HX_CPLUSPLUS < 202002L // C++20 defaults != from ==.
	/// Returns `true` if any bits differ from those of `x`.
	/// - `x` : The `hxbitset` to compare with.
	hxattr_nodiscard bool operator!=(const hxbitset& x_) const { return !(*this == x_); }
#endif

	/// Returns `true` if all bits are set.
	hxattr_nodiscard hxconstexpr bool all(void) const;

	/// Returns `true` if at least one bit is set.
	hxattr_nodiscard hxconstexpr bool any(void) const;

	/// Returns the size of the underlying storage in bytes.
	hxattr_nodiscard static constexpr size_t bytes(void) { return s_words_ * sizeof(size_t); }

	/// Returns a pointer to the underlying word storage.
	hxattr_nodiscard hxconstexpr size_t* data(void) { return m_data_; }

	/// Returns a const pointer to the underlying word storage.
	hxattr_nodiscard hxconstexpr const size_t* data(void) const { return m_data_; }

	/// Flips all bits.
	hxconstexpr hxbitset& flip(void);

	/// Flips the bit at position `pos`. Asserts that `pos` is in range.
	/// - `pos` : Bit index that must be less than `bit_count`.
	hxconstexpr hxbitset& flip(size_t pos_);

	/// Copies `len` bytes from `src` into the `hxbitset` storage. Asserts that
	/// `len` does not exceed `bytes()`. Missing bytes or trailing bits beyond
	/// `bit_count` are masked to zero after the copy.
	/// - `src` : Pointer to the source data.
	/// - `len` : Number of bytes to copy. Must not exceed `bytes()`.
	void load(const void* src_, size_t len_);

	/// Returns `true` if no bits are set.
	hxattr_nodiscard hxconstexpr bool none(void) const { return !this->any(); }

	/// Clears all bits to 0.
	hxconstexpr hxbitset& reset(void);

	/// Clears the bit at position `pos`. Asserts that `pos` is in range.
	/// - `pos` : Bit index that must be less than `bit_count`.
	hxconstexpr hxbitset& reset(size_t pos_);

	/// Sets all bits to 1.
	hxconstexpr hxbitset& set(void);

	/// Sets or clears the bit at position `pos`. Asserts that `pos` is in
	/// range.
	/// - `pos` : Bit index that must be less than `bit_count`.
	/// - `value` : The value to assign, defaults to `true`.
	hxconstexpr hxbitset& set(size_t pos_, bool value_=true);

	/// Returns the number of bits.
	hxattr_nodiscard static constexpr size_t size(void) { return bit_count_; }

	/// Returns the value of the bit at position `pos`. Asserts that `pos` is in
	/// range.
	/// - `pos` : Bit index must be less than `bit_count`.
	hxattr_nodiscard hxconstexpr bool test(size_t pos_) const;

private:
	static_assert(bit_count_ > 0, "hxbitset requires bit_count > 0.");
	static hxinline_constexpr size_t s_bits_per_word_ = sizeof(size_t) * 8u;
	static hxinline_constexpr size_t s_log2_bits_per_word_ = (s_bits_per_word_ > 32u) ? 6u : 5u;
	static hxinline_constexpr size_t s_bit_mask_ = s_bits_per_word_ - 1u;
	static hxinline_constexpr size_t s_words_ = (bit_count_ + s_bit_mask_) >> s_log2_bits_per_word_;
	static hxinline_constexpr size_t s_trailing_bits_ = bit_count_ & s_bit_mask_;
	static hxinline_constexpr size_t s_trailing_mask_ = s_trailing_bits_ != 0u
		? (static_cast<size_t>(1u) << s_trailing_bits_) - 1u
		: ~static_cast<size_t>(0u);

	size_t m_data_[s_words_];
};

#include "detail/hxbitset.inl"

HX_NS_END_
