#pragma once
// SPDX-FileCopyrightText: © 2025 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

#include "libhatchet.h"

/// A fixed-size bitset stored as an array of `size_t` words with no heap
/// allocation.
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

#if HX_CPLUSPLUS < 202002L
	/// Returns `true` if any bits differ from those of `x`. Only defined when
	/// `HX_CPLUSPLUS < 202002L`.
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
	/// `len` does not exceed `bytes()`. Trailing bits beyond `bit_count` are masked
	/// to zero after the copy.
	/// - `src` : Pointer to the source data.
	/// - `len` : Number of bytes to copy. Must not exceed `bytes()`.
	void load(const char* src_, size_t len_);

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
	static_assert(bit_count_ > 0u, "hxbitset requires bit_count_ > 0.");
	static constexpr size_t s_bits_per_word_ = sizeof(size_t) * 8u;
	static constexpr size_t s_words_ = (bit_count_ + s_bits_per_word_ - 1u) / s_bits_per_word_;
	static constexpr size_t s_trailing_bits_ = bit_count_ % s_bits_per_word_;
	static constexpr size_t s_trailing_mask_ = s_trailing_bits_ != 0u
		? (static_cast<size_t>(1u) << s_trailing_bits_) - 1u
		: ~static_cast<size_t>(0u);

	hxconstexpr void assert_no_trailing_bits_(void) const;

	size_t m_data_[s_words_];
};

// ----------------------------------------------------------------------------

template<size_t bit_count_>
inline hxconstexpr hxbitset<bit_count_>::hxbitset(size_t val_) {
	static_assert(bit_count_ == s_bits_per_word_,
		"hxbitset(size_t) requires bit_count_ == sizeof(size_t) * 8.");
	m_data_[0] = val_;
}

template<size_t bit_count_>
inline hxconstexpr hxbitset<bit_count_>::hxbitset(const hxbitset& x_) {
	const size_t* hxrestrict src_ = x_.m_data_;
	size_t* hxrestrict dst_ = m_data_;
	const size_t* const end_ = dst_ + s_words_;
	while(dst_ != end_) { *dst_++ = *src_++; }
}

template<size_t bit_count_>
inline hxconstexpr void hxbitset<bit_count_>::operator=(const hxbitset& x_) {
	hxassertmsg(static_cast<const void*>(this) != static_cast<const void*>(&x_),
		"invalid_reference Assignment to self.");
	const size_t* hxrestrict src_ = x_.m_data_;
	size_t* hxrestrict dst_ = m_data_;
	const size_t* const end_ = dst_ + s_words_;
	while(dst_ != end_) { *dst_++ = *src_++; }
}

template<size_t bit_count_>
inline hxconstexpr bool hxbitset<bit_count_>::operator[](size_t pos_) const {
	hxassertmsg(pos_ < bit_count_, "invalid_index %zu", pos_);
	return (m_data_[pos_ / s_bits_per_word_]
		& (static_cast<size_t>(1u) << (pos_ % s_bits_per_word_))) != 0u;
}

template<size_t bit_count_>
inline hxconstexpr hxbitset<bit_count_>& hxbitset<bit_count_>::operator&=(const hxbitset& x_) {
	hxassertmsg(static_cast<const void*>(this) != static_cast<const void*>(&x_),
		"invalid_reference Operation with self.");
	size_t* hxrestrict dst_ = m_data_;
	const size_t* hxrestrict src_ = x_.m_data_;
	const size_t* const end_ = dst_ + s_words_;
	while(dst_ != end_) { *dst_++ &= *src_++; }
	return *this;
}

template<size_t bit_count_>
inline hxconstexpr hxbitset<bit_count_>& hxbitset<bit_count_>::operator|=(const hxbitset& x_) {
	hxassertmsg(static_cast<const void*>(this) != static_cast<const void*>(&x_),
		"invalid_reference Operation with self.");
	size_t* hxrestrict dst_ = m_data_;
	const size_t* hxrestrict src_ = x_.m_data_;
	const size_t* const end_ = dst_ + s_words_;
	while(dst_ != end_) { *dst_++ |= *src_++; }
	assert_no_trailing_bits_();
	return *this;
}

