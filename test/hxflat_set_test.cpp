// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

#include <hx/hxflat_set.hpp>
#include <hx/hxmemory_manager.h>
#include "test_trackers.hpp"

HX_NS_USE

hxattr_noinline static void hxtest_gdb_break_hxflat_set_static(void) { }
hxattr_noinline static void hxtest_gdb_break_hxflat_set_dynamic(void) { }

namespace {
class hxflat_set_test_f : public hxtest_object_fixture { };
} // namespace

TEST_F(hxflat_set_test_f, lower_bound_empty_returns_begin) {
	const hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 1> s;
	EXPECT_EQ(s.lower_bound(hxtest_object(5)), s.begin());
	EXPECT_EQ(s.lower_bound(hxtest_object(5)), s.end());
}

TEST_F(hxflat_set_test_f, lower_bound_key_before_first_element) {
	hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 2> s;
	const hxtest_object v10(10), v20(20);
	s.insert(v10);
	s.insert(v20);
	const hxtest_object* it = s.lower_bound(hxtest_object(5));
	EXPECT_EQ(it, s.begin());
	EXPECT_EQ(it->id, 10);
}

TEST_F(hxflat_set_test_f, lower_bound_key_at_last_element) {
	hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 3> s;
	const hxtest_object v10(10), v20(20), v30(30);
	s.insert(v10);
	s.insert(v20);
	s.insert(v30);
	const hxtest_object* it = s.lower_bound(hxtest_object(30));
	EXPECT_NE(it, s.end());
	EXPECT_EQ(it->id, 30);
}

TEST_F(hxflat_set_test_f, lower_bound_key_between_last_two) {
	hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 3> s;
	const hxtest_object v10(10), v20(20), v30(30);
	s.insert(v10);
	s.insert(v20);
	s.insert(v30);
	const hxtest_object* it = s.lower_bound(hxtest_object(25));
	EXPECT_NE(it, s.end());
	EXPECT_EQ(it->id, 30);
}

TEST_F(hxflat_set_test_f, lower_bound_key_between_first_two) {
	hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 3> s;
	const hxtest_object v10(10), v20(20), v30(30);
	s.insert(v10);
	s.insert(v20);
	s.insert(v30);
	const hxtest_object* it = s.lower_bound(hxtest_object(15));
	EXPECT_NE(it, s.end());
	EXPECT_EQ(it->id, 20);
}

TEST_F(hxflat_set_test_f, insert_at_shift_preserves_adjacent_element) {
	hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 3> s;
	const hxtest_object v10(10), v20(20), v15(15);
	s.insert(v10);
	s.insert(v20);
	s.insert(v15);
	const hxtest_object* it = s.begin();
	EXPECT_EQ(it->id, 10); ++it;
	EXPECT_EQ(it->id, 15); ++it;
	EXPECT_EQ(it->id, 20);
}

TEST_F(hxflat_set_test_f, insert_at_shift_two_elements_right) {
	hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 3> s;
	const hxtest_object v10(10), v20(20), v5(5);
	s.insert(v10);
	s.insert(v20);
	s.insert(v5);
	const hxtest_object* it = s.begin();
	EXPECT_EQ(it->id, 5); ++it;
	EXPECT_EQ(it->id, 10); ++it;
	EXPECT_EQ(it->id, 20);
}

TEST_F(hxflat_set_test_f, erase_iterator_second_of_three_leaves_correct_tail) {
	hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 3> s;
	const hxtest_object v10(10), v20(20), v30(30);
	s.insert(v10);
	s.insert(v20);
	s.insert(v30);
	const hxtest_object* next = s.erase(s.begin() + 1);
	EXPECT_EQ(next->id, 30);
	EXPECT_EQ(s.size(), 2);
	EXPECT_EQ(s.begin()->id, 10);
	EXPECT_EQ((s.begin() + 1)->id, 30);
}

TEST_F(hxflat_set_test_f, erase_iterator_returns_correct_position_after_first) {
	hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 3> s;
	const hxtest_object v10(10), v20(20), v30(30);
	s.insert(v10);
	s.insert(v20);
	s.insert(v30);
	const hxtest_object* next = s.erase(s.begin());
	EXPECT_EQ(next->id, 20);
	EXPECT_EQ(s.size(), 2);
	EXPECT_EQ(s.begin()->id, 20);
	EXPECT_EQ((s.begin() + 1)->id, 30);
}

