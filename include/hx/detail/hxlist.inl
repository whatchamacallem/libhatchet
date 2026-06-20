#pragma once
// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

#ifndef LIBHATCHET_VER
#error Internal. Do not include this file directly.
#endif

HX_BEGIN_INL_

template<typename node_t_, typename deleter_t_>
inline hxconstexpr auto hxlist<node_t_, deleter_t_>::const_iterator::operator++(void)
		-> const_iterator& {
#if (HX_HARDENING_MODE) >= HX_HARDENING_MODE_VERBOSE
	hxassert_hard(this->m_current_node_ != hxnull && this->m_current_node_ != this->m_sentinel_,
		"invalid_iterator");
#endif
	m_current_node_ = m_current_node_->m_list_next_;
	return *this;
}

template<typename node_t_, typename deleter_t_>
inline hxconstexpr auto hxlist<node_t_, deleter_t_>::const_iterator::operator++(int)
		-> const_iterator {
	const_iterator t_(*this);
	this->operator++();
	return t_;
}

template<typename node_t_, typename deleter_t_>
inline hxconstexpr auto hxlist<node_t_, deleter_t_>::const_iterator::operator--(void)
		-> const_iterator& {
	// std::list allows decrementing from the end().
#if (HX_HARDENING_MODE) >= HX_HARDENING_MODE_VERBOSE
	hxassert_hard(this->m_sentinel_ != hxnull && m_current_node_->m_list_prev_ != this->m_sentinel_,
		"invalid_iterator");
#endif
	m_current_node_ = m_current_node_->m_list_prev_;
	return *this;
}

template<typename node_t_, typename deleter_t_>
inline hxconstexpr auto hxlist<node_t_, deleter_t_>::const_iterator::operator--(int)
		-> const_iterator {
	const_iterator t_(*this);
	this->operator--();
	return t_;
}

template<typename node_t_, typename deleter_t_>
inline hxconstexpr bool hxlist<node_t_, deleter_t_>::const_iterator::operator==(
		const const_iterator& x_) const {
	return m_current_node_ == x_.m_current_node_;
}

#if HX_CPLUSPLUS < 202002L // C++20 defaults != from ==.
template<typename node_t_, typename deleter_t_>
inline hxconstexpr bool hxlist<node_t_, deleter_t_>::const_iterator::operator!=(
		const const_iterator& x_) const {
	return m_current_node_ != x_.m_current_node_;
}
#endif

// iterator

template<typename node_t_, typename deleter_t_>
inline hxconstexpr auto hxlist<node_t_, deleter_t_>::iterator::operator++(int) -> iterator {
	iterator t_(*this);
	this->const_iterator::operator++();
	return t_;
}

template<typename node_t_, typename deleter_t_>
inline hxconstexpr auto hxlist<node_t_, deleter_t_>::iterator::operator--(int) -> iterator {
	iterator t_(*this);
	this->const_iterator::operator--();
	return t_;
}

// hxlist

template<typename node_t_, typename deleter_t_>
inline hxconstexpr hxlist<node_t_, deleter_t_>::hxlist(void) {
	m_size_ = 0u;
	m_head_.m_list_prev_ = &m_head_;
	m_head_.m_list_next_ = &m_head_;
}

template<typename node_t_, typename deleter_t_>
inline hxconstexpr node_t_& hxlist<node_t_, deleter_t_>::back(void) {
	hxassert_hard(!this->empty(), "empty_list");
	return *static_cast<node_t_*>(m_head_.m_list_prev_);
}

template<typename node_t_, typename deleter_t_>
inline hxconstexpr const node_t_& hxlist<node_t_, deleter_t_>::back(void) const {
	hxassert_hard(!this->empty(), "empty_list");
	return *static_cast<const node_t_*>(m_head_.m_list_prev_);
}

template<typename node_t_, typename deleter_t_>
inline hxconstexpr auto hxlist<node_t_, deleter_t_>::begin(void) const -> const_iterator {
	return const_iterator(m_head_.m_list_next_, &m_head_);
}

template<typename node_t_, typename deleter_t_>
inline hxconstexpr auto hxlist<node_t_, deleter_t_>::end(void) const -> const_iterator {
	return const_iterator(const_cast<hxlist_node*>(&m_head_), &m_head_);
}

