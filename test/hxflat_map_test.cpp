// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

#include <hx/hxflat_map.hpp>
#include <hx/hxmemory_manager.h>
#include "test_trackers.hpp"

HX_NS_USE

hxattr_noinline static void hxtest_gdb_break_hxflat_map_static(void) { }
hxattr_noinline static void hxtest_gdb_break_hxflat_map_dynamic(void) { }

namespace {
class hxflat_map_test_f : public hxtest_object_fixture { };
} // namespace

TEST_F(hxflat_map_test_f, gdb_static) {
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 4> m;
	const hxtest_object v1(10);
	const hxtest_object v2(20);
	const hxtest_object v3(30);
	m.insert(1, v1);
	m.insert(2, v2);
	m.insert(3, v3);
	hxtest_gdb_break_hxflat_map_static();
	EXPECT_EQ(m.size(), 3);
}

TEST_F(hxflat_map_test_f, gdb_dynamic) {
	const hxsystem_allocator_scope temp_scope(hxsystem_allocator_stack_0);
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false> m;
	m.reserve(8);
	const hxtest_object v1(10);
	const hxtest_object v2(20);
	const hxtest_object v3(30);
	m.insert(1, v1);
	m.insert(2, v2);
	m.insert(3, v3);
	hxtest_gdb_break_hxflat_map_dynamic();
	EXPECT_EQ(m.size(), 3);
}

TEST_F(hxflat_map_test_f, construct_static_empty) {
	const hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 1> m;
	EXPECT_TRUE(m.empty());
	EXPECT_EQ(m.size(), 0);
	EXPECT_EQ(m.capacity(), 1);
	EXPECT_FALSE(m.full());
}

TEST_F(hxflat_map_test_f, construct_dynamic_empty) {
	const hxsystem_allocator_scope temp_scope(hxsystem_allocator_stack_0);
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false> m;
	EXPECT_TRUE(m.empty());
	EXPECT_EQ(m.size(), 0);
	EXPECT_EQ(m.capacity(), 0);
	m.reserve(8);
	EXPECT_EQ(m.capacity(), 8);
}

TEST_F(hxflat_map_test_f, reserve_static_exact) {
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 1> m;
	m.reserve(1);
	EXPECT_EQ(m.capacity(), 1);
}

TEST_F(hxflat_map_test_f, insert_unique_into_empty) {
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 1> m;
	const hxtest_object v(10);
	const hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 1>::iterator it = m.insert(1, v);
	EXPECT_EQ(it.key(), 1);
	EXPECT_EQ(it.value().id, 10);
	EXPECT_EQ(m.size(), 1);
}

TEST_F(hxflat_map_test_f, insert_unique_duplicate_returns_existing) {
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 2> m;
	const hxtest_object v1(10);
	const hxtest_object v2(20);
	m.insert(1, v1);
	const hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 2>::iterator it = m.insert(1, v2);
	EXPECT_EQ(it.key(), 1);
	EXPECT_EQ(it.value().id, 10);
	EXPECT_EQ(m.size(), 1);
}

TEST_F(hxflat_map_test_f, insert_unique_maintains_sorted_order) {
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 3> m;
	const hxtest_object v3(3), v1(1), v2(2);
	m.insert(30, v3);
	m.insert(10, v1);
	m.insert(20, v2);
	EXPECT_EQ(m.size(), 3);
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 3>::const_iterator it = m.begin();
	EXPECT_EQ(it.key(), 10); ++it;
	EXPECT_EQ(it.key(), 20); ++it;
	EXPECT_EQ(it.key(), 30);
}

TEST_F(hxflat_map_test_f, insert_unique_move) {
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 1> m;
	hxtest_object v(42);
	const hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 1>::iterator it = m.insert(5, hxmove(v));
	EXPECT_EQ(it.key(), 5);
	EXPECT_EQ(it.value().id, 42);
	EXPECT_TRUE(v.moved_from);
}

TEST_F(hxflat_map_test_f, insert_unique_move_duplicate_returns_existing) {
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 2> m;
	const hxtest_object v1(10);
	m.insert(1, v1);
	hxtest_object v2(20);
	const hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 2>::iterator it = m.insert(1, hxmove(v2));
	EXPECT_EQ(it.value().id, 10);
	EXPECT_EQ(m.size(), 1);
}

TEST_F(hxflat_map_test_f, insert_multi_allows_duplicates) {
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, true, 2> m;
	const hxtest_object v1(10), v2(20);
	m.insert(1, v1);
	m.insert(1, v2);
	EXPECT_EQ(m.size(), 2);
	EXPECT_EQ(m.count(1), 2);
}

TEST_F(hxflat_map_test_f, insert_multi_move_duplicate) {
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, true, 2> m;
	const hxtest_object v1(10);
	hxtest_object v2(20);
	m.insert(5, v1);
	m.insert(5, hxmove(v2));
	EXPECT_EQ(m.size(), 2);
	EXPECT_TRUE(v2.moved_from);
}