TEST_F(hxflat_set_test_f, equal_same_size_different_last_element) {
	hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 2> a;
	hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 2> b;
	const hxtest_object a1(10), a2(20), b1(10), b2(30);
	a.insert(a1); a.insert(a2);
	b.insert(b1); b.insert(b2);
	EXPECT_FALSE(a.equal(b));
}

TEST_F(hxflat_set_test_f, equal_same_size_same_elements) {
	hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 2> a;
	hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 2> b;
	const hxtest_object a1(10), a2(20), b1(10), b2(20);
	a.insert(a1); a.insert(a2);
	b.insert(b1); b.insert(b2);
	EXPECT_TRUE(a.equal(b));
}

TEST_F(hxflat_set_test_f, less_equal_length_equal_content_is_not_less) {
	hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 2> a;
	hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 2> b;
	const hxtest_object a1(10), a2(20), b1(10), b2(20);
	a.insert(a1); a.insert(a2);
	b.insert(b1); b.insert(b2);
	EXPECT_FALSE(a.less(b));
	EXPECT_FALSE(b.less(a));
}

TEST_F(hxflat_set_test_f, less_shorter_is_less_when_prefix_matches) {
	hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 3> a;
	hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 3> b;
	const hxtest_object a1(10), b1(10), b2(20);
	a.insert(a1);
	b.insert(b1); b.insert(b2);
	EXPECT_TRUE(a.less(b));
	EXPECT_FALSE(b.less(a));
}

TEST_F(hxflat_set_test_f, count_multi_stops_at_different_key) {
	hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, true, 4> s;
	const hxtest_object v5a(5), v5b(5), v10(10), v10b(10);
	s.insert(v5a); s.insert(v5b);
	s.insert(v10); s.insert(v10b);
	EXPECT_EQ(s.count(hxtest_object(5)), 2);
	EXPECT_EQ(s.count(hxtest_object(10)), 2);
}

TEST_F(hxflat_set_test_f, gdb_static) {
	hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 4> s;
	s.insert(hxtest_object(10));
	s.insert(hxtest_object(20));
	s.insert(hxtest_object(30));
	hxtest_gdb_break_hxflat_set_static();
	EXPECT_EQ(s.size(), 3);
}

TEST_F(hxflat_set_test_f, gdb_dynamic) {
	const hxsystem_allocator_scope temp_scope(hxsystem_allocator_stack_0);
	hxflat_set<hxtest_object> s;
	s.reserve(8);
	s.insert(hxtest_object(10));
	s.insert(hxtest_object(20));
	s.insert(hxtest_object(30));
	hxtest_gdb_break_hxflat_set_dynamic();
	EXPECT_EQ(s.size(), 3);
}

TEST_F(hxflat_set_test_f, construct_static_empty) {
	const hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 1> s;
	EXPECT_TRUE(s.empty());
	EXPECT_EQ(s.size(), 0);
	EXPECT_EQ(s.capacity(), 1);
	EXPECT_FALSE(s.full());
}

TEST_F(hxflat_set_test_f, construct_dynamic_empty) {
	const hxsystem_allocator_scope temp_scope(hxsystem_allocator_stack_0);
	hxflat_set<hxtest_object> s;
	EXPECT_TRUE(s.empty());
	EXPECT_EQ(s.size(), 0);
	EXPECT_EQ(s.capacity(), 0);
	s.reserve(8);
	EXPECT_EQ(s.capacity(), 8);
}

TEST_F(hxflat_set_test_f, reserve_static_exact) {
	hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 1> s;
	s.reserve(1);
	EXPECT_EQ(s.capacity(), 1);
}

TEST_F(hxflat_set_test_f, insert_unique_into_empty) {
	hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 1> s;
	const hxtest_object v(10);
	const hxtest_object* it = s.insert(v);
	EXPECT_TRUE(it != hxnull);
	EXPECT_EQ(it->id, 10);
	EXPECT_EQ(s.size(), 1);
}

