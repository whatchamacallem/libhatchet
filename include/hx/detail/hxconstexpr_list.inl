#pragma once
// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

#ifndef LIBHATCHET_VER
#error Internal. Do not include this file directly.
#endif

#ifndef HX_DOXYGEN_PARSER
HX_INL_BEGIN_

// const_iterator

template<hxconstexpr_list_concept_ T_, typename deleter_t_>
hxinline hxconstexpr auto hxconstexpr_list<T_, deleter_t_>::const_iterator::operator++(void)
		-> const_iterator& {
	// Checking for m_sentinel_ is not done for symmetry.
	hxassertmsg(this->m_current_node_ != hxnull, "bad_iter");
	m_current_node_ = m_current_node_->m_list_next_;
	return *this;
}

template<hxconstexpr_list_concept_ T_, typename deleter_t_>
hxinline hxattr_flatten hxconstexpr auto hxconstexpr_list<T_, deleter_t_>::const_iterator::operator++(int)
		-> const_iterator {
	const_iterator t_(*this);
	this->operator++();
	return t_;
}

template<hxconstexpr_list_concept_ T_, typename deleter_t_>
hxinline hxconstexpr auto hxconstexpr_list<T_, deleter_t_>::const_iterator::operator--(void)
		-> const_iterator& {
	hxassertmsg(this->m_current_node_ != hxnull, "bad_iter");
	m_current_node_ = m_current_node_->m_list_prev_;
	return *this;
}

template<hxconstexpr_list_concept_ T_, typename deleter_t_>
hxinline hxattr_flatten hxconstexpr auto hxconstexpr_list<T_, deleter_t_>::const_iterator::operator--(int)
		-> const_iterator {
	const_iterator t_(*this);
	this->operator--();
	return t_;
}

template<hxconstexpr_list_concept_ T_, typename deleter_t_>
hxinline hxattr_flatten hxconstexpr bool hxconstexpr_list<T_, deleter_t_>::const_iterator::operator==(
		const const_iterator& x_) const {
	return m_current_node_ == x_.m_current_node_;
}

// iterator

template<hxconstexpr_list_concept_ T_, typename deleter_t_>
hxinline hxattr_flatten hxconstexpr auto hxconstexpr_list<T_, deleter_t_>::iterator::operator++(int) -> iterator {
	iterator t_(*this);
	this->const_iterator::operator++();
	return t_;
}

template<hxconstexpr_list_concept_ T_, typename deleter_t_>
hxinline hxattr_flatten hxconstexpr auto hxconstexpr_list<T_, deleter_t_>::iterator::operator--(int) -> iterator {
	iterator t_(*this);
	this->const_iterator::operator--();
	return t_;
}

// hxconstexpr_list

template<hxconstexpr_list_concept_ T_, typename deleter_t_>
hxinline hxattr_flatten hxconstexpr hxconstexpr_list<T_, deleter_t_>::hxconstexpr_list(deleter_t_ deleter_)
		: deleter_t_(hxmove(deleter_)) {
	m_size_ = 0;
	m_sentinel_.m_list_prev_ = &m_sentinel_;
	m_sentinel_.m_list_next_ = &m_sentinel_;
}

template<hxconstexpr_list_concept_ T_, typename deleter_t_>
hxinline hxattr_flatten hxconstexpr bool hxconstexpr_list<T_, deleter_t_>::operator==(
		const hxconstexpr_list& x_) const {
	return hxequal_range(*this, x_);
}

template<hxconstexpr_list_concept_ T_, typename deleter_t_>
hxinline hxattr_flatten hxconstexpr bool hxconstexpr_list<T_, deleter_t_>::operator<(
		const hxconstexpr_list& x_) const {
	return hxless_range(*this, x_);
}

template<hxconstexpr_list_concept_ T_, typename deleter_t_>
template<hxrange_concept_ range_t_, hxenable_if_t<!hxis_lvalue_reference<range_t_>(), int> >
hxinline hxattr_flatten hxconstexpr void hxconstexpr_list<T_, deleter_t_>::add_range(range_t_&& range_) noexcept {
	hxrestrict_t<decltype(range_.begin())> it_(range_.begin());
	for(const auto end_ = range_.end(); it_ != end_; ++it_) {
		this->push_back(&*it_);
	}
}

