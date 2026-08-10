// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

#include <hx/hxflat_set.hpp>
#include <hx/hxmemory_manager.h>
#include "./hxtest_util.hpp"

HX_NS_USE

hxattr_noinline static void hxtest_gdb_break_hxflat_set_static(void) { }
hxattr_noinline static void hxtest_gdb_break_hxflat_set_dynamic(void) { }

#if !defined _MSC_VER && !defined __wasm__
static_assert(sizeof(size_t) != 4 || (
		sizeof(hxflat_set<int32_t, hxkey_less_t<int32_t>, false, 4>) == 20u
		&& sizeof(hxflat_set<int32_t, hxkey_less_t<int32_t>, false>) == 12u),
	"hxflat_set must pack fixed storage as capacity * sizeof(T) key array plus"
	" a T* end pointer and dynamic storage as a hxsize_t/T* allocator pair"
	" plus a T* end pointer with no padding");
static_assert(sizeof(size_t) != 8 || (
		sizeof(hxflat_set<int32_t, hxkey_less_t<int32_t>, false, 4>) == 24u
		&& sizeof(hxflat_set<int32_t, hxkey_less_t<int32_t>, false>) == 24u),
	"hxflat_set must pack fixed storage as capacity * sizeof(T) key array plus"
	" a T* end pointer and dynamic storage as a hxsize_t/T* allocator pair"
	" plus a T* end pointer with no padding");
#endif

using hxflat_set_test_f = hxtest_object_fixture;

TEST(hxflat_set_test, static_initializer_list_ctor_sorts_and_rejects_duplicates) {
	const hxflat_set<int, hxkey_less_t<int>, false, 4> s{3, 1, 2, 1};
	EXPECT_EQ(s.size(), 3);
	EXPECT_EQ(*s[0], 1);
	EXPECT_EQ(*s[1], 2);
	EXPECT_EQ(*s[2], 3);
}

TEST(hxflat_set_test, dynamic_initializer_list_ctor_allows_duplicates) {
	const hxsystem_allocator_scope temp_scope(hxsystem_allocator_stack_0);
	const hxflat_set<int> s{3, 1, 2, 1};
	EXPECT_EQ(s.size(), 4);
	EXPECT_EQ(*s[0], 1);
	EXPECT_EQ(*s[1], 1);
	EXPECT_EQ(*s[2], 2);
	EXPECT_EQ(*s[3], 3);
}

TEST_F(hxflat_set_test_f, gdb_static) {
	const hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 4> s{
		hxtest_object(10), hxtest_object(20), hxtest_object(30)};
	hxtest_gdb_break_hxflat_set_static();
	EXPECT_EQ(s.size(), 3);
	EXPECT_TRUE(check_stats(6,3,3,0,0,0,0,2));
}

TEST_F(hxflat_set_test_f, gdb_dynamic) {
	const hxsystem_allocator_scope temp_scope(hxsystem_allocator_stack_0);
	const hxflat_set<hxtest_object> s{
		hxtest_object(10), hxtest_object(20), hxtest_object(30)};
	hxtest_gdb_break_hxflat_set_dynamic();
	EXPECT_EQ(s.size(), 3);
	EXPECT_TRUE(check_stats(6,3,3,0,0,0,0,2));
}

TEST_F(hxflat_set_test_f, construct) {
	const hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 1> ss;
	EXPECT_TRUE(ss.empty());
	EXPECT_EQ(ss.size(), 0);
	EXPECT_EQ(ss.capacity(), 1);
	EXPECT_EQ(ss.max_size(), 1);
	EXPECT_FALSE(ss.full());
	const hxsystem_allocator_scope temp_scope(hxsystem_allocator_stack_0);
	hxflat_set<hxtest_object> sd;
	EXPECT_TRUE(sd.empty());
	EXPECT_EQ(sd.size(), 0);
	EXPECT_EQ(sd.capacity(), 0);
	EXPECT_EQ(sd.max_size(), 0);
	sd.reserve(8);
	EXPECT_EQ(sd.capacity(), 8);
	EXPECT_EQ(sd.max_size(), 8);
	EXPECT_TRUE(check_stats(0, 0, 0, 0, 0, 0, 0, 0));
}

