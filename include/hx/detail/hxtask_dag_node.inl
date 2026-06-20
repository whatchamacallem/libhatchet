#pragma once
// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

#ifndef LIBHATCHET_VER
#error Internal. Do not include this file directly.
#endif

#ifndef HX_DOXYGEN_PARSER
HX_BEGIN_INL_

template<hxsize_t max_successors_>
hxtask_dag_node<max_successors_>::hxtask_dag_node(void) {
	m_predecessor_count_ = 0;
	m_successor_count_ = 0;
}

template<hxsize_t max_successors_>
void hxtask_dag_node<max_successors_>::add_successor(hxtask_dag_node* successor_, int priority_) {
	hxassert_always(m_successor_count_ < max_successors_, "max_successors");
	edge_t_& edge_ = m_successors_[m_successor_count_++]; // NOLINT(clang-analyzer-security.ArrayBound)
	edge_.node_ = successor_;
	edge_.priority_ = priority_;
	++successor_->m_predecessor_count_;
}

template<hxsize_t max_successors_>
void hxtask_dag_node<max_successors_>::dag_node_completed_(hxtask_queue* q_, bool is_cancelled_) {
	bool last_predecessor_[max_successors_];
	{
#if HX_USE_THREADS
		const hxunique_lock lock_(q_->m_mutex_);
#endif

		for(hxsize_t i_ = 0; i_ < m_successor_count_; ++i_) {
			const edge_t_& successor_ = m_successors_[i_];

			if(successor_.node_->m_predecessor_count_ > 0) {
				if(is_cancelled_) {
					successor_.node_->m_predecessor_count_ = 0;
					last_predecessor_[i_] = true;
				} else {
					last_predecessor_[i_] = --successor_.node_->m_predecessor_count_ == 0;
				}
			} else {
				last_predecessor_[i_] = false;
			}
		}
	}

	// q_->m_mutex_ cannot be held for these calls.
	if(is_cancelled_) {
		for(hxsize_t i_ = 0; i_ < m_successor_count_; ++i_) {
			if(last_predecessor_[i_]) {
				m_successors_[i_].node_->on_cancel(q_);
			}
		}
	} else {
		for(hxsize_t i_ = 0; i_ < m_successor_count_; ++i_) {
			if(last_predecessor_[i_]) {
				q_->enqueue(m_successors_[i_].node_, m_successors_[i_].priority_);
			}
		}
	}

	m_successor_count_ = 0;
}

HX_END_INL_
#endif // HX_DOXYGEN_PARSER
