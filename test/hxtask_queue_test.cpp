// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

#include <hx/hxtask_queue.hpp>
#include <hx/hxtest.hpp>
#include <hx/hxarray.hpp>

HX_NS_USE

namespace {

hxinline_constexpr int32_t hxs_max_pool = 8;
hxinline_constexpr int32_t hxs_max_tasks = 20;

class hxtask_queue_test_f :
	public testing::Test
{
public:
	class hxtask_test_t : public hxtask {
	public:
		hxtask_test_t() : m_exec_count(0), m_reenqueue_count(0) { }
		bool execute(hxtask_queue* q) override {
			const int32_t count = ++m_exec_count;
			if(m_reenqueue_count > 0) {
				--m_reenqueue_count;
				q->enqueue(this);
			}
			return (count & 1) == 0;
		}
		int32_t get_exec_count(void) const { return m_exec_count; }
		void set_reenqueue_count(int32_t n) { m_reenqueue_count = n; }
	private:
		int32_t m_exec_count;
		int32_t m_reenqueue_count;
	};
};
} // namespace {

TEST_F(hxtask_queue_test_f, nop) {
	const hxsystem_allocator_scope temporary_stack_scope(hxsystem_allocator_stack_0);
	for(int32_t i = 0; i <= hxs_max_pool; ++i) {
		{
				const hxtask_queue q(1, i);
		}
		{
			hxtask_queue q(1, i);
			q.wait_for_all();
		}
	}
	EXPECT_TRUE(true);
}

TEST_F(hxtask_queue_test_f, multiple) {
	const hxsystem_allocator_scope temporary_stack_scope(hxsystem_allocator_stack_0);
	for(int32_t i = 0; i <= hxs_max_pool; ++i) {
		for(int32_t j = 1; j < hxs_max_tasks; ++j) {
			hxtask_test_t tasks0[hxs_max_tasks];
			hxtask_test_t tasks1[hxs_max_tasks];
		{
			hxtask_queue q(hxs_max_tasks, i);
			for(int32_t k = 0; k <= j; ++k) {
				q.enqueue(&tasks0[k]);
			}
			q.wait_for_all();
			for(int32_t k = 0; k <= j; ++k) {
				q.enqueue(&tasks1[k]);
				EXPECT_EQ(tasks0[k].get_exec_count(), 1);
			}
			}
			for(int32_t k = 0; k <= j; ++k) {
				EXPECT_EQ(tasks0[k].get_exec_count(), 1);
				EXPECT_EQ(tasks1[k].get_exec_count(), 1);
			}
			hxtask_test_t tasks2[hxs_max_tasks];
			{
				hxtask_queue q(hxs_max_tasks, i);
				for(int32_t k = 0; k <= j; ++k) {
					q.enqueue(&tasks2[k]);
				}
			}
			for(int32_t k = 0; k <= j; ++k) {
				EXPECT_EQ(tasks2[k].get_exec_count(), 1);
			}
		}
	}
}

TEST_F(hxtask_queue_test_f, multiple_reenqueuing) {
	const hxsystem_allocator_scope temporary_stack_scope(hxsystem_allocator_stack_0);
	for(int32_t i = 0; i <= hxs_max_pool; ++i) {
		for(int32_t j = 1; j < hxs_max_tasks; ++j) {
			hxtask_test_t tasks0[hxs_max_tasks];
			hxtask_test_t tasks1[hxs_max_tasks];
			{
				hxtask_queue q(hxs_max_tasks, i);
				for(int32_t k = 0; k <= j; ++k) {
					tasks0[k].set_reenqueue_count(k);
					q.enqueue(&tasks0[k]);
				}
				q.wait_for_all();
				for(int32_t k = 0; k <= j; ++k) {
					tasks1[k].set_reenqueue_count(k);
					q.enqueue(&tasks1[k]);
				}
			}
			for(int32_t k = 0; k <= j; ++k) {
				EXPECT_EQ(tasks0[k].get_exec_count(), (k + 1));
				EXPECT_EQ(tasks1[k].get_exec_count(), (k + 1));
			}
			hxtask_test_t tasks2[hxs_max_tasks];
			{
				hxtask_queue q(hxs_max_tasks, i);
				for(int32_t k = 0; k <= j; ++k) {
					tasks2[k].set_reenqueue_count(k);
					q.enqueue(&tasks2[k]);
				}
			}
			for(int32_t k = 0; k <= j; ++k) {
				EXPECT_EQ(tasks2[k].get_exec_count(), (k + 1));
			}
		}
	}
}

