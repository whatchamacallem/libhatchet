// SPDX-FileCopyrightText: © 2025 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

#include <hx/hxdeque.hpp>
#include "./hxtest_util.hpp"

HX_NS_USE

hxattr_noinline static void hxtest_gdb_break_hxdeque_static(void) { }
hxattr_noinline static void hxtest_gdb_break_hxdeque_dynamic(void) { }

#if !defined _MSC_VER && !defined __wasm__
static_assert(sizeof(size_t) != 4 || (
		sizeof(hxdeque<int32_t, 4>) == 24u
		&& sizeof(hxdeque<int32_t, hxallocator_dynamic_capacity>) == 16u),
	"hxdeque must pack fixed storage as capacity * sizeof(T) plus two"
	" hxsize_t indices and dynamic storage as a hxsize_t, a T*, and two"
	" hxsize_t indices with no padding");
static_assert(sizeof(size_t) != 8 || (
		sizeof(hxdeque<int32_t, 4>) == 32u
		&& sizeof(hxdeque<int32_t, hxallocator_dynamic_capacity>) == 32u),
	"hxdeque must pack fixed storage as capacity * sizeof(T) plus two"
	" hxsize_t indices and dynamic storage as a hxsize_t, a T*, and two"
	" hxsize_t indices with no padding");
#endif

using hxdeque_test_f = hxtest_object_fixture;

TEST(hxdeque_test, operator_index) {
	hxdeque<int, 8> d;
	for(int i = 0; i < 5; ++i) { d.push_back(i * 10); }
	for(hxsize_t i = 0; i < 5; ++i) { EXPECT_EQ(d[i], static_cast<int>(i * 10)); }
	d.front() = 11;
	d.back() = 22;
	EXPECT_EQ(d[0], 11);
	EXPECT_EQ(d[4], 22);
	const hxdeque<int, 8>& cd = d;
	EXPECT_EQ(cd[0], 11);
	EXPECT_EQ(cd[4], 22);
	d.pop_front(); d.pop_front();
	d.push_back(4); d.push_back(5);
	EXPECT_EQ(d[0], 20);
	EXPECT_EQ(d[1], 30);
	EXPECT_EQ(d[2], 22);
	EXPECT_EQ(d[3], 4);
	EXPECT_EQ(d[4], 5);
}