template<size_t bit_count_>
inline hxconstexpr hxbitset<bit_count_>& hxbitset<bit_count_>::operator^=(const hxbitset& x_) {
	hxassertmsg(static_cast<const void*>(this) != static_cast<const void*>(&x_),
		"invalid_reference Operation with self.");
	size_t* hxrestrict dst_ = m_data_;
	const size_t* hxrestrict src_ = x_.m_data_;
	const size_t* const end_ = dst_ + s_words_;
	while(dst_ != end_) { *dst_++ ^= *src_++; }
	assert_no_trailing_bits_();
	return *this;
}

template<size_t bit_count_>
inline hxconstexpr hxbitset<bit_count_>& hxbitset<bit_count_>::operator<<=(size_t count_) {
	if(count_ == 0u) { return *this; }
	const size_t word_shift_ = count_ / s_bits_per_word_;
	const size_t bit_shift_ = count_ % s_bits_per_word_;
	if(bit_shift_ == 0u) {
		for(size_t i_ = s_words_; i_-- != 0u; ) {
			m_data_[i_] = (i_ >= word_shift_) ? m_data_[i_ - word_shift_] : 0u;
		}
	} else {
		for(size_t i_ = s_words_; i_-- != 0u; ) {
			const size_t lo_ = (i_ >= word_shift_)
				? (m_data_[i_ - word_shift_] << bit_shift_) : 0u;
			const size_t hi_ = (i_ > word_shift_)
				? (m_data_[i_ - word_shift_ - 1u] >> (s_bits_per_word_ - bit_shift_)) : 0u;
			m_data_[i_] = lo_ | hi_;
		}
	}
	m_data_[s_words_ - 1u] &= s_trailing_mask_;
	assert_no_trailing_bits_();
	return *this;
}

template<size_t bit_count_>
inline hxconstexpr hxbitset<bit_count_>& hxbitset<bit_count_>::operator>>=(size_t count_) {
	if(count_ == 0u) { return *this; }
	const size_t word_shift_ = count_ / s_bits_per_word_;
	const size_t bit_shift_ = count_ % s_bits_per_word_;
	if(bit_shift_ == 0u) {
		for(size_t i_ = 0u; i_ < s_words_; ++i_) {
			m_data_[i_] = ((i_ + word_shift_) < s_words_) ? m_data_[i_ + word_shift_] : 0u;
		}
	} else {
		for(size_t i_ = 0u; i_ < s_words_; ++i_) {
			const size_t lo_ = ((i_ + word_shift_) < s_words_)
				? (m_data_[i_ + word_shift_] >> bit_shift_) : 0u;
			const size_t hi_ = ((i_ + word_shift_ + 1u) < s_words_)
				? (m_data_[i_ + word_shift_ + 1u] << (s_bits_per_word_ - bit_shift_)) : 0u;
			m_data_[i_] = lo_ | hi_;
		}
	}
	assert_no_trailing_bits_();
	return *this;
}

template<size_t bit_count_>
inline hxconstexpr bool hxbitset<bit_count_>::operator==(const hxbitset& x_) const {
	hxassertmsg(static_cast<const void*>(this) != static_cast<const void*>(&x_),
		"invalid_reference Operation with self.");
	const size_t* hxrestrict dst_ = m_data_;
	const size_t* hxrestrict src_ = x_.m_data_;
	const size_t* const end_ = dst_ + s_words_;
	while(dst_ != end_) { if(*dst_++ != *src_++) { return false; } }
	return true;
}

template<size_t bit_count_>
inline hxconstexpr bool hxbitset<bit_count_>::all(void) const {
	assert_no_trailing_bits_();
	const size_t* hxrestrict dst_ = m_data_;
	const size_t* const end_ = dst_ + (s_words_ - 1u);
	while(dst_ != end_) {
		if(*dst_++ != ~static_cast<size_t>(0u)) { return false; }
	}
	return m_data_[s_words_ - 1u] == s_trailing_mask_;
}