TEST_F(hxflat_map_test_f, insert_at_middle_shifts_elements) {
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 3> m;
	const hxtest_object va(1), vb(2), vc(3);
	m.insert(10, va);
	m.insert(30, vb);
	m.insert(20, vc);
	EXPECT_EQ(m.size(), 3);
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 3>::const_iterator it = m.begin();
	EXPECT_EQ(it.key(), 10); ++it;
	EXPECT_EQ(it.key(), 20); ++it;
	EXPECT_EQ(it.key(), 30);
}

TEST_F(hxflat_map_test_f, insert_at_front_shifts_single_element) {
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 2> m;
	const hxtest_object va(1), vb(2);
	m.insert(20, va);
	m.insert(10, vb);
	EXPECT_EQ(m.size(), 2);
	EXPECT_EQ(m.begin().key(), 10);
	EXPECT_EQ(m.begin().value().id, 2);
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 2>::const_iterator it = m.begin();
	++it;
	EXPECT_EQ(it.key(), 20);
	EXPECT_EQ(it.value().id, 1);
}

TEST_F(hxflat_map_test_f, insert_dynamic_unique) {
	const hxsystem_allocator_scope temp_scope(hxsystem_allocator_stack_0);
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false> m;
	m.reserve(1);
	const hxtest_object v(7);
	m.insert(3, v);
	EXPECT_EQ(m.size(), 1);
	EXPECT_EQ(m.find(3)->id, 7);
}

TEST_F(hxflat_map_test_f, clear_destroys_elements) {
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 2> m;
	const hxtest_object v1(1), v2(2);
	m.insert(1, v1);
	m.insert(2, v2);
	EXPECT_EQ(m.size(), 2);
	m.clear();
	EXPECT_EQ(m.size(), 0);
	EXPECT_TRUE(m.empty());
}

TEST_F(hxflat_map_test_f, clear_empty_map) {
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 1> m;
	m.clear();
	EXPECT_EQ(m.size(), 0);
}

TEST_F(hxflat_map_test_f, find_existing_key) {
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 1> m;
	const hxtest_object v(99);
	m.insert(7, v);
	const hxtest_object* p = m.find(7);
	EXPECT_TRUE(p != hxnull);
	EXPECT_EQ(p->id, 99);
}

TEST_F(hxflat_map_test_f, find_missing_key_returns_null) {
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 1> m;
	const hxtest_object v(1);
	m.insert(5, v);
	EXPECT_EQ(m.find(99), hxnullptr);
}

TEST_F(hxflat_map_test_f, find_nonconst_mutates) {
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 1> m;
	const hxtest_object v(3);
	m.insert(2, v);
	hxtest_object* p = m.find(2);
	EXPECT_TRUE(p != hxnull);
	p->id = 55;
	EXPECT_EQ(m.find(2)->id, 55);
}

TEST_F(hxflat_map_test_f, find_empty_map_returns_null) {
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 1> m;
	EXPECT_EQ(m.find(1), hxnullptr);
}

TEST_F(hxflat_map_test_f, count_unique_present) {
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 1> m;
	const hxtest_object v(1);
	m.insert(4, v);
	EXPECT_EQ(m.count(4), 1);
}

TEST_F(hxflat_map_test_f, count_unique_absent) {
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 1> m;
	const hxtest_object v(1);
	m.insert(4, v);
	EXPECT_EQ(m.count(9), 0);
}

TEST_F(hxflat_map_test_f, count_multi_multiple) {
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, true, 3> m;
	const hxtest_object v1(1), v2(2), v3(3);
	m.insert(5, v1);
	m.insert(5, v2);
	m.insert(5, v3);
	EXPECT_EQ(m.count(5), 3);
}

TEST_F(hxflat_map_test_f, count_multi_absent) {
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, true, 1> m;
	const hxtest_object v(1);
	m.insert(5, v);
	EXPECT_EQ(m.count(9), 0);
}

TEST_F(hxflat_map_test_f, lower_bound_exact_match) {
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 1> m;
	const hxtest_object v(1);
	m.insert(10, v);
	const hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 1>::const_iterator it = m.lower_bound(10);
	EXPECT_EQ(it.key(), 10);
}

TEST_F(hxflat_map_test_f, lower_bound_between_elements) {
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 2> m;
	const hxtest_object v1(1), v2(2);
	m.insert(10, v1);
	m.insert(30, v2);
	const hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 2>::const_iterator it = m.lower_bound(20);
	EXPECT_EQ(it.key(), 30);
}

TEST_F(hxflat_map_test_f, lower_bound_middle_of_three) {
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 3> m;
	const hxtest_object v1(1), v2(2), v3(3);
	m.insert(10, v1);
	m.insert(20, v2);
	m.insert(30, v3);
	const hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 3>::const_iterator it = m.lower_bound(20);
	EXPECT_EQ(it.key(), 20);
}

