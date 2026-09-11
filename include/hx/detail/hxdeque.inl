#pragma once
// SPDX-FileCopyrightText: © 2025 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

#ifndef LIBHATCHET_VER
#error Internal. Do not include this file directly.
#endif

#ifndef HX_DOXYGEN_PARSER
HX_INL_BEGIN_

template<hxdeque_concept_ T_, hxsize_t capacity_>
hxinline hxattr_flatten hxdeque<T_, capacity_>::hxdeque(hxsize_t dynamic_capacity_)
	: m_head_(0u), m_tail_(0u)
{
	static_assert(capacity_ == 0 || (capacity_ & (capacity_ - 1)) == 0,
		"bad_capacity not pow2");
	hxassert_hard(dynamic_capacity_ == 0 || (dynamic_capacity_ & (dynamic_capacity_ - 1)) == 0,
		"bad_capacity not pow2");
	hxif_constexpr(capacity_ == hxallocator_dynamic_capacity) {
		this->reserve_storage(dynamic_capacity_);
	}
}

template<hxdeque_concept_ T_, hxsize_t capacity_>
template<typename other_value_t_>
hxinline hxattr_flatten hxdeque<T_, capacity_>::hxdeque(
		std::initializer_list<other_value_t_> x_) noexcept
	: m_head_(0u), m_tail_(0u)
{
	hxif_constexpr(capacity_ == hxallocator_dynamic_capacity) {
		hxassert_hard((static_cast<size_t>(x_.size()) & (static_cast<size_t>(x_.size()) - 1u)) == 0u,
			"bad_capacity not pow2");
		this->reserve_storage(static_cast<hxsize_t>(x_.size()));
	}
	else {
		hxassert_hard(static_cast<hxsize_t>(x_.size()) <= capacity_, "bad_capacity not pow2");
	}
	const other_value_t_* hxrestrict src_ = x_.begin();
	for(const other_value_t_*const end_ = x_.end(); src_ != end_; ++src_) {
		this->push_back(*src_);
	}
}

template<hxdeque_concept_ T_, hxsize_t capacity_>
hxinline hxattr_flatten hxdeque<T_, capacity_>::~hxdeque(void) { this->clear(); }

template<hxdeque_concept_ T_, hxsize_t capacity_>
hxinline hxattr_flatten const T_& hxdeque<T_, capacity_>::operator[](hxsize_t index_) const {
	hxassert_hard(static_cast<size_t>(index_) < m_tail_ - m_head_, "bad_index %zd", index_);
	const size_t mask_ = static_cast<size_t>(this->capacity()) - 1u;
	return this->data()[(m_head_ + static_cast<size_t>(index_)) & mask_];
}

template<hxdeque_concept_ T_, hxsize_t capacity_>
hxinline hxattr_flatten T_& hxdeque<T_, capacity_>::operator[](hxsize_t index_) {
	hxassert_hard(static_cast<size_t>(index_) < m_tail_ - m_head_, "bad_index %zd", index_);
	const size_t mask_ = static_cast<size_t>(this->capacity()) - 1u;
	return this->data()[(m_head_ + static_cast<size_t>(index_)) & mask_];
}

#if HX_CPLUSPLUS < 202002L
template<hxdeque_concept_ T_, hxsize_t capacity_>
hxinline hxattr_flatten bool hxdeque<T_, capacity_>::operator!=(const hxdeque& x_) const {
	return !(*this == x_);
}
#endif

template<hxdeque_concept_ T_, hxsize_t capacity_>
template<hxrange_concept_ range_t_>
hxinline hxattr_flatten void hxdeque<T_, capacity_>::add_range(range_t_&& range_) noexcept {
	hxrestrict_t<decltype(range_.begin())> it_(range_.begin());
	for(const auto end_ = range_.end(); it_ != end_; ++it_) {
		this->push_back(hxforward_like<range_t_>(*it_));
	}
}

template<hxdeque_concept_ T_, hxsize_t capacity_>
hxinline hxattr_flatten T_& hxdeque<T_, capacity_>::back(void) {
	hxassert_hard(m_tail_ != m_head_, "queue_empty");
	const size_t mask_ = static_cast<size_t>(this->capacity()) - 1u;
	return this->data()[(m_tail_ - 1u) & mask_];
}

template<hxdeque_concept_ T_, hxsize_t capacity_>
hxinline hxattr_flatten const T_& hxdeque<T_, capacity_>::back(void) const {
	hxassert_hard(m_tail_ != m_head_, "queue_empty");
	const size_t mask_ = static_cast<size_t>(this->capacity()) - 1u;
	return this->data()[(m_tail_ - 1u) & mask_];
}

template<hxdeque_concept_ T_, hxsize_t capacity_>
hxinline hxattr_flatten hxsize_t hxdeque<T_, capacity_>::capacity(void) const {
	return hxallocator<T_, capacity_>::capacity();
}

template<hxdeque_concept_ T_, hxsize_t capacity_>
hxinline hxattr_flatten void hxdeque<T_, capacity_>::clear(void) noexcept {
	T_* const data_ = this->data();
	const size_t mask_ = static_cast<size_t>(this->capacity()) - 1u;
	for(size_t i_ = m_head_; i_ != m_tail_; ++i_) {
		data_[i_ & mask_].T_::~T_();
	}
	m_head_ = 0u;
	m_tail_ = 0u;
}

template<hxdeque_concept_ T_, hxsize_t capacity_>
template<typename... args_t_>
hxinline hxattr_flatten void hxdeque<T_, capacity_>::emplace_back(args_t_&&... args_) noexcept {
	this->push_back(hxforward<args_t_>(args_)...);
}