TEST_F(hxflat_set_test_f, reserve_static_exact) {
	hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 1> s;
	s.reserve(1);
	EXPECT_EQ(s.capacity(), 1);
	EXPECT_TRUE(check_stats(0, 0, 0, 0, 0, 0, 0, 0));
}

TEST_F(hxflat_set_test_f, insert_unique_basic) {
	hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 2> s;
	const hxtest_object v1(10);
	const hxtest_object* it = s.insert(v1);
	EXPECT_TRUE(it != hxnull);
	EXPECT_EQ(it->value, 10);
	EXPECT_EQ(s.size(), 1);
	const hxtest_object v2(10);
	const hxtest_object* it2 = s.insert(v2);
	EXPECT_EQ(it2->value, 10);
	EXPECT_EQ(s.size(), 1);
	EXPECT_TRUE(check_stats(3,0,1,0,0,0,0,2));
}

TEST_F(hxflat_set_test_f, insert_unique_move) {
	hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 2> s;
	hxtest_object v(34);
	const hxtest_object* it = s.insert(hxmove(v));
	EXPECT_TRUE(it != hxnull);
	EXPECT_EQ(it->value, 34);
	EXPECT_TRUE(v.moved_from);
	hxtest_object v2(34);
	const hxtest_object* it2 = s.insert(hxmove(v2));
	EXPECT_EQ(it2->value, 34);
	EXPECT_EQ(s.size(), 1);
	hxtest_object v3(1);
	const hxtest_object* it3 = s.insert(hxmove(v3));
	EXPECT_EQ(it3->value, 1);
	EXPECT_EQ(s.size(), 2);
	EXPECT_TRUE(check_stats(5,0,0,2,0,1,0,4));
}

TEST_F(hxflat_set_test_f, insert_multi) {
	hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, true, 2> s;
	const hxtest_object v1(5);
	hxtest_object v2(5);
	s.insert(v1);
	s.insert(hxmove(v2));
	EXPECT_EQ(s.size(), 2);
	EXPECT_EQ(s.count(hxtest_object(5)), 2);
	EXPECT_TRUE(v2.moved_from);
	EXPECT_TRUE(check_stats(5,1,1,1,0,1,0,4));
}

TEST_F(hxflat_set_test_f, insert_maintains_sorted_order) {
	const hxtest_object v30(30), v10(10), v20(20);
	const hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 3> s{
		v30, v10, v20};
	EXPECT_EQ(s.size(), 3);
	const hxtest_object* it = s.begin();
	EXPECT_EQ(it->value, 10); ++it;
	EXPECT_EQ(it->value, 20); ++it;
	EXPECT_EQ(it->value, 30);
	EXPECT_TRUE(check_stats(9,3,4,2,2,0,0,5));
}

TEST_F(hxflat_set_test_f, insert_shifts_elements) {
	const hxtest_object v10(10), v20(20), v15(15);
	const hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 3> s{
		v10, v20, v15};
	const hxtest_object* it = s.begin();
	EXPECT_EQ(it->value, 10); ++it;
	EXPECT_EQ(it->value, 15); ++it;
	EXPECT_EQ(it->value, 20);
	const hxtest_object w10(10), w20(20), w5(5);
	const hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 3> s2{
		w10, w20, w5};
	const hxtest_object* it2 = s2.begin();
	EXPECT_EQ(it2->value, 5); ++it2;
	EXPECT_EQ(it2->value, 10); ++it2;
	EXPECT_EQ(it2->value, 20);
	EXPECT_TRUE(check_stats(18,6,10,2,2,1,0,8));
}

TEST_F(hxflat_set_test_f, insert_dynamic) {
	const hxsystem_allocator_scope temp_scope(hxsystem_allocator_stack_0);
	hxflat_set<hxtest_object> s;
	s.reserve(1);
	const hxtest_object v(7);
	s.insert(v);
	EXPECT_EQ(s.size(), 1);
	EXPECT_TRUE(s.find(hxtest_object(7)) != hxnull);
	EXPECT_TRUE(check_stats(3,1,1,0,0,0,0,2));
}

