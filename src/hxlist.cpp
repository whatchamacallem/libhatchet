// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

#include "../include/hx/hxlist.hpp"

HX_NS_BEGIN_

// A sentinel node eliminates checking for null when empty. And XOR linking
// stores "prev XOR next" in a single pointer, halving per-node pointer memory
// at the cost of requiring the previous node's address to traverse.

hxlist_base_::hxlist_base_(void) : m_size_(0), m_tail_(&m_sentinel_) {
}

hxlist_node* hxlist_base_::front_(void) const {
	return reinterpret_cast<hxlist_node*>(
		reinterpret_cast<intptr_t>(m_tail_) ^ m_sentinel_.m_list_link_);
}

void hxlist_base_::extract_(hxlist_node* prev_, hxlist_node* current_) {
	hxassert_hard(m_size_ != 0, "list_empty");
	hxassertmsg(current_ != hxnull, "bad_node");
	hxlist_node* const next_ = reinterpret_cast<hxlist_node*>(
		reinterpret_cast<intptr_t>(prev_) ^ current_->m_list_link_);
	prev_->m_list_link_ ^= reinterpret_cast<intptr_t>(current_) ^ reinterpret_cast<intptr_t>(next_);
	next_->m_list_link_ ^= reinterpret_cast<intptr_t>(current_) ^ reinterpret_cast<intptr_t>(prev_);
	if(next_ == &m_sentinel_) {
		m_tail_ = prev_;
	}
	--m_size_;
}

void hxlist_base_::insert_after_(hxlist_node* prev_, hxlist_node* current_, hxlist_node* ptr_) {
	hxlist_node* const new_prev_ = current_;
	hxlist_node* const new_next_ = reinterpret_cast<hxlist_node*>(
		reinterpret_cast<intptr_t>(prev_) ^ new_prev_->m_list_link_);
	hxassertmsg(ptr_ != hxnull, "bad_node");
	ptr_->m_list_link_ = reinterpret_cast<intptr_t>(new_prev_) ^ reinterpret_cast<intptr_t>(new_next_);
	new_prev_->m_list_link_ ^= reinterpret_cast<intptr_t>(new_next_) ^ reinterpret_cast<intptr_t>(ptr_);
	new_next_->m_list_link_ ^= reinterpret_cast<intptr_t>(new_prev_) ^ reinterpret_cast<intptr_t>(ptr_);
	if(new_next_ == &m_sentinel_) {
		m_tail_ = ptr_;
	}
	++m_size_;
}

void hxlist_base_::insert_before_(hxlist_node* prev_, hxlist_node* current_, hxlist_node* ptr_) {
	hxassertmsg(ptr_ != hxnull, "bad_node");
	ptr_->m_list_link_ = reinterpret_cast<intptr_t>(prev_) ^ reinterpret_cast<intptr_t>(current_);
	prev_->m_list_link_ ^= reinterpret_cast<intptr_t>(current_) ^ reinterpret_cast<intptr_t>(ptr_);
	current_->m_list_link_ ^= reinterpret_cast<intptr_t>(prev_) ^ reinterpret_cast<intptr_t>(ptr_);
	if(current_ == &m_sentinel_) {
		m_tail_ = ptr_;
	}
	++m_size_;
}

hxlist_node* hxlist_base_::pop_back_(void) {
	hxassert_hard(m_size_ != 0, "list_empty");
	hxlist_node* const node_ = m_tail_;
	hxlist_node* const prev_ = reinterpret_cast<hxlist_node*>(
		reinterpret_cast<intptr_t>(&m_sentinel_) ^ node_->m_list_link_);
	prev_->m_list_link_ ^= reinterpret_cast<intptr_t>(node_) ^ reinterpret_cast<intptr_t>(&m_sentinel_);
	m_sentinel_.m_list_link_ ^= reinterpret_cast<intptr_t>(node_) ^ reinterpret_cast<intptr_t>(prev_);
	m_tail_ = prev_;
	--m_size_;
	return node_;
}

