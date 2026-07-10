#pragma once
// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

#ifndef LIBHATCHET_VER
#error Internal. Do not include this file directly.
#endif

#ifndef HX_DOXYGEN_PARSER
HX_BEGIN_INL_

// A sentinel node eliminates checking for null when empty. And XOR linking
// stores "prev XOR next" in a single pointer, halving per-node pointer memory
// at the cost of requiring the previous node's address to traverse.

// const_iterator

template<typename T_, typename deleter_t_>
inline auto hxlist<T_, deleter_t_>::const_iterator::operator++(void)
		-> const_iterator& {
	// Checking for m_sentinel_ is not done for symmetry.
	hxassertmsg(this->m_sentinel_ != hxnull, "invalid_iterator");
	hxlist_node* next_ = reinterpret_cast<hxlist_node*>(
		reinterpret_cast<intptr_t>(m_prev_) ^ m_current_node_->m_list_link_);
	m_prev_ = m_current_node_;
	m_current_node_ = next_;
	return *this;
}

template<typename T_, typename deleter_t_>
inline auto hxlist<T_, deleter_t_>::const_iterator::operator++(int)
		-> const_iterator {
	const_iterator t_(*this);
	this->operator++();
	return t_;
}

template<typename T_, typename deleter_t_>
inline auto hxlist<T_, deleter_t_>::const_iterator::operator--(void)
		-> const_iterator& {
	hxassertmsg(this->m_sentinel_ != hxnull, "invalid_iterator");
	hxlist_node* prev_of_prev_ = reinterpret_cast<hxlist_node*>(
		reinterpret_cast<intptr_t>(m_current_node_) ^ m_prev_->m_list_link_);
	m_current_node_ = m_prev_;
	m_prev_ = prev_of_prev_;
	return *this;
}

template<typename T_, typename deleter_t_>
inline auto hxlist<T_, deleter_t_>::const_iterator::operator--(int)
		-> const_iterator {
	const_iterator t_(*this);
	this->operator--();
	return t_;
}

template<typename T_, typename deleter_t_>
inline bool hxlist<T_, deleter_t_>::const_iterator::operator==(const const_iterator& x_) const {
	return m_current_node_ == x_.m_current_node_;
}

// iterator

template<typename T_, typename deleter_t_>
inline auto hxlist<T_, deleter_t_>::iterator::operator++(int) -> iterator {
	iterator t_(*this);
	this->const_iterator::operator++();
	return t_;
}

template<typename T_, typename deleter_t_>
inline auto hxlist<T_, deleter_t_>::iterator::operator--(int) -> iterator {
	iterator t_(*this);
	this->const_iterator::operator--();
	return t_;
}

// hxlist

template<typename T_, typename deleter_t_>
inline hxlist<T_, deleter_t_>::hxlist(deleter_t_ deleter_)
		: deleter_t_(deleter_), m_size_(0), m_tail_(&m_sentinel_) {
}

template<typename T_, typename deleter_t_>
inline T_& hxlist<T_, deleter_t_>::back(void) {
	hxassert_hard(!this->empty(), "empty_list");
	return *static_cast<T_*>(m_tail_);
}

template<typename T_, typename deleter_t_>
inline const T_& hxlist<T_, deleter_t_>::back(void) const {
	hxassert_hard(!this->empty(), "empty_list");
	return *static_cast<const T_*>(m_tail_);
}

template<typename T_, typename deleter_t_>
inline auto hxlist<T_, deleter_t_>::begin(void) -> iterator {
	hxlist_node* first_ = reinterpret_cast<hxlist_node*>(
		reinterpret_cast<intptr_t>(m_tail_) ^ m_sentinel_.m_list_link_);
	return iterator(&m_sentinel_, first_, &m_sentinel_);
}

