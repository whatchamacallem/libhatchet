// SPDX-FileCopyrightText: © 2025 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

#include <hx/hxdeque.hpp>
#include <hx/hxtest.hpp>

HX_NS_USE

hxattr_noinline static void hxtest_gdb_break_hxdeque_static(void) { }
hxattr_noinline static void hxtest_gdb_break_hxdeque_dynamic(void) { }

namespace {

class hxdeque_test_f* hxs_deque_current = hxnull;
class hxdeque_test_f : public testing::Test {
public:
	class hxtest_object {
	public:
		hxtest_object(void) = delete;
		explicit hxtest_object(int32_t x) {
			++hxs_deque_current->m_constructed;
			id = x;
			moved_from = false;
		}
		~hxtest_object(void) {
			++hxs_deque_current->m_destructed;
			id = 0xefef;
			moved_from = true;
		}
		hxtest_object(const hxtest_object&) = delete;
		hxtest_object(hxtest_object&&) = delete;
		void operator=(const hxtest_object&) = delete;
		hxtest_object& operator=(hxtest_object&&) = delete;
		bool operator==(int32_t) const = delete;
		int32_t id;
		bool moved_from;
	};
	hxdeque_test_f(void) {
		hxassert(hxs_deque_current == hxnull);
		m_constructed = 0;
		m_destructed = 0;
		m_next_id = -1;
		hxs_deque_current = this;
	}
	~hxdeque_test_f(void) {
		hxassert_always(m_constructed == m_destructed,
			"hxdeque_test_f lifecycle mismatch");
		hxs_deque_current = hxnull;
	}
	hxsize_t m_constructed;
	hxsize_t m_destructed;
	int32_t m_next_id;
};
} // namespace

TEST(hxdeque_test, static_ctor_empty_state) {
	const hxdeque<int, 4> d;
	EXPECT_EQ(d.size(), 0);
	EXPECT_EQ(d.capacity(), 4);
	EXPECT_EQ(d.max_size(), 4);
	EXPECT_TRUE(d.empty());
	EXPECT_FALSE(d.full());
}

TEST(hxdeque_test, dynamic_ctor_empty_state) {
	const hxsystem_allocator_scope temporary_stack_scope(hxsystem_allocator_stack_0);
	const hxdeque<int> d(4);
	EXPECT_EQ(d.size(), 0);
	EXPECT_EQ(d.capacity(), 4);
	EXPECT_EQ(d.max_size(), 4);
	EXPECT_TRUE(d.empty());
	EXPECT_FALSE(d.full());
}

TEST(hxdeque_test, push_back_pop_front_fifo_order) {
	hxdeque<int, 4> d;
	d.push_back(10);
	d.push_back(20);
	d.push_back(30);
	EXPECT_EQ(d.size(), 3);
	EXPECT_FALSE(d.empty());
	int v = 0;
	v = d.front(); d.pop_front(); EXPECT_EQ(v, 10);
	v = d.front(); d.pop_front(); EXPECT_EQ(v, 20);
	v = d.front(); d.pop_front(); EXPECT_EQ(v, 30);
	EXPECT_EQ(d.size(), 0);
	EXPECT_TRUE(d.empty());
}

TEST(hxdeque_test, push_front_pop_back_order) {
	hxdeque<int, 4> d;
	d.push_front(10);
	d.push_front(20);
	d.push_front(30);
	EXPECT_EQ(d.size(), 3);
	int v = 0;
	v = d.back(); d.pop_back(); EXPECT_EQ(v, 10);
	v = d.back(); d.pop_back(); EXPECT_EQ(v, 20);
	v = d.back(); d.pop_back(); EXPECT_EQ(v, 30);
	EXPECT_TRUE(d.empty());
}

TEST(hxdeque_test, ring_wraparound_push_back_pop_front) {
	hxdeque<int, 4> d;
	d.push_back(1);
	d.push_back(2);
	d.push_back(3);
	d.push_back(4);
	int v = 0;
	v = d.front(); d.pop_front(); EXPECT_EQ(v, 1);
	v = d.front(); d.pop_front(); EXPECT_EQ(v, 2);
	v = d.front(); d.pop_front(); EXPECT_EQ(v, 3);
	d.push_back(10);
	d.push_back(11);
	d.push_back(12);
	v = d.front(); d.pop_front(); EXPECT_EQ(v, 4);
	v = d.front(); d.pop_front(); EXPECT_EQ(v, 10);
	v = d.front(); d.pop_front(); EXPECT_EQ(v, 11);
	v = d.front(); d.pop_front(); EXPECT_EQ(v, 12);
	EXPECT_TRUE(d.empty());
}

