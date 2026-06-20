#pragma once
// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

static_assert(LIBHATCHET_VER, "Internal. Do not include this file directly.");

template<size_t max_successors_>
hxtask_dag_node<max_successors_>::hxtask_dag_node(void) {
	m_successor_count_ = 0u;
	m_pending_ = 0;
	m_is_cancelled_ = false;
}

template<size_t max_successors_>
void hxtask_dag_node<max_successors_>::add_successor(hxtask_dag_node* successor_, int priority_) {
	hxassert_always(m_successor_count_ < max_successors_, "max_successors");
	m_successors_[m_successor_count_++] = { successor_, priority_ };
	++successor_->m_pending_;
}

template<size_t max_successors_>
void hxtask_dag_node<max_successors_>::dag_node_completed_(hxtask_queue* q_, bool is_cancelled_) {
	for(size_t i_ = 0u; i_ < m_successor_count_; ++i_) {
		hxtask_dag_node* const successor_ = m_successors_[i_].node_;

		if(is_cancelled_) {
#if (HX_USE_THREADS)
			atomic_store_explicit(&successor_->m_is_cancelled_, true, memory_order_relaxed);
		}
		if(atomic_fetch_sub_explicit(&successor_->m_pending_, 1, memory_order_acq_rel) == 1) {
			if(!atomic_load_explicit(&successor_->m_is_cancelled_, memory_order_acquire)) {
#else
			successor_->m_is_cancelled_ = true;
		}
		if(--successor_->m_pending_ == 0) {
			if(!successor_->m_is_cancelled_) {
#endif
				q_->enqueue(successor_, m_successors_[i_].priority_);
			} else {
				successor_->on_cancel(q_);
			}
		}
	}
	m_successor_count_ = 0u;
}
