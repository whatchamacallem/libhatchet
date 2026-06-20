#pragma once
// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

/// \file hxtask_dag_node.hpp Implements DAG dependency tracking as a layer on
/// top of hxtask_queue.

#include "hxtask_queue.hpp"

HX_NS_BEGIN_

/// `hxtask_dag_node<max_successors>` - A `hxtask` subclass that implements DAG
/// dependency tracking as a layer on top of `hxtask_queue`. To connect nodes
/// use `add_successor` before enqueueing roots (those with no predecessors).
/// WARNING: The DAG is not intended to be modified once submitted as
/// `add_successor` does not implement locking. Successor nodes are enqueued
/// automatically once all their predecessors complete successfully. If any
/// predecessor fails or is cancelled, all successors are cancelled: they
/// receive `on_cancel` rather than being enqueued. Subclasses that override
/// `on_completion`, `on_failure`, or `on_cancel` must call the corresponding
/// `hxtask_dag_node` base method to preserve task propagation.
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

	/// Clears predecessors and successors. WARNING: Do not use while the DAG
	/// is being executed. 
	void reset(void) { m_predecessor_count_ = 0u; m_successor_count_ = 0u; }

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
	class edge_t_ {
	public:
		hxtask_dag_node* node_;
		int priority_;
	};

	void dag_node_completed_(hxtask_queue* q_, bool is_cancelled_);

	size_t  m_predecessor_count_;
	size_t  m_successor_count_;
	edge_t_ m_successors_[max_successors_];
};

#include "detail/hxtask_dag_node.inl"

HX_NS_END_