TEST_F(hxflat_map_test_f, lower_bound_past_end) {
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 1> m;
	const hxtest_object v(1);
	m.insert(10, v);
	EXPECT_TRUE(m.lower_bound(99) == m.end());
}

TEST_F(hxflat_map_test_f, lower_bound_nonconst) {
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 1> m;
	const hxtest_object v(5);
	m.insert(10, v);
	const hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 1>::iterator it = m.lower_bound(10);
	EXPECT_EQ(it.key(), 10);
	it.value().id = 99;
	EXPECT_EQ(m.find(10)->id, 99);
}

TEST_F(hxflat_map_test_f, upper_bound_unique_present) {
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 2> m;
	const hxtest_object v1(1), v2(2);
	m.insert(10, v1);
	m.insert(20, v2);
	const hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 2>::const_iterator it = m.upper_bound(10);
	EXPECT_EQ(it.key(), 20);
}

TEST_F(hxflat_map_test_f, upper_bound_unique_absent) {
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 2> m;
	const hxtest_object v1(1), v2(2);
	m.insert(10, v1);
	m.insert(20, v2);
	const hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 2>::const_iterator it = m.upper_bound(15);
	EXPECT_EQ(it.key(), 20);
}

TEST_F(hxflat_map_test_f, upper_bound_unique_past_end) {
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 1> m;
	const hxtest_object v(1);
	m.insert(10, v);
	EXPECT_TRUE(m.upper_bound(10) == m.end());
}

TEST_F(hxflat_map_test_f, upper_bound_multi_skips_all_duplicates) {
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, true, 3> m;
	const hxtest_object v1(1), v2(2), v3(3);
	m.insert(5, v1);
	m.insert(5, v2);
	m.insert(10, v3);
	const hxflat_map<int, hxtest_object, hxkey_less_t<int>, true, 3>::const_iterator it = m.upper_bound(5);
	EXPECT_EQ(it.key(), 10);
}

TEST_F(hxflat_map_test_f, upper_bound_multi_absent_past_end) {
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, true, 1> m;
	const hxtest_object v(1);
	m.insert(5, v);
	EXPECT_TRUE(m.upper_bound(5) == m.end());
}

TEST_F(hxflat_map_test_f, upper_bound_nonconst) {
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 2> m;
	const hxtest_object v1(1), v2(2);
	m.insert(10, v1);
	m.insert(20, v2);
	const hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 2>::iterator it = m.upper_bound(10);
	EXPECT_EQ(it.key(), 20);
}

TEST_F(hxflat_map_test_f, erase_key_unique_found) {
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 1> m;
	const hxtest_object v(1);
	m.insert(5, v);
	EXPECT_EQ(m.erase(5), 1);
	EXPECT_TRUE(m.empty());
}

TEST_F(hxflat_map_test_f, erase_key_unique_not_found) {
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 1> m;
	const hxtest_object v(1);
	m.insert(5, v);
	EXPECT_EQ(m.erase(9), 0);
	EXPECT_EQ(m.size(), 1);
}

TEST_F(hxflat_map_test_f, erase_key_unique_past_end) {
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 1> m;
	const hxtest_object v(1);
	m.insert(5, v);
	EXPECT_EQ(m.erase(99), 0);
}

TEST_F(hxflat_map_test_f, erase_key_multi_erases_all) {
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, true, 3> m;
	const hxtest_object v1(1), v2(2), v3(3);
	m.insert(5, v1);
	m.insert(5, v2);
	m.insert(10, v3);
	EXPECT_EQ(m.erase(5), 2);
	EXPECT_EQ(m.size(), 1);
	EXPECT_EQ(m.count(10), 1);
}

TEST_F(hxflat_map_test_f, erase_key_multi_not_found) {
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, true, 1> m;
	const hxtest_object v(1);
	m.insert(5, v);
	EXPECT_EQ(m.erase(9), 0);
}

TEST_F(hxflat_map_test_f, erase_key_multi_tail_less_than_count) {
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, true, 4> m;
	const hxtest_object v1(1), v2(2), v3(3), v4(4);
	m.insert(5, v1);
	m.insert(5, v2);
	m.insert(5, v3);
	m.insert(10, v4);
	EXPECT_EQ(m.erase(5), 3);
	EXPECT_EQ(m.size(), 1);
	EXPECT_EQ(m.find(10)->id, 4);
}

TEST_F(hxflat_map_test_f, erase_key_multi_count_le_tail) {
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, true, 4> m;
	const hxtest_object v1(1), v2(2), v3(3), v4(4);
	m.insert(5, v1);
	m.insert(10, v2);
	m.insert(10, v3);
	m.insert(10, v4);
	EXPECT_EQ(m.erase(5), 1);
	EXPECT_EQ(m.size(), 3);
	EXPECT_EQ(m.count(10), 3);
}

