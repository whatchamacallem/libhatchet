// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

#include <hx/hxflat_set.hpp>
#include <hx/hxmemory_manager.h>
#include <hx/hxarray.hpp>
#include <hx/hxvector.hpp>
#if HX_CPLUSPLUS >= 202302L
#include <hx/hxexpected.hpp>
#endif // HX_CPLUSPLUS >= 202302L
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

TEST_F(hxflat_set_test_f, static_initializer_list_ctor_sorts_and_rejects_duplicates) {
	const hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 4> s{
		hxtest_object(3), hxtest_object(1), hxtest_object(2), hxtest_object(1)};
	EXPECT_EQ(s.size(), 3);
	EXPECT_EQ(s[0]->value(), 1);
	EXPECT_EQ(s[1]->value(), 2);
	EXPECT_EQ(s[2]->value(), 3);
	EXPECT_TRUE(check_stats(7, 4, 0, 4, 1, 2, 2, 0, 0, 8));
}

TEST_F(hxflat_set_test_f, dynamic_initializer_list_ctor_allows_duplicates) {
	const hxflat_set<hxtest_object> s{
		hxtest_object(3), hxtest_object(1), hxtest_object(2), hxtest_object(1)};
	EXPECT_EQ(s.size(), 4);
	EXPECT_EQ(s[0]->value(), 1);
	EXPECT_EQ(s[1]->value(), 1);
	EXPECT_EQ(s[2]->value(), 2);
	EXPECT_EQ(s[3]->value(), 3);
	EXPECT_TRUE(check_stats(8, 4, 0, 4, 1, 3, 3, 2, 0, 5));
}

TEST(hxflat_set_test, operator_equal_int_key_type_detects_length_and_value_mismatch) {
	const hxflat_set<int, hxkey_less_t<int>, false, 4> a{1, 2};
	const hxflat_set<int, hxkey_less_t<int>, false, 4> b{1, 2};
	EXPECT_TRUE(a == b);
	const hxflat_set<int, hxkey_less_t<int>, false, 4> c{1, 3};
	EXPECT_FALSE(a == c);
	const hxflat_set<int, hxkey_less_t<int>, false, 4> d{1, 2, 3};
	EXPECT_FALSE(a == d);
	EXPECT_FALSE(d == a);
}

#if HX_CPLUSPLUS >= 202302L
TEST(hxflat_set_test, expected_lookup_and_emplace) {
	hxflat_set<int, hxkey_less_t<int>, false, 3> s;
	EXPECT_EQ(*s.emplace(2), 2);
	EXPECT_EQ(*s.emplace(1), 1);
	EXPECT_EQ(*s.emplace(2), 2);
	EXPECT_EQ(s.size(), 2);
	EXPECT_TRUE(s.has_value(1));
	EXPECT_FALSE(s.has_value(3));
	bool called = false;
	EXPECT_EQ(s.and_then(2, [&called](const int& value) {
		called = true;
		return hxexpected<int>(false, value + 1);
	}), 3);
	EXPECT_TRUE(called);
	called = false;
	EXPECT_FALSE((bool)s.and_then(3, [&called](const int& value) {
		// GCOVR_EXCL_START
		called = true;
		return hxexpected<int>(false, value);
		// GCOVR_EXCL_STOP
	}));
	EXPECT_FALSE(called);
	EXPECT_EQ(hxmove(s).and_then(2, [](const int&& value) {
		return hxexpected<int>(false, value);
	}), 2);
	EXPECT_EQ(*s.or_else(2, [&called, &s]() {
		// GCOVR_EXCL_START
		called = true;
		return s.end();
		// GCOVR_EXCL_STOP
	}), 2);
	EXPECT_FALSE(called);
	EXPECT_EQ(*s.or_else(3, [&called, &s]() {
		called = true;
		return s.find(1);
	}), 1);
	EXPECT_TRUE(called);
	EXPECT_EQ(s.value_or(2, 9), 2);
	EXPECT_EQ(s.value_or(3, 9), 9);
	EXPECT_EQ(hxmove(s).value_or(2, 9), 2);
	const auto and_then = [](const int& value) {
		return hxexpected<long>(false, value + 1);
	};
	static_assert(hxis_same<decltype(s.and_then(s.find(2), and_then)),
		hxexpected<long>>());
	EXPECT_EQ(s.and_then(s.find(2), and_then).value(), 3);
	EXPECT_FALSE((bool)s.and_then(s.end(), and_then));
	const auto or_else = [&s]() { return s.find(1); };
	EXPECT_EQ(*s.or_else(s.find(2), or_else), 2);
	EXPECT_EQ(*s.or_else(s.end(), or_else), 1);
	EXPECT_EQ(s.value_or(s.find(2), 10), 2);
	EXPECT_EQ(s.value_or(s.end(), 10), 10);
}

