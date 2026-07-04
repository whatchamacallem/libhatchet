#pragma once
// SPDX-FileCopyrightText: © 2025 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

#ifndef LIBHATCHET_VER
#error Internal. Do not include this file directly.
#endif

#ifndef HX_DOXYGEN_PARSER
HX_BEGIN_INL_

template<typename T_, hxsize_t capacity_>
hxdeque<T_, capacity_>::hxdeque(hxsize_t dynamic_capacity_)
	: m_head_(0u), m_tail_(0u)
{
	static_assert(capacity_ == 0 || (capacity_ & (capacity_ - 1)) == 0,
		"invalid_capacity capacity must be a power of 2");
	hxassert_hard(dynamic_capacity_ == 0 || (dynamic_capacity_ & (dynamic_capacity_ - 1)) == 0,
		"invalid_capacity capacity must be a power of 2");
	if(dynamic_capacity_ != 0) {
		this->reserve_storage(dynamic_capacity_);
	}
}

template<typename T_, hxsize_t capacity_>
hxdeque<T_, capacity_>::~hxdeque(void) { this->clear(); }

template<typename T_, hxsize_t capacity_>
T_& hxdeque<T_, capacity_>::operator[](hxsize_t index_) {
	hxassert_hard(static_cast<size_t>(index_) < m_tail_ - m_head_, "invalid_index %zu", index_);
	const size_t mask_ = static_cast<size_t>(this->capacity()) - 1u;
	return this->data()[(m_head_ + static_cast<size_t>(index_)) & mask_];
}

template<typename T_, hxsize_t capacity_>
const T_& hxdeque<T_, capacity_>::operator[](hxsize_t index_) const {
	hxassert_hard(static_cast<size_t>(index_) < m_tail_ - m_head_, "invalid_index %zu", index_);
	const size_t mask_ = static_cast<size_t>(this->capacity()) - 1u;
	return this->data()[(m_head_ + static_cast<size_t>(index_)) & mask_];
}

template<typename T_, hxsize_t capacity_>
T_& hxdeque<T_, capacity_>::at(hxsize_t index_) {
	hxassert_hard(static_cast<size_t>(index_) < m_tail_ - m_head_, "invalid_index %zu", index_);
	const size_t mask_ = static_cast<size_t>(this->capacity()) - 1u;
	return this->data()[(m_head_ + static_cast<size_t>(index_)) & mask_];
}

template<typename T_, hxsize_t capacity_>
const T_& hxdeque<T_, capacity_>::at(hxsize_t index_) const {
	hxassert_hard(static_cast<size_t>(index_) < m_tail_ - m_head_, "invalid_index %zu", index_);
	const size_t mask_ = static_cast<size_t>(this->capacity()) - 1u;
	return this->data()[(m_head_ + static_cast<size_t>(index_)) & mask_];
}

template<typename T_, hxsize_t capacity_>
T_& hxdeque<T_, capacity_>::back(void) {
	hxassert_hard(m_tail_ != m_head_, "empty_deque");
	const size_t mask_ = static_cast<size_t>(this->capacity()) - 1u;
	return this->data()[(m_tail_ - 1u) & mask_];
}

template<typename T_, hxsize_t capacity_>
const T_& hxdeque<T_, capacity_>::back(void) const {
	hxassert_hard(m_tail_ != m_head_, "empty_deque");
	const size_t mask_ = static_cast<size_t>(this->capacity()) - 1u;
	return this->data()[(m_tail_ - 1u) & mask_];
}

template<typename T_, hxsize_t capacity_>
hxsize_t hxdeque<T_, capacity_>::capacity(void) const {
	return hxallocator<T_, capacity_>::capacity();
}

template<typename T_, hxsize_t capacity_>
void hxdeque<T_, capacity_>::clear(void) noexcept {
	T_* const data_ = this->data();
	const size_t mask_ = static_cast<size_t>(this->capacity()) - 1u;
	for(size_t i_ = m_head_; i_ != m_tail_; ++i_) {
		data_[i_ & mask_].T_::~T_();
	}
	m_head_ = 0u;
	m_tail_ = 0u;
}