template<hxdeque_concept_ T_, hxsize_t capacity_>
template<typename... args_t_>
hxinline hxattr_flatten void hxdeque<T_, capacity_>::emplace_front(args_t_&&... args_) noexcept {
	this->push_front(hxforward<args_t_>(args_)...);
}

template<hxdeque_concept_ T_, hxsize_t capacity_>
hxinline hxattr_flatten bool hxdeque<T_, capacity_>::empty(void) const {
	return m_tail_ == m_head_;
}

template<hxdeque_concept_ T_, hxsize_t capacity_>
hxinline hxattr_flatten typename hxdeque<T_, capacity_>::const_iterator
hxdeque<T_, capacity_>::find(const T_& value_) const {
	const const_iterator end_ = this->end();
	for(const_iterator it_ = this->begin(); it_ != end_; ++it_) {
		if(hxkey_equal(*it_, value_)) {
			return it_;
		}
	}
	return end_;
}

template<hxdeque_concept_ T_, hxsize_t capacity_>
hxinline hxattr_flatten typename hxdeque<T_, capacity_>::iterator
hxdeque<T_, capacity_>::find(const T_& value_) {
	const iterator end_ = this->end();
	for(iterator it_ = this->begin(); it_ != end_; ++it_) {
		if(hxkey_equal(*it_, value_)) {
			return it_;
		}
	}
	return end_;
}

template<hxdeque_concept_ T_, hxsize_t capacity_>
hxinline hxattr_flatten T_& hxdeque<T_, capacity_>::front(void) {
	hxassert_hard(m_tail_ != m_head_, "queue_empty");
	const size_t mask_ = static_cast<size_t>(this->capacity()) - 1u;
	return this->data()[m_head_ & mask_];
}

template<hxdeque_concept_ T_, hxsize_t capacity_>
hxinline hxattr_flatten const T_& hxdeque<T_, capacity_>::front(void) const {
	hxassert_hard(m_tail_ != m_head_, "queue_empty");
	const size_t mask_ = static_cast<size_t>(this->capacity()) - 1u;
	return this->data()[m_head_ & mask_];
}

template<hxdeque_concept_ T_, hxsize_t capacity_>
hxinline hxattr_flatten bool hxdeque<T_, capacity_>::full(void) const {
	return m_tail_ - m_head_ == static_cast<size_t>(this->capacity());
}

template<hxdeque_concept_ T_, hxsize_t capacity_>
hxinline hxattr_flatten void hxdeque<T_, capacity_>::pop_back(void) noexcept {
	hxassert_hard(m_tail_ != m_head_, "queue_empty");
	const size_t mask_ = static_cast<size_t>(this->capacity()) - 1u;
	--m_tail_;
	this->data()[m_tail_ & mask_].T_::~T_();
}

template<hxdeque_concept_ T_, hxsize_t capacity_>
hxinline hxattr_flatten void hxdeque<T_, capacity_>::pop_front(void) noexcept {
	hxassert_hard(m_tail_ != m_head_, "queue_empty");
	const size_t mask_ = static_cast<size_t>(this->capacity()) - 1u;
	this->data()[m_head_ & mask_].T_::~T_();
	++m_head_;
}

template<hxdeque_concept_ T_, hxsize_t capacity_>
template<typename... args_t_>
hxinline hxattr_flatten void hxdeque<T_, capacity_>::push_back(args_t_&&... args_) noexcept {
	hxassert_hard(m_tail_ - m_head_ < static_cast<size_t>(this->capacity()), "deque_full");
	const size_t mask_ = static_cast<size_t>(this->capacity()) - 1u;
	T_* const slot_ = this->data() + (m_tail_ & mask_);
	++m_tail_;
	::new(slot_) T_(hxforward<args_t_>(args_)...);
}

template<hxdeque_concept_ T_, hxsize_t capacity_>
template<typename... args_t_>
hxinline hxattr_flatten void hxdeque<T_, capacity_>::push_front(args_t_&&... args_) noexcept {
	hxassert_hard(m_tail_ - m_head_ < static_cast<size_t>(this->capacity()), "deque_full");
	const size_t mask_ = static_cast<size_t>(this->capacity()) - 1u;
	--m_head_;
	T_* const slot_ = this->data() + (m_head_ & mask_);
	::new(slot_) T_(hxforward<args_t_>(args_)...);
}

template<hxdeque_concept_ T_, hxsize_t capacity_>
hxinline hxattr_flatten void hxdeque<T_, capacity_>::reserve(hxsize_t dynamic_capacity_) {
	hxassert_hard(dynamic_capacity_ > hxallocator_dynamic_capacity
		&& (dynamic_capacity_ & (dynamic_capacity_ - 1)) == 0,
		"bad_capacity not pow2");
	this->reserve_storage(dynamic_capacity_);
}

template<hxdeque_concept_ T_, hxsize_t capacity_>
hxinline hxattr_flatten hxsize_t hxdeque<T_, capacity_>::size(void) const {
	return static_cast<hxsize_t>(m_tail_ - m_head_);
}

template<hxdeque_concept_ T_, hxsize_t capacity_>
hxinline hxattr_flatten void hxdeque<T_, capacity_>::swap(hxdeque& x_) noexcept {
	static_assert(capacity_ == hxallocator_dynamic_capacity,
		"Dynamic capacity required for hxdeque::swap");
	hxswap_memcpy(*this, x_);
}

HX_INL_END_
#endif // HX_DOXYGEN_PARSER