TEST(hxdeque_test, ring_wraparound_push_front_pop_back) {
	hxdeque<int, 4> d;
	d.push_front(4);
	d.push_front(3);
	d.push_front(2);
	d.push_front(1);
	int v = 0;
	v = d.back(); d.pop_back(); EXPECT_EQ(v, 4);
	v = d.back(); d.pop_back(); EXPECT_EQ(v, 3);
	d.push_front(0);
	d.push_front(-1);
	EXPECT_EQ(d[0], -1);
	EXPECT_EQ(d[1],  0);
	EXPECT_EQ(d[2],  1);
	EXPECT_EQ(d[3],  2);
	hxtest_gdb_break_hxdeque_static();
}

TEST_F(hxdeque_test_f, emplace_back_constructs_in_place) {
	hxdeque<hxtest_object, 4> d;
	d.emplace_back(34);
	EXPECT_EQ(d.back().id, 34);
	EXPECT_FALSE(d.back().moved_from);
	EXPECT_EQ(d.size(), 1);
	EXPECT_EQ(m_constructed, 1);
	EXPECT_EQ(m_destructed, 0);
}

TEST_F(hxdeque_test_f, emplace_front_constructs_in_place) {
	hxdeque<hxtest_object, 4> d;
	d.emplace_back(10);
	d.emplace_front(99);
	EXPECT_EQ(d.size(), 2);
	EXPECT_EQ(d.front().id, 99);
	EXPECT_EQ(d.back().id, 10);
}

TEST_F(hxdeque_test_f, emplace_back_forward_multiple_args) {
	struct hxtest_pair_t {
		hxtest_pair_t(int a, int b) : x(a), y(b) { }
		int x, y;
	};
	hxdeque<hxtest_pair_t, 2> d;
	d.emplace_back(3, 7);
	EXPECT_EQ(d.back().x, 3);
	EXPECT_EQ(d.back().y, 7);
}

TEST(hxdeque_test, front_and_back_multiple_elements) {
	hxdeque<int, 4> d;
	d.push_back(1);
	d.push_back(2);
	d.push_back(3);
	EXPECT_EQ(d.front(), 1);
	EXPECT_EQ(d.back(), 3);
}

TEST(hxdeque_test, const_front_and_back) {
	hxdeque<int, 4> d;
	d.push_back(7);
	d.push_back(8);
	const hxdeque<int, 4>& cd = d;
	EXPECT_EQ(cd.front(), 7);
	EXPECT_EQ(cd.back(), 8);
}

TEST(hxdeque_test, front_back_reference_mutation) {
	hxdeque<int, 4> d;
	d.push_back(1);
	d.push_back(2);
	d.front() = 10;
	d.back()  = 20;
	EXPECT_EQ(d[0], 10);
	EXPECT_EQ(d[1], 20);
}

TEST(hxdeque_test, operator_index_sequential) {
	hxdeque<int, 8> d;
	for(int i = 0; i < 5; ++i) {
		d.push_back(i * 10);
	}
	for(hxsize_t i = 0; i < 5; ++i) {
		EXPECT_EQ(d[i], static_cast<int>(i * 10));
	}
}

TEST(hxdeque_test, const_operator_index) {
	hxdeque<int, 4> d;
	d.push_back(3);
	d.push_back(6);
	const hxdeque<int, 4>& cd = d;
	EXPECT_EQ(cd[0], 3);
	EXPECT_EQ(cd[1], 6);
}

TEST(hxdeque_test, at_const_and_non_const) {
	hxdeque<int, 4> d;
	d.push_back(3);
	d.push_back(6);
	d.at(0) = 30;
	EXPECT_EQ(d.at(0), 30);
	EXPECT_EQ(d.at(1), 6);
	const hxdeque<int, 4>& cd = d;
	EXPECT_EQ(cd.at(0), 30);
	EXPECT_EQ(cd.at(1), 6);
}

