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

template<hxlist_concept_ T_, typename deleter_t_>
hxinline auto hxlist<T_, deleter_t_>::const_iterator::operator++(void)
		-> const_iterator& {
	// Checking for m_sentinel_ is not done for symmetry.
	hxassertf(m_sentinel_ != hxnull, "bad_iter");
	hxlist_node* next_ = reinterpret_cast<hxlist_node*>(
		reinterpret_cast<intptr_t>(m_prev_) ^ m_current_node_->m_list_link_);
	m_prev_ = m_current_node_;
	m_current_node_ = next_;
	return *this;
}

template<hxlist_concept_ T_, typename deleter_t_>
hxinline hxattr_flatten auto hxlist<T_, deleter_t_>::const_iterator::operator++(int)
		-> const_iterator {
	const_iterator t_(*this);
	this->operator++();
	return t_;
}

template<hxlist_concept_ T_, typename deleter_t_>
hxinline auto hxlist<T_, deleter_t_>::const_iterator::operator--(void)
		-> const_iterator& {
	hxassertf(m_sentinel_ != hxnull, "bad_iter");
	hxlist_node* prev_of_prev_ = reinterpret_cast<hxlist_node*>(
		reinterpret_cast<intptr_t>(m_current_node_) ^ m_prev_->m_list_link_);
	m_current_node_ = m_prev_;
	m_prev_ = prev_of_prev_;
	return *this;
}

template<hxlist_concept_ T_, typename deleter_t_>
hxinline hxattr_flatten auto hxlist<T_, deleter_t_>::const_iterator::operator--(int)
		-> const_iterator {
	const_iterator t_(*this);
	this->operator--();
	return t_;
}

template<hxlist_concept_ T_, typename deleter_t_>
hxinline bool hxlist<T_, deleter_t_>::const_iterator::operator==(const const_iterator& x_) const {
	return m_current_node_ == x_.m_current_node_;
}

// iterator

template<hxlist_concept_ T_, typename deleter_t_>
hxinline hxattr_flatten auto hxlist<T_, deleter_t_>::iterator::operator++(int) -> iterator {
	iterator t_(*this);
	this->const_iterator::operator++();
	return t_;
}

template<hxlist_concept_ T_, typename deleter_t_>
hxinline hxattr_flatten auto hxlist<T_, deleter_t_>::iterator::operator--(int) -> iterator {
	iterator t_(*this);
	this->const_iterator::operator--();
	return t_;
}

// hxlist

template<hxlist_concept_ T_, typename deleter_t_>
hxinline hxattr_flatten hxlist<T_, deleter_t_>::hxlist(deleter_t_ deleter_)
		: deleter_t_(deleter_) {
}

template<hxlist_concept_ T_, typename deleter_t_>
hxinline hxattr_flatten bool hxlist<T_, deleter_t_>::operator==(const hxlist& x_) const {
	return hxequal_range(*this, x_);
}

template<hxlist_concept_ T_, typename deleter_t_>
hxinline hxattr_flatten bool hxlist<T_, deleter_t_>::operator<(const hxlist& x_) const {
	return hxless_range(*this, x_);
}

template<hxlist_concept_ T_, typename deleter_t_>
template<hxrange_concept_ range_t_, hxenable_if_t<!hxis_lvalue_reference<range_t_>(), int> >
hxinline hxattr_flatten void hxlist<T_, deleter_t_>::add_range(range_t_&& range_) noexcept {
	hxrestrict_t<decltype(range_.begin())> it_(range_.begin());
	for(const auto end_ = range_.end(); it_ != end_; ++it_) {
		this->push_back(&*it_);
	}
}

template<hxlist_concept_ T_, typename deleter_t_>
hxinline const T_& hxlist<T_, deleter_t_>::back(void) const {
	hxassert_hard(m_base_.m_size_ != 0, "list_empty");
	return *static_cast<const T_*>(m_base_.m_tail_);
}

