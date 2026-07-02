// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

#include "../include/hx/hxtask_queue.hpp"
#include "../include/hx/hxprofiler.hpp"

HX_NS_BEGIN_

// Should abort if exceptions are enabled and the thread pool cannot be created.
hxtask_queue::hxtask_queue(int32_t task_queue_size, int32_t thread_pool_size) noexcept
#if HX_USE_THREADS
	: m_queue_run_level_(run_level_::running_)
	, m_thread_pool_size_(thread_pool_size)
	, m_executing_count_(0)
	, m_threads_(hxnull)
#endif
{
	m_tasks_.reserve(task_queue_size);
	(void)thread_pool_size;
#if HX_USE_THREADS
	if(m_thread_pool_size_ > 0) {
		m_threads_ = reinterpret_cast<hxthread*>(hxmalloc(
			static_cast<size_t>(m_thread_pool_size_) * sizeof(hxthread)));
		hxthread* const end = m_threads_ + m_thread_pool_size_;
		for(hxthread* hxrestrict it = m_threads_; it != end; ++it) {
			::new(it) hxthread(thread_task_loop_entry_, this);
		}
	}
#endif
}

hxtask_queue::~hxtask_queue(void) {
#if HX_USE_THREADS
	if(m_thread_pool_size_ > 0) {
		thread_task_loop_(*this, thread_mode_::stopping_);
		hxassertmsg(m_queue_run_level_ == run_level_::stopped_, "threading_error");

		hxthread* const end = m_threads_ + m_thread_pool_size_;
		for(hxthread* hxrestrict it = m_threads_; it != end; ++it) {
			it->join();
			it->~hxthread();
		}
		hxfree(m_threads_);
	}
	else
#endif
	{
		wait_for_all();
	}
}

void hxtask_queue::clear(void) noexcept {
	hxtask_queue_lock_;
	m_tasks_.clear();
#if HX_USE_THREADS
	m_cond_var_completion_.notify_all();
#endif
}

bool hxtask_queue::cancel(hxtask* task_) noexcept {
	bool erased_ = false;
	{
		hxtask_queue_lock_;
		record_t* const found_ = m_tasks_.find_if(
			[task_](const record_t& r_) { return r_.task == task_; });
		if(found_ != m_tasks_.end()) {
			m_tasks_.erase_unordered(found_);
			hxdetail_::hxmake_heap_(m_tasks_.begin(), m_tasks_.end(), hxkey_less_t<record_t>{});
			erased_ = true;
#if HX_USE_THREADS
			if(m_tasks_.empty()) {
				m_cond_var_completion_.notify_all();
			}
#endif
		}
	}
	if(erased_) {
		task_->on_cancel(this);
		return true;
	}
	return false;
}

void hxtask_queue::enqueue(hxtask* task, int priority) noexcept {
	const record_t entry = { task, priority
#if (HX_HARDENING_MODE) == HX_HARDENING_MODE_DEBUG
		, task->get_label()
#endif
	};

#if HX_USE_THREADS
	if(m_thread_pool_size_ > 0) {
		const hxunique_lock lock(m_mutex_);
		hxassert_hard(m_queue_run_level_ == run_level_::running_, "stopped_queue");
		m_tasks_.push_heap(entry);
		m_cond_var_new_tasks_.notify_one();
	}
	else
#endif
	{
		m_tasks_.push_heap(entry);
	}
}

void hxtask_queue::wait_for_all(void) noexcept {
#if HX_USE_THREADS
	if(m_thread_pool_size_ > 0) {
		// Contribute current thread and request waiting until completion.
		thread_task_loop_(*this, thread_mode_::waiting_);
	}
	else
#endif
	{
		while(!m_tasks_.empty()) {
			hxtask* const task = m_tasks_.front().task;
			m_tasks_.pop_heap();

			// This is the last time this object is touched. It may delete or
			// re-enqueue itself. Label is a static string.
			hxprofile_scope(task->get_label());
			task->process(this);
		}
	}
}

#if HX_USE_THREADS
hxthread::return_t hxtask_queue::thread_task_loop_entry_(hxtask_queue* q) noexcept {
	thread_task_loop_(*q, thread_mode_::pool_);
	return hxnull;
}

void hxtask_queue::thread_task_loop_(hxtask_queue& q, thread_mode_ mode) noexcept {
	hxtask* task = hxnull;
	for(;;) {
		{
			// The task executes outside of this RAII lock.
			hxunique_lock lock(q.m_mutex_);

			if(task != hxnull) {
				// Finished reacquiring the critical section after the previous task.
				task = hxnull;
				hxassertmsg(q.m_executing_count_ > 0, "internal_error");
				if((--q.m_executing_count_ == 0) && q.m_tasks_.empty()) {
					q.m_cond_var_completion_.notify_all();
				}
			}

			// Workers wait for a next task or run_level_::stopped_. The
			// predicate handles spurious wakeups.
			if(mode == thread_mode_::pool_) {
				q.m_cond_var_new_tasks_.wait(lock, [&q](void) {
					return !q.m_tasks_.empty()
						|| q.m_queue_run_level_ == run_level_::stopped_;
				});
			}

			// Waiting threads contribute to the work.
			if(!q.m_tasks_.empty()) {
				task = q.m_tasks_.front().task;
				q.m_tasks_.pop_heap();
				++q.m_executing_count_;
			}
			else {
				// Nothing left for worker threads to do. Pool threads exit when
				// stopped. Waiting threads still have work to do before leaving.
				if(mode == thread_mode_::pool_) {
					return;
				}

				// All tasks are dispatched. Now wait for m_executing_count_ to
				// hit 0. Tasks may enqueue subtasks before processing is
				// considered done. Neither wait state should occur after
				// shutdown has started.
				q.m_cond_var_completion_.wait(lock, [&q](void) {
					hxassertmsg(q.m_queue_run_level_ == run_level_::running_,
						"threading_error");
					return q.m_tasks_.empty() && q.m_executing_count_ == 0;
				});

				// All tasks are now considered complete. The workers can be
				// released if the queue is shutting down.
				if(mode == thread_mode_::stopping_) {
					q.m_queue_run_level_ = run_level_::stopped_;
					q.m_cond_var_new_tasks_.notify_all();

					// This triggers a release assert in any unexpected waiting threads.
					q.m_cond_var_completion_.notify_all();
				}
				return;
			}
		}

		hxprofile_scope(task->get_label());

		// This is actually the last time this object is touched. It may delete or
		// re-enqueue itself. The queue is not locked and completion is not reported
		// until after the task is done.
		task->process(&q);
	}
}
#endif

HX_NS_END_
