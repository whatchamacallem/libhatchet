#pragma once
// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

// A few things are missing: No task dependency / ordering mechanism is
// provided. For efficiency this would make more sense as a second layer on top
// of hxtask_queue. Thread affinity has not been added for no other reason than
// it is not portable.

// TODO: Consider adding try_enqueue. Is this a good problem to have?

#include "libhatchet.h"
#include "hxarray.hpp"
#include "hxtask.hpp"
#include "hxthread.hpp"

/// `hxtask_queue` - Provides a priority queue of tasks and a worker thread pool.
/// Implements single-threaded task queuing when `HX_USE_THREADS=0`. Does not
/// support scheduling tasks that are not read to run. Handle that at a higher
/// level. See `<hx/hxtask.hpp>`.
class hxtask_queue {
public:
	/// `record_t` - Iterated over by `all_of`, `any_of`, `erase_if` and
	/// `for_each`. callables passed to either of `erase_if` or `for_each` can
	/// modify their `record_t` callable args in order to re-prioritize the
	/// queue. This also allows examining the state of the queue in the debugger
	/// watch window.
	class record_t {
	public:
		hxtask* task;
		int priority;

		bool operator<(const record_t& x_) const { return this->priority < x_.priority; }

#if (HX_RELEASE) == 0
		const char* label;
		~record_t() { ::memset((void*)this, 0xefu, sizeof *this); } // NOLINT
#endif
	};

	/// Creates a new task queue. `task_queue_size` reserves storage for enqueued
	/// tasks. `thread_pool_size` determines the size of the worker thread pool.
	/// A `thread_pool_size` of `0` does not use threads.
	explicit hxtask_queue(size_t task_queue_size_, size_t thread_pool_size_);

	/// Calls `wait_for_all` before destruction.
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

	/// Removes a specific queued task without executing it. Returns true if the
	/// task was found and removed. Thread-safe. Returns false if the task is
	/// already executing or was not queued.
	/// - `task` : Non-null pointer to the task to cancel.
	bool cancel(const hxtask* task_) hxattr_nonnull(2);

	/// Removes all queued tasks without executing them. Thread-safe. Does not
	/// affect tasks that are already executing. Subtasks enqueued by executing
	/// tasks after a call to `clear` will remain in the queue.
	void clear(void);

	/// Returns true when no tasks are queued. Thread-safe.
	hxattr_nodiscard bool empty(void) const;

	/// Queues a task for later execution. Does not delete the task after
	/// execution. When `HX_USE_THREADS` is enabled, this is thread-safe and
	/// callable from running tasks. When `HX_USE_THREADS` is `0` there is no
	/// locking and external synchronization is the caller's responsibility.
	/// - `task` : Non-null pointer to the task to be enqueued for execution.
	/// - `priority` : Optional priority for scheduling. Higher values run sooner.
	void enqueue(hxtask* task_, int priority_=0) hxattr_nonnull(2);

	/// Locks the queue and calls `fn` on each task. Removes queued tasks for
	/// which `fn` evaluates true. The `record_t&` passed to `erase_if` may be
	/// modified and the tasks will be re-prioritized according to their new
	/// priorities.
	/// - `fn` : Predicate accepting a `record_t&`.
	template<typename callable_t_>
	size_t erase_if(callable_t_&& fn_);

	/// Locks the queue and calls `fn` on each task record.
	/// - `fn` : callable accepting a `record_t&`.
	template<typename callable_t_>
	void for_each(callable_t_&& fn_) const;

	/// Non-const version of `for_each`. This version will perform `make_heap`
	/// on the queue after calling `fn` on each task record. Reestablishing the
	/// heap allows rescheduling everything by adjusting `record_t::priority` in
	/// a lambda. Use `for_each_immutable` to iterate a non-const queue without
	/// the heap rebuild cost.
	template<typename callable_t_>
	void for_each(callable_t_&& fn_);

	/// Locks the queue and calls `fn` on each task record without rebuilding
	/// the heap. Use this instead of `for_each` when priorities are not
	/// modified.
	/// - `fn` : callable accepting a `record_t&`.
	template<typename callable_t_>
	void for_each_immutable(callable_t_&& fn_);

	/// Returns true if the queue capacity has been reached.
	hxattr_nodiscard bool full(void) const;

	/// Returns the maximum number of tasks that can be queued. This value is
	/// fixed at construction and does not require locking.
	hxattr_nodiscard size_t max_size(void) const;