TEST_F(hxflat_set_test_f, value_or_emplaces_fallback) {
	hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 1> s;
	s.emplace(10);
	const hxtest_object found(10);
	const hxtest_object missing(11);
	EXPECT_EQ(s.value_or(found, 14, 17).value(), 10);
	EXPECT_EQ(s.value_or(missing, 14, 17).value(), 31);
	EXPECT_EQ(s.value_or(s.end(), 20, 23).value(), 43);
	EXPECT_TRUE(check_stats(7, 4, 0, 5, 1, 1, 0, 0, 0, 3));
}
#endif // HX_CPLUSPLUS >= 202302L

TEST_F(hxflat_set_test_f, gdb_static) {
	const hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 4> a{
		hxtest_object(10), hxtest_object(20), hxtest_object(30)};
	hxtest_gdb_break_hxflat_set_static();
	EXPECT_EQ(a.size(), 3);
	EXPECT_TRUE(check_stats(6, 3, 0, 3, 3, 0, 0, 0, 0, 2));
}

TEST_F(hxflat_set_test_f, gdb_dynamic) {
	const hxflat_set<hxtest_object> a{
		hxtest_object(10), hxtest_object(20), hxtest_object(30)};
	const hxflat_set<hxtest_object> b;
	hxflat_set<hxtest_object> c;
	c.reserve(8);
	hxtest_gdb_break_hxflat_set_dynamic();
	EXPECT_EQ(a.size(), 3);
	EXPECT_TRUE(check_stats(6, 3, 0, 3, 3, 0, 0, 0, 0, 2));
}

TEST_F(hxflat_set_test_f, construct) {
	const hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 1> ss;
	EXPECT_TRUE(ss.empty());
	EXPECT_EQ(ss.size(), 0);
	EXPECT_EQ(ss.capacity(), 1);
	EXPECT_EQ(ss.max_size(), 1);
	EXPECT_FALSE(ss.full());
	hxflat_set<hxtest_object> sd;
	EXPECT_TRUE(sd.empty());
	EXPECT_EQ(sd.size(), 0);
	EXPECT_EQ(sd.capacity(), 0);
	EXPECT_EQ(sd.max_size(), 0);
	sd.reserve(8);
	EXPECT_EQ(sd.capacity(), 8);
	EXPECT_EQ(sd.max_size(), 8);
	EXPECT_TRUE(check_no_stats());
}

TEST_F(hxflat_set_test_f, reserve_static_exact) {
	hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 1> s;
	s.reserve(1);
	EXPECT_EQ(s.capacity(), 1);
	EXPECT_TRUE(check_no_stats());
}

TEST_F(hxflat_set_test_f, insert_unique_basic) {
	hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 2> s;
	const hxtest_object v1(10);
	const hxtest_object* it = s.insert(v1);
	EXPECT_TRUE(it != hxnull);
	EXPECT_EQ(it->value(), 10);
	EXPECT_EQ(s.size(), 1);
	const hxtest_object v2(10);
	const hxtest_object* it2 = s.insert(v2);
	EXPECT_EQ(it2->value(), 10);
	EXPECT_EQ(s.size(), 1);
	EXPECT_TRUE(check_stats(3, 0, 0, 2, 1, 0, 0, 0, 0, 2));
}

TEST_F(hxflat_set_test_f, insert_unique_move) {
	hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 2> s;
	hxtest_object v(34);
	const hxtest_object* it = s.insert(hxmove(v));
	EXPECT_TRUE(it != hxnull);
	EXPECT_EQ(it->value(), 34);
	EXPECT_EQ(v.state(), hxtest_object_state::moved);
	hxtest_object v2(34);
	const hxtest_object* it2 = s.insert(hxmove(v2));
	EXPECT_EQ(it2->value(), 34);
	EXPECT_EQ(s.size(), 1);
	hxtest_object v3(1);
	const hxtest_object* it3 = s.insert(hxmove(v3));
	EXPECT_EQ(it3->value(), 1);
	EXPECT_EQ(s.size(), 2);
	EXPECT_TRUE(check_stats(5, 0, 0, 3, 0, 2, 0, 1, 0, 4));
}

