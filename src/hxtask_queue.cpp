// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

#include "../include/hx/hxtask_queue.hpp"
#include "../include/hx/hxprofiler.hpp"

HX_NS_BEGIN_

#if HX_USE_THREADS
// hxtask_wait_for_tasks_ keeps worker threads waiting for tasks or shutdown.
class hxtask_wait_for_tasks_ {
public:
	hxtask_wait_for_tasks_(hxtask_queue* q) : m_q(q) { }
	bool operator()(void) const {
		return !m_q->m_tasks_.empty()
			|| m_q->m_queue_run_level_ == hxtask_queue::run_level_stopped_;
	}
	hxtask_queue* m_q;
};

// hxtask_wait_for_completion_ waits for all work to complete and may also wait
// to shut down the queue. Neither wait state should occur after shutdown has
// started.
class hxtask_wait_for_completion_ {
public:
	hxtask_wait_for_completion_(hxtask_queue* q) : m_q(q) { }
	bool operator()(void) const {
		hxassertmsg(m_q->m_queue_run_level_ == hxtask_queue::run_level_running_,
			"threading_error");
		return m_q->m_tasks_.empty() && m_q->m_executing_count_ == 0;
	}
	hxtask_queue* m_q;
};
#endif

// Should abort if exceptions are enabled and the thread pool cannot be created.
hxtask_queue::hxtask_queue(hxsize_t task_queue_size, hxsize_t thread_pool_size)
#if HX_USE_THREADS
	: m_queue_run_level_(run_level_running_)
	, m_thread_pool_size_(thread_pool_size)
	, m_threads_(hxnull)
	, m_executing_count_(0)
#endif
{
	m_tasks_.reserve(task_queue_size);

	(void)thread_pool_size;
#if HX_USE_THREADS
	if(m_thread_pool_size_ > 0) {
		m_threads_ = reinterpret_cast<hxthread*>(hxmalloc(static_cast<size_t>(m_thread_pool_size_) * sizeof(hxthread)));
		for(hxsize_t i = m_thread_pool_size_; i-- != 0;) {
			::new(m_threads_ + i) hxthread(thread_task_loop_entry_, this);
		}
	}
#endif
}

hxtask_queue::~hxtask_queue(void) {
#if HX_USE_THREADS
	if(m_thread_pool_size_ > 0) {
		thread_task_loop_(this, thread_mode_stopping_);
		hxassertmsg(m_queue_run_level_ == run_level_stopped_, "threading_error");

		for(hxsize_t i = m_thread_pool_size_; i-- != 0;) {
			m_threads_[i].join();
			m_threads_[i].~hxthread();
		}
		hxfree(m_threads_);
		m_threads_ = hxnull;
	}
	else
#endif
	{
		wait_for_all();
	}
}

void hxtask_queue::enqueue(hxtask* task, int priority) {
	record_t entry = { task, priority
#if (HX_HARDENING_MODE) == HX_HARDENING_MODE_DEBUG
		, task->get_label()
#endif
	};

#if HX_USE_THREADS
	if(m_thread_pool_size_ > 0) {
		const hxunique_lock lock(m_mutex_);
		hxassert_hard(m_queue_run_level_ == run_level_running_, "stopped_queue");
		m_tasks_.push_heap(entry);
		m_cond_var_new_tasks_.notify_one();
	}
	else
#endif
	{
		m_tasks_.push_heap(entry);
	}
}

void hxtask_queue::wait_for_all(void) {
#if HX_USE_THREADS
	if(m_thread_pool_size_ > 0) {
		// Contribute current thread and request waiting until completion.
		thread_task_loop_(this, thread_mode_waiting_);
	}
	else
#endif
	{
		while(!m_tasks_.empty()) {
			hxtask* task = m_tasks_.front().task;
			m_tasks_.pop_heap();

			// This is the last time this object is touched. It may delete or
			// re-enqueue itself. Label is a static string.
			hxprofile_scope(task->get_label());
			task->process(this);
		}
	}
}

#if HX_USE_THREADS
hxthread::return_t hxtask_queue::thread_task_loop_entry_(hxtask_queue* q) {
	thread_task_loop_(q, thread_mode_pool_);
	return hxnull;
}

void hxtask_queue::thread_task_loop_(hxtask_queue* q, thread_mode_t_ mode) {
	hxtask* task = hxnull;
	for(;;) {
		{
			// The task executes outside of this RAII lock.
			hxunique_lock lk(q->m_mutex_);

			if(task != hxnull) {
				// Finished reacquiring the critical section after the previous task.
				task = hxnull;
				hxassertmsg(q->m_executing_count_ > 0, "internal_error");
				if((--q->m_executing_count_ == 0) && q->m_tasks_.empty()) {
					q->m_cond_var_completion_.notify_all();
				}
			}

			// Workers wait for a next task or run_level_stopped_.
			if(mode == thread_mode_pool_) {
				// Use a predicate to handle spurious wakeups.
				q->m_cond_var_new_tasks_.wait(lk, hxtask_wait_for_tasks_(q));
			}

			// Waiting threads contribute to the work.
			if(!q->m_tasks_.empty()) {
				task = q->m_tasks_.front().task;
				q->m_tasks_.pop_heap();
				++q->m_executing_count_;
			}
			else {
				// Nothing left for worker threads to do. Pool threads exit when
				// stopped. Waiting threads still have work to do before leaving.

				if(mode == thread_mode_pool_) {
					return;
				}
				else {
					// All tasks are dispatched. Now wait for m_executing_count_ to hit 0.
					// Tasks may enqueue subtasks before processing is considered done.
					// This asserts the queue is still running.
					q->m_cond_var_completion_.wait(lk, hxtask_wait_for_completion_(q));

					// All tasks are now considered complete. The workers can be
					// released if the queue is shutting down.
					if(mode == thread_mode_stopping_) {
						q->m_queue_run_level_ = run_level_stopped_;
						q->m_cond_var_new_tasks_.notify_all();

						// This triggers a release assert in any unexpected waiting threads.
						q->m_cond_var_completion_.notify_all();
					}
					return;
				}
			}
		}

		hxprofile_scope(task->get_label());

		// This is actually the last time this object is touched. It may delete or
		// re-enqueue itself. The queue is not locked and completion is not reported
		// until after the task is done.
		task->process(q);
	}
}
#endif

HX_NS_END_
