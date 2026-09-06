// SPDX-FileCopyrightText: © 2025 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

#include <hx/hxdeque.hpp>
#include <hx/hxvector.hpp>
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

#if HX_CPLUSPLUS >= 202002L
TEST(hxdeque_test, add_range_empty) {
	hxdeque<int, 4> d;
	hxvector<int, 4> empty;
	d.add_range(empty);
	EXPECT_TRUE(d.empty());
	d.add_range(hxvector<int, 4>());
	EXPECT_TRUE(d.empty());
}

TEST(hxdeque_test, add_range_appends_to_non_empty) {
	hxdeque<int, 4> d;
	d.push_back(31);
	d.add_range(hxvector<int, 2>{ 32, 33 });
	EXPECT_EQ(d.size(), 3);
	EXPECT_EQ(d[0], 31);
	EXPECT_EQ(d[1], 32);
	EXPECT_EQ(d[2], 33);
}

TEST(hxdeque_test, add_range_from_lvalue_copies) {
	hxdeque<int, 4> d;
	hxvector<int, 3> source{ 31, 32, 33 };
	d.add_range(source);
	EXPECT_EQ(source.size(), 3);
	EXPECT_EQ(d.size(), 3);
	EXPECT_EQ(d[0], 31);
	EXPECT_EQ(d[1], 32);
	EXPECT_EQ(d[2], 33);
}

TEST_F(hxdeque_test_f, add_range_from_rvalue_moves) {
	hxdeque<hxtest_object, 4> d;
	hxvector<hxtest_object, 3> source{
		hxtest_object(31), hxtest_object(32), hxtest_object(33) };
	d.add_range(hxmove(source));
	EXPECT_EQ(d.size(), 3);
	EXPECT_EQ(d[0].value(), 31);
	EXPECT_EQ(d[1].value(), 32);
	EXPECT_EQ(d[2].value(), 33);
	EXPECT_EQ(source[0].state(), hxtest_object_state::moved);
	EXPECT_TRUE(check_stats(9, 3, 0, 3, 3, 3, 0, 0, 0, 0));
}
#endif

TEST(hxdeque_test, operator_index) {
	hxdeque<int, 8> d;
	d.add_range(hxvector<int, 5>{ 0, 10, 20, 30, 40 });
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
		EXPECT_TRUE(check_stats(2, 0, 0, 2, 0, 0, 0, 0, 0, 0));
	}
	EXPECT_TRUE(check_stats(2, 2, 0, 2, 0, 0, 0, 0, 0, 0));
}

TEST(hxdeque_test, capacity) {
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
	EXPECT_TRUE(check_no_stats());
	d.emplace_back(1);
	d.emplace_back(2);
	d.emplace_back(3);
	EXPECT_TRUE(check_stats(3, 0, 0, 3, 0, 0, 0, 0, 0, 0));
	d.clear();
	EXPECT_EQ(d.size(), 0);
	EXPECT_TRUE(d.empty());
	EXPECT_TRUE(check_stats(3, 3, 0, 3, 0, 0, 0, 0, 0, 0));
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
	EXPECT_TRUE(check_stats(6, 6, 0, 6, 0, 0, 0, 0, 0, 0));
}

TEST_F(hxdeque_test_f, emplace_back) {
	hxdeque<hxtest_object, 4> d;
	d.emplace_back(34);
	EXPECT_EQ(d.back().value(), 34);
	EXPECT_EQ(d.back().state(), hxtest_object_state::valid);
	EXPECT_EQ(d.size(), 1);
	struct hxtest_pair_t {
		hxtest_pair_t(int a, int b) : x(a), y(b) { }
		int x, y;
	};
	hxdeque<hxtest_pair_t, 2> pd;
	pd.emplace_back(3, 7);
	EXPECT_EQ(pd.back().x, 3);
	EXPECT_EQ(pd.back().y, 7);
	EXPECT_TRUE(check_stats(1, 0, 0, 1, 0, 0, 0, 0, 0, 0));
}

TEST_F(hxdeque_test_f, emplace_front) {
	hxdeque<hxtest_object, 4> d;
	d.emplace_back(10);
	d.emplace_front(99);
	EXPECT_EQ(d.size(), 2);
	EXPECT_EQ(d.front().value(), 99);
	EXPECT_EQ(d.back().value(), 10);
	EXPECT_TRUE(check_stats(2, 0, 0, 2, 0, 0, 0, 0, 0, 0));
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
	EXPECT_TRUE(check_stats(1, 1, 0, 1, 0, 0, 0, 0, 0, 0));
}

TEST_F(hxdeque_test_f, pop_front) {
	hxdeque<hxtest_object, 4> d;
	d.emplace_back(55);
	EXPECT_TRUE(check_stats(1, 0, 0, 1, 0, 0, 0, 0, 0, 0));
	d.pop_front();
	EXPECT_TRUE(d.empty());
	EXPECT_TRUE(check_stats(1, 1, 0, 1, 0, 0, 0, 0, 0, 0));
}