TEST_F(hxflat_set_test_f, insert_multi) {
	hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, true, 2> s;
	const hxtest_object v1(5);
	hxtest_object v2(5);
	s.insert(v1);
	s.insert(hxmove(v2));
	EXPECT_EQ(s.size(), 2);
	EXPECT_EQ(s.count(hxtest_object(5)), 2);
	EXPECT_EQ(v2.state(), hxtest_object_state::moved);
	EXPECT_TRUE(check_stats(5, 1, 0, 3, 1, 1, 0, 1, 0, 4));
}

TEST_F(hxflat_set_test_f, insert_maintains_sorted_order) {
	const hxtest_object v30(30), v10(10), v20(20);
	const hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 3> s{
		v30, v10, v20};
	EXPECT_EQ(s.size(), 3);
	const hxtest_object* it = s.begin();
	EXPECT_EQ(it->value(), 10); ++it;
	EXPECT_EQ(it->value(), 20); ++it;
	EXPECT_EQ(it->value(), 30);
	EXPECT_TRUE(check_stats(9, 3, 0, 3, 4, 2, 2, 0, 0, 5));
}

TEST_F(hxflat_set_test_f, insert_shifts_elements) {
	const hxtest_object v10(10), v20(20), v15(15);
	const hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 3> s{
		v10, v20, v15};
	const hxtest_object* it = s.begin();
	EXPECT_EQ(it->value(), 10); ++it;
	EXPECT_EQ(it->value(), 15); ++it;
	EXPECT_EQ(it->value(), 20);
	const hxtest_object w10(10), w20(20), w5(5);
	const hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 3> s2{
		w10, w20, w5};
	const hxtest_object* it2 = s2.begin();
	EXPECT_EQ(it2->value(), 5); ++it2;
	EXPECT_EQ(it2->value(), 10); ++it2;
	EXPECT_EQ(it2->value(), 20);
	EXPECT_TRUE(check_stats(18, 6, 0, 6, 10, 2, 2, 1, 0, 8));
}

TEST_F(hxflat_set_test_f, insert_dynamic) {
	hxflat_set<hxtest_object> s;
	s.reserve(1);
	const hxtest_object v(7);
	s.insert(v);
	EXPECT_EQ(s.size(), 1);
	EXPECT_NE(s.find(hxtest_object(7)), s.end());
	EXPECT_TRUE(check_stats(3, 1, 0, 2, 1, 0, 0, 0, 0, 2));
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
	EXPECT_TRUE(check_stats(6, 4, 0, 2, 4, 0, 0, 0, 0, 1));
}

TEST_F(hxflat_set_test_f, find) {
	hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 2> s;
	const hxtest_object v(99);
	s.insert(v);
	const hxtest_object* p = s.find(hxtest_object(99));
	EXPECT_NE(p, s.end());
	EXPECT_EQ(p->value(), 99);
	EXPECT_EQ(s.find(hxtest_object(1)), s.end());
	const hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 1> empty;
	EXPECT_EQ(empty.find(hxtest_object(1)), empty.end());
	EXPECT_TRUE(check_stats(5, 3, 0, 4, 1, 0, 0, 0, 0, 4));
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
	EXPECT_TRUE(check_stats(16, 6, 0, 11, 3, 2, 2, 0, 0, 19));
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
	EXPECT_EQ(it->value(), 10);
	it = s.lower_bound(hxtest_object(20));
	EXPECT_TRUE(it != s.end());
	EXPECT_EQ(it->value(), 30);
	EXPECT_TRUE(s.lower_bound(hxtest_object(99)) == s.end());
	EXPECT_TRUE(check_stats(9, 5, 0, 7, 2, 0, 0, 0, 0, 6));
}

TEST_F(hxflat_set_test_f, lower_bound_edge_cases) {
	const hxtest_object v10(10), v20(20), v30(30);
	const hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 3> s{
		v10, v20, v30};
	const hxtest_object* it = s.lower_bound(hxtest_object(5));
	EXPECT_EQ(it, s.begin());
	EXPECT_EQ(it->value(), 10);
	it = s.lower_bound(hxtest_object(15));
	EXPECT_EQ(it->value(), 20);
	it = s.lower_bound(hxtest_object(25));
	EXPECT_EQ(it->value(), 30);
	it = s.lower_bound(hxtest_object(30));
	EXPECT_EQ(it->value(), 30);
	EXPECT_TRUE(check_stats(13, 7, 0, 7, 6, 0, 0, 0, 0, 10));
}