TEST(hxtask_queue_test, priority_ordering_single_threaded) {
	const hxsystem_allocator_scope temporary_stack_scope(hxsystem_allocator_stack_0);
	class hxtask_queue_test_task_t : public hxtask {
	public:
		void configure(int* o, int32_t* i, int p) {
			execution_order = o;
			write_index = i;
			priority_value = p;
		}
		bool execute(hxtask_queue*) override {
			hxassertmsg(execution_order, "priority_task_unconfigured");
			hxassertmsg(write_index, "priority_task_unconfigured");
			const int32_t slot = (*write_index)++;
			execution_order[slot] = priority_value;
			return true;
		}
	private:
		int* execution_order = hxnull;
		int32_t* write_index = hxnull;
		int priority_value = 0;
	};
	const int32_t task_count = 5;
	int execution_order[task_count] = { 0, 0, 0, 0, 0 };
	int32_t write_index = 0;
	hxtask_queue_test_task_t tasks[task_count];
	const int priorities[task_count] = { 1, 3, -5, 2, 10 };
	hxtask_queue q(task_count, 0);
	for(int32_t i = 0; i < task_count; ++i) {
		tasks[i].configure(execution_order, &write_index, priorities[i]);
		q.enqueue(&tasks[i], priorities[i]);
	}
	q.wait_for_all();
	EXPECT_EQ(write_index, task_count);
	const int expected[task_count] = { 10, 3, 2, 1, -5 };
	for(int32_t i = 0; i < task_count; ++i) {
		EXPECT_EQ(execution_order[i], expected[i]);
	}
}

TEST(hxtask_queue_test, predicates_cover_all_any_erase) {
	const hxsystem_allocator_scope temporary_stack_scope(hxsystem_allocator_stack_0);
	class hxtask_queue_test_predicate_task_t : public hxtask {
	public:
		void configure(bool* f, bool* c) { executed_flag = f; cancelled_flag = c; }
		bool execute(hxtask_queue*) override { *executed_flag = true; return true; }
		// GCOVR_EXCL_START
		void on_cancel(hxtask_queue*) override { hxassert_hard(false, "unused_overload"); }
		// GCOVR_EXCL_STOP
	private:
		bool* executed_flag = hxnull;
		bool* cancelled_flag = hxnull;
	};
	bool executed_flags[3] = { false, false, false };
	bool cancelled_flags[3] = { false, false, false };
	hxtask_queue_test_predicate_task_t tasks[3];
	for(int32_t i = 0; i < 3; ++i) {
		tasks[i].configure(&executed_flags[i], &cancelled_flags[i]);
	}
	hxtask_queue q(3, 0);
	q.enqueue(&tasks[0], 5);
	q.enqueue(&tasks[1], 10);
	q.enqueue(&tasks[2], 1);
	EXPECT_EQ(q.max_size(), 3);
	EXPECT_EQ(q.size(), 3);
	EXPECT_TRUE(q.full());
	EXPECT_TRUE(!q.empty());
	bool visited[3] = { false, false, false };
	int32_t visit_count = 0;
	q.for_each([&](hxtask_queue::record_t& record) {
		++visit_count;
		if(record.task == &tasks[0]) {
			visited[0] = true;
		} else if(record.task == &tasks[1]) {
			visited[1] = true;
		} else if(record.task == &tasks[2]) {
			visited[2] = true;
		}
	});
	EXPECT_EQ(visit_count, 3);
	for(int32_t i = 0; i < 3; ++i) {
		EXPECT_TRUE(visited[i]);
	}
	const bool all_priority_non_negative = q.all_of([](const hxtask_queue::record_t& record) {
		return record.priority >= 0;
	});
	EXPECT_TRUE(all_priority_non_negative);
	const bool any_high_priority = q.any_of([](const hxtask_queue::record_t& record) {
		return record.priority > 8;
	});
	EXPECT_TRUE(any_high_priority);
	const int32_t removed_low_priority = q.erase_if([](const hxtask_queue::record_t& record) {
		return record.priority < 4;
	});
	EXPECT_EQ(removed_low_priority, 1);
	EXPECT_EQ(q.size(), 2);
	EXPECT_TRUE(!q.full());
	EXPECT_FALSE(cancelled_flags[2]);
	const bool any_remaining_low_priority = q.any_of([](const hxtask_queue::record_t& record) {
		return record.priority < 4;
	});
	EXPECT_TRUE(!any_remaining_low_priority);
	q.wait_for_all();
	EXPECT_TRUE(q.empty());
	EXPECT_TRUE(executed_flags[0]);
	EXPECT_TRUE(executed_flags[1]);
	EXPECT_TRUE(!executed_flags[2]);
	executed_flags[2] = false;
	q.enqueue(&tasks[2], 7);
	EXPECT_EQ(q.size(), 1);
	EXPECT_FALSE(q.empty());
	q.clear();
	EXPECT_EQ(q.size(), 0);
	EXPECT_TRUE(q.empty());
	q.wait_for_all();
	EXPECT_TRUE(!executed_flags[2]);
	EXPECT_FALSE(cancelled_flags[2]);
}

