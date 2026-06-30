#pragma once
// SPDX-FileCopyrightText: © 2025 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

#ifndef LIBHATCHET_VER
#error Internal. Do not include this file directly.
#endif

#ifndef HX_DOXYGEN_PARSER
HX_BEGIN_INL_

template<size_t bit_count_>
inline hxconstexpr hxbitset<bit_count_>::hxbitset(size_t val_) {
	static_assert(bit_count_ == s_bits_per_word_,
		"hxbitset(size_t) requires bit_count_ == sizeof(size_t) * 8.");
	m_data_[0] = val_;
}

template<size_t bit_count_>
inline hxconstexpr hxbitset<bit_count_>::hxbitset(const hxbitset& x_) {
	const size_t* hxrestrict src_ = x_.m_data_;
	for(size_t* hxrestrict dst_ = m_data_, *const end_ = m_data_ + s_words_;
			dst_ != end_; ++dst_, ++src_) {
		*dst_ = *src_;
	}
}

template<size_t bit_count_>
inline hxconstexpr void hxbitset<bit_count_>::operator=(const hxbitset& x_) {
	hxassertmsg(static_cast<const void*>(this) != static_cast<const void*>(&x_),
		"invalid_reference");
	const size_t* hxrestrict src_ = x_.m_data_;
	for(size_t* hxrestrict dst_ = m_data_, *const end_ = m_data_ + s_words_;
			dst_ != end_; ++dst_, ++src_) {
		*dst_ = *src_;
	}
}

template<size_t bit_count_>
inline hxconstexpr bool hxbitset<bit_count_>::operator[](size_t pos_) const {
	hxassert_hard(pos_ < bit_count_, "invalid_index %zu", pos_);
	return (m_data_[pos_ >> s_log2_bits_per_word_]
		& (static_cast<size_t>(1u) << (pos_ & s_bit_mask_))) != 0u;
}

template<size_t bit_count_>
inline hxconstexpr hxbitset<bit_count_>& hxbitset<bit_count_>::operator&=(const hxbitset& x_) {
	hxassertmsg(static_cast<const void*>(this) != static_cast<const void*>(&x_),
		"invalid_reference");
	const size_t* hxrestrict src_ = x_.m_data_;
	for(size_t* hxrestrict dst_ = m_data_, *const end_ = m_data_ + s_words_;
			dst_ != end_; ++dst_, ++src_) {
		*dst_ &= *src_;
	}
	return *this;
}

template<size_t bit_count_>
inline hxconstexpr hxbitset<bit_count_>& hxbitset<bit_count_>::operator|=(const hxbitset& x_) {
	hxassertmsg(static_cast<const void*>(this) != static_cast<const void*>(&x_),
		"invalid_reference");
	const size_t* hxrestrict src_ = x_.m_data_;
	for(size_t* hxrestrict dst_ = m_data_, *const end_ = m_data_ + s_words_;
			dst_ != end_; ++dst_, ++src_) {
		*dst_ |= *src_;
	}
	hxassertmsg((m_data_[s_words_-1u] & ~s_trailing_mask_) == 0u, "trailing_bits_set");
	return *this;
}

template<size_t bit_count_>
inline hxconstexpr hxbitset<bit_count_>& hxbitset<bit_count_>::operator^=(const hxbitset& x_) {
	hxassertmsg(static_cast<const void*>(this) != static_cast<const void*>(&x_),
		"invalid_reference");
	const size_t* hxrestrict src_ = x_.m_data_;
	for(size_t* hxrestrict dst_ = m_data_, *const end_ = m_data_ + s_words_;
			dst_ != end_; ++dst_, ++src_) {
		*dst_ ^= *src_;
	}
	hxassertmsg((m_data_[s_words_-1u] & ~s_trailing_mask_) == 0u, "trailing_bits_set");
	return *this;
}

template<size_t bit_count_>
inline hxconstexpr hxbitset<bit_count_>& hxbitset<bit_count_>::operator<<=(size_t count_) {
	if(count_ == 0u) { return *this; }
	const size_t word_shift_ = count_ >> s_log2_bits_per_word_;
	const size_t bit_shift_ = count_ & s_bit_mask_;
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
	m_data_[s_words_-1u] &= s_trailing_mask_;
	hxassertmsg((m_data_[s_words_-1u] & ~s_trailing_mask_) == 0u, "trailing_bits_set");
	return *this;
}

template<size_t bit_count_>
inline hxconstexpr hxbitset<bit_count_>& hxbitset<bit_count_>::operator>>=(size_t count_) {
	if(count_ == 0u) { return *this; }
	const size_t word_shift_ = count_ >> s_log2_bits_per_word_;
	const size_t bit_shift_ = count_ & s_bit_mask_;
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
	hxassertmsg((m_data_[s_words_-1u] & ~s_trailing_mask_) == 0u, "trailing_bits_set");
	return *this;
}