TEST(hxdeque_test, operator_index_after_wraparound) {
	hxdeque<int, 4> d;
	d.push_back(1); d.push_back(2);
	d.push_back(3); d.push_back(4);
	d.pop_front(); d.pop_front();
	d.push_back(5); d.push_back(6);
	EXPECT_EQ(d[0], 3);
	EXPECT_EQ(d[1], 4);
	EXPECT_EQ(d[2], 5);
	EXPECT_EQ(d[3], 6);
}

TEST_F(hxdeque_test_f, clear_destroys_all_elements) {
	hxdeque<hxtest_object, 4> d;
	d.emplace_back(1);
	d.emplace_back(2);
	d.emplace_back(3);
	EXPECT_EQ(m_constructed, 3);
	EXPECT_EQ(m_destructed, 0);
	d.clear();
	EXPECT_EQ(m_destructed, 3);
	EXPECT_EQ(d.size(), 0);
	EXPECT_TRUE(d.empty());
}

TEST_F(hxdeque_test_f, clear_empty_deque_is_noop) {
	hxdeque<hxtest_object, 4> d;
	d.clear();
	EXPECT_EQ(m_destructed, 0);
}

TEST_F(hxdeque_test_f, clear_after_ring_wraparound_destroys_all) {
	hxdeque<hxtest_object, 4> d;
	for(int i = 0; i < 4; ++i) { d.emplace_back(i); }
	d.pop_front();
	d.pop_front();
	d.emplace_back(100);
	d.emplace_back(101);
	EXPECT_EQ(d.size(), 4);
	const hxsize_t before_clear = m_destructed;
	d.clear();
	EXPECT_EQ(m_destructed, before_clear + 4);
	EXPECT_TRUE(d.empty());
}

TEST_F(hxdeque_test_f, destructor_calls_clear) {
	{
		hxdeque<hxtest_object, 4> d;
		d.emplace_back(7);
		d.emplace_back(8);
		EXPECT_EQ(m_constructed, 2);
	}
	EXPECT_EQ(m_destructed, 2);
}

TEST(hxdeque_test, full_predicate) {
	hxdeque<int, 2> d;
	EXPECT_FALSE(d.full());
	d.push_back(1);
	EXPECT_FALSE(d.full());
	d.push_back(2);
	EXPECT_TRUE(d.full());
	d.pop_front();
	EXPECT_FALSE(d.full());
}

TEST_F(hxdeque_test_f, pop_front_destroys_slot) {
	hxdeque<hxtest_object, 4> d;
	d.emplace_back(55);
	EXPECT_EQ(m_constructed, 1);
	const hxsize_t d_before = m_destructed;
	d.pop_front();
	EXPECT_EQ(m_destructed, d_before + 1);
	EXPECT_TRUE(d.empty());
}

TEST_F(hxdeque_test_f, pop_back_destroys_slot) {
	hxdeque<hxtest_object, 4> d;
	d.emplace_back(77);
	const hxsize_t d_before = m_destructed;
	d.pop_back();
	EXPECT_EQ(m_destructed, d_before + 1);
	EXPECT_TRUE(d.empty());
}

TEST(hxdeque_test, dynamic_capacity_push_pop) {
	const hxsystem_allocator_scope temporary_stack_scope(hxsystem_allocator_stack_0);
	hxdeque<int> d(4);
	EXPECT_EQ(d.capacity(), 4);
	for(int i = 0; i < 4; ++i) {
		d.push_back(i);
	}
	EXPECT_TRUE(d.full());
	for(int i = 0; i < 4; ++i) {
		const int v = d.front();
		d.pop_front();
		EXPECT_EQ(v, i);
	}
	EXPECT_TRUE(d.empty());
}

TEST(hxdeque_test, dynamic_capacity_ring_wraparound) {
	const hxsystem_allocator_scope temporary_stack_scope(hxsystem_allocator_stack_0);
	hxdeque<int> d(8);
	for(int i = 0; i < 8; ++i) { d.push_back(i); }
	for(int i = 0; i < 4; ++i) { d.pop_front(); }
	for(int i = 8; i < 12; ++i) { d.push_back(i); }
	for(int i = 0; i < 8; ++i) {
		EXPECT_EQ(d[static_cast<hxsize_t>(i)], i + 4);
	}
	hxtest_gdb_break_hxdeque_dynamic();
}