TEST_F(hxflat_set_test_f, upper_bound_unique) {
	hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 2> s;
	const hxtest_object v10(10), v20(20);
	s.insert(v10);
	s.insert(v20);
	const hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 2>& cs = s;
	const hxtest_object* it = cs.upper_bound(hxtest_object(10));
	EXPECT_TRUE(it != cs.end());
	EXPECT_EQ(it->value(), 20);
	it = s.upper_bound(hxtest_object(15));
	EXPECT_EQ(it->value(), 20);
	EXPECT_TRUE(s.upper_bound(hxtest_object(20)) == s.end());
	EXPECT_TRUE(check_stats(7, 3, 0, 5, 2, 0, 0, 0, 0, 6));
}

TEST_F(hxflat_set_test_f, upper_bound_multi) {
	hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, true, 3> s;
	const hxtest_object v5a(5), v5b(5), v10(10);
	s.insert(v5a);
	s.insert(v5b);
	s.insert(v10);
	const hxtest_object* it = s.upper_bound(hxtest_object(5));
	EXPECT_TRUE(it != s.end());
	EXPECT_EQ(it->value(), 10);
	EXPECT_TRUE(s.upper_bound(hxtest_object(10)) == s.end());
	hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, true, 1> s2;
	const hxtest_object v(5);
	s2.insert(v);
	EXPECT_TRUE(s2.upper_bound(hxtest_object(5)) == s2.end());
	EXPECT_TRUE(check_stats(11, 3, 0, 7, 3, 1, 1, 0, 0, 7));
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
	EXPECT_TRUE(check_stats(6, 5, 0, 5, 1, 0, 0, 0, 0, 6));
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
	EXPECT_TRUE(check_stats(9, 5, 0, 6, 2, 1, 1, 1, 0, 10));
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
	EXPECT_EQ(sa.find(hxtest_object(10))->value(), 10);
	hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, true, 4> sb;
	const hxtest_object v5(5), v10a(10), v10b(10), v10c(10);
	sb.insert(v5);
	sb.insert(v10a);
	sb.insert(v10b);
	sb.insert(v10c);
	EXPECT_EQ(sb.erase(hxtest_object(5)), 1);
	EXPECT_EQ(sb.size(), 3);
	EXPECT_EQ(sb.count(hxtest_object(10)), 3);
	EXPECT_TRUE(check_stats(20, 8, 0, 12, 4, 4, 4, 6, 0, 27));
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
	EXPECT_TRUE(check_stats(10, 4, 0, 6, 2, 2, 2, 2, 0, 12));
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
	EXPECT_EQ(next2->value(), 20);
	EXPECT_EQ(s.size(), 1);
	const hxtest_object* next3 = s.erase(s.begin());
	EXPECT_TRUE(next3 == s.end());
	EXPECT_EQ(s.size(), 0);
	EXPECT_TRUE(check_stats(5, 3, 0, 2, 3, 0, 0, 1, 0, 1));
}

TEST_F(hxflat_set_test_f, erase_iterator_last_of_two) {
	hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 2> s;
	const hxtest_object v10(10), v20(20);
	s.insert(v10);
	s.insert(v20);
	const hxtest_object* next = s.erase(s.begin() + 1);
	EXPECT_TRUE(next == s.end());
	EXPECT_EQ(s.size(), 1);
	EXPECT_EQ(s.find(hxtest_object(10))->value(), 10);
	EXPECT_TRUE(check_stats(5, 2, 0, 3, 2, 0, 0, 0, 0, 3));
}

TEST_F(hxflat_set_test_f, erase_iterator_middle_of_three) {
	const hxtest_object v10(10), v20(20), v30(30);
	hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 3> s{
		v10, v20, v30};
	const hxtest_object* next = s.erase(s.begin() + 1);
	EXPECT_EQ(next->value(), 30);
	EXPECT_EQ(s.size(), 2);
	EXPECT_EQ(s.begin()->value(), 10);
	EXPECT_EQ((s.begin() + 1)->value(), 30);
	EXPECT_TRUE(check_stats(9, 4, 0, 3, 6, 0, 0, 1, 0, 2));
}