template<typename T_, hxsize_t capacity_>
template<typename... args_t_>
void hxdeque<T_, capacity_>::emplace_back(args_t_&&... args_) noexcept {
	this->push_back(hxforward<args_t_>(args_)...);
}

template<typename T_, hxsize_t capacity_>
template<typename... args_t_>
void hxdeque<T_, capacity_>::emplace_front(args_t_&&... args_) noexcept {
	this->push_front(hxforward<args_t_>(args_)...);
}

template<typename T_, hxsize_t capacity_>
bool hxdeque<T_, capacity_>::empty(void) const { return m_tail_ == m_head_; }

template<typename T_, hxsize_t capacity_>
T_& hxdeque<T_, capacity_>::front(void) {
	hxassert_hard(m_tail_ != m_head_, "empty_deque");
	const size_t mask_ = static_cast<size_t>(this->capacity()) - 1u;
	return this->data()[m_head_ & mask_];
}

template<typename T_, hxsize_t capacity_>
const T_& hxdeque<T_, capacity_>::front(void) const {
	hxassert_hard(m_tail_ != m_head_, "empty_deque");
	const size_t mask_ = static_cast<size_t>(this->capacity()) - 1u;
	return this->data()[m_head_ & mask_];
}

template<typename T_, hxsize_t capacity_>
bool hxdeque<T_, capacity_>::full(void) const {
	return m_tail_ - m_head_ == static_cast<size_t>(this->capacity());
}

template<typename T_, hxsize_t capacity_>
void hxdeque<T_, capacity_>::pop_back(void) noexcept {
	hxassert_hard(m_tail_ != m_head_, "empty_deque");
	const size_t mask_ = static_cast<size_t>(this->capacity()) - 1u;
	--m_tail_;
	this->data()[m_tail_ & mask_].T_::~T_();
}

template<typename T_, hxsize_t capacity_>
void hxdeque<T_, capacity_>::pop_front(void) noexcept {
	hxassert_hard(m_tail_ != m_head_, "empty_deque");
	const size_t mask_ = static_cast<size_t>(this->capacity()) - 1u;
	this->data()[m_head_ & mask_].T_::~T_();
	++m_head_;
}

template<typename T_, hxsize_t capacity_>
template<typename... args_t_>
void hxdeque<T_, capacity_>::push_back(args_t_&&... args_) noexcept {
	hxassert_hard(m_tail_ - m_head_ < static_cast<size_t>(this->capacity()), "full_deque");
	const size_t mask_ = static_cast<size_t>(this->capacity()) - 1u;
	T_* const slot_ = this->data() + (m_tail_ & mask_);
	++m_tail_;
	::new(slot_) T_(hxforward<args_t_>(args_)...);
}

template<typename T_, hxsize_t capacity_>
template<typename... args_t_>
void hxdeque<T_, capacity_>::push_front(args_t_&&... args_) noexcept {
	hxassert_hard(m_tail_ - m_head_ < static_cast<size_t>(this->capacity()), "full_deque");
	const size_t mask_ = static_cast<size_t>(this->capacity()) - 1u;
	--m_head_;
	T_* const slot_ = this->data() + (m_head_ & mask_);
	::new(slot_) T_(hxforward<args_t_>(args_)...);
}

template<typename T_, hxsize_t capacity_>
void hxdeque<T_, capacity_>::reserve(hxsize_t dynamic_capacity_) {
	hxassert_hard(dynamic_capacity_ > hxallocator_dynamic_capacity
		&& (dynamic_capacity_ & (dynamic_capacity_ - 1)) == 0,
		"invalid_capacity capacity must be a power of 2");
	this->reserve_storage(dynamic_capacity_);
}

template<typename T_, hxsize_t capacity_>
hxsize_t hxdeque<T_, capacity_>::size(void) const {
	return static_cast<hxsize_t>(m_tail_ - m_head_);
}

HX_END_INL_
#endif // HX_DOXYGEN_PARSER