template<typename T_, typename deleter_t_>
inline auto hxlist<T_, deleter_t_>::begin(void) const -> const_iterator {
	hxlist_node* first_ = reinterpret_cast<hxlist_node*>(
		reinterpret_cast<intptr_t>(m_tail_) ^ m_sentinel_.m_list_link_);
	return const_iterator(const_cast<hxlist_node*>(&m_sentinel_), first_, &m_sentinel_);
}

template<typename T_, typename deleter_t_>
inline auto hxlist<T_, deleter_t_>::end(void) -> iterator {
	return iterator(m_tail_, &m_sentinel_, &m_sentinel_);
}

template<typename T_, typename deleter_t_>
inline auto hxlist<T_, deleter_t_>::end(void) const -> const_iterator {
	return const_iterator(m_tail_, const_cast<hxlist_node*>(&m_sentinel_), &m_sentinel_);
}

template<typename T_, typename deleter_t_>
template<typename deleter_u_>
inline void hxlist<T_, deleter_t_>::clear(deleter_u_&& deleter_) noexcept {
	if(m_size_ != 0) {
		hxlist_node* prev_ = &m_sentinel_;
		hxlist_node* current_ = reinterpret_cast<hxlist_node*>(
			reinterpret_cast<intptr_t>(m_tail_) ^ m_sentinel_.m_list_link_);
		this->release_all();
		if(deleter_) {
			while(current_ != &m_sentinel_) {
				hxlist_node* const next_ = reinterpret_cast<hxlist_node*>(
					reinterpret_cast<intptr_t>(prev_) ^ current_->m_list_link_);
				deleter_(static_cast<T_*>(current_));
				prev_ = current_;
				current_ = next_;
			}
		}
	}
}

template<typename T_, typename deleter_t_>
template<typename deleter_u_>
inline void hxlist<T_, deleter_t_>::erase(const_iterator pos_, deleter_u_&& deleter_) noexcept {
	T_* ptr_ = static_cast<T_*>(pos_.m_current_node_);
	this->extract_(pos_.m_prev_, pos_.m_current_node_);
	if(deleter_) {
		deleter_(ptr_);
	}
}

template<typename T_, typename deleter_t_>
inline void hxlist<T_, deleter_t_>::erase(const_iterator pos_) noexcept {
	this->erase(pos_, static_cast<deleter_t_&>(*this));
}

template<typename T_, typename deleter_t_>
inline hxptr<T_, deleter_t_> hxlist<T_, deleter_t_>::extract(const_iterator pos_) {
	T_* const node_ = static_cast<T_*>(pos_.m_current_node_);
	this->extract_(pos_.m_prev_, pos_.m_current_node_);
	return hxptr<T_, deleter_t_>(node_, static_cast<deleter_t_&>(*this));
}

template<typename T_, typename deleter_t_>
template<typename callable_t_>
inline auto hxlist<T_, deleter_t_>::find_if(callable_t_&& callable_) const -> const_iterator {
	const const_iterator end_ = this->end();
	for(const_iterator it_ = this->begin(); it_ != end_; ++it_) {
		if(hxforward<callable_t_>(callable_)(*it_)) {
			return it_;
		}
	}
	return end_;
}

template<typename T_, typename deleter_t_>
template<typename callable_t_>
inline auto hxlist<T_, deleter_t_>::find_if(callable_t_&& callable_) -> iterator {
	const iterator end_ = this->end();
	for(iterator it_ = this->begin(); it_ != end_; ++it_) {
		if(hxforward<callable_t_>(callable_)(*it_)) {
			return it_;
		}
	}
	return end_;
}

template<typename T_, typename deleter_t_>
template<typename callable_t_>
inline void hxlist<T_, deleter_t_>::for_each(callable_t_&& callable_) const {
	const const_iterator end_ = this->end();
	for(const_iterator it_ = this->begin(); it_ != end_; ++it_) {
		hxforward<callable_t_>(callable_)(*it_);
	}
}

