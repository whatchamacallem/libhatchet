#pragma once
// SPDX-FileCopyrightText: © 2025 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

#ifndef LIBHATCHET_VER
#error Internal. Do not include this file directly.
#endif

#ifndef HX_DOXYGEN_PARSER
HX_BEGIN_INL_

template<typename T_, size_t capacity_>
hxdeque<T_, capacity_>::hxdeque(size_t dynamic_capacity_)
	: m_mask_(0u), m_head_(0u), m_tail_(0u), m_count_(0u)
{
	hxassert_hard(dynamic_capacity_ == 0u || (dynamic_capacity_ & (dynamic_capacity_ - 1u)) == 0u,
		"invalid_capacity capacity must be a power of 2");
	if(dynamic_capacity_ != 0u) {
		this->reserve_storage_(dynamic_capacity_);
	}
	m_mask_ = this->capacity() - 1u;
}

template<typename T_, size_t capacity_>
hxdeque<T_, capacity_>::~hxdeque(void) { clear(); }

template<typename T_, size_t capacity_>
T_& hxdeque<T_, capacity_>::operator[](size_t index_) {
	hxassert_hard(index_ < m_count_, "invalid_index %zu", index_);
	return this->data()[(m_head_ + index_) & m_mask_];
}

template<typename T_, size_t capacity_>
const T_& hxdeque<T_, capacity_>::operator[](size_t index_) const {
	hxassert_hard(index_ < m_count_, "invalid_index %zu", index_);
	return this->data()[(m_head_ + index_) & m_mask_];
}

template<typename T_, size_t capacity_>
T_& hxdeque<T_, capacity_>::at(size_t index_) {
	hxassert_hard(index_ < m_count_, "invalid_index %zu", index_);
	return this->data()[(m_head_ + index_) & m_mask_];
}

template<typename T_, size_t capacity_>
const T_& hxdeque<T_, capacity_>::at(size_t index_) const {
	hxassert_hard(index_ < m_count_, "invalid_index %zu", index_);
	return this->data()[(m_head_ + index_) & m_mask_];
}

template<typename T_, size_t capacity_>
T_& hxdeque<T_, capacity_>::back(void) {
	hxassert_hard(m_count_ > 0u, "empty_deque");
	return this->data()[(m_tail_ + m_mask_) & m_mask_];
}

template<typename T_, size_t capacity_>
const T_& hxdeque<T_, capacity_>::back(void) const {
	hxassert_hard(m_count_ > 0u, "empty_deque");
	return this->data()[(m_tail_ + m_mask_) & m_mask_];
}

template<typename T_, size_t capacity_>
void hxdeque<T_, capacity_>::clear(void) {
	T_* const data_ = this->data();
	for(size_t i_ = 0u; i_ < m_count_; ++i_) {
		data_[(m_head_ + i_) & m_mask_].T_::~T_();
	}
	m_head_ = 0u;
	m_tail_ = 0u;
	m_count_ = 0u;
}

template<typename T_, size_t capacity_>
template<typename... args_t_>
void hxdeque<T_, capacity_>::emplace_back(args_t_&&... args_) {
	push_back(hxforward<args_t_>(args_)...);
}

template<typename T_, size_t capacity_>
template<typename... args_t_>
void hxdeque<T_, capacity_>::emplace_front(args_t_&&... args_) {
	push_front(hxforward<args_t_>(args_)...);
}

template<typename T_, size_t capacity_>
bool hxdeque<T_, capacity_>::empty(void) const { return m_count_ == 0u; }

template<typename T_, size_t capacity_>
T_& hxdeque<T_, capacity_>::front(void) {
	hxassert_hard(m_count_ > 0u, "empty_deque");
	return this->data()[m_head_];
}

template<typename T_, size_t capacity_>
const T_& hxdeque<T_, capacity_>::front(void) const {
	hxassert_hard(m_count_ > 0u, "empty_deque");
	return this->data()[m_head_];
}

template<typename T_, size_t capacity_>
bool hxdeque<T_, capacity_>::full(void) const {
	return m_count_ == this->capacity();
}

template<typename T_, size_t capacity_>
void hxdeque<T_, capacity_>::pop_back(void) {
	hxassert_hard(m_count_ != 0u, "empty_deque");
	m_tail_ = (m_tail_ + m_mask_) & m_mask_;
	T_* slot_ = this->data() + m_tail_;
	slot_->T_::~T_();
	--m_count_;
}

template<typename T_, size_t capacity_>
void hxdeque<T_, capacity_>::pop_front(void) {
	hxassert_hard(m_count_ != 0u, "empty_deque");
	T_* slot_ = this->data() + m_head_;
	m_head_ = (m_head_ + 1u) & m_mask_;
	slot_->T_::~T_();
	--m_count_;
}

template<typename T_, size_t capacity_>
template<typename... args_t_>
void hxdeque<T_, capacity_>::push_back(args_t_&&... args_) {
	hxassert_hard(m_count_ < this->capacity(), "full_deque");
	T_* slot_ = this->data() + m_tail_;
	m_tail_ = (m_tail_ + 1u) & m_mask_; ++m_count_;
	::new(slot_) T_(hxforward<args_t_>(args_)...);
}

template<typename T_, size_t capacity_>
template<typename... args_t_>
void hxdeque<T_, capacity_>::push_front(args_t_&&... args_) {
	hxassert_hard(m_count_ < this->capacity(), "full_deque");
	m_head_ = (m_head_ + m_mask_) & m_mask_;
	T_* slot_ = this->data() + m_head_;
	++m_count_;
	::new(slot_) T_(hxforward<args_t_>(args_)...);
}

template<typename T_, size_t capacity_>
void hxdeque<T_, capacity_>::reserve(size_t dynamic_capacity_) {
	hxassert_hard(dynamic_capacity_ > 0u && (dynamic_capacity_ & (dynamic_capacity_ - 1u)) == 0u,
		"invalid_capacity capacity must be a power of 2");
	this->reserve_storage_(dynamic_capacity_);
	m_mask_ = this->capacity() - 1u;
}

template<typename T_, size_t capacity_>
size_t hxdeque<T_, capacity_>::size(void) const { return m_count_; }

HX_END_INL_
#endif // HX_DOXYGEN_PARSER