TEST_F(hxflat_set_test_f, clear) {
	const hxtest_object v1(1), v2(2);
	hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 2> s{v1, v2};
	EXPECT_EQ(s.size(), 2);
	s.clear();
	EXPECT_EQ(s.size(), 0);
	EXPECT_TRUE(s.empty());
	s.clear();
	EXPECT_EQ(s.size(), 0);
	EXPECT_TRUE(check_stats(6,4,4,0,0,0,0,1));
}

TEST_F(hxflat_set_test_f, find) {
	hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 2> s;
	const hxtest_object v(99);
	s.insert(v);
	const hxtest_object* p = s.find(hxtest_object(99));
	EXPECT_TRUE(p != hxnull);
	EXPECT_EQ(p->value, 99);
	EXPECT_EQ(s.find(hxtest_object(1)), hxnullptr);
	const hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 1> empty;
	EXPECT_EQ(empty.find(hxtest_object(1)), hxnullptr);
	EXPECT_TRUE(check_stats(5,3,1,0,0,0,0,4));
}

TEST_F(hxflat_set_test_f, count) {
	hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 1> su;
	const hxtest_object vu(4);
	su.insert(vu);
	EXPECT_EQ(su.count(hxtest_object(4)), 1);
	EXPECT_EQ(su.count(hxtest_object(9)), 0);
	EXPECT_EQ(su.count(hxtest_object(1)), 0);
	hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, true, 4> sm;
	const hxtest_object v5a(5), v5b(5), v10(10), v10b(10);
	sm.insert(v5a); sm.insert(v5b);
	sm.insert(v10); sm.insert(v10b);
	EXPECT_EQ(sm.count(hxtest_object(5)), 2);
	EXPECT_EQ(sm.count(hxtest_object(10)), 2);
	EXPECT_EQ(sm.count(hxtest_object(99)), 0);
	EXPECT_TRUE(check_stats(16,6,3,2,2,0,0,19));
}

TEST_F(hxflat_set_test_f, lower_bound_basic) {
	const hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 1> se;
	EXPECT_EQ(se.lower_bound(hxtest_object(5)), se.begin());
	EXPECT_EQ(se.lower_bound(hxtest_object(5)), se.end());
	hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 2> s;
	const hxtest_object v10(10), v30(30);
	s.insert(v10);
	s.insert(v30);
	const hxtest_object* it = s.lower_bound(hxtest_object(10));
	EXPECT_TRUE(it != s.end());
	EXPECT_EQ(it->value, 10);
	it = s.lower_bound(hxtest_object(20));
	EXPECT_TRUE(it != s.end());
	EXPECT_EQ(it->value, 30);
	EXPECT_TRUE(s.lower_bound(hxtest_object(99)) == s.end());
	EXPECT_TRUE(check_stats(9,5,2,0,0,0,0,6));
}

TEST_F(hxflat_set_test_f, lower_bound_edge_cases) {
	const hxtest_object v10(10), v20(20), v30(30);
	const hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 3> s{
		v10, v20, v30};
	const hxtest_object* it = s.lower_bound(hxtest_object(5));
	EXPECT_EQ(it, s.begin());
	EXPECT_EQ(it->value, 10);
	it = s.lower_bound(hxtest_object(15));
	EXPECT_EQ(it->value, 20);
	it = s.lower_bound(hxtest_object(25));
	EXPECT_EQ(it->value, 30);
	it = s.lower_bound(hxtest_object(30));
	EXPECT_EQ(it->value, 30);
	EXPECT_TRUE(check_stats(13,7,6,0,0,0,0,10));
}

TEST_F(hxflat_set_test_f, upper_bound_unique) {
	hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 2> s;
	const hxtest_object v10(10), v20(20);
	s.insert(v10);
	s.insert(v20);
	const hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 2>& cs = s;
	const hxtest_object* it = cs.upper_bound(hxtest_object(10));
	EXPECT_TRUE(it != cs.end());
	EXPECT_EQ(it->value, 20);
	it = s.upper_bound(hxtest_object(15));
	EXPECT_EQ(it->value, 20);
	EXPECT_TRUE(s.upper_bound(hxtest_object(20)) == s.end());
	EXPECT_TRUE(check_stats(7,3,2,0,0,0,0,6));
}