template<typename T_, typename deleter_t_>
template<typename callable_t_>
inline void hxlist<T_, deleter_t_>::for_each(callable_t_&& callable_) {
	const iterator end_ = this->end();
	for(iterator it_ = this->begin(); it_ != end_; ++it_) {
		hxforward<callable_t_>(callable_)(*it_);
	}
}

template<typename T_, typename deleter_t_>
inline const deleter_t_& hxlist<T_, deleter_t_>::deleter(void) const {
	return static_cast<const deleter_t_&>(*this);
}

template<typename T_, typename deleter_t_>
inline deleter_t_& hxlist<T_, deleter_t_>::deleter(void) {
	return static_cast<deleter_t_&>(*this);
}

template<typename T_, typename deleter_t_>
inline T_& hxlist<T_, deleter_t_>::front(void) {
	hxassert_hard(!this->empty(), "empty_list");
	hxlist_node* first_ = reinterpret_cast<hxlist_node*>(
		reinterpret_cast<intptr_t>(m_tail_) ^ m_sentinel_.m_list_link_);
	return *static_cast<T_*>(first_);
}

template<typename T_, typename deleter_t_>
inline const T_& hxlist<T_, deleter_t_>::front(void) const {
	hxassert_hard(!this->empty(), "empty_list");
	const hxlist_node* first_ = reinterpret_cast<hxlist_node*>(
		reinterpret_cast<intptr_t>(m_tail_) ^ m_sentinel_.m_list_link_);
	return *static_cast<const T_*>(first_);
}

template<typename T_, typename deleter_t_>
template<typename deleter_u_>
inline auto hxlist<T_, deleter_t_>::insert_after(
		const_iterator pos_, hxptr<T_, deleter_u_>&& ptr_) -> iterator {
	return this->insert_after(pos_, ptr_.release());
}

template<typename T_, typename deleter_t_>
inline auto hxlist<T_, deleter_t_>::insert_after(const_iterator pos_, T_* ptr_) -> iterator {
	hxlist_node* prev_ = pos_.m_current_node_;
	hxlist_node* next_ = reinterpret_cast<hxlist_node*>(
		reinterpret_cast<intptr_t>(pos_.m_prev_) ^ prev_->m_list_link_);
	this->insert_(prev_, next_, ptr_);
	return iterator(prev_, ptr_, &m_sentinel_);
}

template<typename T_, typename deleter_t_>
template<typename deleter_u_>
inline auto hxlist<T_, deleter_t_>::insert(
		const_iterator pos_, hxptr<T_, deleter_u_>&& ptr_) -> iterator {
	return this->insert(pos_, ptr_.release());
}

template<typename T_, typename deleter_t_>
inline auto hxlist<T_, deleter_t_>::insert(
		const_iterator pos_, T_* ptr_) -> iterator {
	hxlist_node* prev_ = pos_.m_prev_;
	this->insert_(prev_, pos_.m_current_node_, ptr_);
	return iterator(prev_, ptr_, &m_sentinel_);
}

template<typename T_, typename deleter_t_>
inline hxptr<T_, deleter_t_> hxlist<T_, deleter_t_>::pop_back(void) {
	hxassert_hard(!this->empty(), "empty_list");
	hxlist_node* const node_ = m_tail_;
	hxlist_node* const prev_ = reinterpret_cast<hxlist_node*>(
		reinterpret_cast<intptr_t>(&m_sentinel_) ^ node_->m_list_link_);
	prev_->m_list_link_ ^=
		reinterpret_cast<intptr_t>(node_) ^ reinterpret_cast<intptr_t>(&m_sentinel_);
	m_sentinel_.m_list_link_ ^=
		reinterpret_cast<intptr_t>(node_) ^ reinterpret_cast<intptr_t>(prev_);
	m_tail_ = prev_;
	--m_size_;
	return hxptr<T_, deleter_t_>(static_cast<T_*>(node_), static_cast<deleter_t_&>(*this));
}