TEST(hxdeque_test, at) {
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

TEST(hxdeque_test, back) {
	hxdeque<int, 4> d;
	d.push_back(1);
	d.push_back(2);
	d.push_back(3);
	d.push_back(4);
	EXPECT_EQ(d.back(), 4);
	d.back() = 40;
	EXPECT_EQ(d[3], 40);
	const hxdeque<int, 4>& cd = d;
	EXPECT_EQ(cd.back(), 40);
}

TEST(hxdeque_test, static_ctor_dtor) {
	const hxdeque<int, 4> d;
	EXPECT_EQ(d.size(), 0);
	EXPECT_EQ(d.capacity(), 4);
	EXPECT_EQ(d.max_size(), 4);
	EXPECT_TRUE(d.empty());
	EXPECT_FALSE(d.full());
}

TEST(hxdeque_test, dynamic_ctor_dtor) {
	const hxsystem_allocator_scope temporary_stack_scope(hxsystem_allocator_stack_0);
	const hxdeque<int> unallocated;
	EXPECT_EQ(unallocated.capacity(), 0);
	const hxdeque<int> d(4);
	EXPECT_EQ(d.size(), 0);
	EXPECT_EQ(d.capacity(), 4);
	EXPECT_EQ(d.max_size(), 4);
	EXPECT_TRUE(d.empty());
	EXPECT_FALSE(d.full());
}

TEST_F(hxdeque_test_f, destructor_calls_clear) {
	{
		hxdeque<hxtest_object, 4> d;
		d.emplace_back(7);
		d.emplace_back(8);
		EXPECT_TRUE(check_stats(2, 0, 0, 0, 0, 0, 0, 0));
	}
	EXPECT_TRUE(check_stats(2, 2, 0, 0, 0, 0, 0, 0));
}

TEST(hxdeque_test, capacity) {
	const hxsystem_allocator_scope temporary_stack_scope(hxsystem_allocator_stack_0);
	hxdeque<int> d;
	EXPECT_EQ(d.capacity(), 0);
	d.reserve(8);
	EXPECT_EQ(d.capacity(), 8);
	const hxdeque<int, 1> s;
	EXPECT_EQ(s.capacity(), 1);
}

TEST_F(hxdeque_test_f, clear) {
	hxdeque<hxtest_object, 4> d;
	d.clear();
	EXPECT_TRUE(check_stats(0, 0, 0, 0, 0, 0, 0, 0));
	d.emplace_back(1);
	d.emplace_back(2);
	d.emplace_back(3);
	EXPECT_TRUE(check_stats(3, 0, 0, 0, 0, 0, 0, 0));
	d.clear();
	EXPECT_EQ(d.size(), 0);
	EXPECT_TRUE(d.empty());
	EXPECT_TRUE(check_stats(3, 3, 0, 0, 0, 0, 0, 0));
}

TEST_F(hxdeque_test_f, clear_after_ring_wraparound) {
	hxdeque<hxtest_object, 4> d;
	for(int i = 0; i < 4; ++i) { d.emplace_back(i); }
	d.pop_front();
	d.pop_front();
	d.emplace_back(100);
	d.emplace_back(101);
	EXPECT_EQ(d.size(), 4);
	d.clear();
	EXPECT_TRUE(d.empty());
	EXPECT_TRUE(check_stats(6, 6, 0, 0, 0, 0, 0, 0));
}

TEST_F(hxdeque_test_f, emplace_back) {
	hxdeque<hxtest_object, 4> d;
	d.emplace_back(34);
	EXPECT_EQ(d.back().value, 34);
	EXPECT_FALSE(d.back().moved_from);
	EXPECT_EQ(d.size(), 1);
	struct hxtest_pair_t {
		hxtest_pair_t(int a, int b) : x(a), y(b) { }
		int x, y;
	};
	hxdeque<hxtest_pair_t, 2> pd;
	pd.emplace_back(3, 7);
	EXPECT_EQ(pd.back().x, 3);
	EXPECT_EQ(pd.back().y, 7);
	EXPECT_TRUE(check_stats(1, 0, 0, 0, 0, 0, 0, 0));
}

TEST_F(hxdeque_test_f, emplace_front) {
	hxdeque<hxtest_object, 4> d;
	d.emplace_back(10);
	d.emplace_front(99);
	EXPECT_EQ(d.size(), 2);
	EXPECT_EQ(d.front().value, 99);
	EXPECT_EQ(d.back().value, 10);
	EXPECT_TRUE(check_stats(2, 0, 0, 0, 0, 0, 0, 0));
}

TEST(hxdeque_test, empty) {
	hxdeque<int, 2> d;
	EXPECT_TRUE(d.empty());
	d.push_back(1);
	EXPECT_FALSE(d.empty());
	d.pop_front();
	EXPECT_TRUE(d.empty());
}

TEST(hxdeque_test, front) {
	hxdeque<int, 4> d;
	d.push_front(4);
	d.push_front(3);
	d.push_front(2);
	d.push_front(1);
	EXPECT_EQ(d.front(), 1);
	d.front() = 10;
	EXPECT_EQ(d[0], 10);
	const hxdeque<int, 4>& cd = d;
	EXPECT_EQ(cd.front(), 10);
	d.pop_front();
	EXPECT_EQ(d.front(), 2);
}

TEST(hxdeque_test, full) {
	hxdeque<int, 2> d;
	EXPECT_FALSE(d.full());
	d.push_back(1);
	EXPECT_FALSE(d.full());
	d.push_back(2);
	EXPECT_TRUE(d.full());
	d.pop_front();
	EXPECT_FALSE(d.full());
}

TEST_F(hxdeque_test_f, pop_back) {
	hxdeque<hxtest_object, 4> d;
	d.emplace_back(77);
	d.pop_back();
	EXPECT_TRUE(d.empty());
	EXPECT_TRUE(check_stats(1, 1, 0, 0, 0, 0, 0, 0));
}

TEST_F(hxdeque_test_f, pop_front) {
	hxdeque<hxtest_object, 4> d;
	d.emplace_back(55);
	EXPECT_TRUE(check_stats(1, 0, 0, 0, 0, 0, 0, 0));
	d.pop_front();
	EXPECT_TRUE(d.empty());
	EXPECT_TRUE(check_stats(1, 1, 0, 0, 0, 0, 0, 0));
}

TEST(hxdeque_test, push_back_fifo_and_wraparound) {
	hxdeque<int, 4> d;
	d.push_back(1); d.push_back(2); d.push_back(3);
	int v = 0;
	v = d.front(); d.pop_front(); EXPECT_EQ(v, 1);
	v = d.front(); d.pop_front(); EXPECT_EQ(v, 2);
	d.push_back(4);
	d.push_back(5);
	d.push_back(6);
	EXPECT_EQ(d.size(), 4);
	EXPECT_EQ(d[0], 3);
	EXPECT_EQ(d[1], 4);
	EXPECT_EQ(d[2], 5);
	EXPECT_EQ(d[3], 6);
	hxtest_gdb_break_hxdeque_static();
}

TEST(hxdeque_test, push_front_lifo_and_wraparound) {
	hxdeque<int, 8> d;
	d.push_back(10);
	d.push_back(20);
	d.push_front(5);
	EXPECT_EQ(d.front(), 5);
	EXPECT_EQ(d[0], 5);
	EXPECT_EQ(d[1], 10);
	EXPECT_EQ(d[2], 20);
	d.push_front(0);
	d.push_front(-1);
	EXPECT_EQ(d[0], -1);
	EXPECT_EQ(d[1], 0);
	EXPECT_EQ(d[2], 5);
	int v = 0;
	v = d.back(); d.pop_back(); EXPECT_EQ(v, 20);
	v = d.back(); d.pop_back(); EXPECT_EQ(v, 10);
	EXPECT_EQ(d.size(), 3);
}

TEST(hxdeque_test, interleaved_push_pop) {
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
	int v = 0;
	v = d.front(); d.pop_front(); EXPECT_EQ(v, 1);
	v = d.back();  d.pop_back();  EXPECT_EQ(v, 4);
	EXPECT_EQ(d.size(), 2);
	EXPECT_EQ(d.front(), 2);
	EXPECT_EQ(d.back(), 3);
}

TEST(hxdeque_test, single_element_capacity) {
	hxdeque<int, 1> d;
	EXPECT_TRUE(d.empty());
	EXPECT_EQ(d.capacity(), 1);
	d.push_back(34);
	EXPECT_TRUE(d.full());
	EXPECT_EQ(d.front(), 34);
	EXPECT_EQ(d.back(), 34);
	const int v = d.front();
	d.pop_front();
	EXPECT_EQ(v, 34);
	EXPECT_TRUE(d.empty());
}

TEST(hxdeque_test, reserve) {
	const hxsystem_allocator_scope temporary_stack_scope(hxsystem_allocator_stack_0);
	hxdeque<int> d;
	EXPECT_EQ(d.capacity(), 0);
	d.reserve(8);
	EXPECT_EQ(d.capacity(), 8);
	EXPECT_TRUE(d.empty());
	EXPECT_FALSE(d.full());
	d.reserve(8);
	EXPECT_EQ(d.capacity(), 8);
}

TEST(hxdeque_test, size) {
	const hxsystem_allocator_scope temporary_stack_scope(hxsystem_allocator_stack_0);
	hxdeque<int> d(8);
	EXPECT_EQ(d.size(), 0);
	for(int i = 0; i < 8; ++i) { d.push_back(i); }
	EXPECT_EQ(d.size(), 8);
	for(int i = 0; i < 4; ++i) { d.pop_front(); }
	EXPECT_EQ(d.size(), 4);
	for(int i = 8; i < 12; ++i) { d.push_back(i); }
	EXPECT_EQ(d.size(), 8);
	for(int i = 0; i < 8; ++i) {
		EXPECT_EQ(d[static_cast<hxsize_t>(i)], i + 4);
	}
	hxtest_gdb_break_hxdeque_dynamic();
}