template<hxlist_concept_ T_, typename deleter_t_>
hxinline T_& hxlist<T_, deleter_t_>::back(void) {
	hxassert_hard(m_base_.m_size_ != 0, "list_empty");
	return *static_cast<T_*>(m_base_.m_tail_);
}

template<hxlist_concept_ T_, typename deleter_t_>
hxinline hxattr_flatten auto hxlist<T_, deleter_t_>::begin(void) const -> const_iterator {
	hxlist_node* const sentinel_ = &const_cast<hxlist_base_&>(m_base_).m_sentinel_;
	return const_iterator(sentinel_, m_base_.front_(), sentinel_);
}

template<hxlist_concept_ T_, typename deleter_t_>
hxinline hxattr_flatten auto hxlist<T_, deleter_t_>::begin(void) -> iterator {
	return iterator(&m_base_.m_sentinel_, m_base_.front_(), &m_base_.m_sentinel_);
}

template<hxlist_concept_ T_, typename deleter_t_>
hxinline hxattr_flatten auto hxlist<T_, deleter_t_>::end(void) const -> const_iterator {
	hxlist_node* const sentinel_ = &const_cast<hxlist_base_&>(m_base_).m_sentinel_;
	return const_iterator(m_base_.m_tail_, sentinel_, sentinel_);
}

template<hxlist_concept_ T_, typename deleter_t_>
hxinline hxattr_flatten auto hxlist<T_, deleter_t_>::end(void) -> iterator {
	return iterator(m_base_.m_tail_, &m_base_.m_sentinel_, &m_base_.m_sentinel_);
}

template<hxlist_concept_ T_, typename deleter_t_>
template<typename deleter_u_>
hxattr_flatten void hxlist<T_, deleter_t_>::clear(deleter_u_&& deleter_) noexcept {
	if(m_base_.m_size_ != 0) {
		hxlist_node* prev_ = &m_base_.m_sentinel_;
		hxlist_node* current_ = m_base_.front_();
		m_base_.release_all_();
		if(deleter_) {
			const hxlist_node* const sentinel_ = prev_;
			while(current_ != sentinel_) {
				hxlist_node* const next_ = reinterpret_cast<hxlist_node*>(
					reinterpret_cast<intptr_t>(prev_) ^ current_->m_list_link_);
				deleter_(static_cast<T_*>(current_));
				prev_ = current_;
				current_ = next_;
			}
		}
	}
}

template<hxlist_concept_ T_, typename deleter_t_>
template<typename deleter_u_>
hxinline hxattr_flatten void hxlist<T_, deleter_t_>::erase(const_iterator it_, deleter_u_&& deleter_) noexcept {
	T_* ptr_ = static_cast<T_*>(it_.m_current_node_);
	m_base_.extract_(it_.m_prev_, it_.m_current_node_);
	if(deleter_) {
		deleter_(ptr_);
	}
}

template<hxlist_concept_ T_, typename deleter_t_>
hxinline hxattr_flatten void hxlist<T_, deleter_t_>::erase(const_iterator it_) noexcept {
	this->erase(it_, static_cast<deleter_t_&>(*this));
}

template<hxlist_concept_ T_, typename deleter_t_>
hxinline hxattr_flatten hxptr<T_, deleter_t_> hxlist<T_, deleter_t_>::extract(const_iterator it_) {
	T_* const node_ = static_cast<T_*>(it_.m_current_node_);
	m_base_.extract_(it_.m_prev_, it_.m_current_node_);
	return hxptr<T_, deleter_t_>(node_, static_cast<deleter_t_&>(*this));
}

template<hxlist_concept_ T_, typename deleter_t_>
template<typename callable_t_>
hxinline hxattr_flatten auto hxlist<T_, deleter_t_>::find_if(callable_t_&& callable_) const -> const_iterator {
	const const_iterator end_ = this->end();
	for(const_iterator it_ = this->begin(); it_ != end_; ++it_) {
		if(hxforward<callable_t_>(callable_)(*it_)) {
			return it_;
		}
	}
	return end_;
}