TEST_F(hxflat_set_test_f, erase_iterator_first_of_three) {
	const hxtest_object v10(10), v20(20), v30(30);
	hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 3> s{
		v10, v20, v30};
	const hxtest_object* next = s.erase(s.begin());
	EXPECT_EQ(next->value(), 20);
	EXPECT_EQ(s.size(), 2);
	EXPECT_EQ(s.begin()->value(), 20);
	EXPECT_EQ((s.begin() + 1)->value(), 30);
	EXPECT_TRUE(check_stats(9, 4, 0, 3, 6, 0, 0, 2, 0, 2));
}

TEST_F(hxflat_set_test_f, begin_end) {
	hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 1> s;
	EXPECT_TRUE(s.begin() == s.end());
	EXPECT_TRUE(s.cbegin() == s.cend());
	const hxtest_object v(5);
	s.insert(v);
	EXPECT_FALSE(s.begin() == s.end());
	EXPECT_EQ(s.begin()->value(), 5);
	EXPECT_TRUE(check_stats(2, 0, 0, 1, 1, 0, 0, 0, 0, 0));
}

TEST_F(hxflat_set_test_f, full) {
	hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 2> s;
	const hxtest_object v1(1), v2(2);
	EXPECT_FALSE(s.full());
	s.insert(v1);
	EXPECT_FALSE(s.full());
	s.insert(v2);
	EXPECT_TRUE(s.full());
	EXPECT_TRUE(check_stats(4, 0, 0, 2, 2, 0, 0, 0, 0, 1));
}

TEST_F(hxflat_set_test_f, dynamic_multiset_insert_erase) {
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
	EXPECT_TRUE(check_stats(15, 9, 0, 10, 3, 2, 2, 3, 0, 27));
}

TEST_F(hxflat_set_test_f, destructor_destroys_elements) {
	{
		const hxtest_object v1(10), v2(20);
		const hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 2> s{v1, v2};
	}
	EXPECT_TRUE(check_stats(6, 6, 0, 2, 4, 0, 0, 0, 0, 1));
}

TEST_F(hxflat_set_test_f, subscript) {
	const hxtest_object v10(10), v20(20), v30(30);
	hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 3> s{ // NOLINT(misc-const-correctness)
		v10, v20, v30};
	const hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 3>& cs = s;
	EXPECT_EQ(cs[0]->value(), 10);
	EXPECT_EQ(cs[1]->value(), 20);
	EXPECT_EQ(cs[2]->value(), 30);
	EXPECT_EQ(s[s.size() - 1]->value(), 30);
	EXPECT_TRUE(check_stats(9, 3, 0, 3, 6, 0, 0, 0, 0, 2));
}

TEST_F(hxflat_set_test_f, copy_assign_basic) {
	hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 3> a;
	hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 3> b;
	const hxtest_object v10(10), v20(20);
	a.insert(v10);
	a.insert(v20);
	b = a;
	EXPECT_EQ(b.size(), 2);
	EXPECT_NE(b.find(hxtest_object(10)), b.end());
	EXPECT_NE(b.find(hxtest_object(20)), b.end());
	EXPECT_TRUE(check_stats(8, 2, 0, 4, 4, 0, 0, 0, 0, 7));
}

TEST_F(hxflat_set_test_f, copy_assign_edge_cases) {
	hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 3> a;
	hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 3> b;
	const hxtest_object va(5), vb(7);
	a.insert(va);
	b.insert(vb);
	b = a;
	EXPECT_EQ(b.size(), 1);
	EXPECT_NE(b.find(hxtest_object(5)), b.end());
	EXPECT_EQ(b.find(hxtest_object(7)), b.end());
	const hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 3> empty;
	b = empty;
	EXPECT_TRUE(b.empty());
	EXPECT_TRUE(check_stats(7, 4, 0, 4, 3, 0, 0, 0, 0, 3));
}

TEST_F(hxflat_set_test_f, copy_assign_different_capacity) {
	hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 2> a;
	hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 4> b;
	const hxtest_object v10(10), v20(20);
	a.insert(v10);
	a.insert(v20);
	b = a;
	EXPECT_EQ(b.size(), 2);
	EXPECT_NE(b.find(hxtest_object(10)), b.end());
	EXPECT_NE(b.find(hxtest_object(20)), b.end());
	EXPECT_TRUE(check_stats(8, 2, 0, 4, 4, 0, 0, 0, 0, 7));
}