TEST_F(hxflat_set_test_f, upper_bound_multi) {
	hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, true, 3> s;
	const hxtest_object v5a(5), v5b(5), v10(10);
	s.insert(v5a);
	s.insert(v5b);
	s.insert(v10);
	const hxtest_object* it = s.upper_bound(hxtest_object(5));
	EXPECT_TRUE(it != s.end());
	EXPECT_EQ(it->value, 10);
	EXPECT_TRUE(s.upper_bound(hxtest_object(10)) == s.end());
	hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, true, 1> s2;
	const hxtest_object v(5);
	s2.insert(v);
	EXPECT_TRUE(s2.upper_bound(hxtest_object(5)) == s2.end());
	EXPECT_TRUE(check_stats(11,3,3,1,1,0,0,7));
}

TEST_F(hxflat_set_test_f, erase_key_unique) {
	hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 1> s;
	const hxtest_object v(5);
	s.insert(v);
	EXPECT_EQ(s.erase(hxtest_object(9)), 0);
	EXPECT_EQ(s.size(), 1);
	EXPECT_EQ(s.erase(hxtest_object(99)), 0);
	EXPECT_EQ(s.erase(hxtest_object(1)), 0);
	EXPECT_EQ(s.size(), 1);
	EXPECT_EQ(s.erase(hxtest_object(5)), 1);
	EXPECT_TRUE(s.empty());
	EXPECT_TRUE(check_stats(6,5,1,0,0,0,0,6));
}

TEST_F(hxflat_set_test_f, erase_key_multi_basic) {
	hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, true, 3> s;
	const hxtest_object v5a(5), v5b(5), v10(10);
	s.insert(v5a);
	s.insert(v5b);
	s.insert(v10);
	EXPECT_EQ(s.erase(hxtest_object(5)), 2);
	EXPECT_EQ(s.size(), 1);
	EXPECT_EQ(s.count(hxtest_object(10)), 1);
	EXPECT_EQ(s.erase(hxtest_object(9)), 0);
	EXPECT_TRUE(check_stats(9,5,2,1,1,1,0,10));
}

TEST_F(hxflat_set_test_f, erase_key_multi_tail_relative_to_count) {
	hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, true, 4> sa;
	const hxtest_object v5a(5), v5b(5), v5c(5), v10(10);
	sa.insert(v5a);
	sa.insert(v5b);
	sa.insert(v5c);
	sa.insert(v10);
	EXPECT_EQ(sa.erase(hxtest_object(5)), 3);
	EXPECT_EQ(sa.size(), 1);
	EXPECT_EQ(sa.find(hxtest_object(10))->value, 10);
	hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, true, 4> sb;
	const hxtest_object v5(5), v10a(10), v10b(10), v10c(10);
	sb.insert(v5);
	sb.insert(v10a);
	sb.insert(v10b);
	sb.insert(v10c);
	EXPECT_EQ(sb.erase(hxtest_object(5)), 1);
	EXPECT_EQ(sb.size(), 3);
	EXPECT_EQ(sb.count(hxtest_object(10)), 3);
	EXPECT_TRUE(check_stats(20,8,4,4,4,6,0,27));
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
	EXPECT_TRUE(check_stats(10,4,2,2,2,2,0,12));
}

TEST_F(hxflat_set_test_f, erase_iterator_only_and_pair) {
	hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 2> s;
	const hxtest_object v10(10), v20(20);
	s.insert(v10);
	const hxtest_object* next1 = s.erase(s.begin());
	EXPECT_TRUE(next1 == s.end());
	EXPECT_TRUE(s.empty());
	s.insert(v10);
	s.insert(v20);
	const hxtest_object* next2 = s.erase(s.begin());
	EXPECT_EQ(next2->value, 20);
	EXPECT_EQ(s.size(), 1);
	const hxtest_object* next3 = s.erase(s.begin());
	EXPECT_TRUE(next3 == s.end());
	EXPECT_EQ(s.size(), 0);
	EXPECT_TRUE(check_stats(5,3,3,0,0,1,0,1));
}