TEST_F(hxflat_set_test_f, insert_unique_duplicate_returns_existing) {
	hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 2> s;
	const hxtest_object v1(10);
	const hxtest_object v2(10);
	s.insert(v1);
	const hxtest_object* it = s.insert(v2);
	EXPECT_EQ(it->id, 10);
	EXPECT_EQ(s.size(), 1);
}

TEST_F(hxflat_set_test_f, insert_unique_maintains_sorted_order) {
	hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 3> s;
	const hxtest_object v30(30), v10(10), v20(20);
	s.insert(v30);
	s.insert(v10);
	s.insert(v20);
	EXPECT_EQ(s.size(), 3);
	const hxtest_object* it = s.begin();
	EXPECT_EQ(it->id, 10); ++it;
	EXPECT_EQ(it->id, 20); ++it;
	EXPECT_EQ(it->id, 30);
}

TEST_F(hxflat_set_test_f, insert_unique_move) {
	hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 1> s;
	hxtest_object v(42);
	const hxtest_object* it = s.insert(hxmove(v));
	EXPECT_TRUE(it != hxnull);
	EXPECT_EQ(it->id, 42);
	EXPECT_TRUE(v.moved_from);
}

TEST_F(hxflat_set_test_f, insert_unique_move_duplicate_returns_existing) {
	hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 2> s;
	const hxtest_object v1(10);
	s.insert(v1);
	hxtest_object v2(10);
	const hxtest_object* it = s.insert(hxmove(v2));
	EXPECT_EQ(it->id, 10);
	EXPECT_EQ(s.size(), 1);
}

TEST_F(hxflat_set_test_f, insert_multi_allows_duplicates) {
	hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, true, 2> s;
	const hxtest_object v1(5), v2(5);
	s.insert(v1);
	s.insert(v2);
	EXPECT_EQ(s.size(), 2);
	EXPECT_EQ(s.count(hxtest_object(5)), 2);
}

TEST_F(hxflat_set_test_f, insert_multi_move_duplicate) {
	hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, true, 2> s;
	const hxtest_object v1(5);
	hxtest_object v2(5);
	s.insert(v1);
	s.insert(hxmove(v2));
	EXPECT_EQ(s.size(), 2);
	EXPECT_TRUE(v2.moved_from);
}

TEST_F(hxflat_set_test_f, insert_at_middle_shifts_elements) {
	hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 3> s;
	const hxtest_object v10(10), v30(30), v20(20);
	s.insert(v10);
	s.insert(v30);
	s.insert(v20);
	EXPECT_EQ(s.size(), 3);
	const hxtest_object* it = s.begin();
	EXPECT_EQ(it->id, 10); ++it;
	EXPECT_EQ(it->id, 20); ++it;
	EXPECT_EQ(it->id, 30);
}

TEST_F(hxflat_set_test_f, insert_dynamic_unique) {
	const hxsystem_allocator_scope temp_scope(hxsystem_allocator_stack_0);
	hxflat_set<hxtest_object> s;
	s.reserve(1);
	const hxtest_object v(7);
	s.insert(v);
	EXPECT_EQ(s.size(), 1);
	EXPECT_TRUE(s.find(hxtest_object(7)) != hxnull);
}

TEST_F(hxflat_set_test_f, clear_destroys_elements) {
	hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 2> s;
	const hxtest_object v1(1), v2(2);
	s.insert(v1);
	s.insert(v2);
	EXPECT_EQ(s.size(), 2);
	s.clear();
	EXPECT_EQ(s.size(), 0);
	EXPECT_TRUE(s.empty());
}

TEST_F(hxflat_set_test_f, clear_empty_set) {
	hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 1> s;
	s.clear();
	EXPECT_EQ(s.size(), 0);
}

TEST_F(hxflat_set_test_f, find_existing_key) {
	hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 1> s;
	const hxtest_object v(99);
	s.insert(v);
	const hxtest_object* p = s.find(hxtest_object(99));
	EXPECT_TRUE(p != hxnull);
	EXPECT_EQ(p->id, 99);
}

TEST_F(hxflat_set_test_f, find_missing_key_returns_null) {
	hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 1> s;
	const hxtest_object v(5);
	s.insert(v);
	EXPECT_EQ(s.find(hxtest_object(99)), hxnullptr);
}