template<typename T_, typename deleter_t_>
inline hxptr<T_, deleter_t_> hxlist<T_, deleter_t_>::pop_front(void) {
	hxassert_hard(!this->empty(), "empty_list");
	hxlist_node* node_ = reinterpret_cast<hxlist_node*>(
		reinterpret_cast<intptr_t>(m_tail_) ^ m_sentinel_.m_list_link_);
	this->extract_(&m_sentinel_, node_);
	return hxptr<T_, deleter_t_>(static_cast<T_*>(node_), static_cast<deleter_t_&>(*this));
}

template<typename T_, typename deleter_t_>
template<typename deleter_u_>
inline auto hxlist<T_, deleter_t_>::push_back(hxptr<T_, deleter_u_>&& ptr_) -> iterator {
	return this->push_back(ptr_.release());
}

template<typename T_, typename deleter_t_>
inline auto hxlist<T_, deleter_t_>::push_back(T_* ptr_) -> iterator {
	hxassert_hard(ptr_ != hxnull, "invalid_node");
	hxlist_node* prev_ = m_tail_;
	this->insert_(prev_, &m_sentinel_, ptr_);
	return iterator(prev_, ptr_, &m_sentinel_);
}

template<typename T_, typename deleter_t_>
template<typename deleter_u_>
inline auto hxlist<T_, deleter_t_>::push_front(hxptr<T_, deleter_u_>&& ptr_) -> iterator {
	return this->push_front(ptr_.release());
}

template<typename T_, typename deleter_t_>
inline auto hxlist<T_, deleter_t_>::push_front(T_* ptr_) -> iterator {
	hxassert_hard(ptr_ != hxnull, "invalid_node");
	hxlist_node* first_ = reinterpret_cast<hxlist_node*>(
		reinterpret_cast<intptr_t>(m_tail_) ^ m_sentinel_.m_list_link_);
	this->insert_(&m_sentinel_, first_, ptr_);
	return iterator(&m_sentinel_, ptr_, &m_sentinel_);
}

template<typename T_, typename deleter_t_>
inline void hxlist<T_, deleter_t_>::release_all(void) {
	m_sentinel_.m_list_link_ = 0;
	m_tail_ = &m_sentinel_;
	m_size_ = 0;
}

template<typename T_, typename deleter_t_>
template<typename callable_t_, typename deleter_u_>
inline hxsize_t hxlist<T_, deleter_t_>::remove_if(callable_t_&& callable_,
		deleter_u_&& deleter_) noexcept {
	hxsize_t size_ = m_size_;
	hxlist_node* prev_ = &m_sentinel_;
	hxlist_node* current_ = reinterpret_cast<hxlist_node*>(
		reinterpret_cast<intptr_t>(m_tail_) ^ m_sentinel_.m_list_link_);
	while(current_ != &m_sentinel_) {
		hxlist_node* const next_ = reinterpret_cast<hxlist_node*>(
			reinterpret_cast<intptr_t>(prev_) ^ current_->m_list_link_);
		T_* const node_ = static_cast<T_*>(current_);
		if(hxforward<callable_t_>(callable_)(*node_)) {
			prev_->m_list_link_ ^=
				reinterpret_cast<intptr_t>(current_) ^ reinterpret_cast<intptr_t>(next_);
			next_->m_list_link_ ^=
				reinterpret_cast<intptr_t>(current_) ^ reinterpret_cast<intptr_t>(prev_);
			--size_;
			if(deleter_) {
				deleter_(node_);
			}
		} else {
			prev_ = current_;
		}
		current_ = next_;
	}
	m_tail_ = prev_;
	const hxsize_t count_ = m_size_ - size_;
	m_size_ = size_;
	return count_;
}

template<typename T_, typename deleter_t_>
template<typename callable_t_>
inline hxsize_t hxlist<T_, deleter_t_>::remove_if(callable_t_&& callable_) noexcept {
	return this->remove_if(hxforward<callable_t_>(callable_), static_cast<const deleter_t_&>(*this));
}