template<size_t bit_count_>
inline hxconstexpr bool hxbitset<bit_count_>::any(void) const {
	assert_no_trailing_bits_();
	const size_t* hxrestrict dst_ = m_data_;
	const size_t* const end_ = dst_ + s_words_;
	while(dst_ != end_) {
		if(*dst_++ != 0u) { return true; }
	}
	return false;
}

template<size_t bit_count_>
inline hxconstexpr hxbitset<bit_count_>& hxbitset<bit_count_>::flip(void) {
	size_t* hxrestrict dst_ = m_data_;
	const size_t* const end_ = dst_ + s_words_;
	while(dst_ != end_) { *dst_++ ^= ~static_cast<size_t>(0u); }
	m_data_[s_words_ - 1u] &= s_trailing_mask_;
	return *this;
}

template<size_t bit_count_>
inline hxconstexpr hxbitset<bit_count_>& hxbitset<bit_count_>::flip(size_t pos_) {
	hxassertmsg(pos_ < bit_count_, "invalid_index %zu", pos_);
	m_data_[pos_ / s_bits_per_word_] ^= (static_cast<size_t>(1u) << (pos_ % s_bits_per_word_));
	return *this;
}

template<size_t bit_count_>
inline void hxbitset<bit_count_>::load(const char* src_, size_t len_) {
	hxassertmsg(len_ <= bytes(), "overflow_load %zu", len_);
	::memcpy(m_data_, src_, len_); // NOLINT
	m_data_[s_words_ - 1u] &= s_trailing_mask_;
}

template<size_t bit_count_>
inline hxconstexpr hxbitset<bit_count_>& hxbitset<bit_count_>::reset(void) {
	size_t* hxrestrict dst_ = m_data_;
	const size_t* const end_ = dst_ + s_words_;
	while(dst_ != end_) { *dst_++ = 0u; }
	return *this;
}

template<size_t bit_count_>
inline hxconstexpr hxbitset<bit_count_>& hxbitset<bit_count_>::reset(size_t pos_) {
	hxassertmsg(pos_ < bit_count_, "invalid_index %zu", pos_);
	m_data_[pos_ / s_bits_per_word_] &= ~(static_cast<size_t>(1u) << (pos_ % s_bits_per_word_));
	return *this;
}

template<size_t bit_count_>
inline hxconstexpr hxbitset<bit_count_>& hxbitset<bit_count_>::set(void) {
	size_t* hxrestrict dst_ = m_data_;
	const size_t* const end_ = dst_ + s_words_;
	while(dst_ != end_) { *dst_++ = ~static_cast<size_t>(0u); }
	m_data_[s_words_ - 1u] &= s_trailing_mask_;
	return *this;
}

template<size_t bit_count_>
inline hxconstexpr hxbitset<bit_count_>& hxbitset<bit_count_>::set(size_t pos_, bool value_) {
	hxassertmsg(pos_ < bit_count_, "invalid_index %zu", pos_);
	const size_t mask_ = static_cast<size_t>(1u) << (pos_ % s_bits_per_word_);
	if(value_) {
		m_data_[pos_ / s_bits_per_word_] |= mask_;
	} else {
		m_data_[pos_ / s_bits_per_word_] &= ~mask_;
	}
	return *this;
}

template<size_t bit_count_>
inline hxconstexpr bool hxbitset<bit_count_>::test(size_t pos_) const {
	hxassertmsg(pos_ < bit_count_, "invalid_index %zu", pos_);
	return (m_data_[pos_ / s_bits_per_word_]
		& (static_cast<size_t>(1u) << (pos_ % s_bits_per_word_))) != 0u;
}

template<size_t bit_count_>
inline hxconstexpr void hxbitset<bit_count_>::assert_no_trailing_bits_(void) const {
	hxassertmsg((m_data_[s_words_ - 1u] & ~s_trailing_mask_) == 0u, "trailing_bits_set");
}