TEST(hxdeque_test, reserve_sets_capacity_and_mask) {
	const hxsystem_allocator_scope temporary_stack_scope(hxsystem_allocator_stack_0);
	hxdeque<int> d;
	EXPECT_EQ(d.capacity(), 0);
	d.reserve(8);
	EXPECT_EQ(d.capacity(), 8);
	EXPECT_TRUE(d.empty());
	EXPECT_FALSE(d.full());
}

TEST(hxdeque_test, interleaved_push_back_push_front) {
	hxdeque<int, 8> d;
	d.push_back(3);
	d.push_front(2);
	d.push_back(4);
	d.push_front(1);
	EXPECT_EQ(d.size(), 4);
	EXPECT_EQ(d[0], 1);
	EXPECT_EQ(d[1], 2);
	EXPECT_EQ(d[2], 3);
	EXPECT_EQ(d[3], 4);
}

TEST(hxdeque_test, interleaved_pop_back_pop_front) {
	hxdeque<int, 4> d;
	d.push_back(10);
	d.push_back(20);
	d.push_back(30);
	int v = 0;
	v = d.front(); d.pop_front(); EXPECT_EQ(v, 10);
	v = d.back();  d.pop_back();  EXPECT_EQ(v, 30);
	EXPECT_EQ(d.size(), 1);
	EXPECT_EQ(d.front(), 20);
	EXPECT_EQ(d.back(), 20);
}

TEST(hxdeque_test, single_element_capacity) {
	hxdeque<int, 1> d;
	EXPECT_TRUE(d.empty());
	EXPECT_EQ(d.capacity(), 1);
	d.push_back(34);
	EXPECT_TRUE(d.full());
	EXPECT_EQ(d.front(), 34);
	EXPECT_EQ(d.back(), 34);
	int v = 0;
	v = d.front(); d.pop_front();
	EXPECT_EQ(v, 34);
	EXPECT_TRUE(d.empty());
}

TEST(hxdeque_test, capacity_two_full_wraparound) {
	hxdeque<int, 2> d;
	d.push_back(1); d.push_back(2);
	int v = 0;
	d.pop_front();
	d.push_back(3);
	EXPECT_EQ(d[0], 2);
	EXPECT_EQ(d[1], 3);
	v = d.back(); d.pop_back(); EXPECT_EQ(v, 3);
	v = d.back(); d.pop_back(); EXPECT_EQ(v, 2);
	EXPECT_TRUE(d.empty());
}

TEST(hxdeque_test, push_front_wraps_head_from_slot_zero) {
	hxdeque<int, 4> d;
	d.push_back(10);
	d.push_back(20);
	d.push_front(5);
	EXPECT_EQ(d.size(), 3);
	EXPECT_EQ(d.front(), 5);
	EXPECT_EQ(d[0], 5);
	EXPECT_EQ(d[1], 10);
	EXPECT_EQ(d[2], 20);
}

TEST(hxdeque_test, pop_front_head_wraps_from_last_slot) {
	hxdeque<int, 4> d;
	d.push_front(99);
	d.push_back(10);
	d.push_back(20);
	const int v = d.front();
	d.pop_front();
	EXPECT_EQ(v, 99);
	EXPECT_EQ(d.size(), 2);
	EXPECT_EQ(d.front(), 10);
}

TEST(hxdeque_test, back_when_tail_is_at_slot_zero) {
	hxdeque<int, 4> d;
	d.push_back(1);
	d.push_back(2);
	d.push_back(3);
	d.push_back(4);
	EXPECT_EQ(d.back(), 4);
	EXPECT_EQ(d[3], 4);
}

TEST(hxdeque_test, operator_index_last_after_head_advance) {
	hxdeque<int, 4> d;
	d.push_back(1); d.push_back(2); d.push_back(3);
	d.pop_front();
	d.push_back(4);
	EXPECT_EQ(d[2], 4);
	EXPECT_EQ(d[0], 2);
}