TEST_F(hxflat_map_test_f, erase_iterator_only_element) {
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 1> m;
	const hxtest_object v(1);
	m.insert(7, v);
	const hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 1>::iterator next = m.erase(m.begin());
	EXPECT_TRUE(next == m.end());
	EXPECT_TRUE(m.empty());
}

TEST_F(hxflat_map_test_f, erase_iterator_first_of_two) {
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 2> m;
	const hxtest_object v1(1), v2(2);
	m.insert(10, v1);
	m.insert(20, v2);
	const hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 2>::iterator next = m.erase(m.begin());
	EXPECT_EQ(next.key(), 20);
	EXPECT_EQ(m.size(), 1);
}

TEST_F(hxflat_map_test_f, erase_iterator_last_of_two) {
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 2> m;
	const hxtest_object v1(1), v2(2);
	m.insert(10, v1);
	m.insert(20, v2);
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 2>::iterator it = m.begin();
	++it;
	const hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 2>::iterator next = m.erase(it);
	EXPECT_TRUE(next == m.end());
	EXPECT_EQ(m.size(), 1);
	EXPECT_EQ(m.find(10)->id, 1);
}

TEST_F(hxflat_map_test_f, erase_iterator_middle_of_three) {
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 3> m;
	const hxtest_object v1(1), v2(2), v3(3);
	m.insert(10, v1);
	m.insert(20, v2);
	m.insert(30, v3);
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 3>::iterator mid = m.begin();
	++mid;
	const hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 3>::iterator next = m.erase(mid);
	EXPECT_EQ(next.key(), 30);
	EXPECT_EQ(m.size(), 2);
}

TEST_F(hxflat_map_test_f, erase_iterator_first_of_three) {
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 3> m;
	const hxtest_object v1(1), v2(2), v3(3);
	m.insert(10, v1);
	m.insert(20, v2);
	m.insert(30, v3);
	const hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 3>::iterator next = m.erase(m.begin());
	EXPECT_EQ(next.key(), 20);
	EXPECT_EQ(m.size(), 2);
	EXPECT_EQ(m.find(10), hxnullptr);
	EXPECT_EQ(m.find(20)->id, 2);
	EXPECT_EQ(m.find(30)->id, 3);
}

TEST_F(hxflat_map_test_f, iterator_operator_plus_minus) {
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 3> m;
	const hxtest_object v1(1), v2(2), v3(3);
	m.insert(10, v1);
	m.insert(20, v2);
	m.insert(30, v3);
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 3>::const_iterator it = m.begin();
	it += 2;
	EXPECT_EQ(it.key(), 30);
	it -= 1;
	EXPECT_EQ(it.key(), 20);
}

TEST_F(hxflat_map_test_f, iterator_operator_plus_returns_new) {
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 3> m;
	const hxtest_object v1(1), v2(2), v3(3);
	m.insert(10, v1);
	m.insert(20, v2);
	m.insert(30, v3);
	const hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 3>::const_iterator it = m.begin();
	const hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 3>::const_iterator it2 = it + 1;
	EXPECT_EQ(it.key(), 10);
	EXPECT_EQ(it2.key(), 20);
}

TEST_F(hxflat_map_test_f, iterator_operator_minus_distance) {
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 3> m;
	const hxtest_object v1(1), v2(2), v3(3);
	m.insert(10, v1);
	m.insert(20, v2);
	m.insert(30, v3);
	const hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 3>::const_iterator a = m.begin();
	const hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 3>::const_iterator b = m.end();
	EXPECT_EQ(b - a, ptrdiff_t{3});
}

TEST_F(hxflat_map_test_f, iterator_subscript) {
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 3> m;
	const hxtest_object v1(1), v2(2), v3(3);
	m.insert(10, v1);
	m.insert(20, v2);
	m.insert(30, v3);
	const hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 3>::const_iterator it = m.begin();
	EXPECT_EQ(it[1].key(), 20);
	EXPECT_EQ(it[2].key(), 30);
}

TEST_F(hxflat_map_test_f, iterator_pre_post_increment_decrement) {
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 2> m;
	const hxtest_object v1(1), v2(2);
	m.insert(10, v1);
	m.insert(20, v2);
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 2>::const_iterator it = m.begin();
	const hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 2>::const_iterator it2 = ++it;
	EXPECT_EQ(it.key(), 20);
	EXPECT_EQ(it2.key(), 20);
	--it;
	EXPECT_EQ(it.key(), 10);
	const hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 2>::const_iterator it3 = it++;
	EXPECT_EQ(it3.key(), 10);
	EXPECT_EQ(it.key(), 20);
	const hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 2>::const_iterator it4 = it--;
	EXPECT_EQ(it4.key(), 20);
	EXPECT_EQ(it.key(), 10);
}