TEST(hxtask_queue_test, for_each_reschedules_queue) {
	const hxsystem_allocator_scope temporary_stack_scope(hxsystem_allocator_stack_0);
	class hxtask_queue_test_reschedule_task_t : public hxtask {
	public:
		void configure(int32_t i, int* e, int32_t* w) {
			task_index = i;
			execution_order = e;
			write_index = w;
		}
		bool execute(hxtask_queue*) override {
				const int32_t slot = (*write_index)++;
			execution_order[slot] = static_cast<int>(task_index);
			return true;
		}
		int32_t get_index(void) const { return task_index; }
	private:
		int32_t task_index = 0;
		int* execution_order = hxnull;
		int32_t* write_index = hxnull;
	};
	const int32_t task_count = 4;
	hxtask_queue_test_reschedule_task_t tasks[task_count];
	int execution_order[task_count] = { -1, -1, -1, -1 };
	int32_t write_index = 0;
	const int initial_priorities[task_count] = { 0, 0, 0, 0 };
	const int rescheduled_priorities[task_count] = { 4, -3, 9, 1 };
	hxtask_queue q(task_count, 0);
	for(int32_t i = 0; i < task_count; ++i) {
		tasks[i].configure(i, execution_order, &write_index);
		q.enqueue(&tasks[i], initial_priorities[i]);
	}
	int32_t mutate_count = 0;
	q.for_each([&](hxtask_queue::record_t& record) {
		const hxtask_queue_test_reschedule_task_t* const task =
			static_cast<hxtask_queue_test_reschedule_task_t*>(record.task);
			const int32_t index = task->get_index();
		record.priority = rescheduled_priorities[index];
		++mutate_count;
	});
	EXPECT_EQ(mutate_count, task_count);
	int32_t verify_count = 0;
	const hxtask_queue& const_q = q;
	const_q.for_each([&](const hxtask_queue::record_t& record) {
		const hxtask_queue_test_reschedule_task_t* task =
			static_cast<const hxtask_queue_test_reschedule_task_t*>(record.task);
			const int32_t index = task->get_index();
		EXPECT_EQ(record.priority, rescheduled_priorities[index]);
		++verify_count;
	});
	EXPECT_EQ(verify_count, task_count);
	q.wait_for_all();
	EXPECT_EQ(write_index, task_count);
	EXPECT_EQ(execution_order[0], 2);
	EXPECT_EQ(execution_order[1], 0);
	EXPECT_EQ(execution_order[2], 3);
	EXPECT_EQ(execution_order[3], 1);
}