hxlist_node* hxlist_base_::pop_front_(void) {
	hxassert_hard(m_size_ != 0, "list_empty");
	hxlist_node* const node_ = reinterpret_cast<hxlist_node*>(
		reinterpret_cast<intptr_t>(m_tail_) ^ m_sentinel_.m_list_link_);
	hxlist_node* const next_ = reinterpret_cast<hxlist_node*>(
		reinterpret_cast<intptr_t>(&m_sentinel_) ^ node_->m_list_link_);
	m_sentinel_.m_list_link_ ^= reinterpret_cast<intptr_t>(node_) ^ reinterpret_cast<intptr_t>(next_);
	next_->m_list_link_ ^= reinterpret_cast<intptr_t>(node_) ^ reinterpret_cast<intptr_t>(&m_sentinel_);
	if(next_ == &m_sentinel_) {
		m_tail_ = &m_sentinel_;
	}
	--m_size_;
	return node_;
}

void hxlist_base_::push_back_(hxlist_node* ptr_) {
	hxassert_hard(ptr_ != hxnull, "bad_node");
	hxlist_node* const prev_ = m_tail_;
	ptr_->m_list_link_ = reinterpret_cast<intptr_t>(prev_) ^ reinterpret_cast<intptr_t>(&m_sentinel_);
	prev_->m_list_link_ ^= reinterpret_cast<intptr_t>(&m_sentinel_) ^ reinterpret_cast<intptr_t>(ptr_);
	m_sentinel_.m_list_link_ ^= reinterpret_cast<intptr_t>(prev_) ^ reinterpret_cast<intptr_t>(ptr_);
	m_tail_ = ptr_;
	++m_size_;
}

void hxlist_base_::push_front_(hxlist_node* ptr_) {
	hxassert_hard(ptr_ != hxnull, "bad_node");
	hxlist_node* const first_ = reinterpret_cast<hxlist_node*>(
		reinterpret_cast<intptr_t>(m_tail_) ^ m_sentinel_.m_list_link_);
	ptr_->m_list_link_ = reinterpret_cast<intptr_t>(&m_sentinel_) ^ reinterpret_cast<intptr_t>(first_);
	m_sentinel_.m_list_link_ ^= reinterpret_cast<intptr_t>(first_) ^ reinterpret_cast<intptr_t>(ptr_);
	first_->m_list_link_ ^= reinterpret_cast<intptr_t>(&m_sentinel_) ^ reinterpret_cast<intptr_t>(ptr_);
	if(first_ == &m_sentinel_) {
		m_tail_ = ptr_;
	}
	++m_size_;
}

void hxlist_base_::release_all_(void) {
	m_sentinel_.m_list_link_ = 0;
	m_tail_ = &m_sentinel_;
	m_size_ = 0;
}

void hxlist_base_::reverse_(void) {
	// Nodes are unchanged by reversal since XOR is commutative.
	m_tail_ = reinterpret_cast<hxlist_node*>(
		reinterpret_cast<intptr_t>(m_tail_) ^ m_sentinel_.m_list_link_);
}

void hxlist_base_::splice_(hxlist_node* prev_, hxlist_node* current_, hxlist_base_& x_) {
	hxassertmsg(&x_ != this, "bad_splice");
	if(x_.m_size_ == 0) {
		return;
	}
	hxlist_node* const other_first_ = reinterpret_cast<hxlist_node*>(
		reinterpret_cast<intptr_t>(x_.m_tail_) ^ x_.m_sentinel_.m_list_link_);
	hxlist_node* const other_last_ = x_.m_tail_;
	prev_->m_list_link_ ^=
		reinterpret_cast<intptr_t>(current_) ^ reinterpret_cast<intptr_t>(other_first_);
	other_first_->m_list_link_ ^=
		reinterpret_cast<intptr_t>(&x_.m_sentinel_) ^ reinterpret_cast<intptr_t>(prev_);
	other_last_->m_list_link_ ^=
		reinterpret_cast<intptr_t>(&x_.m_sentinel_) ^ reinterpret_cast<intptr_t>(current_);
	current_->m_list_link_ ^=
		reinterpret_cast<intptr_t>(prev_) ^ reinterpret_cast<intptr_t>(other_last_);
	if(current_ == &m_sentinel_) {
		m_tail_ = other_last_;
	}
	m_size_ += x_.m_size_;
	x_.release_all_();
}

HX_NS_END_