TEST_F(hxflat_map_test_f, iterator_equality_and_order) {
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 2> m;
	const hxtest_object v1(1), v2(2);
	m.insert(10, v1);
	m.insert(20, v2);
	const hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 2>::const_iterator a = m.begin();
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 2>::const_iterator b = m.begin();
	++b;
	EXPECT_TRUE(a == a);
	EXPECT_FALSE(a == b);
	EXPECT_TRUE(a < b);
	EXPECT_FALSE(b < a);
	EXPECT_TRUE(b > a);
	EXPECT_TRUE(a <= b);
	EXPECT_TRUE(a <= a);
	EXPECT_TRUE(b >= a);
	EXPECT_TRUE(b >= b);
	EXPECT_TRUE(a != b);
	EXPECT_FALSE(a != a);
}

TEST_F(hxflat_map_test_f, iterator_dereference_proxy) {
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 1> m;
	const hxtest_object v(7);
	m.insert(3, v);
	const hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 1>::const_iterator it = m.begin();
	EXPECT_EQ((*it).key(), 3);
	EXPECT_EQ((*it).value().id, 7);
}

TEST_F(hxflat_map_test_f, mutable_iterator_dereference_proxy) {
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 1> m;
	const hxtest_object v(7);
	m.insert(3, v);
	const hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 1>::iterator it = m.begin();
	EXPECT_EQ((*it).key(), 3);
	(*it).value().id = 99;
	EXPECT_EQ(m.find(3)->id, 99);
}

TEST_F(hxflat_map_test_f, mutable_iterator_arithmetic) {
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 3> m;
	const hxtest_object v1(1), v2(2), v3(3);
	m.insert(10, v1);
	m.insert(20, v2);
	m.insert(30, v3);
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 3>::iterator it = m.begin();
	it += 2;
	EXPECT_EQ(it.key(), 30);
	it -= 1;
	EXPECT_EQ(it.key(), 20);
	const hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 3>::iterator it2 = it + 1;
	EXPECT_EQ(it2.key(), 30);
	const hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 3>::iterator it3 = it2 - 1;
	EXPECT_EQ(it3.key(), 20);
	EXPECT_EQ(it[1].key(), 30);
}

TEST_F(hxflat_map_test_f, mutable_iterator_pre_post_inc_dec) {
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 2> m;
	const hxtest_object v1(1), v2(2);
	m.insert(10, v1);
	m.insert(20, v2);
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 2>::iterator it = m.begin();
	const hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 2>::iterator before = it++;
	EXPECT_EQ(before.key(), 10);
	EXPECT_EQ(it.key(), 20);
	const hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 2>::iterator after = it--;
	EXPECT_EQ(after.key(), 20);
	EXPECT_EQ(it.key(), 10);
	++it;
	EXPECT_EQ(it.key(), 20);
	--it;
	EXPECT_EQ(it.key(), 10);
}

TEST_F(hxflat_map_test_f, begin_end_empty) {
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 1> m;
	EXPECT_TRUE(m.begin() == m.end());
	EXPECT_TRUE(m.cbegin() == m.cend());
}

TEST_F(hxflat_map_test_f, begin_end_nonempty) {
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 1> m;
	const hxtest_object v(1);
	m.insert(5, v);
	EXPECT_FALSE(m.begin() == m.end());
	EXPECT_EQ(m.begin().key(), 5);
}

TEST_F(hxflat_map_test_f, full_static) {
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 2> m;
	const hxtest_object v1(1), v2(2);
	EXPECT_FALSE(m.full());
	m.insert(1, v1);
	EXPECT_FALSE(m.full());
	m.insert(2, v2);
	EXPECT_TRUE(m.full());
}

TEST_F(hxflat_map_test_f, dynamic_multimap_insert_erase) {
	const hxsystem_allocator_scope temp_scope(hxsystem_allocator_stack_0);
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, true> m;
	m.reserve(5);
	const hxtest_object va(1), vb(2), vc(3), vd(4), ve(5);
	m.insert(10, va);
	m.insert(10, vb);
	m.insert(20, vc);
	m.insert(30, vd);
	m.insert(30, ve);
	EXPECT_EQ(m.size(), 5);
	EXPECT_EQ(m.count(10), 2);
	EXPECT_EQ(m.count(20), 1);
	EXPECT_EQ(m.count(30), 2);
	EXPECT_EQ(m.erase(10), 2);
	EXPECT_EQ(m.size(), 3);
	EXPECT_EQ(m.erase(30), 2);
	EXPECT_EQ(m.size(), 1);
}

TEST_F(hxflat_map_test_f, erase_key_multi_count_equals_tail) {
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, true, 4> m;
	const hxtest_object v1(1), v2(2), v3(3), v4(4);
	m.insert(5, v1);
	m.insert(5, v2);
	m.insert(10, v3);
	m.insert(10, v4);
	EXPECT_EQ(m.erase(5), 2);
	EXPECT_EQ(m.size(), 2);
	EXPECT_EQ(m.count(10), 2);
}

TEST_F(hxflat_map_test_f, destructor_destroys_elements) {
	hxsize_t destructed_before = 0;
	{
		hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 2> m;
		{
			const hxtest_object v1(10), v2(20);
			m.insert(1, v1);
			m.insert(2, v2);
		}
		destructed_before = m_destructed;
	}
	EXPECT_EQ(m_destructed, destructed_before + 2);
}