template<hxlist_concept_ T_, typename deleter_t_>
template<typename callable_t_>
hxinline hxattr_flatten auto hxlist<T_, deleter_t_>::find_if(callable_t_&& callable_) -> iterator {
	const iterator end_ = this->end();
	for(iterator it_ = this->begin(); it_ != end_; ++it_) {
		if(hxforward<callable_t_>(callable_)(*it_)) {
			return it_;
		}
	}
	return end_;
}

template<hxlist_concept_ T_, typename deleter_t_>
template<typename callable_t_>
hxinline hxattr_flatten void hxlist<T_, deleter_t_>::for_each(callable_t_&& callable_) const {
	const const_iterator end_ = this->end();
	for(const_iterator it_ = this->begin(); it_ != end_; ++it_) {
		hxforward<callable_t_>(callable_)(*it_);
	}
}

template<hxlist_concept_ T_, typename deleter_t_>
template<typename callable_t_>
hxinline hxattr_flatten void hxlist<T_, deleter_t_>::for_each(callable_t_&& callable_) {
	const iterator end_ = this->end();
	for(iterator it_ = this->begin(); it_ != end_; ++it_) {
		hxforward<callable_t_>(callable_)(*it_);
	}
}

template<hxlist_concept_ T_, typename deleter_t_>
hxinline hxattr_flatten const deleter_t_& hxlist<T_, deleter_t_>::deleter(void) const {
	return static_cast<const deleter_t_&>(*this);
}

template<hxlist_concept_ T_, typename deleter_t_>
hxinline hxattr_flatten deleter_t_& hxlist<T_, deleter_t_>::deleter(void) {
	return static_cast<deleter_t_&>(*this);
}

template<hxlist_concept_ T_, typename deleter_t_>
hxinline hxattr_flatten const T_& hxlist<T_, deleter_t_>::front(void) const {
	hxassert_hard(m_base_.m_size_ != 0, "list_empty");
	return *static_cast<const T_*>(m_base_.front_());
}

template<hxlist_concept_ T_, typename deleter_t_>
hxinline hxattr_flatten T_& hxlist<T_, deleter_t_>::front(void) {
	hxassert_hard(m_base_.m_size_ != 0, "list_empty");
	return *static_cast<T_*>(m_base_.front_());
}

template<hxlist_concept_ T_, typename deleter_t_>
template<typename deleter_u_>
hxinline hxattr_flatten auto hxlist<T_, deleter_t_>::insert_after(
		const_iterator it_, hxptr<T_, deleter_u_>&& ptr_) -> iterator {
	return this->insert_after(it_, ptr_.release());
}

template<hxlist_concept_ T_, typename deleter_t_>
hxinline hxattr_flatten auto hxlist<T_, deleter_t_>::insert_after(const_iterator it_, T_* ptr_) -> iterator {
	m_base_.insert_after_(it_.m_prev_, it_.m_current_node_, ptr_);
	return iterator(it_.m_current_node_, ptr_, &m_base_.m_sentinel_);
}

template<hxlist_concept_ T_, typename deleter_t_>
template<typename deleter_u_>
hxinline hxattr_flatten auto hxlist<T_, deleter_t_>::insert(
		const_iterator it_, hxptr<T_, deleter_u_>&& ptr_) -> iterator {
	return this->insert(it_, ptr_.release());
}

template<hxlist_concept_ T_, typename deleter_t_>
hxinline hxattr_flatten auto hxlist<T_, deleter_t_>::insert(
		const_iterator it_, T_* ptr_) -> iterator {
	m_base_.insert_before_(it_.m_prev_, it_.m_current_node_, ptr_);
	return iterator(it_.m_prev_, ptr_, &m_base_.m_sentinel_);
}

template<hxlist_concept_ T_, typename deleter_t_>
hxinline hxattr_flatten hxptr<T_, deleter_t_> hxlist<T_, deleter_t_>::pop_back(void) {
	return hxptr<T_, deleter_t_>(static_cast<T_*>(m_base_.pop_back_()), static_cast<deleter_t_&>(*this));
}