TEST_F(hxflat_set_test_f, find_nonconst) {
	hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 1> s;
	const hxtest_object v(3);
	s.insert(v);
	const hxtest_object* p = s.find(hxtest_object(3));
	EXPECT_TRUE(p != hxnull);
	EXPECT_EQ(p->id, 3);
}

TEST_F(hxflat_set_test_f, find_empty_set_returns_null) {
	const hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 1> s;
	EXPECT_EQ(s.find(hxtest_object(1)), hxnullptr);
}

TEST_F(hxflat_set_test_f, count_unique_present) {
	hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 1> s;
	const hxtest_object v(4);
	s.insert(v);
	EXPECT_EQ(s.count(hxtest_object(4)), 1);
}

TEST_F(hxflat_set_test_f, count_unique_absent) {
	hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 1> s;
	const hxtest_object v(4);
	s.insert(v);
	EXPECT_EQ(s.count(hxtest_object(9)), 0);
}

TEST_F(hxflat_set_test_f, count_multi_multiple) {
	hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, true, 3> s;
	const hxtest_object v1(5), v2(5), v3(5);
	s.insert(v1);
	s.insert(v2);
	s.insert(v3);
	EXPECT_EQ(s.count(hxtest_object(5)), 3);
}

TEST_F(hxflat_set_test_f, count_multi_absent) {
	hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, true, 1> s;
	const hxtest_object v(5);
	s.insert(v);
	EXPECT_EQ(s.count(hxtest_object(9)), 0);
}

TEST_F(hxflat_set_test_f, lower_bound_exact_match) {
	hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 1> s;
	const hxtest_object v(10);
	s.insert(v);
	const hxtest_object* it = s.lower_bound(hxtest_object(10));
	EXPECT_TRUE(it != s.end());
	EXPECT_EQ(it->id, 10);
}

TEST_F(hxflat_set_test_f, lower_bound_between_elements) {
	hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 2> s;
	const hxtest_object v10(10), v30(30);
	s.insert(v10);
	s.insert(v30);
	const hxtest_object* it = s.lower_bound(hxtest_object(20));
	EXPECT_TRUE(it != s.end());
	EXPECT_EQ(it->id, 30);
}

TEST_F(hxflat_set_test_f, lower_bound_past_end) {
	hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 1> s;
	const hxtest_object v(10);
	s.insert(v);
	EXPECT_TRUE(s.lower_bound(hxtest_object(99)) == s.end());
}

TEST_F(hxflat_set_test_f, lower_bound_nonconst) {
	hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 1> s;
	const hxtest_object v(10);
	s.insert(v);
	const hxtest_object* it = s.lower_bound(hxtest_object(10));
	EXPECT_TRUE(it != s.end());
	EXPECT_EQ(it->id, 10);
}

TEST_F(hxflat_set_test_f, upper_bound_unique_present) {
	hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 2> s;
	const hxtest_object v10(10), v20(20);
	s.insert(v10);
	s.insert(v20);
	const hxtest_object* it = s.upper_bound(hxtest_object(10));
	EXPECT_TRUE(it != s.end());
	EXPECT_EQ(it->id, 20);
}

TEST_F(hxflat_set_test_f, upper_bound_unique_absent) {
	hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 2> s;
	const hxtest_object v10(10), v20(20);
	s.insert(v10);
	s.insert(v20);
	const hxtest_object* it = s.upper_bound(hxtest_object(15));
	EXPECT_TRUE(it != s.end());
	EXPECT_EQ(it->id, 20);
}

TEST_F(hxflat_set_test_f, upper_bound_unique_past_end) {
	hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 1> s;
	const hxtest_object v(10);
	s.insert(v);
	EXPECT_TRUE(s.upper_bound(hxtest_object(10)) == s.end());
}

TEST_F(hxflat_set_test_f, upper_bound_multi_skips_all_duplicates) {
	hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, true, 3> s;
	const hxtest_object v5a(5), v5b(5), v10(10);
	s.insert(v5a);
	s.insert(v5b);
	s.insert(v10);
	const hxtest_object* it = s.upper_bound(hxtest_object(5));
	EXPECT_TRUE(it != s.end());
	EXPECT_EQ(it->id, 10);
}

TEST_F(hxflat_set_test_f, upper_bound_multi_absent_past_end) {
	hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, true, 1> s;
	const hxtest_object v(5);
	s.insert(v);
	EXPECT_TRUE(s.upper_bound(hxtest_object(5)) == s.end());
}