TEST_F(hxflat_map_test_f, get_valid_index_returns_iterator) {
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 3> m;
	const hxtest_object v1(10), v2(20), v3(30);
	m.insert(1, v1);
	m.insert(2, v2);
	m.insert(3, v3);
	const hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 3>::const_iterator it = m.get(1);
	EXPECT_EQ(it.key(), 2);
	EXPECT_EQ(it.value().id, 20);
}

TEST_F(hxflat_map_test_f, get_index_zero) {
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 2> m;
	const hxtest_object v1(10), v2(20);
	m.insert(5, v1);
	m.insert(10, v2);
	EXPECT_EQ(m.get(0).key(), 5);
}

TEST_F(hxflat_map_test_f, get_out_of_range_returns_end) {
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 2> m;
	const hxtest_object v(1);
	m.insert(1, v);
	EXPECT_TRUE(m.get(1) == m.end());
	EXPECT_TRUE(m.get(2) == m.end());
}

TEST_F(hxflat_map_test_f, get_nonconst_allows_mutation) {
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 2> m;
	const hxtest_object v1(10), v2(20);
	m.insert(1, v1);
	m.insert(2, v2);
	m.get(0).value().id = 99;
	EXPECT_EQ(m.find(1)->id, 99);
}

TEST_F(hxflat_map_test_f, equal_identical_maps) {
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 3> a;
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 3> b;
	const hxtest_object v1(10), v2(20), v3(10), v4(20);
	a.insert(1, v1);
	a.insert(2, v2);
	b.insert(1, v3);
	b.insert(2, v4);
	EXPECT_TRUE(a.equal(b));
}

TEST_F(hxflat_map_test_f, equal_different_sizes) {
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 3> a;
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 3> b;
	const hxtest_object v1(10), v2(20), v3(10);
	a.insert(1, v1);
	a.insert(2, v2);
	b.insert(1, v3);
	EXPECT_FALSE(a.equal(b));
}

TEST_F(hxflat_map_test_f, equal_same_keys_different_values) {
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 2> a;
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 2> b;
	const hxtest_object v1(10), v2(99);
	a.insert(1, v1);
	b.insert(1, v2);
	EXPECT_FALSE(a.equal(b));
}

TEST_F(hxflat_map_test_f, equal_different_keys) {
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 2> a;
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 2> b;
	const hxtest_object v1(10), v2(10);
	a.insert(1, v1);
	b.insert(2, v2);
	EXPECT_FALSE(a.equal(b));
}

TEST_F(hxflat_map_test_f, equal_empty_maps) {
	const hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 1> a;
	const hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 1> b;
	EXPECT_TRUE(a.equal(b));
}

TEST_F(hxflat_map_test_f, equal_different_static_capacities) {
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 2> a;
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 4> b;
	const hxtest_object v1(10), v2(10);
	a.insert(1, v1);
	b.insert(1, v2);
	EXPECT_TRUE(a.equal(b));
}

TEST_F(hxflat_map_test_f, less_equal_maps_is_false) {
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 2> a;
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 2> b;
	const hxtest_object v1(10), v2(10);
	a.insert(1, v1);
	b.insert(1, v2);
	EXPECT_FALSE(a.less(b));
	EXPECT_FALSE(b.less(a));
}

TEST_F(hxflat_map_test_f, less_shorter_prefix_is_less) {
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 3> a;
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 3> b;
	const hxtest_object v1(10), v2(10), v3(10);
	a.insert(1, v1);
	b.insert(1, v2);
	b.insert(2, v3);
	EXPECT_TRUE(a.less(b));
	EXPECT_FALSE(b.less(a));
}

TEST_F(hxflat_map_test_f, less_smaller_key_is_less) {
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 2> a;
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 2> b;
	const hxtest_object v1(10), v2(10);
	a.insert(1, v1);
	b.insert(2, v2);
	EXPECT_TRUE(a.less(b));
	EXPECT_FALSE(b.less(a));
}

TEST_F(hxflat_map_test_f, less_same_key_smaller_value_is_less) {
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 2> a;
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 2> b;
	const hxtest_object v1(5), v2(10);
	a.insert(1, v1);
	b.insert(1, v2);
	EXPECT_TRUE(a.less(b));
	EXPECT_FALSE(b.less(a));
}

TEST_F(hxflat_map_test_f, less_empty_maps_both_false) {
	const hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 1> a;
	const hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 1> b;
	EXPECT_FALSE(a.less(b));
	EXPECT_FALSE(b.less(a));
}

TEST_F(hxflat_map_test_f, less_empty_is_less_than_nonempty) {
	const hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 2> a;
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 2> b;
	const hxtest_object v(1);
	b.insert(1, v);
	EXPECT_TRUE(a.less(b));
	EXPECT_FALSE(b.less(a));
}

