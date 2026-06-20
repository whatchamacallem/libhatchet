#pragma once
// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

#ifndef LIBHATCHET_VER
#error Internal. Do not include this file directly.
#endif

#ifndef HX_DOXYGEN_PARSER
HX_BEGIN_INL_

// const_iterator

template<typename node_t_, typename deleter_t_>
inline hxconstexpr auto hxconstexpr_list<node_t_, deleter_t_>::const_iterator::operator++(void)
		-> const_iterator& {
	// Checking for m_sentinel_ is not done for symmetry.
	hxassertmsg(this->m_current_node_ != hxnull, "invalid_iterator");
	m_current_node_ = m_current_node_->m_list_next_;
	return *this;
}

template<typename node_t_, typename deleter_t_>
inline hxconstexpr auto hxconstexpr_list<node_t_, deleter_t_>::const_iterator::operator++(int)
		-> const_iterator {
	const_iterator t_(*this);
	this->operator++();
	return t_;
}

template<typename node_t_, typename deleter_t_>
inline hxconstexpr auto hxconstexpr_list<node_t_, deleter_t_>::const_iterator::operator--(void)
		-> const_iterator& {
	hxassertmsg(this->m_current_node_ != hxnull, "invalid_iterator");
	m_current_node_ = m_current_node_->m_list_prev_;
	return *this;
}

template<typename node_t_, typename deleter_t_>
inline hxconstexpr auto hxconstexpr_list<node_t_, deleter_t_>::const_iterator::operator--(int)
		-> const_iterator {
	const_iterator t_(*this);
	this->operator--();
	return t_;
}

template<typename node_t_, typename deleter_t_>
inline hxconstexpr bool hxconstexpr_list<node_t_, deleter_t_>::const_iterator::operator==(
		const const_iterator& x_) const {
	return m_current_node_ == x_.m_current_node_;
}

#if HX_CPLUSPLUS < 202002L // C++20 defaults != from ==.
template<typename node_t_, typename deleter_t_>
inline hxconstexpr bool hxconstexpr_list<node_t_, deleter_t_>::const_iterator::operator!=(
		const const_iterator& x_) const {
	return m_current_node_ != x_.m_current_node_;
}
#endif

// iterator

template<typename node_t_, typename deleter_t_>
inline hxconstexpr auto hxconstexpr_list<node_t_, deleter_t_>::iterator::operator++(int) -> iterator {
	iterator t_(*this);
	this->const_iterator::operator++();
	return t_;
}

template<typename node_t_, typename deleter_t_>
inline hxconstexpr auto hxconstexpr_list<node_t_, deleter_t_>::iterator::operator--(int) -> iterator {
	iterator t_(*this);
	this->const_iterator::operator--();
	return t_;
}

// hxconstexpr_list

template<typename node_t_, typename deleter_t_>
inline hxconstexpr hxconstexpr_list<node_t_, deleter_t_>::hxconstexpr_list(void) {
	m_size_ = 0u;
	m_sentinel_.m_list_prev_ = &m_sentinel_;
	m_sentinel_.m_list_next_ = &m_sentinel_;
}

template<typename node_t_, typename deleter_t_>
inline hxconstexpr node_t_& hxconstexpr_list<node_t_, deleter_t_>::back(void) {
	hxassert_hard(!this->empty(), "empty_list");
	return *static_cast<node_t_*>(m_sentinel_.m_list_prev_);
}

template<typename node_t_, typename deleter_t_>
inline hxconstexpr const node_t_& hxconstexpr_list<node_t_, deleter_t_>::back(void) const {
	hxassert_hard(!this->empty(), "empty_list");
	return *static_cast<const node_t_*>(m_sentinel_.m_list_prev_);
}

template<typename node_t_, typename deleter_t_>
inline hxconstexpr auto hxconstexpr_list<node_t_, deleter_t_>::begin(void) const -> const_iterator {
	return const_iterator(m_sentinel_.m_list_next_, &m_sentinel_);
}

template<typename node_t_, typename deleter_t_>
inline hxconstexpr auto hxconstexpr_list<node_t_, deleter_t_>::end(void) const -> const_iterator {
	return const_iterator(const_cast<hxconst_list_node*>(&m_sentinel_), &m_sentinel_);
}