#if HX_USE_THREADS
TEST(hxtask_queue_test, three_threads_three_stacks_stress) {
	class hxtask_queue_test_stack_stress_task_t : public hxtask {
	public:
		void configure(hxsystem_allocator_t a, int32_t r) {
			allocator = a;
			reenqueue_count = r;
		}
		bool execute(hxtask_queue* q) override {
			{
				const hxsystem_allocator_scope stack_scope(allocator);
				const int32_t allocation_count = 8;
				const size_t bytes = 16u;
				hxarray<void*> allocations;
				allocations.reserve(allocation_count, allocator);
				for(int32_t i = 0; i < allocation_count; ++i) {
					void* p = hxmalloc(bytes);
					hxassertmsg(p, "stress_alloc");
					::memset(p, 0x5a, bytes);
					allocations[i] = p;
				}
				for(int32_t i = 0; i < allocation_count; ++i) {
					hxfree(allocations[i]);
				}
			}
			++total_exec_count;
			if(reenqueue_count > 0) {
				--reenqueue_count;
				q->enqueue(this);
			}
			return true;
		}
		int32_t get_total_exec_count(void) const { return total_exec_count; }
	private:
		hxsystem_allocator_t allocator = hxsystem_allocator_stack_0;
		int32_t reenqueue_count = 0;
		int32_t total_exec_count = 0;
	};
	const int32_t thread_count = 3;
	const int32_t reenqueue_count = 5;
	hxtask_queue_test_stack_stress_task_t tasks[thread_count];
	hxtask_queue q(thread_count, thread_count);
	for(int32_t i = 0; i < thread_count; ++i) {
		tasks[i].configure(static_cast<hxsystem_allocator_t>(
			hxsystem_allocator_stack_0 + i), reenqueue_count);
		q.enqueue(&tasks[i]);
	}
	q.wait_for_all();
	for(int32_t i = 0; i < thread_count; ++i) {
		EXPECT_EQ(tasks[i].get_total_exec_count(), reenqueue_count + 1);
	}
}

TEST(hxtask_queue_test, wait_for_all_with_thread_pool) {
	const hxsystem_allocator_scope temporary_stack_scope(hxsystem_allocator_stack_0);
	class hxtask_queue_test_counter_task_t : public hxtask {
	public:
		bool execute(hxtask_queue*) override { ++count; return true; }
		int32_t count = 0;
	};
	const int32_t task_count = 4;
	hxtask_queue_test_counter_task_t tasks[task_count];
	hxtask_queue q(task_count, 2);
	for(int32_t i = 0; i < task_count; ++i) {
		q.enqueue(&tasks[i]);
	}
	q.wait_for_all();
	for(int32_t i = 0; i < task_count; ++i) {
		EXPECT_EQ(tasks[i].count, 1);
	}
}
#endif

TEST(hxtask_queue_test, size_boundaries) {
	const hxsystem_allocator_scope temporary_stack_scope(hxsystem_allocator_stack_0);
	class hxtask_queue_test_nop_task_t : public hxtask {
	public:
		bool execute(hxtask_queue*) override { return true; }
	};
	hxtask_queue_test_nop_task_t t0, t1, t2;
	hxtask_queue q(3, 0);
	EXPECT_EQ(q.size(), 0);
	EXPECT_EQ(q.max_size(), 3);
	EXPECT_TRUE(q.empty());
	EXPECT_FALSE(q.full());
	q.enqueue(&t0);
	EXPECT_EQ(q.size(), 1);
	EXPECT_FALSE(q.empty());
	EXPECT_FALSE(q.full());
	q.enqueue(&t1);
	q.enqueue(&t2);
	EXPECT_EQ(q.size(), 3);
	EXPECT_EQ(q.size(), q.max_size());
	EXPECT_TRUE(q.full());
	EXPECT_FALSE(q.empty());
	q.wait_for_all();
	EXPECT_EQ(q.size(), 0);
	EXPECT_TRUE(q.empty());
	EXPECT_FALSE(q.full());
}