TEST_F(hxflat_map_test_f, subscript_const_returns_iterator) {
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 3> m;
	const hxtest_object v1(10), v2(20), v3(30);
	m.insert(1, v1);
	m.insert(2, v2);
	m.insert(3, v3);
	const hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 3>& cm = m;
	EXPECT_EQ(cm[0].key(), 1);
	EXPECT_EQ(cm[0].value().id, 10);
	EXPECT_EQ(cm[1].key(), 2);
	EXPECT_EQ(cm[2].key(), 3);
}

TEST_F(hxflat_map_test_f, subscript_mutable_allows_mutation) {
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 2> m;
	const hxtest_object v1(10), v2(20);
	m.insert(1, v1);
	m.insert(2, v2);
	m[0].value().id = 99;
	EXPECT_EQ(m.find(1)->id, 99);
	EXPECT_EQ(m[1].value().id, 20);
}

TEST_F(hxflat_map_test_f, subscript_last_element) {
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 2> m;
	const hxtest_object v1(1), v2(2);
	m.insert(10, v1);
	m.insert(20, v2);
	EXPECT_EQ(m[m.size() - 1].key(), 20);
}

TEST_F(hxflat_map_test_f, copy_assign_same_capacity) {
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 3> a;
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 3> b;
	const hxtest_object v1(10), v2(20);
	a.insert(1, v1);
	a.insert(2, v2);
	b = a;
	EXPECT_EQ(b.size(), 2);
	EXPECT_EQ(b.find(1)->id, 10);
	EXPECT_EQ(b.find(2)->id, 20);
}

TEST_F(hxflat_map_test_f, copy_assign_replaces_existing_contents) {
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 3> a;
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 3> b;
	const hxtest_object va(1), vb(99);
	a.insert(5, va);
	b.insert(7, vb);
	b = a;
	EXPECT_EQ(b.size(), 1);
	EXPECT_EQ(b.find(5)->id, 1);
	EXPECT_EQ(b.find(7), hxnullptr);
}

TEST_F(hxflat_map_test_f, copy_assign_empty_source) {
	const hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 2> a;
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 2> b;
	const hxtest_object v(1);
	b.insert(1, v);
	b = a;
	EXPECT_TRUE(b.empty());
}

TEST_F(hxflat_map_test_f, copy_assign_different_capacity) {
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 2> a;
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 4> b;
	const hxtest_object v1(10), v2(20);
	a.insert(1, v1);
	a.insert(2, v2);
	b = a;
	EXPECT_EQ(b.size(), 2);
	EXPECT_EQ(b.find(1)->id, 10);
	EXPECT_EQ(b.find(2)->id, 20);
}

TEST_F(hxflat_map_test_f, move_assign_transfers_elements) {
	const hxsystem_allocator_scope temp_scope(hxsystem_allocator_stack_0);
	{
		hxflat_map<int, hxtest_object> a;
		a.reserve(3);
		hxflat_map<int, hxtest_object> b;
		b.reserve(1);
		const hxtest_object v1(10), v2(20), v3(30);
		a.insert(1, v1);
		a.insert(2, v2);
		a.insert(3, v3);
		b = hxmove(a);
		EXPECT_EQ(b.size(), 3);
		EXPECT_EQ(b.find(1)->id, 10);
		EXPECT_EQ(b.find(2)->id, 20);
		EXPECT_EQ(b.find(3)->id, 30);
		EXPECT_EQ(a.size(), 0); // NOLINT(clang-analyzer-cplusplus.Move)
		EXPECT_EQ(a.capacity(), 1); // NOLINT(clang-analyzer-cplusplus.Move)
	}
	EXPECT_EQ(m_constructed, m_destructed);
}

TEST_F(hxflat_map_test_f, move_constructor_transfers_elements) {
	const hxsystem_allocator_scope temp_scope(hxsystem_allocator_stack_0);
	{
		hxflat_map<int, hxtest_object> src;
		src.reserve(3);
		const hxtest_object v1(10), v2(20), v3(30);
		src.insert(1, v1);
		src.insert(2, v2);
		src.insert(3, v3);
		hxflat_map<int, hxtest_object> dst(hxmove(src));
		EXPECT_EQ(dst.size(), 3);
		EXPECT_EQ(dst.find(1)->id, 10);
		EXPECT_EQ(dst.find(2)->id, 20);
		EXPECT_EQ(dst.find(3)->id, 30);
		EXPECT_EQ(src.size(), 0); // NOLINT(clang-analyzer-cplusplus.Move)
		EXPECT_EQ(src.capacity(), 0); // NOLINT(clang-analyzer-cplusplus.Move)
	}
	EXPECT_EQ(m_constructed, m_destructed);
}