TEST_F(hxflat_set_test_f, move_assign_transfers_elements) {
	{
		const hxtest_object v10(10), v20(20), v30(30);
		hxflat_set<hxtest_object> a{v10, v20, v30};
		hxflat_set<hxtest_object> b;
		b.reserve(1);
		b = hxmove(a);
		EXPECT_EQ(b.size(), 3);
		EXPECT_NE(b.find(hxtest_object(10)), b.end());
		EXPECT_NE(b.find(hxtest_object(20)), b.end());
		EXPECT_NE(b.find(hxtest_object(30)), b.end());
		EXPECT_EQ(a.size(), 0);
		EXPECT_EQ(a.capacity(), 1);
	}
	EXPECT_TRUE(check_stats(12, 12, 0, 6, 6, 0, 0, 0, 0, 11));
}

TEST_F(hxflat_set_test_f, move_constructor_transfers_elements) {
	{
		const hxtest_object v10(10), v20(20), v30(30);
		hxflat_set<hxtest_object> src{v10, v20, v30};
		const hxflat_set<hxtest_object> dst(hxmove(src));
		EXPECT_EQ(dst.size(), 3);
		EXPECT_EQ(dst.find(hxtest_object(10))->value(), 10);
		EXPECT_EQ(dst.find(hxtest_object(20))->value(), 20);
		EXPECT_EQ(dst.find(hxtest_object(30))->value(), 30);
		EXPECT_EQ(src.size(), 0);
		EXPECT_EQ(src.capacity(), 0);
	}
	EXPECT_TRUE(check_stats(12, 12, 0, 6, 6, 0, 0, 0, 0, 11));
}

TEST_F(hxflat_set_test_f, copy_constructor_basic) {
	const hxtest_object v10(10), v20(20), v30(30);
	const hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 3> src{
		v10, v20, v30};
	const hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 3> dst(src); // NOLINT(performance-unnecessary-copy-initialization)
	EXPECT_EQ(dst.size(), 3);
	EXPECT_EQ(dst.find(hxtest_object(10))->value(), 10);
	EXPECT_EQ(dst.find(hxtest_object(20))->value(), 20);
	EXPECT_EQ(dst.find(hxtest_object(30))->value(), 30);
	EXPECT_EQ(src.size(), 3);
	EXPECT_TRUE(check_stats(15, 6, 0, 6, 9, 0, 0, 0, 0, 11));
}

TEST_F(hxflat_set_test_f, copy_constructor_empty) {
	const hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 4> src;
	const hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 4> dst(src); // NOLINT(performance-unnecessary-copy-initialization)
	EXPECT_TRUE(dst.empty());
	EXPECT_EQ(dst.size(), 0);
	EXPECT_TRUE(check_no_stats());
}

TEST_F(hxflat_set_test_f, copy_constructor_is_independent) {
	hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 2> src;
	const hxtest_object v10(10), v20(20);
	src.insert(v10);
	const hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 2> dst(src);
	src.insert(v20);
	EXPECT_EQ(dst.find(hxtest_object(10))->value(), 10);
	EXPECT_EQ(dst.find(hxtest_object(20)), dst.end());
	EXPECT_TRUE(check_stats(7, 2, 0, 4, 3, 0, 0, 0, 0, 4));
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
	EXPECT_TRUE(check_stats(8, 8, 0, 2, 6, 0, 0, 0, 0, 1));
}

TEST(hxflat_set_test, implements_rand_iterator_api) {
	hxflat_set<int, hxkey_less_t<int>, false, 4> s{1, 2, 3}; // NOLINT(misc-const-correctness)
	typedef hxflat_set<int, hxkey_less_t<int>, false, 4> set_t;
	const set_t& cs = s;
	EXPECT_TRUE(hxtest_check_rand_iterator_api<set_t::iterator>(s.begin(), s.end()));
	EXPECT_TRUE(hxtest_check_rand_iterator_api<set_t::const_iterator>(cs.begin(), cs.end()));
}

TEST_F(hxflat_set_test_f, operator_equal) {
	hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 2> a;
	hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 2> b;
	const hxtest_object a1(10), a2(20), b1(10), b2(30);
	a.insert(a1); a.insert(a2);
	b.insert(b1); b.insert(b2);
	EXPECT_FALSE(a == b);
	b.clear();
	b.insert(a1); b.insert(a2);
	EXPECT_TRUE(a == b);
	b.clear();
	b.insert(a1);
	EXPECT_FALSE(a == b);
	EXPECT_TRUE(check_stats(11, 4, 0, 4, 7, 0, 0, 0, 4, 3));
}