TEST_F(hxflat_set_test_f, upper_bound_nonconst) {
	hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 2> s;
	const hxtest_object v10(10), v20(20);
	s.insert(v10);
	s.insert(v20);
	const hxtest_object* it = s.upper_bound(hxtest_object(10));
	EXPECT_TRUE(it != s.end());
	EXPECT_EQ(it->id, 20);
}

TEST_F(hxflat_set_test_f, erase_key_unique_found) {
	hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 1> s;
	const hxtest_object v(5);
	s.insert(v);
	EXPECT_EQ(s.erase(hxtest_object(5)), 1);
	EXPECT_TRUE(s.empty());
}

TEST_F(hxflat_set_test_f, erase_key_unique_not_found) {
	hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 1> s;
	const hxtest_object v(5);
	s.insert(v);
	EXPECT_EQ(s.erase(hxtest_object(9)), 0);
	EXPECT_EQ(s.size(), 1);
}

TEST_F(hxflat_set_test_f, erase_key_unique_past_end) {
	hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 1> s;
	const hxtest_object v(5);
	s.insert(v);
	EXPECT_EQ(s.erase(hxtest_object(99)), 0);
}

TEST_F(hxflat_set_test_f, erase_key_multi_erases_all) {
	hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, true, 3> s;
	const hxtest_object v5a(5), v5b(5), v10(10);
	s.insert(v5a);
	s.insert(v5b);
	s.insert(v10);
	EXPECT_EQ(s.erase(hxtest_object(5)), 2);
	EXPECT_EQ(s.size(), 1);
	EXPECT_EQ(s.count(hxtest_object(10)), 1);
}

TEST_F(hxflat_set_test_f, erase_key_multi_not_found) {
	hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, true, 1> s;
	const hxtest_object v(5);
	s.insert(v);
	EXPECT_EQ(s.erase(hxtest_object(9)), 0);
}

TEST_F(hxflat_set_test_f, erase_key_multi_tail_less_than_count) {
	hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, true, 4> s;
	const hxtest_object v5a(5), v5b(5), v5c(5), v10(10);
	s.insert(v5a);
	s.insert(v5b);
	s.insert(v5c);
	s.insert(v10);
	EXPECT_EQ(s.erase(hxtest_object(5)), 3);
	EXPECT_EQ(s.size(), 1);
	EXPECT_EQ(s.find(hxtest_object(10))->id, 10);
}

TEST_F(hxflat_set_test_f, erase_key_multi_count_le_tail) {
	hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, true, 4> s;
	const hxtest_object v5(5), v10a(10), v10b(10), v10c(10);
	s.insert(v5);
	s.insert(v10a);
	s.insert(v10b);
	s.insert(v10c);
	EXPECT_EQ(s.erase(hxtest_object(5)), 1);
	EXPECT_EQ(s.size(), 3);
	EXPECT_EQ(s.count(hxtest_object(10)), 3);
}

TEST_F(hxflat_set_test_f, erase_iterator_only_element) {
	hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 1> s;
	const hxtest_object v(7);
	s.insert(v);
	const hxtest_object* next = s.erase(s.begin());
	EXPECT_TRUE(next == s.end());
	EXPECT_TRUE(s.empty());
}

TEST_F(hxflat_set_test_f, erase_iterator_first_of_two) {
	hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 2> s;
	const hxtest_object v10(10), v20(20);
	s.insert(v10);
	s.insert(v20);
	const hxtest_object* next = s.erase(s.begin());
	EXPECT_EQ(next->id, 20);
	EXPECT_EQ(s.size(), 1);
}

TEST_F(hxflat_set_test_f, erase_iterator_last_of_two) {
	hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 2> s;
	const hxtest_object v10(10), v20(20);
	s.insert(v10);
	s.insert(v20);
	const hxtest_object* next = s.erase(s.begin() + 1);
	EXPECT_TRUE(next == s.end());
	EXPECT_EQ(s.size(), 1);
	EXPECT_EQ(s.find(hxtest_object(10))->id, 10);
}