TEST_F(hxflat_map_test_f, copy_constructor_static) {
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 3> src;
	const hxtest_object v1(10), v2(20), v3(30);
	src.insert(1, v1);
	src.insert(2, v2);
	src.insert(3, v3);
	const hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 3> dst(src);
	EXPECT_EQ(dst.size(), 3);
	EXPECT_EQ(dst.find(1)->id, 10);
	EXPECT_EQ(dst.find(2)->id, 20);
	EXPECT_EQ(dst.find(3)->id, 30);
	EXPECT_EQ(src.size(), 3);
}

TEST_F(hxflat_map_test_f, copy_constructor_empty) {
	const hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 4> src;
	const hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 4> dst(src); // NOLINT(performance-unnecessary-copy-initialization)
	EXPECT_TRUE(dst.empty());
	EXPECT_EQ(dst.size(), 0);
}

TEST_F(hxflat_map_test_f, copy_constructor_is_independent) {
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 2> src;
	const hxtest_object v(10);
	src.insert(1, v);
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 2> dst(src);
	src.find(1)->id = 99;
	EXPECT_EQ(dst.find(1)->id, 10);
}

TEST_F(hxflat_map_test_f, copy_constructor_lifecycle) {
	{
		hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 3> src;
		const hxtest_object v1(10), v2(20);
		src.insert(1, v1);
		src.insert(2, v2);
		{
			const hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 3> dst(src); // NOLINT(performance-unnecessary-copy-initialization)
			EXPECT_EQ(dst.size(), 2);
		}
	}
	EXPECT_EQ(m_constructed, m_destructed);
}

#if HX_CPLUSPLUS >= 202002L
TEST_F(hxflat_map_test_f, hxkey_equal_identical_maps) {
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 2> a;
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 2> b;
	const hxtest_object v1(10), v2(10);
	a.insert(1, v1);
	b.insert(1, v2);
	EXPECT_TRUE(hxkey_equal(a, b));
}

TEST_F(hxflat_map_test_f, hxkey_equal_different_maps) {
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 2> a;
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 2> b;
	const hxtest_object v1(10), v2(20);
	a.insert(1, v1);
	b.insert(1, v2);
	EXPECT_FALSE(hxkey_equal(a, b));
}

TEST_F(hxflat_map_test_f, hxkey_equal_different_capacities) {
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 2> a;
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 4> b;
	const hxtest_object v1(10), v2(10);
	a.insert(1, v1);
	b.insert(1, v2);
	EXPECT_TRUE(hxkey_equal(a, b));
}

TEST_F(hxflat_map_test_f, hxkey_less_smaller_key) {
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 2> a;
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 2> b;
	const hxtest_object v1(10), v2(10);
	a.insert(1, v1);
	b.insert(2, v2);
	EXPECT_TRUE(hxkey_less(a, b));
	EXPECT_FALSE(hxkey_less(b, a));
}

TEST_F(hxflat_map_test_f, hxkey_less_equal_maps) {
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 2> a;
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 2> b;
	const hxtest_object v1(10), v2(10);
	a.insert(1, v1);
	b.insert(1, v2);
	EXPECT_FALSE(hxkey_less(a, b));
	EXPECT_FALSE(hxkey_less(b, a));
}

TEST_F(hxflat_map_test_f, hxkey_less_different_capacities) {
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 2> a;
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 4> b;
	const hxtest_object v1(10), v2(10);
	a.insert(1, v1);
	b.insert(2, v2);
	EXPECT_TRUE(hxkey_less(a, b));
	EXPECT_FALSE(hxkey_less(b, a));
}

TEST_F(hxflat_map_test_f, hxswap_exchanges_contents) {
	const hxsystem_allocator_scope temp_scope(hxsystem_allocator_stack_0);
	{
		hxflat_map<int, hxtest_object> a;
		a.reserve(2);
		hxflat_map<int, hxtest_object> b;
		b.reserve(2);
		const hxtest_object v1(10), v2(20);
		a.insert(1, v1);
		b.insert(2, v2);
		hxswap(a, b);
		EXPECT_EQ(a.size(), 1);
		EXPECT_EQ(a.find(2)->id, 20);
		EXPECT_EQ(b.size(), 1);
		EXPECT_EQ(b.find(1)->id, 10);
	}
	EXPECT_EQ(m_constructed, m_destructed);
}

TEST_F(hxflat_map_test_f, hxswap_empty_and_nonempty) {
	const hxsystem_allocator_scope temp_scope(hxsystem_allocator_stack_0);
	{
		hxflat_map<int, hxtest_object> a;
		a.reserve(2);
		hxflat_map<int, hxtest_object> b;
		b.reserve(1);
		const hxtest_object v1(10), v2(20);
		a.insert(1, v1);
		a.insert(2, v2);
		hxswap(a, b);
		EXPECT_TRUE(a.empty());
		EXPECT_EQ(b.size(), 2);
		EXPECT_EQ(b.find(1)->id, 10);
		EXPECT_EQ(b.find(2)->id, 20);
	}
	EXPECT_EQ(m_constructed, m_destructed);
}
#endif // HX_CPLUSPLUS >= 202002L