TEST_F(hxflat_set_test_f, erase_iterator_last_of_two) {
	hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 2> s;
	const hxtest_object v10(10), v20(20);
	s.insert(v10);
	s.insert(v20);
	const hxtest_object* next = s.erase(s.begin() + 1);
	EXPECT_TRUE(next == s.end());
	EXPECT_EQ(s.size(), 1);
	EXPECT_EQ(s.find(hxtest_object(10))->value, 10);
	EXPECT_TRUE(check_stats(5,2,2,0,0,0,0,3));
}

TEST_F(hxflat_set_test_f, erase_iterator_middle_of_three) {
	const hxtest_object v10(10), v20(20), v30(30);
	hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 3> s{
		v10, v20, v30};
	const hxtest_object* next = s.erase(s.begin() + 1);
	EXPECT_EQ(next->value, 30);
	EXPECT_EQ(s.size(), 2);
	EXPECT_EQ(s.begin()->value, 10);
	EXPECT_EQ((s.begin() + 1)->value, 30);
	EXPECT_TRUE(check_stats(9,4,6,0,0,1,0,2));
}

TEST_F(hxflat_set_test_f, erase_iterator_first_of_three) {
	const hxtest_object v10(10), v20(20), v30(30);
	hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 3> s{
		v10, v20, v30};
	const hxtest_object* next = s.erase(s.begin());
	EXPECT_EQ(next->value, 20);
	EXPECT_EQ(s.size(), 2);
	EXPECT_EQ(s.begin()->value, 20);
	EXPECT_EQ((s.begin() + 1)->value, 30);
	EXPECT_TRUE(check_stats(9,4,6,0,0,2,0,2));
}

TEST_F(hxflat_set_test_f, begin_end) {
	hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 1> s;
	EXPECT_TRUE(s.begin() == s.end());
	EXPECT_TRUE(s.cbegin() == s.cend());
	const hxtest_object v(5);
	s.insert(v);
	EXPECT_FALSE(s.begin() == s.end());
	EXPECT_EQ(s.begin()->value, 5);
	EXPECT_TRUE(check_stats(2,0,1,0,0,0,0,0));
}

TEST_F(hxflat_set_test_f, full) {
	hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 2> s;
	const hxtest_object v1(1), v2(2);
	EXPECT_FALSE(s.full());
	s.insert(v1);
	EXPECT_FALSE(s.full());
	s.insert(v2);
	EXPECT_TRUE(s.full());
	EXPECT_TRUE(check_stats(4,0,2,0,0,0,0,1));
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
	EXPECT_TRUE(check_stats(15,9,3,2,2,3,0,27));
}

TEST_F(hxflat_set_test_f, destructor_destroys_elements) {
	{
		const hxtest_object v1(10), v2(20);
		const hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 2> s{v1, v2};
	}
	EXPECT_TRUE(check_stats(6, 6, 4, 0, 0, 0, 0, 1));
}

TEST_F(hxflat_set_test_f, subscript) {
	const hxtest_object v10(10), v20(20), v30(30);
	hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 3> s{ // NOLINT(misc-const-correctness)
		v10, v20, v30};
	const hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 3>& cs = s;
	EXPECT_EQ(cs[0]->value, 10);
	EXPECT_EQ(cs[1]->value, 20);
	EXPECT_EQ(cs[2]->value, 30);
	EXPECT_EQ(s[s.size() - 1]->value, 30);
	EXPECT_TRUE(check_stats(9,3,6,0,0,0,0,2));
}

TEST_F(hxflat_set_test_f, copy_assign_basic) {
	hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 3> a;
	hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 3> b;
	const hxtest_object v10(10), v20(20);
	a.insert(v10);
	a.insert(v20);
	b = a;
	EXPECT_EQ(b.size(), 2);
	EXPECT_TRUE(b.find(hxtest_object(10)) != hxnull);
	EXPECT_TRUE(b.find(hxtest_object(20)) != hxnull);
	EXPECT_TRUE(check_stats(8,2,4,0,0,0,0,7));
}