template<size_t bit_count_>
inline hxconstexpr bool hxbitset<bit_count_>::operator==(const hxbitset& x_) const {
	hxassertmsg(static_cast<const void*>(this) != static_cast<const void*>(&x_),
		"invalid_reference");
	for(const size_t* dst_ = m_data_, * src_ = x_.m_data_, *const end_ = m_data_ + s_words_;
			dst_ != end_; ++dst_, ++src_) {
		if(*dst_ != *src_) { return false; }
	}
	return true;
}

template<size_t bit_count_>
inline hxconstexpr bool hxbitset<bit_count_>::all(void) const {
	hxassertmsg((m_data_[s_words_-1u] & ~s_trailing_mask_) == 0u, "trailing_bits_set");
	for(const size_t* dst_ = m_data_, *const end_ = m_data_ + (s_words_-1u);
			dst_ != end_; ++dst_) {
		if(*dst_ != ~static_cast<size_t>(0u)) { return false; }
	}
	return m_data_[s_words_-1u] == s_trailing_mask_;
}

template<size_t bit_count_>
inline hxconstexpr bool hxbitset<bit_count_>::any(void) const {
	hxassertmsg((m_data_[s_words_-1u] & ~s_trailing_mask_) == 0u, "trailing_bits_set");
	for(const size_t* dst_ = m_data_, *const end_ = m_data_ + s_words_;
			dst_ != end_; ++dst_) {
		if(*dst_ != 0u) { return true; }
	}
	return false;
}

template<size_t bit_count_>
inline hxconstexpr hxbitset<bit_count_>& hxbitset<bit_count_>::flip(void) {
	for(size_t* dst_ = m_data_, *const end_ = m_data_ + s_words_;
			dst_ != end_; ++dst_) {
		*dst_ ^= ~static_cast<size_t>(0u);
	}
	m_data_[s_words_-1u] &= s_trailing_mask_;
	return *this;
}

template<size_t bit_count_>
inline hxconstexpr hxbitset<bit_count_>& hxbitset<bit_count_>::flip(size_t pos_) {
	hxassert_hard(pos_ < bit_count_, "invalid_index %zu", pos_);
	m_data_[pos_ >> s_log2_bits_per_word_] ^= (static_cast<size_t>(1u) << (pos_ & s_bit_mask_));
	return *this;
}

template<size_t bit_count_>
inline void hxbitset<bit_count_>::load(const void* src_, size_t len_) {
	hxassert_hard(len_ <= this->bytes(), "overflow_load %zu", len_);
	::memcpy(m_data_, src_, len_);
	if(len_ < this->bytes()) {
		::memset(reinterpret_cast<char*>(m_data_) + len_, 0, this->bytes() - len_);
	}
	m_data_[s_words_-1u] &= s_trailing_mask_;
}

template<size_t bit_count_>
inline hxconstexpr hxbitset<bit_count_>& hxbitset<bit_count_>::reset(void) {
	for(size_t* dst_ = m_data_, *const end_ = m_data_ + s_words_;
			dst_ != end_; ++dst_) {
		*dst_ = 0u;
	}
	return *this;
}

template<size_t bit_count_>
inline hxconstexpr hxbitset<bit_count_>& hxbitset<bit_count_>::reset(size_t pos_) {
	hxassert_hard(pos_ < bit_count_, "invalid_index %zu", pos_);
	m_data_[pos_ >> s_log2_bits_per_word_] &= ~(static_cast<size_t>(1u) << (pos_ & s_bit_mask_));
	return *this;
}

template<size_t bit_count_>
inline hxconstexpr hxbitset<bit_count_>& hxbitset<bit_count_>::set(void) {
	for(size_t* dst_ = m_data_, *const end_ = m_data_ + s_words_;
			dst_ != end_; ++dst_) {
		*dst_ = ~static_cast<size_t>(0u);
	}
	m_data_[s_words_-1u] &= s_trailing_mask_;
	return *this;
}

template<size_t bit_count_>
inline hxconstexpr hxbitset<bit_count_>& hxbitset<bit_count_>::set(size_t pos_, bool value_) {
	hxassert_hard(pos_ < bit_count_, "invalid_index %zu", pos_);
	const size_t mask_ = static_cast<size_t>(1u) << (pos_ & s_bit_mask_);
	if(value_) {
		m_data_[pos_ >> s_log2_bits_per_word_] |= mask_;
	} else {
		m_data_[pos_ >> s_log2_bits_per_word_] &= ~mask_;
	}
	return *this;
}

template<size_t bit_count_>
inline hxconstexpr bool hxbitset<bit_count_>::test(size_t pos_) const {
	hxassert_hard(pos_ < bit_count_, "invalid_index %zu", pos_);
	return (m_data_[pos_ >> s_log2_bits_per_word_]
		& (static_cast<size_t>(1u) << (pos_ & s_bit_mask_))) != 0u;
}

HX_END_INL_
#endif // HX_DOXYGEN_PARSER
