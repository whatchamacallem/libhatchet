#pragma once
// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

#ifndef LIBHATCHET_VER
#error Internal. Do not include this file directly.
#endif

#ifndef HX_DOXYGEN_PARSER
HX_BEGIN_INL_

template<int32_t max_successors_>
hxtask_dag_node<max_successors_>::hxtask_dag_node(void) :
	m_predecessor_count_(0), m_successor_count_(0) { }

template<int32_t max_successors_>
void hxtask_dag_node<max_successors_>::add_successor(hxtask_dag_node* successor_, int priority_) {
	hxassert_always(m_successor_count_ < max_successors_, "max_successors");
	edge_t_& edge_ = m_successors_[m_successor_count_++]; // NOLINT(clang-analyzer-security.ArrayBound)
	edge_.node_ = successor_;
	edge_.priority_ = priority_;
	++successor_->m_predecessor_count_;
}

template<int32_t max_successors_>
void hxtask_dag_node<max_successors_>::dag_node_completed_(hxtask_queue* q_, bool is_cancelled_) {
	edge_t_ ready_[max_successors_];
	edge_t_* ready_end_ = ready_;
	{
#if HX_USE_THREADS
		const hxunique_lock lock_(q_->m_mutex_);
#endif
		const edge_t_* hxrestrict it_ = m_successors_;
		const edge_t_* const end_ = m_successors_ + m_successor_count_;
		if(is_cancelled_) {
			for(; it_ != end_; ++it_) {
				if(it_->node_->m_predecessor_count_ != 0u) {
					it_->node_->m_predecessor_count_ = 0u;
					*ready_end_++ = *it_;
				}
			}
		} else {
			for(; it_ != end_; ++it_) {
				if(it_->node_->m_predecessor_count_ != 0u) {
					if(--it_->node_->m_predecessor_count_ == 0u) {
						*ready_end_++ = *it_;
					}
				}
			}
		}
		m_successor_count_ = 0u;
	}

	// q_->m_mutex_ cannot be held for these calls.
	if(is_cancelled_) {
		for(const edge_t_* hxrestrict it_ = ready_; it_ != ready_end_; ++it_) {
			it_->node_->on_cancel(q_);
		}
	} else {
		for(const edge_t_* hxrestrict it_ = ready_; it_ != ready_end_; ++it_) {
			q_->enqueue(it_->node_, it_->priority_);
		}
	}
}

HX_END_INL_
#endif // HX_DOXYGEN_PARSER