template<hxconstexpr_list_concept_ T_, typename deleter_t_>
hxinline hxconstexpr const T_& hxconstexpr_list<T_, deleter_t_>::back(void) const {
	hxassert_hard(!this->empty(), "list_empty");
	return *static_cast<const T_*>(m_sentinel_.m_list_prev_);
}

template<hxconstexpr_list_concept_ T_, typename deleter_t_>
hxinline hxconstexpr T_& hxconstexpr_list<T_, deleter_t_>::back(void) {
	hxassert_hard(!this->empty(), "list_empty");
	return *static_cast<T_*>(m_sentinel_.m_list_prev_);
}

template<hxconstexpr_list_concept_ T_, typename deleter_t_>
hxinline hxattr_flatten hxconstexpr auto hxconstexpr_list<T_, deleter_t_>::begin(void) const -> const_iterator {
	return const_iterator(m_sentinel_.m_list_next_, &m_sentinel_);
}

template<hxconstexpr_list_concept_ T_, typename deleter_t_>
hxinline hxattr_flatten hxconstexpr auto hxconstexpr_list<T_, deleter_t_>::end(void) const -> const_iterator {
	return const_iterator(const_cast<hxconstexpr_list_node*>(&m_sentinel_), &m_sentinel_);
}

template<hxconstexpr_list_concept_ T_, typename deleter_t_>
hxinline hxattr_flatten hxconstexpr bool hxconstexpr_list<T_, deleter_t_>::empty(void) const {
	return m_size_ == 0;
}

template<hxconstexpr_list_concept_ T_, typename deleter_t_>
template<typename deleter_u_>
hxinline hxattr_flatten hxconstexpr void hxconstexpr_list<T_, deleter_t_>::clear(deleter_u_&& deleter_) noexcept {
	if(m_size_ != 0) {
		if(deleter_) {
			hxconstexpr_list_node* node_ = m_sentinel_.m_list_next_;
			while(node_ != &m_sentinel_) {
				hxconstexpr_list_node* next_ = node_->m_list_next_;
				deleter_(static_cast<T_*>(node_));
				node_ = next_;
			}
		}
		this->release_all();
	}
}

template<hxconstexpr_list_concept_ T_, typename deleter_t_>
template<typename deleter_u_>
hxinline hxattr_flatten hxconstexpr void hxconstexpr_list<T_, deleter_t_>::erase(
		const_iterator it_, deleter_u_&& deleter_) noexcept {
	T_* ptr_ = static_cast<T_*>(it_.m_current_node_);
	this->extract_(it_.m_current_node_);
	if(deleter_) {
		deleter_(ptr_);
	}
}

template<hxconstexpr_list_concept_ T_, typename deleter_t_>
hxinline hxattr_flatten hxconstexpr void hxconstexpr_list<T_, deleter_t_>::erase(const_iterator it_) noexcept {
	this->erase(it_, static_cast<const deleter_t_&>(*this));
}

template<hxconstexpr_list_concept_ T_, typename deleter_t_>
hxinline hxattr_flatten hxconstexpr hxptr<T_, deleter_t_> hxconstexpr_list<T_, deleter_t_>::extract(const_iterator it_) {
	T_* const ptr_ = static_cast<T_*>(it_.m_current_node_);
	this->extract_(it_.m_current_node_);
	return hxptr<T_, deleter_t_>(ptr_, static_cast<deleter_t_&>(*this));
}

template<hxconstexpr_list_concept_ T_, typename deleter_t_>
template<typename callable_t_>
hxinline hxattr_flatten hxconstexpr auto hxconstexpr_list<T_, deleter_t_>::find_if(
		callable_t_&& callable_) const -> const_iterator {
	const const_iterator end_ = this->end();
	for(const_iterator it_ = this->begin(); it_ != end_; ++it_) {
		if(hxforward<callable_t_>(callable_)(*it_)) {
			return it_;
		}
	}
	return end_;
}