template<typename node_t_, typename deleter_t_>
template<typename deleter_override_t_>
inline hxconstexpr void hxconstexpr_list<node_t_, deleter_t_>::clear(
	const deleter_override_t_& deleter_) {
	if(m_size_ != 0u) {
		if(deleter_) {
			hxconst_list_node* n_ = m_sentinel_.m_list_next_;
			while(n_ != &m_sentinel_) {
				hxconst_list_node* next_ = n_->m_list_next_;
				deleter_(static_cast<node_t_*>(n_));
				n_ = next_;
			}
		}
		this->release_all();
	}
}

template<typename node_t_, typename deleter_t_>
template<typename deleter_override_t_>
inline hxconstexpr void hxconstexpr_list<node_t_, deleter_t_>::erase(
		const_iterator pos_, const deleter_override_t_& deleter_) {
	node_t_* ptr_ = static_cast<node_t_*>(pos_.m_current_node_);
	this->extract_(pos_.m_current_node_);
	if(deleter_) {
		deleter_(ptr_);
	}
}

template<typename node_t_, typename deleter_t_>
inline hxconstexpr hxptr<node_t_, deleter_t_> hxconstexpr_list<node_t_, deleter_t_>::extract(
		const_iterator pos_) {
	node_t_* ptr_ = static_cast<node_t_*>(pos_.m_current_node_);
	this->extract_(pos_.m_current_node_);
	return hxptr<node_t_, deleter_t_>(ptr_);
}

template<typename node_t_, typename deleter_t_>
inline hxconstexpr node_t_& hxconstexpr_list<node_t_, deleter_t_>::front(void) {
	hxassert_hard(!this->empty(), "empty_list");
	return *static_cast<node_t_*>(m_sentinel_.m_list_next_);
}

template<typename node_t_, typename deleter_t_>
inline hxconstexpr const node_t_& hxconstexpr_list<node_t_, deleter_t_>::front(void) const {
	hxassert_hard(!this->empty(), "empty_list");
	return *static_cast<const node_t_*>(m_sentinel_.m_list_next_);
}

template<typename node_t_, typename deleter_t_>
template<typename ptr_deleter_t_>
inline hxconstexpr auto hxconstexpr_list<node_t_, deleter_t_>::insert_after(
		const_iterator pos_, hxptr<node_t_, ptr_deleter_t_>&& ptr_) -> iterator {
	return this->insert_after(pos_, ptr_.release());
}

template<typename node_t_, typename deleter_t_>
inline hxconstexpr auto hxconstexpr_list<node_t_, deleter_t_>::insert_after(
		const_iterator pos_, node_t_* ptr_) -> iterator {
	hxconst_list_node* pos_node_ = pos_.m_current_node_;
	this->insert_(pos_node_, pos_node_->m_list_next_, ptr_);
	return iterator(ptr_, &m_sentinel_);
}

template<typename node_t_, typename deleter_t_>
template<typename ptr_deleter_t_>
inline hxconstexpr auto hxconstexpr_list<node_t_, deleter_t_>::insert(
		const_iterator pos_, hxptr<node_t_, ptr_deleter_t_>&& ptr_) -> iterator {
	return this->insert(pos_, ptr_.release());
}

template<typename node_t_, typename deleter_t_>
inline hxconstexpr auto hxconstexpr_list<node_t_, deleter_t_>::insert(
		const_iterator pos_, node_t_* ptr_) -> iterator {
	hxconst_list_node* pos_node_ = pos_.m_current_node_;
	this->insert_(pos_node_->m_list_prev_, pos_node_, ptr_);
	return iterator(ptr_, &m_sentinel_);
}

template<typename node_t_, typename deleter_t_>
inline hxconstexpr hxptr<node_t_, deleter_t_> hxconstexpr_list<node_t_, deleter_t_>::pop_back(void) {
	hxassert_hard(!this->empty(), "empty_list");
	node_t_* ptr_ = static_cast<node_t_*>(m_sentinel_.m_list_prev_);
	this->extract_(ptr_);
	return hxptr<node_t_, deleter_t_>(ptr_);
}

template<typename node_t_, typename deleter_t_>
inline hxconstexpr hxptr<node_t_, deleter_t_> hxconstexpr_list<node_t_, deleter_t_>::pop_front(void) {
	hxassert_hard(!this->empty(), "empty_list");
	node_t_* ptr_ = static_cast<node_t_*>(m_sentinel_.m_list_next_);
	this->extract_(ptr_);
	return hxptr<node_t_, deleter_t_>(ptr_);
}

template<typename node_t_, typename deleter_t_>
template<typename ptr_deleter_t_>
inline hxconstexpr auto hxconstexpr_list<node_t_, deleter_t_>::push_back(
		hxptr<node_t_, ptr_deleter_t_>&& ptr_) -> iterator {
	return this->push_back(ptr_.release());
}

