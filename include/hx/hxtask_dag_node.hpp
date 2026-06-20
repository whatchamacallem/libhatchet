#pragma once
// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

/// \file hx/hxconsole.hpp Implements DAG dependency tracking as a layer on top
/// of hxtask_queue.

#include "hxtask_queue.hpp"

// This functionality is only available when !HX_USE_THREADS || HX_CPLUSPLUS >=
// 202302L.
#if HX_USE_THREADS
#if HX_CPLUSPLUS < 202302L
#error "<stdatomic.h> requires C++23."
#endif
#include <stdatomic.h>
#endif

/// `hxtask_dag_node<max_successors>` - A `hxtask` subclass that implements DAG
/// dependency tracking as a layer on top of hxtask_queue. Connect nodes with
/// `add_successor` before enqueueing roots (those with no predecessors).
/// Successor nodes are enqueued automatically once all their predecessors
/// complete successfully. If any predecessor fails or is cancelled, all
/// reachable successors are cancelled — they receive `on_cancel` rather than
/// being enqueued. Each node may be used only once. Subclasses that override
/// `on_completion`, `on_failure`, or `on_cancel` must call the corresponding
/// `hxtask_dag_node` base method to preserve propagation.
/// - `max_successors` : Maximum direct successors per node. Default is 4.
template<size_t max_successors_=4u>
class hxtask_dag_node : public hxtask {
public:
	hxtask_dag_node(void);

	/// Records a dependency: `successor` runs after this node completes,
	/// enqueued at `priority`. Not thread safe, must be called before
	/// enqueueing any root nodes.
	/// - `successor` : Non-null pointer to the dependent node.
	/// - `priority`  : Priority at which `successor` will be enqueued.
	void add_successor(hxtask_dag_node* successor_, int priority_ = 0) hxattr_nonnull(2);

	/// Propagates successful completion to successors. Subclass overrides must
	/// call `hxtask_dag_node::on_completion(q_)`.
	void on_completion(hxtask_queue* q_) override { this->dag_node_completed_(q_, false); }

	/// Cancels all successors. Subclass overrides must call
	/// `hxtask_dag_node::on_failure(q_)`.
	void on_failure(hxtask_queue* q_) override { this->dag_node_completed_(q_, true); }

	/// Cancels all successors. Subclass overrides must call
	/// `hxtask_dag_node::on_cancel(q_)`.
	void on_cancel(hxtask_queue* q_) override { this->dag_node_completed_(q_, true); }

private:
	struct edge_t_ {
		hxtask_dag_node* node_;
		int priority_;
	};

	void dag_node_completed_(hxtask_queue* q_, bool is_cancelled_);

	edge_t_ m_successors_[max_successors_];
	size_t  m_successor_count_;

#if HX_USE_THREADS
	atomic_int  m_pending_;
	atomic_bool m_is_cancelled_;
#else
	int  m_pending_;
	bool m_is_cancelled_;
#endif
};

#include "detail/hxtask_dag_node.inl"
