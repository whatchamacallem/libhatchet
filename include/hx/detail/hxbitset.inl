#pragma once
// SPDX-FileCopyrightText: © 2025 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

#ifndef LIBHATCHET_VER
#error Internal. Do not include this file directly.
#endif

#ifndef HX_DOXYGEN_PARSER
HX_INL_BEGIN_

// GCOVR_EXCL_START
template<size_t bit_count_>
hxinline hxconstexpr hxbitset<bit_count_>::hxbitset(size_t val_) {
	static_assert(bit_count_ == s_bits_per_word_,
		"hxbitset(size_t) requires bit_count_ == sizeof(size_t) * 8");
	m_data_[0] = val_;
}
// GCOVR_EXCL_STOP

template<size_t bit_count_>
hxinline hxconstexpr hxbitset<bit_count_>::hxbitset(const hxbitset& x_) {
	const size_t* hxrestrict src_ = x_.m_data_;
	for(size_t* hxrestrict dst_ = m_data_, *const end_ = m_data_ + s_words_;
			dst_ != end_; ++dst_, ++src_) {
		*dst_ = *src_;
	}
}

// GCOVR_EXCL_START
template<size_t bit_count_>
template<typename int_t_>
hxinline hxconstexpr hxbitset<bit_count_>::hxbitset(std::initializer_list<int_t_> x_) {
	static_assert(sizeof(size_t) % sizeof(int_t_) == 0u,
		"sizeof(size_t) must be a multiple of sizeof(int_t_)");
	hxassertf(x_.size() == s_words_ * (sizeof(size_t) / sizeof(int_t_)),
		"initializer_list size %zu want %zu", x_.size(), s_words_ * (sizeof(size_t) / sizeof(int_t_)));
	const int_t_* hxrestrict src_ = x_.begin();
	hxif_constexpr(sizeof(int_t_) == sizeof(size_t)) {
		for(size_t* hxrestrict dst_ = m_data_, *const end_ = m_data_ + s_words_;
				dst_ != end_; ++dst_, ++src_) {
			*dst_ = static_cast<size_t>(*src_);
		}
	}
	else {
		const size_t element_bits_ = sizeof(int_t_) * 8u;
		const size_t element_mask_ = (static_cast<size_t>(1u) << element_bits_) - 1u;
		const size_t per_word_ = sizeof(size_t) / sizeof(int_t_);
		for(size_t* hxrestrict dst_ = m_data_, *const end_ = m_data_ + s_words_;
				dst_ != end_; ++dst_) {
			size_t word_ = 0u;
			for(size_t i_ = 0u; i_ < per_word_; ++i_, ++src_) {
				word_ |= (static_cast<size_t>(*src_) & element_mask_) << (i_ * element_bits_);
			}
			*dst_ = word_;
		}
	}
	hxassertf((m_data_[s_words_-1u] & ~s_trailing_mask_) == 0u, "stray_bits");
}
// GCOVR_EXCL_STOP

template<size_t bit_count_>
hxinline hxconstexpr void hxbitset<bit_count_>::operator=(const hxbitset& x_) {
	hxassertf(static_cast<const void*>(this) != static_cast<const void*>(&x_), "bad_ref");
	const size_t* hxrestrict src_ = x_.m_data_;
	for(size_t* hxrestrict dst_ = m_data_, *const end_ = m_data_ + s_words_;
			dst_ != end_; ++dst_, ++src_) {
		*dst_ = *src_;
	}
}

template<size_t bit_count_>
hxinline hxconstexpr bool hxbitset<bit_count_>::operator[](size_t pos_) const {
	hxassert_hard(pos_ < bit_count_, "bad_index %zu", pos_);
	return (m_data_[pos_ >> s_log2_bits_per_word_]
		& (static_cast<size_t>(1u) << (pos_ & s_bit_mask_))) != 0u;
}

template<size_t bit_count_>
hxinline hxconstexpr hxbitset<bit_count_>& hxbitset<bit_count_>::operator&=(const hxbitset& x_) {
	hxassertf(static_cast<const void*>(this) != static_cast<const void*>(&x_), "bad_ref");
	const size_t* hxrestrict src_ = x_.m_data_;
	for(size_t* hxrestrict dst_ = m_data_, *const end_ = m_data_ + s_words_;
			dst_ != end_; ++dst_, ++src_) {
		*dst_ &= *src_;
	}
	return *this;
}