template<typename node_t_, typename deleter_t_>
inline hxconstexpr auto hxconstexpr_list<node_t_, deleter_t_>::push_back(node_t_* ptr_) -> iterator {
	this->insert_(m_sentinel_.m_list_prev_, &m_sentinel_, ptr_);
	return iterator(ptr_, &m_sentinel_);
}

template<typename node_t_, typename deleter_t_>
template<typename ptr_deleter_t_>
inline hxconstexpr auto hxconstexpr_list<node_t_, deleter_t_>::push_front(
		hxptr<node_t_, ptr_deleter_t_>&& ptr_) -> iterator {
	return this->push_front(ptr_.release());
}

template<typename node_t_, typename deleter_t_>
inline hxconstexpr auto hxconstexpr_list<node_t_, deleter_t_>::push_front(node_t_* ptr_) -> iterator {
	this->insert_(&m_sentinel_, m_sentinel_.m_list_next_, ptr_);
	return iterator(ptr_, &m_sentinel_);
}

template<typename node_t_, typename deleter_t_>
inline hxconstexpr void hxconstexpr_list<node_t_, deleter_t_>::release_all(void) {
	m_sentinel_.m_list_prev_ = &m_sentinel_;
	m_sentinel_.m_list_next_ = &m_sentinel_;
	m_size_ = 0u;
}

template<typename node_t_, typename deleter_t_>
template<typename predicate_t_>
inline hxconstexpr size_t hxconstexpr_list<node_t_, deleter_t_>::remove_if(predicate_t_ predicate_) {
	size_t count_ = 0u;
	hxconst_list_node* n_ = m_sentinel_.m_list_next_;
	while(n_ != &m_sentinel_) {
		hxconst_list_node* next_ = n_->m_list_next_;
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
inline hxconstexpr void hxconstexpr_list<node_t_, deleter_t_>::reverse(void) {
	hxconst_list_node* n_ = &m_sentinel_;
	do {
		hxconst_list_node* next_ = n_->m_list_next_;
		n_->m_list_next_ = n_->m_list_prev_;
		n_->m_list_prev_ = next_;
		n_ = next_;
	} while(n_ != &m_sentinel_)
		/* do */;
}

template<typename node_t_, typename deleter_t_>
inline hxconstexpr void hxconstexpr_list<node_t_, deleter_t_>::splice(
		const_iterator pos_, hxconstexpr_list& other_) {
	hxassertmsg(&other_ != this, "invalid_splice self");
	hxassertmsg(pos_.m_current_node_ != hxnull, "invalid_iterator");
	if(other_.empty()) {
		return;
	}
	hxconst_list_node* pos_node_ = pos_.m_current_node_;
	hxconst_list_node* prev_node_ = pos_node_->m_list_prev_;
	hxconst_list_node* other_first_ = other_.m_sentinel_.m_list_next_;
	hxconst_list_node* other_last_ = other_.m_sentinel_.m_list_prev_;
	prev_node_->m_list_next_ = other_first_;
	other_first_->m_list_prev_ = prev_node_;
	other_last_->m_list_next_ = pos_node_;
	pos_node_->m_list_prev_ = other_last_;
	m_size_ += other_.m_size_;
	other_.release_all();
}

template<typename node_t_, typename deleter_t_>
inline hxconstexpr void hxconstexpr_list<node_t_, deleter_t_>::insert_(
		hxconst_list_node* prev_, hxconst_list_node* next_, hxconst_list_node* ptr_) {
	hxassertmsg(ptr_ != hxnull, "invalid_node");
	ptr_->m_list_prev_ = prev_;
	ptr_->m_list_next_ = next_;
	prev_->m_list_next_ = ptr_;
	next_->m_list_prev_ = ptr_;
	++m_size_;
}

template<typename node_t_, typename deleter_t_>
inline hxconstexpr void hxconstexpr_list<node_t_, deleter_t_>::extract_(hxconst_list_node* ptr_) {
	hxassert_hard(!this->empty(), "empty_list");
	hxassertmsg(ptr_ != hxnull, "invalid_node");
	ptr_->m_list_prev_->m_list_next_ = ptr_->m_list_next_;
	ptr_->m_list_next_->m_list_prev_ = ptr_->m_list_prev_;
	--m_size_;
}

HX_END_INL_
#endif // HX_DOXYGEN_PARSER