template<hxconstexpr_list_concept_ T_, typename deleter_t_>
template<typename callable_t_>
hxinline hxattr_flatten hxconstexpr auto hxconstexpr_list<T_, deleter_t_>::find_if(callable_t_&& callable_) -> iterator {
	const iterator end_ = this->end();
	for(iterator it_ = this->begin(); it_ != end_; ++it_) {
		if(hxforward<callable_t_>(callable_)(*it_)) {
			return it_;
		}
	}
	return end_;
}

template<hxconstexpr_list_concept_ T_, typename deleter_t_>
template<typename callable_t_>
hxinline hxattr_flatten hxconstexpr void hxconstexpr_list<T_, deleter_t_>::for_each(callable_t_&& callable_) const {
	const hxconstexpr_list_node* node_ = m_sentinel_.m_list_next_;
	while(node_ != &m_sentinel_) {
		hxforward<callable_t_>(callable_)(*static_cast<const T_*>(node_));
		node_ = node_->m_list_next_;
	}
}

template<hxconstexpr_list_concept_ T_, typename deleter_t_>
template<typename callable_t_>
hxinline hxattr_flatten hxconstexpr void hxconstexpr_list<T_, deleter_t_>::for_each(callable_t_&& callable_) {
	hxconstexpr_list_node* node_ = m_sentinel_.m_list_next_;
	while(node_ != &m_sentinel_) {
		hxforward<callable_t_>(callable_)(*static_cast<T_*>(node_));
		node_ = node_->m_list_next_;
	}
}

template<hxconstexpr_list_concept_ T_, typename deleter_t_>
hxinline hxconstexpr const deleter_t_& hxconstexpr_list<T_, deleter_t_>::deleter(void) const {
	return static_cast<const deleter_t_&>(*this);
}

template<hxconstexpr_list_concept_ T_, typename deleter_t_>
hxinline hxconstexpr deleter_t_& hxconstexpr_list<T_, deleter_t_>::deleter(void) {
	return static_cast<deleter_t_&>(*this);
}

template<hxconstexpr_list_concept_ T_, typename deleter_t_>
hxinline hxconstexpr const T_& hxconstexpr_list<T_, deleter_t_>::front(void) const {
	hxassert_hard(!this->empty(), "list_empty");
	return *static_cast<const T_*>(m_sentinel_.m_list_next_);
}

template<hxconstexpr_list_concept_ T_, typename deleter_t_>
hxinline hxconstexpr T_& hxconstexpr_list<T_, deleter_t_>::front(void) {
	hxassert_hard(!this->empty(), "list_empty");
	return *static_cast<T_*>(m_sentinel_.m_list_next_);
}

template<hxconstexpr_list_concept_ T_, typename deleter_t_>
template<typename deleter_u_>
hxinline hxattr_flatten hxconstexpr auto hxconstexpr_list<T_, deleter_t_>::insert_after(
		const_iterator it_, hxptr<T_, deleter_u_>&& ptr_) -> iterator {
	return this->insert_after(it_, ptr_.release());
}

template<hxconstexpr_list_concept_ T_, typename deleter_t_>
hxinline hxattr_flatten hxconstexpr auto hxconstexpr_list<T_, deleter_t_>::insert_after(
		const_iterator it_, T_* ptr_) -> iterator {
	hxconstexpr_list_node* pos_node_ = it_.m_current_node_;
	this->insert_(pos_node_, pos_node_->m_list_next_, ptr_);
	return iterator(ptr_, &m_sentinel_);
}

template<hxconstexpr_list_concept_ T_, typename deleter_t_>
template<typename deleter_u_>
hxinline hxattr_flatten hxconstexpr auto hxconstexpr_list<T_, deleter_t_>::insert(
		const_iterator it_, hxptr<T_, deleter_u_>&& ptr_) -> iterator {
	return this->insert(it_, ptr_.release());
}