template<size_t bit_count_>
hxinline hxconstexpr hxbitset<bit_count_>& hxbitset<bit_count_>::operator|=(const hxbitset& x_) {
	hxassertf(static_cast<const void*>(this) != static_cast<const void*>(&x_), "bad_ref");
	const size_t* hxrestrict src_ = x_.m_data_;
	for(size_t* hxrestrict dst_ = m_data_, *const end_ = m_data_ + s_words_;
			dst_ != end_; ++dst_, ++src_) {
		*dst_ |= *src_;
	}
	hxassertf((m_data_[s_words_-1u] & ~s_trailing_mask_) == 0u, "stray_bits");
	return *this;
}

template<size_t bit_count_>
hxinline hxconstexpr hxbitset<bit_count_>& hxbitset<bit_count_>::operator^=(const hxbitset& x_) {
	hxassertf(static_cast<const void*>(this) != static_cast<const void*>(&x_), "bad_ref");
	const size_t* hxrestrict src_ = x_.m_data_;
	for(size_t* hxrestrict dst_ = m_data_, *const end_ = m_data_ + s_words_;
			dst_ != end_; ++dst_, ++src_) {
		*dst_ ^= *src_;
	}
	hxassertf((m_data_[s_words_-1u] & ~s_trailing_mask_) == 0u, "stray_bits");
	return *this;
}

template<size_t bit_count_>
inline hxconstexpr hxbitset<bit_count_>& hxbitset<bit_count_>::operator<<=(size_t count_) {
	if(count_ == 0u) { return *this; }
	const size_t word_shift_ = count_ >> s_log2_bits_per_word_;
	const size_t bit_shift_ = count_ & s_bit_mask_;
	size_t i_ = s_words_;
	if(bit_shift_ == 0u) {
		while(i_ > word_shift_) {
			--i_;
			m_data_[i_] = m_data_[i_ - word_shift_];
		}
	} else {
		while(i_ > word_shift_ + 1u) {
			--i_;
			m_data_[i_] = (m_data_[i_ - word_shift_] << bit_shift_)
				| (m_data_[i_ - word_shift_ - 1u] >> (s_bits_per_word_ - bit_shift_));
		}
		if(i_ > word_shift_) {
			--i_;
			m_data_[i_] = m_data_[0] << bit_shift_;
		}
	}
	while(i_ != 0u) {
		m_data_[--i_] = 0u;
	}
	m_data_[s_words_-1u] &= s_trailing_mask_;
	return *this;
}

template<size_t bit_count_>
inline hxconstexpr hxbitset<bit_count_>& hxbitset<bit_count_>::operator>>=(size_t count_) {
	if(count_ == 0u) { return *this; }
	const size_t word_shift_ = count_ >> s_log2_bits_per_word_;
	const size_t bit_shift_ = count_ & s_bit_mask_;
	size_t i_ = 0u;
	if(bit_shift_ == 0u) {
		for(; i_ + word_shift_ < s_words_; ++i_) {
			m_data_[i_] = m_data_[i_ + word_shift_];
		}
	} else {
		for(; i_ + word_shift_ + 1u < s_words_; ++i_) {
			m_data_[i_] = (m_data_[i_ + word_shift_] >> bit_shift_)
				| (m_data_[i_ + word_shift_ + 1u] << (s_bits_per_word_ - bit_shift_));
		}
		if(i_ + word_shift_ < s_words_) {
			m_data_[i_] = m_data_[i_ + word_shift_] >> bit_shift_;
			++i_;
		}
	}
	for(; i_ < s_words_; ++i_) {
		m_data_[i_] = 0u;
	}
	hxassertf((m_data_[s_words_-1u] & ~s_trailing_mask_) == 0u, "stray_bits");
	return *this;
}

template<size_t bit_count_>
hxinline hxconstexpr bool hxbitset<bit_count_>::operator==(const hxbitset& x_) const {
	hxassertf(static_cast<const void*>(this) != static_cast<const void*>(&x_), "bad_ref");
	size_t difference_ = 0u;
	const size_t* hxrestrict src_ = x_.m_data_;
	for(const size_t* hxrestrict dst_ = m_data_, *const end_ = m_data_ + s_words_;
			dst_ != end_; ++dst_, ++src_) {
		difference_ |= *dst_ ^ *src_;
	}
	return difference_ == 0u;
}