TEST_F(hxflat_set_test_f, erase_iterator_middle_of_three) {
	hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 3> s;
	const hxtest_object v10(10), v20(20), v30(30);
	s.insert(v10);
	s.insert(v20);
	s.insert(v30);
	const hxtest_object* next = s.erase(s.begin() + 1);
	EXPECT_EQ(next->id, 30);
	EXPECT_EQ(s.size(), 2);
}

TEST_F(hxflat_set_test_f, begin_end_empty) {
	const hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 1> s;
	EXPECT_TRUE(s.begin() == s.end());
	EXPECT_TRUE(s.cbegin() == s.cend());
}

TEST_F(hxflat_set_test_f, begin_end_nonempty) {
	hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 1> s;
	const hxtest_object v(5);
	s.insert(v);
	EXPECT_FALSE(s.begin() == s.end());
	EXPECT_EQ(s.begin()->id, 5);
}

TEST_F(hxflat_set_test_f, full_static) {
	hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 2> s;
	const hxtest_object v1(1), v2(2);
	EXPECT_FALSE(s.full());
	s.insert(v1);
	EXPECT_FALSE(s.full());
	s.insert(v2);
	EXPECT_TRUE(s.full());
}

TEST_F(hxflat_set_test_f, dynamic_multiset_insert_erase) {
	const hxsystem_allocator_scope temp_scope(hxsystem_allocator_stack_0);
	hxflat_set<hxtest_object> s;
	s.reserve(5);
	const hxtest_object v10a(10), v10b(10), v20(20), v30a(30), v30b(30);
	s.insert(v10a);
	s.insert(v10b);
	s.insert(v20);
	s.insert(v30a);
	s.insert(v30b);
	EXPECT_EQ(s.size(), 5);
	EXPECT_EQ(s.count(hxtest_object(10)), 2);
	EXPECT_EQ(s.count(hxtest_object(20)), 1);
	EXPECT_EQ(s.count(hxtest_object(30)), 2);
	EXPECT_EQ(s.erase(hxtest_object(10)), 2);
	EXPECT_EQ(s.size(), 3);
	EXPECT_EQ(s.erase(hxtest_object(30)), 2);
	EXPECT_EQ(s.size(), 1);
}

TEST_F(hxflat_set_test_f, erase_key_multi_count_equals_tail) {
	hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, true, 4> s;
	const hxtest_object v5a(5), v5b(5), v10a(10), v10b(10);
	s.insert(v5a);
	s.insert(v5b);
	s.insert(v10a);
	s.insert(v10b);
	EXPECT_EQ(s.erase(hxtest_object(5)), 2);
	EXPECT_EQ(s.size(), 2);
	EXPECT_EQ(s.count(hxtest_object(10)), 2);
}

TEST_F(hxflat_set_test_f, destructor_destroys_elements) {
	hxsize_t destructed_before = 0;
	{
		hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 2> s;
		{
			const hxtest_object v1(10), v2(20);
			s.insert(v1);
			s.insert(v2);
		}
		destructed_before = m_destructed;
	}
	EXPECT_EQ(m_destructed, destructed_before + 2);
}

TEST_F(hxflat_set_test_f, subscript_const_returns_pointer) {
	hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 3> s;
	const hxtest_object v10(10), v20(20), v30(30);
	s.insert(v10);
	s.insert(v20);
	s.insert(v30);
	const hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 3>& cs = s;
	EXPECT_EQ(cs[0]->id, 10);
	EXPECT_EQ(cs[1]->id, 20);
	EXPECT_EQ(cs[2]->id, 30);
}

TEST_F(hxflat_set_test_f, subscript_last_element) {
	hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 2> s;
	const hxtest_object v10(10), v20(20);
	s.insert(v10);
	s.insert(v20);
	EXPECT_EQ(s[s.size() - 1]->id, 20);
}

TEST_F(hxflat_set_test_f, copy_assign_same_capacity) {
	hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 3> a;
	hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 3> b;
	const hxtest_object v10(10), v20(20);
	a.insert(v10);
	a.insert(v20);
	b = a;
	EXPECT_EQ(b.size(), 2);
	EXPECT_TRUE(b.find(hxtest_object(10)) != hxnull);
	EXPECT_TRUE(b.find(hxtest_object(20)) != hxnull);
}

