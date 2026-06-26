#pragma once
// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

/// \file
/// Provides a priority queue of tasks and a worker thread pool.

#include "libhatchet.h"

// HX_USE_MODULE allows including macros in addition to the hx module.
#if HX_USE_MODULE
#error Header does not provide macros only.
#endif

#include "hxvector.hpp"
#include "hxtask.hpp"
#include "hxthread.hpp"

HX_NS_BEGIN_

/// `hxtask_queue` - Provides a priority queue of tasks and a worker thread
/// pool. Implements single-threaded task queuing when `HX_USE_THREADS=0`. For
/// DAG dependency tracking see `<hx/hxtask_dag_node.hpp>`.
class hxtask_queue {
public:
	/// `record_t` - Iterated over by `all_of`, `any_of`, `erase_if` and
	/// `for_each`. Callables passed to either of `erase_if` or `for_each` can
	/// modify their `record_t` callable arg in order to re-prioritize the
	/// queue. This object allows examining the state of the queue in the
	/// debugger watch window.
	class record_t {
	public:
		hxtask* task;
		int priority;

		/// Orders records by `priority` so the highest priority sorts last.
		/// - `x` : The record to compare against.
		bool operator<(const record_t& x_) const { return this->priority < x_.priority; }

#if (HX_HARDENING_MODE) == HX_HARDENING_MODE_DEBUG
		const char* label;
		~record_t() { ::memset(reinterpret_cast<void*>(this), 0xefu, sizeof *this); }
#endif
	};

	/// Creates a new task queue. `task_queue_size` reserves storage for enqueued
	/// tasks. `thread_pool_size` determines the size of the worker thread pool.
	/// A `thread_pool_size` of `0` does not use threads.
	explicit hxtask_queue(hxsize_t task_queue_size_, hxsize_t thread_pool_size_);

	/// Waits for all queued and executing tasks to complete before destruction.
	~hxtask_queue(void);

	/// Locks the queue and calls `fn` on each task. Returns true if the
	/// predicate returns true for every element and false otherwise. Will stop
	/// iterating when the predicate returns false. Use `for_each` to modify
	/// priorities.
	/// - `fn` : A callable returning boolean. `!all_of(x)` -> `any_not(x)`.
	template<typename callable_t_>
	hxattr_nodiscard bool all_of(callable_t_&& fn_) const;

	/// Locks the queue and calls `fn` on each task. Returns true if the
	/// predicate returns true for any element and false otherwise. Will stop
	/// iterating when the predicate returns true. Use `for_each` to modify
	/// priorities.
	/// - `fn` : A callable returning boolean. `!any_of(x)` -> `none_of(x)`.
	template<typename callable_t_>
	hxattr_nodiscard bool any_of(callable_t_&& fn_) const;

	/// Returns true if the task was found and removed. Calls `on_cancel` on the
	/// task if found. Thread-safe. Returns false if the task is already
	/// executing or was not queued.
	/// - `task` : Non-null pointer to the task to cancel.
	bool cancel(hxtask* task_) noexcept hxattr_nonnull(2);

	/// Removes all queued tasks without executing them. Does not call
	/// `on_cancel` on each. Does not affect tasks that are already executing.
	/// Subtasks enqueued by executing tasks after a call to `clear` will remain
	/// in the queue.
	void clear(void) noexcept;

	/// Returns true when no tasks are queued. Thread-safe.
	hxattr_nodiscard bool empty(void) const;

	/// Queues a task for later execution. Does not delete the task after
	/// execution. Thread-safe only when `HX_USE_THREADS` is enabled and the
	/// thread pool size is greater than zero.
	/// - `task` : Non-null pointer to the task to be enqueued for execution.
	/// - `priority` : Optional priority for scheduling. Higher values run sooner.
	void enqueue(hxtask* task_, int priority_=0) hxattr_nonnull(2);

	/// Locks the queue and calls `fn` on each task. Removes queued tasks for
	/// which `fn` evaluates true. Does not call `on_cancel` on each. Returns
	/// the number of records removed. The `record_t&` passed to `erase_if` may
	/// be modified and the tasks will be re-prioritized according to their new
	/// priorities.
	/// - `fn` : Predicate accepting a `record_t&`.
	template<typename callable_t_>
	hxsize_t erase_if(callable_t_&& fn_) noexcept;

	/// Locks the queue and calls `fn` on each task record.
	/// - `fn` : callable accepting a `const record_t&`.
	template<typename callable_t_>
	void for_each(callable_t_&& fn_) const;

	/// Non-const version of `for_each`. This version will perform `make_heap`
	/// on the queue after calling `fn` on each task record. The `record_t&`
	/// passed to `for_each` may be modified and the tasks will be
	/// re-prioritized according to their new priorities.
	template<typename callable_t_>
	void for_each(callable_t_&& fn_) noexcept;

	/// Returns true if the queue capacity has been reached.
	hxattr_nodiscard bool full(void) const;

	/// Returns the maximum number of tasks that can be queued. This value is
	/// fixed at construction and does not require locking.
	hxattr_nodiscard hxsize_t max_size(void) const;

	/// Returns the number of queued tasks. Thread-safe.
	hxattr_nodiscard hxsize_t size(void) const;

	/// Execute remaining tasks. The thread calling `wait_for_all` executes
	/// tasks as well. Intended to be called by the thread that owns the queue
	/// and must not be called from `hxtask::execute`. Tasks may safely call
	/// `enqueue` during `execute` to schedule additional work before
	/// `wait_for_all` returns. WARNING: Calling `wait_for_all` from inside
	/// `hxtask::execute` deadlocks permanently.
	void wait_for_all(void);

private:
	hxtask_queue(const hxtask_queue&) = delete;
	void operator=(const hxtask_queue&) = delete;

	hxvector<record_t> m_tasks_;

#if HX_USE_THREADS
#define hxtask_queue_lock_ const hxunique_lock lock_(m_mutex_)

	friend class hxtask_wait_for_tasks_;
	friend class hxtask_wait_for_completion_;
	template<hxsize_t> friend class hxtask_dag_node;

	enum thread_mode_t_ : uint8_t {
		thread_mode_pool_,
		thread_mode_waiting_,
		thread_mode_stopping_
	};
	enum run_level_t_ : uint32_t {
		run_level_running_ = 0x00c0ffeeu,
		run_level_stopped_ = 0xdeadbeefu
	};

	static hxthread::return_t thread_task_loop_entry_(hxtask_queue* q_);
	static void thread_task_loop_(hxtask_queue* q_, thread_mode_t_ mode_);

	run_level_t_ m_queue_run_level_;
	hxsize_t m_thread_pool_size_;
	hxthread* m_threads_;
	mutable hxmutex m_mutex_;
	hxcondition_variable m_cond_var_new_tasks_;
	hxcondition_variable m_cond_var_completion_;
	int32_t m_executing_count_;
#else
#define hxtask_queue_lock_ ((void)0)
#endif
};

#include "detail/hxtask_queue.inl"
HX_NS_END_