template<typename T_, typename deleter_t_>
inline void hxlist<T_, deleter_t_>::reverse(void) {
	// Nodes are unchanged by reversal since XOR is commutative.
	m_tail_ = reinterpret_cast<hxlist_node*>(
		reinterpret_cast<intptr_t>(m_tail_) ^ m_sentinel_.m_list_link_);
}

template<typename T_, typename deleter_t_>
inline void hxlist<T_, deleter_t_>::splice(
		const_iterator pos_, hxlist& other_) {
	hxassertmsg(&other_ != this, "invalid_splice");
	if(other_.empty()) {
		return;
	}
	hxlist_node* prev_node_ = pos_.m_prev_;
	hxlist_node* pos_node_ = pos_.m_current_node_;
	hxlist_node* other_first_ = reinterpret_cast<hxlist_node*>(
		reinterpret_cast<intptr_t>(other_.m_tail_) ^ other_.m_sentinel_.m_list_link_);
	hxlist_node* other_last_ = other_.m_tail_;
	prev_node_->m_list_link_ ^=
		reinterpret_cast<intptr_t>(pos_node_) ^ reinterpret_cast<intptr_t>(other_first_);
	other_first_->m_list_link_ ^=
		reinterpret_cast<intptr_t>(&other_.m_sentinel_) ^ reinterpret_cast<intptr_t>(prev_node_);
	other_last_->m_list_link_ ^=
		reinterpret_cast<intptr_t>(&other_.m_sentinel_) ^ reinterpret_cast<intptr_t>(pos_node_);
	pos_node_->m_list_link_ ^=
		reinterpret_cast<intptr_t>(prev_node_) ^ reinterpret_cast<intptr_t>(other_last_);
	if(pos_node_ == &m_sentinel_) {
		m_tail_ = other_last_;
	}
	m_size_ += other_.m_size_;
	other_.release_all();
}

template<typename T_, typename deleter_t_>
inline void hxlist<T_, deleter_t_>::insert_(
		hxlist_node* prev_, hxlist_node* next_, hxlist_node* ptr_) {
	hxassertmsg(ptr_ != hxnull, "invalid_node");
	ptr_->m_list_link_ = reinterpret_cast<intptr_t>(prev_) ^ reinterpret_cast<intptr_t>(next_);
	prev_->m_list_link_ ^= reinterpret_cast<intptr_t>(next_) ^ reinterpret_cast<intptr_t>(ptr_);
	next_->m_list_link_ ^= reinterpret_cast<intptr_t>(prev_) ^ reinterpret_cast<intptr_t>(ptr_);
	// This should get hoisted out of an insertion loop.
	if(next_ == &m_sentinel_) {
		m_tail_ = ptr_;
	}
	++m_size_;
}

template<typename T_, typename deleter_t_>
inline void hxlist<T_, deleter_t_>::extract_(hxlist_node* prev_, hxlist_node* ptr_) {
	hxassert_hard(!this->empty(), "empty_list");
	hxassertmsg(ptr_ != hxnull, "invalid_node");
	hxlist_node* next_ = reinterpret_cast<hxlist_node*>(
		reinterpret_cast<intptr_t>(prev_) ^ ptr_->m_list_link_);
	prev_->m_list_link_ ^= reinterpret_cast<intptr_t>(ptr_) ^ reinterpret_cast<intptr_t>(next_);
	next_->m_list_link_ ^= reinterpret_cast<intptr_t>(ptr_) ^ reinterpret_cast<intptr_t>(prev_);
	// This should get hoisted out of a deletion loop.
	if(next_ == &m_sentinel_) {
		m_tail_ = prev_;
	}
	--m_size_;
}

HX_END_INL_
#endif // HX_DOXYGEN_PARSER