TEST_F(hxflat_set_test_f, copy_assign_edge_cases) {
	hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 3> a;
	hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 3> b;
	const hxtest_object va(5), vb(7);
	a.insert(va);
	b.insert(vb);
	b = a;
	EXPECT_EQ(b.size(), 1);
	EXPECT_TRUE(b.find(hxtest_object(5)) != hxnull);
	EXPECT_EQ(b.find(hxtest_object(7)), hxnullptr);
	const hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 3> empty;
	b = empty;
	EXPECT_TRUE(b.empty());
	EXPECT_TRUE(check_stats(7,4,3,0,0,0,0,3));
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
	EXPECT_TRUE(check_stats(8,2,4,0,0,0,0,7));
}

TEST_F(hxflat_set_test_f, move_assign_transfers_elements) {
	const hxsystem_allocator_scope temp_scope(hxsystem_allocator_stack_0);
	{
		const hxtest_object v10(10), v20(20), v30(30);
		hxflat_set<hxtest_object> a{v10, v20, v30};
		hxflat_set<hxtest_object> b;
		b.reserve(1);
		b = hxmove(a);
		EXPECT_EQ(b.size(), 3);
		EXPECT_TRUE(b.find(hxtest_object(10)) != hxnull);
		EXPECT_TRUE(b.find(hxtest_object(20)) != hxnull);
		EXPECT_TRUE(b.find(hxtest_object(30)) != hxnull);
		EXPECT_EQ(a.size(), 0);
		EXPECT_EQ(a.capacity(), 1);
	}
	EXPECT_TRUE(check_stats(12, 12, 6, 0, 0, 0, 0, 11));
}

TEST_F(hxflat_set_test_f, move_constructor_transfers_elements) {
	const hxsystem_allocator_scope temp_scope(hxsystem_allocator_stack_0);
	{
		const hxtest_object v10(10), v20(20), v30(30);
		hxflat_set<hxtest_object> src{v10, v20, v30};
		const hxflat_set<hxtest_object> dst(hxmove(src));
		EXPECT_EQ(dst.size(), 3);
		EXPECT_EQ(dst.find(hxtest_object(10))->value, 10);
		EXPECT_EQ(dst.find(hxtest_object(20))->value, 20);
		EXPECT_EQ(dst.find(hxtest_object(30))->value, 30);
		EXPECT_EQ(src.size(), 0);
		EXPECT_EQ(src.capacity(), 0);
	}
	EXPECT_TRUE(check_stats(12, 12, 6, 0, 0, 0, 0, 11));
}

TEST_F(hxflat_set_test_f, copy_constructor_basic) {
	const hxtest_object v10(10), v20(20), v30(30);
	const hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 3> src{
		v10, v20, v30};
	const hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 3> dst(src); // NOLINT(performance-unnecessary-copy-initialization)
	EXPECT_EQ(dst.size(), 3);
	EXPECT_EQ(dst.find(hxtest_object(10))->value, 10);
	EXPECT_EQ(dst.find(hxtest_object(20))->value, 20);
	EXPECT_EQ(dst.find(hxtest_object(30))->value, 30);
	EXPECT_EQ(src.size(), 3);
	EXPECT_TRUE(check_stats(15,6,9,0,0,0,0,11));
}

TEST_F(hxflat_set_test_f, copy_constructor_empty) {
	const hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 4> src;
	const hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 4> dst(src); // NOLINT(performance-unnecessary-copy-initialization)
	EXPECT_TRUE(dst.empty());
	EXPECT_EQ(dst.size(), 0);
	EXPECT_TRUE(check_stats(0,0,0,0,0,0,0,0));
}

TEST_F(hxflat_set_test_f, copy_constructor_is_independent) {
	hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 2> src;
	const hxtest_object v10(10), v20(20);
	src.insert(v10);
	const hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 2> dst(src);
	src.insert(v20);
	EXPECT_EQ(dst.find(hxtest_object(10))->value, 10);
	EXPECT_EQ(dst.find(hxtest_object(20)), hxnullptr);
	EXPECT_TRUE(check_stats(7,2,3,0,0,0,0,4));
}