template<typename node_t_, typename deleter_t_>
template<typename deleter_override_t_>
inline hxconstexpr void hxlist<node_t_, deleter_t_>::clear(
	const deleter_override_t_& deleter_) {
	if(m_size_ != 0u) {
		if(deleter_) {
			hxlist_node* n_ = m_head_.m_list_next_;
			while(n_ != &m_head_) {
				hxlist_node* next_ = n_->m_list_next_;
				deleter_(static_cast<node_t_*>(n_));
				n_ = next_;
			}
		}
		this->release_all();
	}
}

template<typename node_t_, typename deleter_t_>
template<typename deleter_override_t_>
inline hxconstexpr void hxlist<node_t_, deleter_t_>::erase(
	node_t_* ptr_, const deleter_override_t_& deleter_) {
	this->extract_(ptr_);
	if(deleter_) {
		deleter_(ptr_);
	}
}

template<typename node_t_, typename deleter_t_>
inline hxconstexpr hxptr<node_t_, deleter_t_> hxlist<node_t_, deleter_t_>::extract(node_t_* ptr_) {
	this->extract_(ptr_);
	return hxptr<node_t_, deleter_t_>(ptr_);
}

template<typename node_t_, typename deleter_t_>
inline hxconstexpr node_t_& hxlist<node_t_, deleter_t_>::front(void) {
	hxassert_hard(!this->empty(), "empty_list");
	return *static_cast<node_t_*>(m_head_.m_list_next_);
}

template<typename node_t_, typename deleter_t_>
inline hxconstexpr const node_t_& hxlist<node_t_, deleter_t_>::front(void) const {
	hxassert_hard(!this->empty(), "empty_list");
	return *static_cast<const node_t_*>(m_head_.m_list_next_);
}

template<typename node_t_, typename deleter_t_>
template<typename ptr_deleter_t_>
inline hxconstexpr void hxlist<node_t_, deleter_t_>::insert_after(
		node_t_* pos_, hxptr<node_t_, ptr_deleter_t_>&& ptr_) {
	this->insert_after(pos_, ptr_.release());
}

template<typename node_t_, typename deleter_t_>
inline hxconstexpr void hxlist<node_t_, deleter_t_>::insert_after(
		node_t_* pos_, node_t_* ptr_) {
	hxassertmsg(pos_ != hxnull, "invalid_node");
	this->insert_(pos_, pos_->m_list_next_, ptr_);
}

template<typename node_t_, typename deleter_t_>
template<typename ptr_deleter_t_>
inline hxconstexpr void hxlist<node_t_, deleter_t_>::insert(
		node_t_* pos_, hxptr<node_t_, ptr_deleter_t_>&& ptr_) {
	this->insert(pos_, ptr_.release());
}

template<typename node_t_, typename deleter_t_>
inline hxconstexpr void hxlist<node_t_, deleter_t_>::insert(
		node_t_* pos_, node_t_* ptr_) {
	hxassertmsg(pos_ != hxnull, "invalid_node");
	this->insert_(pos_->m_list_prev_, pos_, ptr_);
}

template<typename node_t_, typename deleter_t_>
inline hxconstexpr hxptr<node_t_, deleter_t_> hxlist<node_t_, deleter_t_>::pop_back(void) {
	hxassert_hard(!this->empty(), "empty_list");
	node_t_* ptr_ = static_cast<node_t_*>(m_head_.m_list_prev_);
	this->extract_(ptr_);
	return hxptr<node_t_, deleter_t_>(ptr_);
}

template<typename node_t_, typename deleter_t_>
inline hxconstexpr hxptr<node_t_, deleter_t_> hxlist<node_t_, deleter_t_>::pop_front(void) {
	hxassert_hard(!this->empty(), "empty_list");
	node_t_* ptr_ = static_cast<node_t_*>(m_head_.m_list_next_);
	this->extract_(ptr_);
	return hxptr<node_t_, deleter_t_>(ptr_);
}

template<typename node_t_, typename deleter_t_>
template<typename ptr_deleter_t_>
inline hxconstexpr void hxlist<node_t_, deleter_t_>::push_back(
		hxptr<node_t_, ptr_deleter_t_>&& ptr_) {
	this->push_back(ptr_.release());
}