template<hxlist_concept_ T_, typename deleter_t_>
hxinline hxattr_flatten hxptr<T_, deleter_t_> hxlist<T_, deleter_t_>::pop_front(void) {
	return hxptr<T_, deleter_t_>(static_cast<T_*>(m_base_.pop_front_()), static_cast<deleter_t_&>(*this));
}

template<hxlist_concept_ T_, typename deleter_t_>
template<typename deleter_u_>
hxinline hxattr_flatten auto hxlist<T_, deleter_t_>::push_back(hxptr<T_, deleter_u_>&& ptr_) -> iterator {
	return this->push_back(ptr_.release());
}

template<hxlist_concept_ T_, typename deleter_t_>
hxinline hxattr_flatten auto hxlist<T_, deleter_t_>::push_back(T_* ptr_) -> iterator {
	hxlist_node* const prev_ = m_base_.m_tail_;
	m_base_.push_back_(ptr_);
	return iterator(prev_, ptr_, &m_base_.m_sentinel_);
}

template<hxlist_concept_ T_, typename deleter_t_>
template<typename deleter_u_>
hxinline hxattr_flatten auto hxlist<T_, deleter_t_>::push_front(hxptr<T_, deleter_u_>&& ptr_) -> iterator {
	return this->push_front(ptr_.release());
}

template<hxlist_concept_ T_, typename deleter_t_>
hxinline hxattr_flatten auto hxlist<T_, deleter_t_>::push_front(T_* ptr_) -> iterator {
	m_base_.push_front_(ptr_);
	return iterator(&m_base_.m_sentinel_, ptr_, &m_base_.m_sentinel_);
}

template<hxlist_concept_ T_, typename deleter_t_>
hxinline hxattr_flatten void hxlist<T_, deleter_t_>::release_all(void) {
	m_base_.release_all_();
}

template<hxlist_concept_ T_, typename deleter_t_>
template<typename callable_t_, typename deleter_u_>
hxattr_flatten hxsize_t hxlist<T_, deleter_t_>::remove_if(callable_t_&& callable_,
		deleter_u_&& deleter_) noexcept {
	hxsize_t removed_ = 0;
	hxlist_node* const sentinel_ = &m_base_.m_sentinel_;
	hxlist_node* prev_ = sentinel_;
	hxlist_node* current_ = m_base_.front_();
	while(current_ != sentinel_) {
		hxlist_node* const next_ = reinterpret_cast<hxlist_node*>(
			reinterpret_cast<intptr_t>(prev_) ^ current_->m_list_link_);
		T_* const ptr_ = static_cast<T_*>(current_);
		if(hxforward<callable_t_>(callable_)(*ptr_)) {
			m_base_.extract_(prev_, current_);
			if(deleter_) {
				deleter_(ptr_);
			}
			++removed_;
		} else {
			prev_ = current_;
		}
		current_ = next_;
	}
	return removed_;
}

template<hxlist_concept_ T_, typename deleter_t_>
template<typename callable_t_>
hxinline hxattr_flatten hxsize_t hxlist<T_, deleter_t_>::remove_if(callable_t_&& callable_) noexcept {
	return this->remove_if(hxforward<callable_t_>(callable_), static_cast<const deleter_t_&>(*this));
}

template<hxlist_concept_ T_, typename deleter_t_>
hxinline hxattr_flatten void hxlist<T_, deleter_t_>::reverse(void) {
	m_base_.reverse_();
}

template<hxlist_concept_ T_, typename deleter_t_>
hxattr_flatten void hxlist<T_, deleter_t_>::splice(
		const_iterator it_, hxlist& x_) {
	m_base_.splice_(it_.m_prev_, it_.m_current_node_, x_.m_base_);
}

HX_INL_END_
#endif // HX_DOXYGEN_PARSER