template<hxconstexpr_list_concept_ T_, typename deleter_t_>
hxinline hxattr_flatten hxconstexpr auto hxconstexpr_list<T_, deleter_t_>::insert(
		const_iterator it_, T_* ptr_) -> iterator {
	hxconstexpr_list_node* pos_node_ = it_.m_current_node_;
	this->insert_(pos_node_->m_list_prev_, pos_node_, ptr_);
	return iterator(ptr_, &m_sentinel_);
}

template<hxconstexpr_list_concept_ T_, typename deleter_t_>
hxinline hxattr_flatten hxconstexpr hxptr<T_, deleter_t_> hxconstexpr_list<T_, deleter_t_>::pop_back(void) {
	hxassert_hard(!this->empty(), "list_empty");
	T_* const ptr_ = static_cast<T_*>(m_sentinel_.m_list_prev_);
	hxconstexpr_list_node* const prev_ = ptr_->m_list_prev_;
	prev_->m_list_next_ = &m_sentinel_;
	m_sentinel_.m_list_prev_ = prev_;
	--m_size_;
	return hxptr<T_, deleter_t_>(ptr_, static_cast<deleter_t_&>(*this));
}

template<hxconstexpr_list_concept_ T_, typename deleter_t_>
hxinline hxattr_flatten hxconstexpr hxptr<T_, deleter_t_> hxconstexpr_list<T_, deleter_t_>::pop_front(void) {
	hxassert_hard(!this->empty(), "list_empty");
	T_* const ptr_ = static_cast<T_*>(m_sentinel_.m_list_next_);
	hxconstexpr_list_node* const next_ = ptr_->m_list_next_;
	m_sentinel_.m_list_next_ = next_;
	next_->m_list_prev_ = &m_sentinel_;
	--m_size_;
	return hxptr<T_, deleter_t_>(ptr_, static_cast<deleter_t_&>(*this));
}

template<hxconstexpr_list_concept_ T_, typename deleter_t_>
template<typename deleter_u_>
hxinline hxattr_flatten hxconstexpr auto hxconstexpr_list<T_, deleter_t_>::push_back(
		hxptr<T_, deleter_u_>&& ptr_) -> iterator {
	return this->push_back(ptr_.release());
}

template<hxconstexpr_list_concept_ T_, typename deleter_t_>
hxinline hxattr_flatten hxconstexpr auto hxconstexpr_list<T_, deleter_t_>::push_back(T_* ptr_) -> iterator {
	this->insert_(m_sentinel_.m_list_prev_, &m_sentinel_, ptr_);
	return iterator(ptr_, &m_sentinel_);
}

template<hxconstexpr_list_concept_ T_, typename deleter_t_>
template<typename deleter_u_>
hxinline hxattr_flatten hxconstexpr auto hxconstexpr_list<T_, deleter_t_>::push_front(
		hxptr<T_, deleter_u_>&& ptr_) -> iterator {
	return this->push_front(ptr_.release());
}

template<hxconstexpr_list_concept_ T_, typename deleter_t_>
hxinline hxattr_flatten hxconstexpr auto hxconstexpr_list<T_, deleter_t_>::push_front(T_* ptr_) -> iterator {
	this->insert_(&m_sentinel_, m_sentinel_.m_list_next_, ptr_);
	return iterator(ptr_, &m_sentinel_);
}

template<hxconstexpr_list_concept_ T_, typename deleter_t_>
hxinline hxconstexpr void hxconstexpr_list<T_, deleter_t_>::release_all(void) {
	m_sentinel_.m_list_prev_ = &m_sentinel_;
	m_sentinel_.m_list_next_ = &m_sentinel_;
	m_size_ = 0;
}