TEST_F(hxflat_set_test_f, copy_constructor_lifecycle) {
	{
		const hxtest_object v10(10), v20(20);
		const hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 3> src{v10, v20};
		{
			const hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 3> dst(src); // NOLINT(performance-unnecessary-copy-initialization)
			EXPECT_EQ(dst.size(), 2);
		}
	}
	EXPECT_TRUE(check_stats(8, 8, 6, 0, 0, 0, 0, 1));
}

TEST(hxflat_set_test, implements_rand_iter_api) {
	hxflat_set<int, hxkey_less_t<int>, false, 4> s{1, 2, 3}; // NOLINT(misc-const-correctness)
	typedef hxflat_set<int, hxkey_less_t<int>, false, 4> set_t;
	const set_t& cs = s;
	EXPECT_TRUE(hxtest_check_rand_iter_api<set_t::iterator>(s.begin(), s.end()));
	EXPECT_TRUE(hxtest_check_rand_iter_api<set_t::const_iterator>(cs.begin(), cs.end()));
}

TEST_F(hxflat_set_test_f, equal) {
	hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 2> a;
	hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 2> b;
	const hxtest_object a1(10), a2(20), b1(10), b2(30);
	a.insert(a1); a.insert(a2);
	b.insert(b1); b.insert(b2);
	EXPECT_FALSE(a.equal(b));
	b.clear();
	b.insert(a1); b.insert(a2);
	EXPECT_TRUE(a.equal(b));
	b.clear();
	b.insert(a1);
	EXPECT_FALSE(a.equal(b));
	EXPECT_TRUE(check_stats(11,4,7,0,0,0,4,3));
}

TEST_F(hxflat_set_test_f, less) {
	hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 3> a;
	hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 3> b;
	const hxtest_object a1(10), b1(10), b2(20);
	a.insert(a1);
	b.insert(b1); b.insert(b2);
	EXPECT_TRUE(a.less(b));
	EXPECT_FALSE(b.less(a));
	a.insert(b2);
	EXPECT_FALSE(a.less(b));
	EXPECT_FALSE(b.less(a));
	EXPECT_TRUE(check_stats(7,0,4,0,0,0,6,2));
}

#if HX_CPLUSPLUS >= 202002L
TEST_F(hxflat_set_test_f, hxkey_equal) {
	hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 2> a;
	hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 2> b;
	const hxtest_object v1(10), v2(10);
	a.insert(v1);
	b.insert(v2);
	EXPECT_TRUE(hxkey_equal(a, b));
	hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 4> c;
	const hxtest_object v3(10);
	c.insert(v3);
	EXPECT_TRUE(hxkey_equal(a, c));
	hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 2> d;
	const hxtest_object v4(20);
	d.insert(v4);
	EXPECT_FALSE(hxkey_equal(a, d));
	EXPECT_TRUE(check_stats(8,0,4,0,0,0,3,0));
}

TEST_F(hxflat_set_test_f, hxkey_less) {
	hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 2> a;
	hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 2> b;
	const hxtest_object v1(10), v2(20);
	a.insert(v1);
	b.insert(v2);
	EXPECT_TRUE(hxkey_less(a, b));
	EXPECT_FALSE(hxkey_less(b, a));
	hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 2> c;
	const hxtest_object v3(10);
	c.insert(v3);
	EXPECT_FALSE(hxkey_less(a, c));
	EXPECT_FALSE(hxkey_less(c, a));
	hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 4> d;
	const hxtest_object v4(20);
	d.insert(v4);
	EXPECT_TRUE(hxkey_less(a, d));
	EXPECT_FALSE(hxkey_less(d, a));
	EXPECT_TRUE(check_stats(8,0,4,0,0,0,6,4));
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
	EXPECT_TRUE(check_stats(6, 6, 2, 0, 0, 0, 0, 4));
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
	EXPECT_TRUE(check_stats(6, 6, 2, 0, 0, 0, 0, 7));
}
#endif // HX_CPLUSPLUS >= 202002L