TEST_F(hxflat_set_test_f, copy_assign_replaces_existing_contents) {
	hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 3> a;
	hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 3> b;
	const hxtest_object va(5), vb(7);
	a.insert(va);
	b.insert(vb);
	b = a;
	EXPECT_EQ(b.size(), 1);
	EXPECT_TRUE(b.find(hxtest_object(5)) != hxnull);
	EXPECT_EQ(b.find(hxtest_object(7)), hxnullptr);
}

TEST_F(hxflat_set_test_f, copy_assign_empty_source) {
	const hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 2> a;
	hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 2> b;
	const hxtest_object v(1);
	b.insert(v);
	b = a;
	EXPECT_TRUE(b.empty());
}

TEST_F(hxflat_set_test_f, copy_assign_different_capacity) {
	hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 2> a;
	hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 4> b;
	const hxtest_object v10(10), v20(20);
	a.insert(v10);
	a.insert(v20);
	b = a;
	EXPECT_EQ(b.size(), 2);
	EXPECT_TRUE(b.find(hxtest_object(10)) != hxnull);
	EXPECT_TRUE(b.find(hxtest_object(20)) != hxnull);
}

TEST_F(hxflat_set_test_f, move_assign_transfers_elements) {
	const hxsystem_allocator_scope temp_scope(hxsystem_allocator_stack_0);
	{
		hxflat_set<hxtest_object> a;
		a.reserve(3);
		hxflat_set<hxtest_object> b;
		b.reserve(1);
		const hxtest_object v10(10), v20(20), v30(30);
		a.insert(v10);
		a.insert(v20);
		a.insert(v30);
		b = hxmove(a);
		EXPECT_EQ(b.size(), 3);
		EXPECT_TRUE(b.find(hxtest_object(10)) != hxnull);
		EXPECT_TRUE(b.find(hxtest_object(20)) != hxnull);
		EXPECT_TRUE(b.find(hxtest_object(30)) != hxnull);
		EXPECT_EQ(a.size(), 0); // NOLINT(clang-analyzer-cplusplus.Move)
		EXPECT_EQ(a.capacity(), 1); // NOLINT(clang-analyzer-cplusplus.Move)
	}
	EXPECT_EQ(m_constructed, m_destructed);
}

TEST_F(hxflat_set_test_f, move_constructor_transfers_elements) {
	const hxsystem_allocator_scope temp_scope(hxsystem_allocator_stack_0);
	{
		hxflat_set<hxtest_object> src;
		src.reserve(3);
		const hxtest_object v10(10), v20(20), v30(30);
		src.insert(v10);
		src.insert(v20);
		src.insert(v30);
		const hxflat_set<hxtest_object> dst(hxmove(src));
		EXPECT_EQ(dst.size(), 3);
		EXPECT_EQ(dst.find(hxtest_object(10))->id, 10);
		EXPECT_EQ(dst.find(hxtest_object(20))->id, 20);
		EXPECT_EQ(dst.find(hxtest_object(30))->id, 30);
		EXPECT_EQ(src.size(), 0); // NOLINT(clang-analyzer-cplusplus.Move)
		EXPECT_EQ(src.capacity(), 0); // NOLINT(clang-analyzer-cplusplus.Move)
	}
	EXPECT_EQ(m_constructed, m_destructed);
}

TEST_F(hxflat_set_test_f, copy_constructor_static) {
	hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 3> src;
	const hxtest_object v10(10), v20(20), v30(30);
	src.insert(v10);
	src.insert(v20);
	src.insert(v30);
	const hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 3> dst(src);
	EXPECT_EQ(dst.size(), 3);
	EXPECT_EQ(dst.find(hxtest_object(10))->id, 10);
	EXPECT_EQ(dst.find(hxtest_object(20))->id, 20);
	EXPECT_EQ(dst.find(hxtest_object(30))->id, 30);
	EXPECT_EQ(src.size(), 3);
}

TEST_F(hxflat_set_test_f, copy_constructor_empty) {
	const hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 4> src;
	const hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 4> dst(src); // NOLINT(performance-unnecessary-copy-initialization)
	EXPECT_TRUE(dst.empty());
	EXPECT_EQ(dst.size(), 0);
}