template<hxconstexpr_list_concept_ T_, typename deleter_t_>
template<typename callable_t_, typename deleter_u_>
hxattr_flatten hxconstexpr hxsize_t hxconstexpr_list<T_, deleter_t_>::remove_if(callable_t_&& callable_,
		deleter_u_&& deleter_) noexcept {
	hxsize_t size_ = m_size_;
	hxconstexpr_list_node* prev_ = &m_sentinel_;
	hxconstexpr_list_node* node_ = m_sentinel_.m_list_next_;
	while(node_ != &m_sentinel_) {
		hxconstexpr_list_node* const next_ = node_->m_list_next_;
		T_* const n_ = static_cast<T_*>(node_);
		if(hxforward<callable_t_>(callable_)(*n_)) {
			prev_->m_list_next_ = next_;
			next_->m_list_prev_ = prev_;
			--size_;
			if(deleter_) {
				deleter_(n_);
			}
		} else {
			prev_ = n_;
		}
		node_ = next_;
	}
	const hxsize_t count_ = m_size_ - size_;
	m_size_ = size_;
	return count_;
}

template<hxconstexpr_list_concept_ T_, typename deleter_t_>
template<typename callable_t_>
hxinline hxattr_flatten hxconstexpr hxsize_t
		hxconstexpr_list<T_, deleter_t_>::remove_if(callable_t_&& callable_) noexcept {
	return this->remove_if(hxforward<callable_t_>(callable_), static_cast<const deleter_t_&>(*this));
}

template<hxconstexpr_list_concept_ T_, typename deleter_t_>
hxinline hxconstexpr void hxconstexpr_list<T_, deleter_t_>::reverse(void) {
	hxconstexpr_list_node* node_ = &m_sentinel_;
	do {
		hxconstexpr_list_node* next_ = node_->m_list_next_;
		node_->m_list_next_ = node_->m_list_prev_;
		node_->m_list_prev_ = next_;
		node_ = next_;
	} while(node_ != &m_sentinel_);
}

template<hxconstexpr_list_concept_ T_, typename deleter_t_>
hxattr_flatten hxconstexpr void hxconstexpr_list<T_, deleter_t_>::splice(
		const_iterator it_, hxconstexpr_list& x_) {
	hxassertmsg(&x_ != this, "bad_splice same list");
	hxassertmsg(it_.m_current_node_ != hxnull, "bad_iter");
	if(x_.empty()) {
		return;
	}
	hxconstexpr_list_node* pos_node_ = it_.m_current_node_;
	hxconstexpr_list_node* prev_node_ = pos_node_->m_list_prev_;
	hxconstexpr_list_node* other_first_ = x_.m_sentinel_.m_list_next_;
	hxconstexpr_list_node* other_last_ = x_.m_sentinel_.m_list_prev_;
	prev_node_->m_list_next_ = other_first_;
	other_first_->m_list_prev_ = prev_node_;
	other_last_->m_list_next_ = pos_node_;
	pos_node_->m_list_prev_ = other_last_;
	m_size_ += x_.m_size_;
	x_.release_all();
}

template<hxconstexpr_list_concept_ T_, typename deleter_t_>
hxinline hxattr_flatten hxconstexpr void hxconstexpr_list<T_, deleter_t_>::insert_(
		hxconstexpr_list_node* prev_, hxconstexpr_list_node* next_, hxconstexpr_list_node* ptr_) {
	hxassertmsg(ptr_ != hxnull, "bad_node");
	ptr_->m_list_prev_ = prev_;
	ptr_->m_list_next_ = next_;
	prev_->m_list_next_ = ptr_;
	next_->m_list_prev_ = ptr_;
	++m_size_;
}

template<hxconstexpr_list_concept_ T_, typename deleter_t_>
hxinline hxattr_flatten hxconstexpr void hxconstexpr_list<T_, deleter_t_>::extract_(hxconstexpr_list_node* ptr_) {
	hxassert_hard(!this->empty(), "list_empty");
	hxassertmsg(ptr_ != hxnull, "bad_node");
	ptr_->m_list_prev_->m_list_next_ = ptr_->m_list_next_;
	ptr_->m_list_next_->m_list_prev_ = ptr_->m_list_prev_;
	--m_size_;
}

HX_INL_END_
#endif // HX_DOXYGEN_PARSER