template<typename node_t_, typename deleter_t_>
inline hxconstexpr void hxlist<node_t_, deleter_t_>::push_back(node_t_* ptr_) {
	hxassert_hard(ptr_ != hxnull, "invalid_node");
	this->insert_(m_head_.m_list_prev_, &m_head_, ptr_);
}

template<typename node_t_, typename deleter_t_>
template<typename ptr_deleter_t_>
inline hxconstexpr void hxlist<node_t_, deleter_t_>::push_front(
		hxptr<node_t_, ptr_deleter_t_>&& ptr_) {
	this->push_front(ptr_.release());
}

template<typename node_t_, typename deleter_t_>
inline hxconstexpr void hxlist<node_t_, deleter_t_>::push_front(node_t_* ptr_) {
	hxassert_hard(ptr_ != hxnull, "invalid_node");
	this->insert_(&m_head_, m_head_.m_list_next_, ptr_);
}

template<typename node_t_, typename deleter_t_>
inline hxconstexpr void hxlist<node_t_, deleter_t_>::release_all(void) {
	m_head_.m_list_prev_ = &m_head_;
	m_head_.m_list_next_ = &m_head_;
	m_size_ = 0u;
}

template<typename node_t_, typename deleter_t_>
template<typename predicate_t_>
inline hxconstexpr size_t hxlist<node_t_, deleter_t_>::remove_if(predicate_t_ predicate_) {
	size_t count_ = 0u;
	hxlist_node* n_ = m_head_.m_list_next_;
	while(n_ != &m_head_) {
		hxlist_node* next_ = n_->m_list_next_;
		node_t_* node_ = static_cast<node_t_*>(n_);
		if(predicate_(*node_)) {
			this->extract_(n_);
			deleter_t_()(node_);
			++count_;
		}
		n_ = next_;
	}
	return count_;
}

template<typename node_t_, typename deleter_t_>
inline hxconstexpr void hxlist<node_t_, deleter_t_>::reverse(void) {
	hxlist_node* n_ = &m_head_;
	do {
		hxlist_node* next_ = n_->m_list_next_;
		n_->m_list_next_ = n_->m_list_prev_;
		n_->m_list_prev_ = next_;
		n_ = next_;
	} while(n_ != &m_head_)
		/* do */;
}

template<typename node_t_, typename deleter_t_>
inline hxconstexpr void hxlist<node_t_, deleter_t_>::splice(
		const_iterator pos_, hxlist& other_) {
	hxassertmsg(&other_ != this, "invalid_splice");
	if(other_.empty()) {
		return;
	}
	hxlist_node* pos_node_ = const_cast<hxlist_node*>(static_cast<const hxlist_node*>(pos_));
	hxlist_node* prev_node_ = pos_node_->m_list_prev_;
	hxlist_node* other_first_ = other_.m_head_.m_list_next_;
	hxlist_node* other_last_ = other_.m_head_.m_list_prev_;
	prev_node_->m_list_next_ = other_first_;
	other_first_->m_list_prev_ = prev_node_;
	other_last_->m_list_next_ = pos_node_;
	pos_node_->m_list_prev_ = other_last_;
	m_size_ += other_.m_size_;
	other_.release_all();
}

template<typename node_t_, typename deleter_t_>
inline hxconstexpr void hxlist<node_t_, deleter_t_>::insert_(
		hxlist_node* prev_, hxlist_node* next_, hxlist_node* ptr_) {
	hxassertmsg(ptr_ != hxnull, "invalid_node");
	ptr_->m_list_prev_ = prev_;
	ptr_->m_list_next_ = next_;
	prev_->m_list_next_ = ptr_;
	next_->m_list_prev_ = ptr_;
	++m_size_;
}

template<typename node_t_, typename deleter_t_>
inline hxconstexpr void hxlist<node_t_, deleter_t_>::extract_(hxlist_node* ptr_) {
	hxassert_hard(m_size_ > 0u && ptr_ != hxnull, "invalid_node");
	ptr_->m_list_prev_->m_list_next_ = ptr_->m_list_next_;
	ptr_->m_list_next_->m_list_prev_ = ptr_->m_list_prev_;
	--m_size_;
}

HX_END_INL_