TEST_F(hxflat_set_test_f, operator_less) {
	hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 3> a;
	hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 3> b;
	const hxtest_object a1(10), b1(10), b2(20);
	a.insert(a1);
	b.insert(b1); b.insert(b2);
	EXPECT_TRUE(a < b);
	EXPECT_FALSE(b < a);
	a.insert(b2);
	EXPECT_FALSE(a < b);
	EXPECT_FALSE(b < a);
	EXPECT_TRUE(check_stats(7, 0, 0, 3, 4, 0, 0, 0, 6, 2));
}

#if HX_CPLUSPLUS >= 202002L
TEST_F(hxflat_set_test_f, hxkey_equal) {
	typedef hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 2> set_t;
	set_t a;
	set_t b;
	const hxtest_object v1(10), v2(10);
	a.insert(v1);
	b.insert(v2);
	EXPECT_TRUE(a == b);
	hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 4> c;
	const hxtest_object v3(10);
	c.insert(v3);
	EXPECT_TRUE(a == c);
	set_t d;
	const hxtest_object v4(20);
	d.insert(v4);
	EXPECT_FALSE(a == d);
	EXPECT_TRUE(check_stats(8, 0, 0, 4, 4, 0, 0, 0, 3, 0));
}

TEST_F(hxflat_set_test_f, hxkey_less) {
	typedef hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 2> set_t;
	set_t a;
	set_t b;
	const hxtest_object v1(10), v2(20);
	a.insert(v1);
	b.insert(v2);
	EXPECT_TRUE(a < b);
	EXPECT_FALSE(b < a);
	set_t c;
	const hxtest_object v3(10);
	c.insert(v3);
	EXPECT_FALSE(a < c);
	EXPECT_FALSE(c < a);
	hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 4> d;
	const hxtest_object v4(20);
	d.insert(v4);
	EXPECT_TRUE(a < d);
	EXPECT_FALSE(d < a);
	EXPECT_TRUE(check_stats(8, 0, 0, 4, 4, 0, 0, 0, 6, 4));
}

TEST_F(hxflat_set_test_f, hxswap_exchanges_contents) {
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
		EXPECT_NE(a.find(hxtest_object(20)), a.end());
		EXPECT_EQ(b.size(), 1);
		EXPECT_NE(b.find(hxtest_object(10)), b.end());
	}
	EXPECT_TRUE(check_stats(6, 6, 0, 4, 2, 0, 0, 0, 0, 4));
}

TEST_F(hxflat_set_test_f, hxswap_empty_and_nonempty) {
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
		EXPECT_NE(b.find(hxtest_object(10)), b.end());
		EXPECT_NE(b.find(hxtest_object(20)), b.end());
	}
	EXPECT_TRUE(check_stats(6, 6, 0, 4, 2, 0, 0, 0, 0, 7));
}

TEST_F(hxflat_set_test_f, add_range_appends_to_nonempty_and_deduplicates) {
	hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 8> s{
		hxtest_object(31), hxtest_object(33)};
	hxarray<hxtest_object, 3> range{33, 34, 32};
	s.add_range(range);
	EXPECT_EQ(s.size(), 4);
	EXPECT_EQ(s[0]->value(), 31);
	EXPECT_EQ(s[1]->value(), 32);
	EXPECT_EQ(s[2]->value(), 33);
	EXPECT_EQ(s[3]->value(), 34);
	EXPECT_TRUE(check_stats(9, 2, 0, 5, 3, 1, 1, 1, 0, 8));
}

TEST_F(hxflat_set_test_f, add_range_unsorted_input_sorts) {
	hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 8> s;
	hxarray<hxtest_object, 5> range{34, 31, 35, 32, 33};
	s.add_range(hxmove(range));
	EXPECT_EQ(s.size(), 5);
	EXPECT_EQ(s[0]->value(), 31);
	EXPECT_EQ(s[1]->value(), 32);
	EXPECT_EQ(s[2]->value(), 33);
	EXPECT_EQ(s[3]->value(), 34);
	EXPECT_EQ(s[4]->value(), 35);
	EXPECT_TRUE(check_stats(10, 0, 0, 5, 0, 5, 0, 5, 0, 9));
}
#endif // HX_CPLUSPLUS >= 202002L