	/// Returns the number of queued tasks. Thread-safe.
	hxattr_nodiscard size_t size(void) const;

	/// Execute remaining tasks. The thread calling `wait_for_all` executes
	/// tasks as well. Intended to be called by the thread that owns the queue
	/// and must not be called from `hxtask::execute`. Tasks may safely call
	/// `enqueue` during `execute` to schedule additional work before
	/// `wait_for_all` returns.
	void wait_for_all(void);

private:
	hxtask_queue(const hxtask_queue&) = delete;
	void operator=(const hxtask_queue&) = delete;

	hxarray<record_t> m_tasks_;

#if HX_USE_THREADS
	friend class hxtask_wait_for_tasks_;
	friend class hxtask_wait_for_completion_;

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
	size_t m_thread_pool_size_;
	hxthread* m_threads_;
	mutable hxmutex m_mutex_;
	hxcondition_variable m_cond_var_new_tasks_;
	hxcondition_variable m_cond_var_completion_;
	int32_t m_executing_count_;
#endif
};

template<typename callable_t_>
bool hxtask_queue::all_of(callable_t_&& fn_) const {
#if HX_USE_THREADS
	const hxunique_lock lock_(m_mutex_);
#endif
	return m_tasks_.all_of(hxforward<callable_t_>(fn_));
}

template<typename callable_t_>
bool hxtask_queue::any_of(callable_t_&& fn_) const {
#if HX_USE_THREADS
	const hxunique_lock lock_(m_mutex_);
#endif
	return m_tasks_.any_of(hxforward<callable_t_>(fn_));
}

inline bool hxtask_queue::cancel(const hxtask* task_) {
#if HX_USE_THREADS
	const hxunique_lock lock_(m_mutex_);
#endif
	const size_t erased_ = m_tasks_.erase_if([task_](const record_t& r_) { return r_.task == task_; });
	if(erased_ != 0u) {
		hxmake_heap_(m_tasks_.begin(), m_tasks_.end(), hxkey_less_t<record_t>{});
		return true;
	}
	return false;
}

inline void hxtask_queue::clear(void) {
#if HX_USE_THREADS
	const hxunique_lock lock_(m_mutex_);
#endif
	m_tasks_.clear();
}

inline bool hxtask_queue::empty(void) const {
#if HX_USE_THREADS
	const hxunique_lock lock_(m_mutex_);
#endif
	return m_tasks_.empty();
}

template<typename callable_t_>
size_t hxtask_queue::erase_if(callable_t_&& fn_) {
#if HX_USE_THREADS
	const hxunique_lock lock_(m_mutex_);
#endif
	const size_t erased_ = m_tasks_.erase_if(hxforward<callable_t_>(fn_));
	if(erased_ != 0u) {
		// Restore the heap property all at once. Allows erase_if to modify
		// priority at the same time.
		hxmake_heap_(m_tasks_.begin(), m_tasks_.end(), hxkey_less_t<record_t>{});
	}
	return erased_;
}

template<typename callable_t_>
void hxtask_queue::for_each(callable_t_&& fn_) const {
#if HX_USE_THREADS
	const hxunique_lock lock_(m_mutex_);
#endif
	m_tasks_.for_each(hxforward<callable_t_>(fn_));
}

template<typename callable_t_>
void hxtask_queue::for_each(callable_t_&& fn_) {
#if HX_USE_THREADS
	const hxunique_lock lock_(m_mutex_);
#endif
	m_tasks_.for_each(hxforward<callable_t_>(fn_));

	// Restore the heap property. Use "for_each_immutable" when not modifying priorities.
	hxmake_heap_(m_tasks_.begin(), m_tasks_.end(), hxkey_less_t<record_t>{});
}

template<typename callable_t_>
void hxtask_queue::for_each_immutable(callable_t_&& fn_) {
#if HX_USE_THREADS
	const hxunique_lock lock_(m_mutex_);
#endif
	m_tasks_.for_each(hxforward<callable_t_>(fn_));
}

inline bool hxtask_queue::full(void) const {
#if HX_USE_THREADS
	const hxunique_lock lock_(m_mutex_);
#endif
	return m_tasks_.full();
}

inline size_t hxtask_queue::max_size(void) const {
	// Capacity is fixed at construction, no lock needed.
	return m_tasks_.max_size();
}

inline size_t hxtask_queue::size(void) const {
#if HX_USE_THREADS
	const hxunique_lock lock_(m_mutex_);
#endif
	return m_tasks_.size();
}