#if HX_CPLUSPLUS < 202002L // C++20 defaults != from ==.
template<size_t bit_count_>
hxinline hxconstexpr bool hxbitset<bit_count_>::operator!=(const hxbitset& x_) const {
	return !(*this == x_);
}
#endif

template<size_t bit_count_>
hxinline hxconstexpr bool hxbitset<bit_count_>::all(void) const {
	hxassertf((m_data_[s_words_-1u] & ~s_trailing_mask_) == 0u, "stray_bits");
	size_t missing_ = m_data_[s_words_-1u] ^ s_trailing_mask_;
	for(const size_t* dst_ = m_data_, *const end_ = m_data_ + (s_words_-1u);
			dst_ != end_; ++dst_) {
		missing_ |= ~*dst_;
	}
	return missing_ == 0u;
}

template<size_t bit_count_>
hxinline hxconstexpr bool hxbitset<bit_count_>::any(void) const {
	hxassertf((m_data_[s_words_-1u] & ~s_trailing_mask_) == 0u, "stray_bits");
	size_t bits_ = 0u;
	for(const size_t* dst_ = m_data_, *const end_ = m_data_ + s_words_;
			dst_ != end_; ++dst_) {
		bits_ |= *dst_;
	}
	return bits_ != 0u;
}

template<size_t bit_count_>
hxinline hxconstexpr hxbitset<bit_count_>& hxbitset<bit_count_>::flip(void) {
	for(size_t* dst_ = m_data_, *const end_ = m_data_ + s_words_;
			dst_ != end_; ++dst_) {
		*dst_ ^= ~static_cast<size_t>(0u);
	}
	m_data_[s_words_-1u] &= s_trailing_mask_;
	return *this;
}

template<size_t bit_count_>
hxinline hxconstexpr hxbitset<bit_count_>& hxbitset<bit_count_>::flip(size_t pos_) {
	hxassert_hard(pos_ < bit_count_, "bad_index %zu", pos_);
	m_data_[pos_ >> s_log2_bits_per_word_] ^= (static_cast<size_t>(1u) << (pos_ & s_bit_mask_));
	return *this;
}

template<size_t bit_count_>
hxinline hxconstexpr hxbitset<bit_count_>& hxbitset<bit_count_>::reset(void) {
	for(size_t* dst_ = m_data_, *const end_ = m_data_ + s_words_;
			dst_ != end_; ++dst_) {
		*dst_ = 0u;
	}
	return *this;
}

template<size_t bit_count_>
hxinline hxconstexpr hxbitset<bit_count_>& hxbitset<bit_count_>::reset(size_t pos_) {
	hxassert_hard(pos_ < bit_count_, "bad_index %zu", pos_);
	m_data_[pos_ >> s_log2_bits_per_word_] &= ~(static_cast<size_t>(1u) << (pos_ & s_bit_mask_));
	return *this;
}

template<size_t bit_count_>
hxinline hxconstexpr hxbitset<bit_count_>& hxbitset<bit_count_>::set(void) {
	for(size_t* dst_ = m_data_, *const end_ = m_data_ + s_words_;
			dst_ != end_; ++dst_) {
		*dst_ = ~static_cast<size_t>(0u);
	}
	m_data_[s_words_-1u] &= s_trailing_mask_;
	return *this;
}

template<size_t bit_count_>
hxinline hxconstexpr hxbitset<bit_count_>& hxbitset<bit_count_>::set(size_t pos_, bool value_) {
	hxassert_hard(pos_ < bit_count_, "bad_index %zu", pos_);
	const size_t shift_ = pos_ & s_bit_mask_;
	size_t& word_ = m_data_[pos_ >> s_log2_bits_per_word_];
	word_ = (word_ & ~(static_cast<size_t>(1u) << shift_))
		| (static_cast<size_t>(value_) << shift_);
	return *this;
}

template<size_t bit_count_>
hxinline hxconstexpr bool hxbitset<bit_count_>::test(size_t pos_) const {
	hxassert_hard(pos_ < bit_count_, "bad_index %zu", pos_);
	return (m_data_[pos_ >> s_log2_bits_per_word_]
		& (static_cast<size_t>(1u) << (pos_ & s_bit_mask_))) != 0u;
}

HX_INL_END_
#endif // HX_DOXYGEN_PARSER