TEST_F(hxflat_set_test_f, copy_constructor_is_independent) {
	hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 2> src;
	const hxtest_object v10(10), v20(20);
	src.insert(v10);
	const hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 2> dst(src);
	src.insert(v20);
	EXPECT_EQ(dst.find(hxtest_object(10))->id, 10);
	EXPECT_EQ(dst.find(hxtest_object(20)), hxnullptr);
}

TEST_F(hxflat_set_test_f, copy_constructor_lifecycle) {
	{
		hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 3> src;
		const hxtest_object v10(10), v20(20);
		src.insert(v10);
		src.insert(v20);
		{
			const hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 3> dst(src); // NOLINT(performance-unnecessary-copy-initialization)
			EXPECT_EQ(dst.size(), 2);
		}
	}
	EXPECT_EQ(m_constructed, m_destructed);
}

#if HX_CPLUSPLUS >= 202002L
TEST_F(hxflat_set_test_f, hxkey_equal_identical_sets) {
	hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 2> a;
	hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 2> b;
	const hxtest_object v1(10), v2(10);
	a.insert(v1);
	b.insert(v2);
	EXPECT_TRUE(hxkey_equal(a, b));
}

TEST_F(hxflat_set_test_f, hxkey_equal_different_sets) {
	hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 2> a;
	hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 2> b;
	const hxtest_object v1(10), v2(20);
	a.insert(v1);
	b.insert(v2);
	EXPECT_FALSE(hxkey_equal(a, b));
}

TEST_F(hxflat_set_test_f, hxkey_equal_different_capacities) {
	hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 2> a;
	hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 4> b;
	const hxtest_object v1(10), v2(10);
	a.insert(v1);
	b.insert(v2);
	EXPECT_TRUE(hxkey_equal(a, b));
}

TEST_F(hxflat_set_test_f, hxkey_less_smaller_key) {
	hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 2> a;
	hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 2> b;
	const hxtest_object v1(10), v2(20);
	a.insert(v1);
	b.insert(v2);
	EXPECT_TRUE(hxkey_less(a, b));
	EXPECT_FALSE(hxkey_less(b, a));
}

TEST_F(hxflat_set_test_f, hxkey_less_equal_sets) {
	hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 2> a;
	hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 2> b;
	const hxtest_object v1(10), v2(10);
	a.insert(v1);
	b.insert(v2);
	EXPECT_FALSE(hxkey_less(a, b));
	EXPECT_FALSE(hxkey_less(b, a));
}

TEST_F(hxflat_set_test_f, hxkey_less_different_capacities) {
	hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 2> a;
	hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 4> b;
	const hxtest_object v1(10), v2(20);
	a.insert(v1);
	b.insert(v2);
	EXPECT_TRUE(hxkey_less(a, b));
	EXPECT_FALSE(hxkey_less(b, a));
}

TEST_F(hxflat_set_test_f, hxswap_exchanges_contents) {
	const hxsystem_allocator_scope temp_scope(hxsystem_allocator_stack_0);
	{
		hxflat_set<hxtest_object> a;
		a.reserve(2);
		hxflat_set<hxtest_object> b;
		b.reserve(2);
		const hxtest_object v10(10), v20(20);
		a.insert(v10);
		b.insert(v20);
		hxswap(a, b);
		EXPECT_EQ(a.size(), 1);
		EXPECT_TRUE(a.find(hxtest_object(20)) != hxnull);
		EXPECT_EQ(b.size(), 1);
		EXPECT_TRUE(b.find(hxtest_object(10)) != hxnull);
	}
	EXPECT_EQ(m_constructed, m_destructed);
}

TEST_F(hxflat_set_test_f, hxswap_empty_and_nonempty) {
	const hxsystem_allocator_scope temp_scope(hxsystem_allocator_stack_0);
	{
		hxflat_set<hxtest_object> a;
		a.reserve(2);
		hxflat_set<hxtest_object> b;
		b.reserve(1);
		const hxtest_object v10(10), v20(20);
		a.insert(v10);
		a.insert(v20);
		hxswap(a, b);
		EXPECT_TRUE(a.empty());
		EXPECT_EQ(b.size(), 2);
		EXPECT_TRUE(b.find(hxtest_object(10)) != hxnull);
		EXPECT_TRUE(b.find(hxtest_object(20)) != hxnull);
	}
	EXPECT_EQ(m_constructed, m_destructed);
}
#endif // HX_CPLUSPLUS >= 202002L