TEST(hxtask_queue_test, erase_if_count_boundaries) {
	const hxsystem_allocator_scope temporary_stack_scope(hxsystem_allocator_stack_0);
	class hxtask_queue_test_nop_task_t : public hxtask {
	public:
		// GCOVR_EXCL_START
		bool execute(hxtask_queue*) override { hxassert_hard(false, "unused_overload"); return true; }
		// GCOVR_EXCL_STOP
	};
	hxtask_queue_test_nop_task_t t0, t1, t2, t3, t4;
	hxtask_queue q(5, 0);
	q.enqueue(&t0, 1);
	q.enqueue(&t1, 2);
	q.enqueue(&t2, 3);
	q.enqueue(&t3, 4);
	q.enqueue(&t4, 5);
	const int32_t removed0 = q.erase_if([](const hxtask_queue::record_t& r) {
		return r.priority > 10;
	});
	EXPECT_EQ(removed0, 0);
	EXPECT_EQ(q.size(), 5);
	const int32_t removed1 = q.erase_if([](const hxtask_queue::record_t& r) {
		return r.priority < 2;
	});
	EXPECT_EQ(removed1, 1);
	EXPECT_EQ(q.size(), 4);
	const int32_t removed2 = q.erase_if([](const hxtask_queue::record_t& r) {
		return r.priority > 4;
	});
	EXPECT_EQ(removed2, 1);
	EXPECT_EQ(q.size(), 3);
	const int32_t removed3 = q.erase_if([](const hxtask_queue::record_t&) {
		return true;
	});
	EXPECT_EQ(removed3, 3);
	EXPECT_EQ(q.size(), 0);
	EXPECT_TRUE(q.empty());
}

TEST(hxtask_queue_test, canceling) {
	const hxsystem_allocator_scope temporary_stack_scope(hxsystem_allocator_stack_0);
	class hxtask_queue_test_cancel_tracker_t : public hxtask {
	public:
		bool execute(hxtask_queue*) override { executed = true; return true; }
		void on_cancel(hxtask_queue*) override { cancelled = true; }
		bool executed = false;
		bool cancelled = false;
	};
	hxtask_queue_test_cancel_tracker_t t0, t1, t2, t3;
	hxtask_queue q(3, 0);
	q.enqueue(&t0);
	q.enqueue(&t1);
	q.enqueue(&t2);
	EXPECT_TRUE(q.cancel(&t1));
	EXPECT_FALSE(q.cancel(&t1));
	q.wait_for_all();
	EXPECT_TRUE(t0.executed);
	EXPECT_FALSE(t0.cancelled);
	EXPECT_FALSE(t1.executed);
	EXPECT_TRUE(t1.cancelled);
	EXPECT_TRUE(t2.executed);
	EXPECT_FALSE(t2.cancelled);
	q.enqueue(&t3);
	q.clear();
	EXPECT_FALSE(t3.executed);
	EXPECT_FALSE(t3.cancelled);
}

TEST(hxtask_queue_test, canceling_task_without_on_cancel_override) {
	const hxsystem_allocator_scope temporary_stack_scope(hxsystem_allocator_stack_0);
	class hxtask_queue_test_default_cancel_t : public hxtask {
	public:
		// GCOVR_EXCL_START
		bool execute(hxtask_queue*) override { hxassert_hard(false, "unused_overload"); return true; }
		// GCOVR_EXCL_STOP
		bool executed = false;
	};
	hxtask_queue_test_default_cancel_t t0;
	hxtask_queue q(1, 0);
	q.enqueue(&t0);
	EXPECT_TRUE(q.cancel(&t0));
	q.wait_for_all();
	EXPECT_FALSE(t0.executed);
}