TEST_F(hxdeque_test_f, push_back_fifo_and_wraparound) {
	hxdeque<hxtest_object, 4> a;
	a.emplace_back(1); a.emplace_back(2); a.emplace_back(3);
	int v = 0;
	v = a.front().value(); a.pop_front(); EXPECT_EQ(v, 1);
	v = a.front().value(); a.pop_front(); EXPECT_EQ(v, 2);
	a.emplace_back(4);
	a.emplace_back(5);
	a.emplace_back(6);
	EXPECT_EQ(a.size(), 4);
	EXPECT_EQ(a[0].value(), 3);
	EXPECT_EQ(a[1].value(), 4);
	EXPECT_EQ(a[2].value(), 5);
	EXPECT_EQ(a[3].value(), 6);
	const hxdeque<int> b;
	EXPECT_EQ(b.capacity(), 0);
	hxdeque<int> c;
	c.reserve(4);
	EXPECT_EQ(c.size(), 0);
	EXPECT_EQ(c.capacity(), 4);
	hxtest_gdb_break_hxdeque_static();
	EXPECT_TRUE(check_stats(6, 2, 0, 6, 0, 0, 0, 0, 0, 0));
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
	hxdeque<int> d;
	EXPECT_EQ(d.capacity(), 0);
	d.reserve(8);
	EXPECT_EQ(d.capacity(), 8);
	EXPECT_TRUE(d.empty());
	EXPECT_FALSE(d.full());
	d.reserve(8);
	EXPECT_EQ(d.capacity(), 8);
}

TEST_F(hxdeque_test_f, size) {
	hxdeque<hxtest_object> a(8);
	EXPECT_EQ(a.size(), 0);
	for(int i = 0; i < 8; ++i) { a.emplace_back(i); }
	EXPECT_EQ(a.size(), 8);
	for(int i = 0; i < 4; ++i) { a.pop_front(); }
	EXPECT_EQ(a.size(), 4);
	for(int i = 8; i < 12; ++i) { a.emplace_back(i); }
	EXPECT_EQ(a.size(), 8);
	for(int i = 0; i < 8; ++i) {
		EXPECT_EQ(a[static_cast<hxsize_t>(i)].value(), i + 4);
	}
	hxtest_gdb_break_hxdeque_dynamic();
	EXPECT_TRUE(check_stats(12, 4, 0, 12, 0, 0, 0, 0, 0, 0));
}

TEST(hxdeque_test, begin_end) {
	hxdeque<int, 4> d;
	EXPECT_EQ(d.begin(), d.end());
	EXPECT_EQ(d.cbegin(), d.cend());
	d.push_back(31);
	d.push_back(32);
	EXPECT_NE(d.begin(), d.end());
	EXPECT_EQ(*d.begin(), 31);
	hxdeque<int, 4>::iterator it = d.begin();
	++it;
	EXPECT_EQ(*it, 32);
	++it;
	EXPECT_EQ(it, d.end());
	const hxdeque<int, 4>& cd = d;
	EXPECT_EQ(*cd.begin(), 31);
	EXPECT_EQ(cd.end() - cd.begin(), 2);
}

TEST(hxdeque_test, iterator_mutation) {
	hxdeque<int, 4> d;
	d.push_back(31);
	d.push_back(32);
	*d.begin() = 99;
	EXPECT_EQ(d[0], 99);
}

TEST(hxdeque_test, implements_rand_iterator_api) {
	hxdeque<int, 4> d;
	d.push_back(1);
	d.push_back(2);
	d.push_back(3);
	typedef hxdeque<int, 4> deque_t;
	const deque_t& cd = d;
	EXPECT_TRUE(hxtest_check_rand_iterator_api<deque_t::iterator>(d.begin(), d.end()));
	EXPECT_TRUE(hxtest_check_rand_iterator_api<deque_t::const_iterator>(cd.begin(), cd.end()));
}

TEST_F(hxdeque_test_f, operator_equal) {
	hxdeque<hxtest_object, 4> a;
	hxdeque<hxtest_object, 4> b;
	EXPECT_TRUE(a == b);
	a.emplace_back(31);
	EXPECT_FALSE(a == b);
	b.emplace_back(31);
	EXPECT_TRUE(a == b);
	a.emplace_back(32);
	b.emplace_back(32);
	EXPECT_TRUE(a == b);
	EXPECT_TRUE(check_stats(4, 0, 0, 4, 0, 0, 0, 0, 3, 0));
}

TEST_F(hxdeque_test_f, operator_equal_detects_differing_element) {
	hxdeque<hxtest_object, 4> a;
	hxdeque<hxtest_object, 4> b;
	a.emplace_back(31);
	b.emplace_back(32);
	EXPECT_FALSE(a == b);
	EXPECT_TRUE(check_stats(2, 0, 0, 2, 0, 0, 0, 0, 1, 0));
}

TEST_F(hxdeque_test_f, operator_less) {
	hxdeque<hxtest_object, 4> a;
	hxdeque<hxtest_object, 4> b;
	EXPECT_FALSE(a < b);
	b.emplace_back(31);
	EXPECT_TRUE(a < b);
	EXPECT_FALSE(b < a);
	a.emplace_back(31);
	EXPECT_FALSE(a < b);
	EXPECT_FALSE(b < a);
	b.emplace_back(32);
	EXPECT_TRUE(a < b);
	EXPECT_FALSE(b < a);
	EXPECT_TRUE(check_stats(3, 0, 0, 3, 0, 0, 0, 0, 4, 0));
}

TEST_F(hxdeque_test_f, operator_less_smaller_element_is_less) {
	hxdeque<hxtest_object, 4> a;
	hxdeque<hxtest_object, 4> b;
	a.emplace_back(31);
	b.emplace_back(32);
	EXPECT_TRUE(a < b);
	EXPECT_FALSE(b < a);
	EXPECT_TRUE(check_stats(2, 0, 0, 2, 0, 0, 0, 0, 2, 2));
}
