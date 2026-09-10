// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

#include <hx/hxflat_map.hpp>
#include <hx/hxslab_allocator.h>
#if HX_CPLUSPLUS >= 202302L
#include <hx/hxexpected.hpp>
#endif // HX_CPLUSPLUS >= 202302L
#include "./hxtest_util.hpp"

HX_NS_USE

hxattr_noinline static void hxtest_gdb_break_hxflat_map_static(void) { }
hxattr_noinline static void hxtest_gdb_break_hxflat_map_dynamic(void) { }

#if !defined _MSC_VER && !defined __wasm__
static_assert(sizeof(size_t) != 4 || (
		sizeof(hxflat_map<int32_t, int32_t, 4>) == 36u
		&& sizeof(hxflat_map<int32_t, int32_t>) == 20u),
	"hxflat_map must pack fixed storage as a hxsize_t size plus two capacity *"
	" sizeof(T) key and value arrays and dynamic storage as a hxsize_t size"
	" plus two hxsize_t/T* allocator pairs with no padding");
static_assert(sizeof(size_t) != 8 || (
		sizeof(hxflat_map<int32_t, int32_t, 4>) == 40u
		&& sizeof(hxflat_map<int32_t, int32_t>) == 40u),
	"hxflat_map must pack fixed storage as a hxsize_t size plus two capacity *"
	" sizeof(T) key and value arrays and dynamic storage as a hxsize_t size"
	" plus two hxsize_t/T* allocator pairs with no padding");
#endif

using hxflat_map_test_f = hxtest_object_fixture;

TEST(hxflat_map_test, static_initializer_list_ctor_sorts_and_rejects_duplicates) {
	const hxflat_map<int, int, 4> m{
		{3, 30}, {1, 10}, {2, 20}, {1, 11}};
	EXPECT_EQ(m.size(), 3);
	EXPECT_EQ(m.find(1).value(), 10);
	EXPECT_EQ(m.find(2).value(), 20);
	EXPECT_EQ(m.find(3).value(), 30);
}

TEST(hxflat_map_test, dynamic_initializer_list_ctor_allows_duplicates) {
	const hxflat_multimap<int, int> m{
		{3, 30}, {1, 10}, {2, 20}, {1, 11}};
	EXPECT_EQ(m.size(), 4);
	EXPECT_EQ(m[0].key(), 1);
	EXPECT_EQ(m[0].value(), 11);
	EXPECT_EQ(m[1].key(), 1);
	EXPECT_EQ(m[1].value(), 10);
	EXPECT_EQ(m[2].key(), 2);
	EXPECT_EQ(m[3].key(), 3);
}

TEST(hxflat_map_test, operator_equal_int_mapped_type_detects_key_and_value_mismatch) {
	const hxflat_map<int, int, 2> a{ {1, 10} };
	const hxflat_map<int, int, 2> b{ {1, 10} };
	EXPECT_TRUE(a == b);
	const hxflat_map<int, int, 2> c{ {1, 99} };
	EXPECT_FALSE(a == c);
	const hxflat_map<int, int, 2> d{ {2, 10} };
	EXPECT_FALSE(a == d);
	const hxflat_map<int, int, 2> e{ {1, 10}, {2, 20} };
	EXPECT_FALSE(a == e);
	EXPECT_FALSE(e == a);
	const hxflat_map<int, int, 2> f{ {1, 10}, {2, 99} };
	EXPECT_FALSE(e == f);
}

TEST(hxflat_map_test, mutable_value_proxy_operator_equal_detects_key_and_value_mismatch) {
	hxflat_map<int, int, 2> a{ {1, 10} };
	hxflat_map<int, int, 2> b{ {1, 10} };
	hxflat_map<int, int, 2> c{ {1, 99} };
	hxflat_map<int, int, 2> d{ {2, 10} };
	EXPECT_TRUE(*a.begin() == *b.begin());
	EXPECT_FALSE(*a.begin() == *c.begin());
	EXPECT_FALSE(*a.begin() == *d.begin());
}

#if HX_CPLUSPLUS >= 202302L
TEST(hxflat_map_test, expected_lookup_and_emplace) {
	hxflat_map<int, int, 3> m;
	EXPECT_EQ(m.emplace(2, 20).value(), 20);
	EXPECT_EQ(m.emplace(1, 10).value(), 10);
	EXPECT_EQ(m.emplace(2, 99).value(), 20);
	EXPECT_EQ(m.size(), 2);
	EXPECT_TRUE(m.has_value(1));
	EXPECT_FALSE(m.has_value(3));
	bool called = false;
	EXPECT_EQ(m.and_then(2, [&called](int& value) {
		called = true;
		value = 21;
		return hxexpected<int>(false, value);
	}), 21);
	EXPECT_TRUE(called);
	called = false;
	EXPECT_FALSE((bool)m.and_then(3, [&called](int& value) {
		// GCOVR_EXCL_START
		called = true;
		return hxexpected<int>(false, value);
		// GCOVR_EXCL_STOP
	}));
	EXPECT_FALSE(called);
	EXPECT_EQ(m.or_else(2, [&called, &m]() {
		// GCOVR_EXCL_START
		called = true;
		return m.end();
		// GCOVR_EXCL_STOP
	}).value(), 21);
	EXPECT_FALSE(called);
	EXPECT_EQ(m.or_else(3, [&called, &m]() {
		called = true;
		return m.find(1);
	}).value(), 10);
	EXPECT_TRUE(called);
	const hxflat_map<int, int, 3>& cm = m;
	called = false;
	EXPECT_EQ(cm.and_then(2, [&called](const int& value) {
		called = true;
		return hxexpected<int>(false, value + 1);
	}), 22);
	EXPECT_TRUE(called);
	called = false;
	EXPECT_FALSE((bool)cm.and_then(3, [&called](const int& value) {
		// GCOVR_EXCL_START
		called = true;
		return hxexpected<int>(false, value);
		// GCOVR_EXCL_STOP
	}));
	EXPECT_FALSE(called);
	EXPECT_EQ(hxmove(m).and_then(2, [](int&& value) {
		return hxexpected<int>(false, value);
	}), 21);
	EXPECT_EQ(hxmove(cm).and_then(2, [](const int&& value) {
		return hxexpected<int>(false, value);
	}), 21);
	EXPECT_EQ(cm.or_else(2, [&called, &cm]() {
		// GCOVR_EXCL_START
		called = true;
		return cm.end();
		// GCOVR_EXCL_STOP
	}).value(), 21);
	EXPECT_FALSE(called);
	EXPECT_EQ(cm.or_else(3, [&called, &cm]() {
		called = true;
		return cm.find(1);
	}).value(), 10);
	EXPECT_TRUE(called);
	EXPECT_EQ(cm.value_or(2, 90), 21);
	EXPECT_EQ(cm.value_or(3, 90), 90);
	EXPECT_EQ(hxmove(m).value_or(2, 90), 21);
	const auto iterator_and_then = [](const int& value) {
		return hxexpected<long>(false, value + 2);
	};
	static_assert(hxis_same<decltype(m.and_then(m.find(2), iterator_and_then)),
		hxexpected<long>>());
	EXPECT_EQ(m.and_then(m.find(2), iterator_and_then).value(), 23);
	EXPECT_FALSE((bool)m.and_then(m.end(), iterator_and_then));
	const auto const_and_then = [](const int& value) {
		return hxexpected<long>(false, value + 3);
	};
	EXPECT_EQ(cm.and_then(cm.find(2), const_and_then).value(), 24);
	EXPECT_FALSE((bool)cm.and_then(cm.end(), const_and_then));
	const auto iterator_or_else = [&m]() { return m.find(1); };
	static_assert(hxis_same<decltype(m.or_else(2, iterator_or_else)),
		decltype(m)::iterator>());
	static_assert(hxis_same<decltype(m.or_else(
		m.find(2), iterator_or_else)), decltype(m)::iterator>());
	EXPECT_EQ(m.or_else(m.find(2), iterator_or_else).value(), 21);
	EXPECT_EQ(m.or_else(m.end(), iterator_or_else).value(), 10);
	const auto const_or_else = [&cm]() { return cm.find(1); };
	EXPECT_EQ(cm.or_else(cm.find(2), const_or_else).value(), 21);
	EXPECT_EQ(cm.or_else(cm.end(), const_or_else).value(), 10);
	EXPECT_EQ(m.value_or(m.find(2), 91), 21);
	EXPECT_EQ(m.value_or(m.end(), 91), 91);
	EXPECT_EQ(cm.value_or(cm.find(2), 92), 21);
	EXPECT_EQ(cm.value_or(cm.end(), 92), 92);
}

TEST_F(hxflat_map_test_f, value_or_emplaces_fallback) {
	hxflat_map<hxtest_object, int, 1> m;
	const hxtest_object k1(1), k2(2);
	m.emplace(k1, 10);
	EXPECT_EQ(m.value_or(k1, 14), 10);
	EXPECT_EQ(m.value_or(k2, 14), 14);
	EXPECT_EQ(m.value_or(m.end(), 20), 20);
	EXPECT_TRUE(check_stats(3, 0, 0, 2, 1, 0, 0, 0, 0, 0, 2));
}
#endif // HX_CPLUSPLUS >= 202302L

TEST_F(hxflat_map_test_f, gdb_break) {
	const hxtest_object k1(1), k2(2), k3(3);
	const hxtest_object v1(10), v2(20), v3(30);
	const hxflat_map<hxtest_object, hxtest_object, 4> a{
		{k1, v1}, {k2, v2}, {k3, v3}};
	hxtest_gdb_break_hxflat_map_static();
	EXPECT_EQ(a.size(), 3);
	const hxflat_multimap<hxtest_object, hxtest_object> b{
		{k1, v1}, {k2, v2}, {k3, v3}};
	const hxflat_multimap<hxtest_object, hxtest_object> c;
	hxflat_multimap<hxtest_object, hxtest_object> d;
	d.reserve(4);
	hxtest_gdb_break_hxflat_map_dynamic();
	EXPECT_EQ(b.size(), 3);
	EXPECT_TRUE(c.empty());
	EXPECT_TRUE(d.empty());
	EXPECT_TRUE(check_stats(30, 12, 0, 6, 24, 0, 0, 0, 0, 0, 4));
}

TEST_F(hxflat_map_test_f, construct) {
	const hxflat_map<int, hxtest_object, 1> ms;
	EXPECT_TRUE(ms.empty());
	EXPECT_EQ(ms.size(), 0);
	EXPECT_EQ(ms.capacity(), 1);
	EXPECT_EQ(ms.max_size(), 1);
	EXPECT_FALSE(ms.full());
	hxflat_map<int, hxtest_object> md;
	EXPECT_TRUE(md.empty());
	EXPECT_EQ(md.capacity(), 0);
	EXPECT_EQ(md.max_size(), 0);
	md.reserve(8);
	EXPECT_EQ(md.capacity(), 8);
	EXPECT_EQ(md.max_size(), 8);
	EXPECT_TRUE(check_no_stats());
}

TEST_F(hxflat_map_test_f, reserve_static_exact) {
	hxflat_map<int, hxtest_object, 1> m;
	m.reserve(1);
	EXPECT_EQ(m.capacity(), 1);
	EXPECT_TRUE(check_no_stats());
}

TEST_F(hxflat_map_test_f, insert_unique_basic) {
	hxflat_map<hxtest_object, int, 2> m;
	const hxtest_object k1(1);
	const hxflat_map<hxtest_object, int, 2>::iterator it1 = m.insert(k1, 10);
	EXPECT_EQ(it1.key().value(), 1);
	EXPECT_EQ(it1.value(), 10);
	EXPECT_EQ(m.size(), 1);
	const hxflat_map<hxtest_object, int, 2>::iterator it2 = m.insert(k1, 20);
	EXPECT_EQ(it2.key().value(), 1);
	EXPECT_EQ(it2.value(), 10);
	EXPECT_EQ(m.size(), 1);
	EXPECT_TRUE(check_stats(2, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1));
}

TEST_F(hxflat_map_test_f, insert_unique_sorted_order) {
	const hxtest_object k30(30), k10(10), k20(20);
	const hxflat_map<hxtest_object, int, 3> m{
		{k30, 3}, {k10, 1}, {k20, 2}};
	EXPECT_EQ(m.size(), 3);
	hxflat_map<hxtest_object, int, 3>::const_iterator it = m.begin();
	EXPECT_EQ(it.key().value(), 10); ++it;
	EXPECT_EQ(it.key().value(), 20); ++it;
	EXPECT_EQ(it.key().value(), 30);
	EXPECT_TRUE(check_stats(9, 3, 0, 3, 4, 2, 2, 0, 0, 0, 3));
}

TEST_F(hxflat_map_test_f, insert_unique_move) {
	hxflat_map<hxtest_object, int, 3> m;
	const hxtest_object k5(5);
	const hxflat_map<hxtest_object, int, 3>::iterator it =
		m.insert(k5, 34);
	EXPECT_EQ(it.key().value(), 5);
	EXPECT_EQ(it.value(), 34);
	const hxtest_object k1(1);
	m.insert(k1, 10);
	const hxflat_map<hxtest_object, int, 3>::iterator it2 =
		m.insert(k1, 20);
	EXPECT_EQ(it2.value(), 10);
	EXPECT_EQ(m.size(), 2);
	const hxtest_object k3(3);
	const hxflat_map<hxtest_object, int, 3>::iterator it3 =
		m.insert(k3, 15);
	EXPECT_EQ(it3.key().value(), 3);
	EXPECT_EQ(it3.value(), 15);
	EXPECT_EQ(m.size(), 3);
	EXPECT_TRUE(check_stats(6, 0, 0, 3, 1, 2, 2, 0, 0, 0, 5));
}

TEST_F(hxflat_map_test_f, insert_multi) {
	hxflat_multimap<hxtest_object, int, 2> m;
	const hxtest_object k1(1);
	m.insert(k1, 10);
	m.insert(k1, 20);
	EXPECT_EQ(m.size(), 2);
	EXPECT_EQ(m.count(k1), 2);
	EXPECT_TRUE(check_stats(3, 0, 0, 1, 1, 1, 1, 0, 0, 0, 4));
}

TEST_F(hxflat_map_test_f, insert_shifts_elements) {
	const hxtest_object ka(10), kb(30), kc(20);
	const hxflat_map<hxtest_object, int, 3> m{
		{ka, 1}, {kb, 2}, {kc, 3}};
	EXPECT_EQ(m.size(), 3);
	hxflat_map<hxtest_object, int, 3>::const_iterator it = m.begin();
	EXPECT_EQ(it.key().value(), 10); ++it;
	EXPECT_EQ(it.key().value(), 20); ++it;
	EXPECT_EQ(it.key().value(), 30);
	EXPECT_TRUE(check_stats(9, 3, 0, 3, 5, 1, 1, 0, 0, 0, 3));
}

TEST_F(hxflat_map_test_f, insert_at_front_shifts_single_element) {
	const hxtest_object ka(20), kb(10);
	const hxflat_map<hxtest_object, int, 2> m{
		{ka, 1}, {kb, 2}};
	EXPECT_EQ(m.size(), 2);
	EXPECT_EQ(m.begin().key().value(), 10);
	EXPECT_EQ(m.begin().value(), 2);
	hxflat_map<hxtest_object, int, 2>::const_iterator it = m.begin();
	++it;
	EXPECT_EQ(it.key().value(), 20);
	EXPECT_EQ(it.value(), 1);
	EXPECT_TRUE(check_stats(6, 2, 0, 2, 3, 1, 1, 0, 0, 0, 1));
}

TEST_F(hxflat_map_test_f, insert_dynamic) {
	hxflat_map<hxtest_object, int> m;
	m.reserve(1);
	const hxtest_object k(3);
	m.insert(k, 7);
	EXPECT_EQ(m.size(), 1);
	EXPECT_EQ(m.find(k).value(), 7);
	EXPECT_TRUE(check_stats(2, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1));
}

TEST_F(hxflat_map_test_f, clear) {
	const hxtest_object k1(1), k2(2);
	hxflat_map<hxtest_object, int, 2> m{
		{k1, 1}, {k2, 2}};
	EXPECT_EQ(m.size(), 2);
	m.clear();
	EXPECT_EQ(m.size(), 0);
	EXPECT_TRUE(m.empty());
	m.clear();
	EXPECT_EQ(m.size(), 0);
	EXPECT_TRUE(check_stats(6, 4, 0, 2, 4, 0, 0, 0, 0, 0, 1));
}

TEST_F(hxflat_map_test_f, find) {
	hxflat_map<hxtest_object, int, 2> m;
	const hxtest_object k(7);
	m.insert(k, 99);
	const hxflat_map<hxtest_object, int, 2>::iterator p = m.find(k);
	EXPECT_NE(p, m.end());
	EXPECT_EQ(p.value(), 99);
	p.value() = 55;
	EXPECT_EQ(m.find(k).value(), 55);
	const hxtest_object k99(99);
	EXPECT_EQ(m.find(k99), m.end());
	const hxflat_map<hxtest_object, int, 2>& cm = m;
	EXPECT_EQ(cm.find(k).value(), 55);
	EXPECT_EQ(cm.find(k99), cm.end());
	const hxflat_map<hxtest_object, int, 2> empty;
	const hxtest_object k1(1);
	EXPECT_EQ(empty.find(k1), empty.end());
	EXPECT_TRUE(check_stats(4, 0, 0, 3, 1, 0, 0, 0, 0, 0, 5));
}

TEST_F(hxflat_map_test_f, count) {
	hxflat_multimap<hxtest_object, int, 3> m;
	const hxtest_object k4(4), k9(9);
	m.insert(k4, 1);
	EXPECT_EQ(m.count(k4), 1);
	EXPECT_EQ(m.count(k9), 0);
	m.insert(k4, 2);
	m.insert(k4, 3);
	EXPECT_EQ(m.count(k4), 3);
	EXPECT_TRUE(check_stats(5, 0, 0, 2, 1, 2, 2, 1, 0, 0, 10));
}

TEST_F(hxflat_map_test_f, lower_bound) {
	hxflat_map<hxtest_object, int, 3> m;
	const hxtest_object k10(10), k30(30), k20(20), k99(99);
	m.insert(k10, 1);
	m.insert(k30, 2);
	EXPECT_EQ(m.lower_bound(k10).key().value(), 10);
	EXPECT_EQ(m.lower_bound(k20).key().value(), 30);
	EXPECT_TRUE(m.lower_bound(k99) == m.end());
	m.insert(k20, 3);
	EXPECT_EQ(m.lower_bound(k20).key().value(), 20);
	const hxflat_map<hxtest_object, int, 3>::iterator it = m.lower_bound(k20);
	it.value() = 99;
	EXPECT_EQ(m.find(k20).value(), 99);
	EXPECT_TRUE(check_stats(7, 0, 0, 4, 2, 1, 1, 0, 0, 0, 14));
}

TEST_F(hxflat_map_test_f, upper_bound) {
	hxflat_multimap<hxtest_object, int, 3> m;
	const hxtest_object k10(10), k20(20), k15(15), k9999(9999);
	m.insert(k10, 1);
	m.insert(k20, 2);
	const hxflat_multimap<hxtest_object, int, 3>& cm = m;
	EXPECT_EQ(cm.upper_bound(k10).key().value(), 20);
	EXPECT_EQ(m.upper_bound(k15).key().value(), 20);
	EXPECT_TRUE(m.upper_bound(k20) == m.end());
	m.insert(k10, 3);
	EXPECT_EQ(m.upper_bound(k10).key().value(), 20);
	EXPECT_TRUE(m.upper_bound(k9999) == m.end());
	EXPECT_TRUE(check_stats(7, 0, 0, 4, 2, 1, 1, 1, 0, 0, 12));
}

TEST_F(hxflat_map_test_f, erase_key_unique) {
	hxflat_map<hxtest_object, int, 1> m;
	const hxtest_object k5(5), k9(9), k99(99), k1(1);
	m.insert(k5, 1);
	EXPECT_EQ(m.erase(k9), 0);
	EXPECT_EQ(m.size(), 1);
	EXPECT_EQ(m.erase(k99), 0);
	EXPECT_EQ(m.erase(k1), 0);
	EXPECT_EQ(m.size(), 1);
	EXPECT_EQ(m.erase(k5), 1);
	EXPECT_TRUE(m.empty());
	EXPECT_TRUE(check_stats(5, 1, 0, 4, 1, 0, 0, 0, 0, 0, 4));
}

TEST_F(hxflat_map_test_f, erase_key_multi) {
	hxflat_multimap<hxtest_object, int, 4> m;
	const hxtest_object k5(5), k9(9), k10(10);
	m.insert(k5, 1);
	EXPECT_EQ(m.erase(k9), 0);
	m.insert(k5, 2);
	m.insert(k10, 3);
	EXPECT_EQ(m.erase(k5), 2);
	EXPECT_EQ(m.size(), 1);
	EXPECT_EQ(m.count(k10), 1);
	m.insert(k10, 4);
	m.insert(k10, 5);
	EXPECT_EQ(m.erase(k10), 3);
	EXPECT_EQ(m.size(), 0);
	EXPECT_TRUE(check_stats(8, 5, 0, 3, 2, 3, 3, 2, 0, 0, 16));
}

TEST_F(hxflat_map_test_f, erase_key_multi_tail_relative_to_count) {
	hxflat_multimap<hxtest_object, int, 4> ma;
	const hxtest_object k5(5), k10(10);
	ma.insert(k5, 1);
	ma.insert(k5, 2);
	ma.insert(k5, 3);
	ma.insert(k10, 4);
	EXPECT_EQ(ma.erase(k5), 3);
	EXPECT_EQ(ma.size(), 1);
	EXPECT_EQ(ma.find(k10).value(), 4);
	hxflat_multimap<hxtest_object, int, 4> mb;
	mb.insert(k5, 1);
	mb.insert(k10, 2);
	mb.insert(k10, 3);
	mb.insert(k10, 4);
	EXPECT_EQ(mb.erase(k5), 1);
	EXPECT_EQ(mb.size(), 3);
	EXPECT_EQ(mb.count(k10), 3);
	EXPECT_TRUE(check_stats(10, 4, 0, 2, 4, 4, 4, 6, 0, 0, 26));
}

TEST_F(hxflat_map_test_f, erase_iterator_only_and_pair) {
	hxflat_map<hxtest_object, int, 2> m;
	const hxtest_object k7(7), k10(10), k20(20);
	m.insert(k7, 1);
	const hxflat_map<hxtest_object, int, 2>::iterator next1 =
		m.erase(m.begin());
	EXPECT_TRUE(next1 == m.end());
	EXPECT_TRUE(m.empty());
	m.insert(k10, 1);
	m.insert(k20, 2);
	const hxflat_map<hxtest_object, int, 2>::iterator next2 =
		m.erase(m.begin());
	EXPECT_EQ(next2.key(), 20);
	EXPECT_EQ(m.size(), 1);
	const hxflat_map<hxtest_object, int, 2>::iterator last = m.begin();
	const hxflat_map<hxtest_object, int, 2>::iterator next3 = m.erase(last);
	EXPECT_TRUE(next3 == m.end());
	EXPECT_EQ(m.size(), 0);
	EXPECT_TRUE(check_stats(6, 3, 0, 3, 3, 0, 0, 1, 0, 0, 1));
}

TEST_F(hxflat_map_test_f, erase_iterator_first_and_middle_of_three) {
	hxflat_map<hxtest_object, int, 3> m;
	const hxtest_object k10(10), k20(20), k30(30);
	m.insert(k10, 1);
	m.insert(k20, 2);
	m.insert(k30, 3);
	hxflat_map<hxtest_object, int, 3>::iterator mid = m.begin();
	++mid;
	const hxflat_map<hxtest_object, int, 3>::iterator next1 = m.erase(mid);
	EXPECT_EQ(next1.key(), 30);
	EXPECT_EQ(m.size(), 2);
	const hxflat_map<hxtest_object, int, 3>::iterator next2 =
		m.erase(m.begin());
	EXPECT_EQ(next2.key(), 30);
	EXPECT_EQ(m.size(), 1);
	EXPECT_EQ(m.find(k10), m.end());
	EXPECT_EQ(m.find(k30).value(), 3);
	EXPECT_TRUE(check_stats(6, 2, 0, 3, 3, 0, 0, 2, 0, 0, 4));
}

TEST_F(hxflat_map_test_f, const_iterator_arithmetic) {
	const hxtest_object k10(10), k20(20), k30(30);
	const hxflat_map<hxtest_object, int, 3> m{
		{k10, 1}, {k20, 2}, {k30, 3}};
	hxflat_map<hxtest_object, int, 3>::const_iterator it = m.begin();
	it += 2;
	EXPECT_EQ(it.key(), 30);
	it -= 1;
	EXPECT_EQ(it.key(), 20);
	const hxflat_map<hxtest_object, int, 3>::const_iterator it2 = it + 1;
	EXPECT_EQ(it2.key(), 30);
	const hxflat_map<hxtest_object, int, 3>::const_iterator it3 = it2 - 1;
	EXPECT_EQ(it3.key(), 20);
	EXPECT_EQ(m.begin()[1].key, 20);
	EXPECT_EQ(m.end() - m.begin(), ptrdiff_t{3});
	EXPECT_TRUE(check_stats(9, 3, 0, 3, 6, 0, 0, 0, 0, 0, 2));
}

TEST_F(hxflat_map_test_f, const_iterator_pre_post_increment_decrement) {
	const hxtest_object k10(10), k20(20);
	const hxflat_map<hxtest_object, int, 2> m{
		{k10, 1}, {k20, 2}};
	hxflat_map<hxtest_object, int, 2>::const_iterator it = m.begin();
	const hxflat_map<hxtest_object, int, 2>::const_iterator it2 = ++it;
	EXPECT_EQ(it.key(), 20);
	EXPECT_EQ(it2.key(), 20);
	--it;
	EXPECT_EQ(it.key(), 10);
	const hxflat_map<hxtest_object, int, 2>::const_iterator it3 = it++;
	EXPECT_EQ(it3.key(), 10);
	EXPECT_EQ(it.key(), 20);
	const hxflat_map<hxtest_object, int, 2>::const_iterator it4 = it--;
	EXPECT_EQ(it4.key(), 20);
	EXPECT_EQ(it.key(), 10);
	EXPECT_TRUE(check_stats(6, 2, 0, 2, 4, 0, 0, 0, 0, 0, 1));
}

TEST_F(hxflat_map_test_f, const_iterator_equality_and_order) {
	const hxtest_object k10(10), k20(20);
	const hxflat_map<hxtest_object, int, 2> m{
		{k10, 1}, {k20, 2}};
	const hxflat_map<hxtest_object, int, 2>::const_iterator a = m.begin();
	hxflat_map<hxtest_object, int, 2>::const_iterator b = m.begin();
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
	EXPECT_TRUE(check_stats(6, 2, 0, 2, 4, 0, 0, 0, 0, 0, 1));
}

TEST_F(hxflat_map_test_f, const_iterator_dereference_proxy) {
	hxflat_map<hxtest_object, int, 1> m;
	const hxtest_object k(3);
	m.insert(k, 7);
	const hxflat_map<hxtest_object, int, 1>::const_iterator it = m.begin();
	EXPECT_EQ((*it).key, 3);
	EXPECT_EQ((*it).value, 7);
	EXPECT_TRUE(check_stats(2, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0));
}

TEST_F(hxflat_map_test_f, iterator_dereference_and_mutation) {
	hxflat_map<hxtest_object, int, 1> m;
	const hxtest_object k(3);
	m.insert(k, 7);
	const hxflat_map<hxtest_object, int, 1>::iterator it = m.begin();
	EXPECT_EQ((*it).key, 3);
	(*it).value = 99;
	EXPECT_EQ(m.find(k).value(), 99);
	EXPECT_TRUE(check_stats(2, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1));
}

TEST_F(hxflat_map_test_f, iterator_arithmetic) {
	const hxtest_object k10(10), k20(20), k30(30);
	hxflat_map<hxtest_object, int, 3> m{
		{k10, 1}, {k20, 2}, {k30, 3}};
	hxflat_map<hxtest_object, int, 3>::iterator it = m.begin();
	it += 2;
	EXPECT_EQ(it.key(), 30);
	it -= 1;
	EXPECT_EQ(it.key(), 20);
	const hxflat_map<hxtest_object, int, 3>::iterator it2 = it + 1;
	EXPECT_EQ(it2.key(), 30);
	const hxflat_map<hxtest_object, int, 3>::iterator it3 = it2 - 1;
	EXPECT_EQ(it3.key(), 20);
	EXPECT_EQ(it[1].key, 30);
	EXPECT_EQ(it2 - it3, hxsize_t{1});
	EXPECT_TRUE(check_stats(9, 3, 0, 3, 6, 0, 0, 0, 0, 0, 2));
}

TEST_F(hxflat_map_test_f, iterator_pre_post_increment_decrement) {
	const hxtest_object k10(10), k20(20);
	hxflat_map<hxtest_object, int, 2> m{
		{k10, 1}, {k20, 2}};
	hxflat_map<hxtest_object, int, 2>::iterator it = m.begin();
	const hxflat_map<hxtest_object, int, 2>::iterator before = it++;
	EXPECT_EQ(before.key(), 10);
	EXPECT_EQ(it.key(), 20);
	const hxflat_map<hxtest_object, int, 2>::iterator after = it--;
	EXPECT_EQ(after.key(), 20);
	EXPECT_EQ(it.key(), 10);
	++it;
	EXPECT_EQ(it.key(), 20);
	--it;
	EXPECT_EQ(it.key(), 10);
	EXPECT_TRUE(check_stats(6, 2, 0, 2, 4, 0, 0, 0, 0, 0, 1));
}

TEST_F(hxflat_map_test_f, begin_end) {
	hxflat_map<hxtest_object, int, 1> m;
	EXPECT_TRUE(m.begin() == m.end());
	EXPECT_TRUE(m.cbegin() == m.cend());
	const hxtest_object k(5);
	m.insert(k, 1);
	EXPECT_FALSE(m.begin() == m.end());
	EXPECT_EQ(m.begin().key(), 5);
	EXPECT_TRUE(check_stats(2, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0));
}

TEST_F(hxflat_map_test_f, full) {
	hxflat_map<hxtest_object, int, 2> m;
	const hxtest_object k1(1), k2(2);
	EXPECT_FALSE(m.full());
	m.insert(k1, 1);
	EXPECT_FALSE(m.full());
	m.insert(k2, 2);
	EXPECT_TRUE(m.full());
	EXPECT_TRUE(check_stats(4, 0, 0, 2, 2, 0, 0, 0, 0, 0, 1));
}

TEST_F(hxflat_map_test_f, dynamic_multimap_insert_erase) {
	hxflat_multimap<hxtest_object, int> m;
	m.reserve(5);
	const hxtest_object k10(10), k20(20), k30(30);
	m.insert(k10, 1);
	m.insert(k10, 2);
	m.insert(k20, 3);
	m.insert(k30, 4);
	m.insert(k30, 5);
	EXPECT_EQ(m.size(), 5);
	EXPECT_EQ(m.count(k10), 2);
	EXPECT_EQ(m.count(k20), 1);
	EXPECT_EQ(m.count(k30), 2);
	EXPECT_EQ(m.erase(k10), 2);
	EXPECT_EQ(m.size(), 3);
	EXPECT_EQ(m.erase(k30), 2);
	EXPECT_EQ(m.size(), 1);
	EXPECT_TRUE(check_stats(8, 4, 0, 3, 3, 2, 2, 3, 0, 0, 27));
}

TEST_F(hxflat_map_test_f, destructor_destroys_elements) {
	{
		hxflat_map<hxtest_object, int, 2> m;
		{
			const hxtest_object k1(1), k2(2);
			m.insert(k1, 10);
			m.insert(k2, 20);
		}
	}
	EXPECT_TRUE(check_stats(4, 4, 0, 2, 2, 0, 0, 0, 0, 0, 1));
}

TEST_F(hxflat_map_test_f, default_constructed_iterator_assignable) {
	hxflat_map<hxtest_object, int, 1> m;
	const hxtest_object k(3);
	m.insert(k, 7);
	hxflat_map<hxtest_object, int, 1>::const_iterator cit;
	cit = m.begin();
	EXPECT_EQ(cit.key(), 3);
	hxflat_map<hxtest_object, int, 1>::iterator it;
	it = m.begin();
	EXPECT_EQ(it.key(), 3);
	it.value() = 99;
	EXPECT_EQ(m.find(k).value(), 99);
	EXPECT_TRUE(check_stats(2, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1));
}

TEST_F(hxflat_map_test_f, operator_equal) {
	hxflat_map<hxtest_object, int, 3> a;
	hxflat_map<hxtest_object, int, 4> b;
	const hxtest_object k1(1), k2(2);
	EXPECT_TRUE(a == b);
	a.insert(k1, 10);
	a.insert(k2, 20);
	b.insert(k1, 10);
	EXPECT_FALSE(a == b);
	b.insert(k2, 20);
	EXPECT_TRUE(a == b);
	EXPECT_TRUE(check_stats(6, 0, 0, 2, 4, 0, 0, 0, 2, 0, 2));
}

TEST_F(hxflat_map_test_f, operator_equal_mismatched_keys_or_values) {
	hxflat_map<hxtest_object, int, 2> a;
	hxflat_map<hxtest_object, int, 2> b;
	const hxtest_object k1(1);
	a.insert(k1, 10);
	b.insert(k1, 99);
	EXPECT_FALSE(a == b);
	hxflat_map<hxtest_object, int, 2> c;
	hxflat_map<hxtest_object, int, 2> d;
	const hxtest_object k1c(1), k2d(2);
	c.insert(k1c, 10);
	d.insert(k2d, 10);
	EXPECT_FALSE(c == d);
	hxflat_map<hxtest_object, int, 2> e;
	hxflat_map<hxtest_object, int, 2> f;
	const hxtest_object k1e(1), k2e(2);
	e.insert(k1e, 10);
	e.insert(k2e, 20);
	f.insert(k1e, 10);
	f.insert(k2e, 99);
	EXPECT_FALSE(e == f);
	hxflat_map<hxtest_object, int, 2> g;
	hxflat_map<hxtest_object, int, 2> h;
	const hxtest_object k1g(1), k2g(2);
	g.insert(k1g, 10);
	g.insert(k2g, 20);
	h.insert(k1g, 10);
	h.insert(k2g, 20);
	EXPECT_TRUE(g == h);
	EXPECT_TRUE(check_stats(19, 0, 0, 7, 12, 0, 0, 0, 6, 0, 4));
}

TEST_F(hxflat_map_test_f, operator_less) {
	hxflat_map<hxtest_object, int, 3> a;
	hxflat_map<hxtest_object, int, 3> b;
	EXPECT_FALSE(a < b);
	const hxtest_object k1(1), k2(2);
	b.insert(k1, 1);
	EXPECT_TRUE(a < b);
	EXPECT_FALSE(b < a);
	a.insert(k1, 1);
	EXPECT_FALSE(a < b);
	EXPECT_FALSE(b < a);
	b.insert(k2, 1);
	EXPECT_TRUE(a < b);
	EXPECT_FALSE(b < a);
	EXPECT_TRUE(check_stats(5, 0, 0, 2, 3, 0, 0, 0, 4, 0, 1));
}

TEST_F(hxflat_map_test_f, operator_less_smaller_key_or_value_is_less) {
	hxflat_map<hxtest_object, int, 2> a;
	hxflat_map<hxtest_object, int, 2> b;
	const hxtest_object k1(1), k2(2);
	a.insert(k1, 10);
	b.insert(k2, 10);
	EXPECT_TRUE(a < b);
	EXPECT_FALSE(b < a);
	hxflat_map<hxtest_object, int, 2> c;
	hxflat_map<hxtest_object, int, 2> d;
	c.insert(k1, 5);
	d.insert(k1, 10);
	EXPECT_TRUE(c < d);
	EXPECT_FALSE(d < c);
	EXPECT_TRUE(check_stats(6, 0, 0, 2, 4, 0, 0, 0, 8, 2, 0));
}

TEST_F(hxflat_map_test_f, subscript) {
	const hxtest_object k1(1), k2(2), k3(3);
	hxflat_map<hxtest_object, int, 3> m{
		{k1, 10}, {k2, 20}, {k3, 30}};
	const hxflat_map<hxtest_object, int, 3>& cm = m;
	EXPECT_EQ(cm[0].key(), 1);
	EXPECT_EQ(cm[0].value(), 10);
	EXPECT_EQ(cm[2].key(), 3);
	m[0].value() = 99;
	EXPECT_EQ(m.find(k1).value(), 99);
	EXPECT_EQ(m[m.size() - 1].key(), 3);
	EXPECT_TRUE(check_stats(9, 3, 0, 3, 6, 0, 0, 0, 0, 0, 4));
}

TEST_F(hxflat_map_test_f, copy_assign) {
	hxflat_map<hxtest_object, int, 3> a;
	hxflat_map<hxtest_object, int, 4> b;
	const hxtest_object k7(7), k1(1), k2(2);
	b.insert(k7, 99);
	a.insert(k1, 10);
	a.insert(k2, 20);
	b = a;
	EXPECT_EQ(b.size(), 2);
	EXPECT_EQ(b.find(k1).value(), 10);
	EXPECT_EQ(b.find(k2).value(), 20);
	EXPECT_EQ(b.find(k7), b.end());
	const hxflat_map<hxtest_object, int, 3> empty;
	b = empty;
	EXPECT_TRUE(b.empty());
	EXPECT_TRUE(check_stats(8, 3, 0, 3, 5, 0, 0, 0, 0, 0, 6));
}

TEST_F(hxflat_map_test_f, move_assign_transfers_elements) {
	{
		const hxtest_object k1(1), k2(2), k3(3);
		hxflat_multimap<hxtest_object, int> a{{k1, 10}, {k2, 20}, {k3, 30}};
		hxflat_multimap<hxtest_object, int> b;
		b.reserve(1);
		b = hxmove(a);
		EXPECT_EQ(b.size(), 3);
		EXPECT_EQ(b.find(k1).value(), 10);
		EXPECT_EQ(b.find(k2).value(), 20);
		EXPECT_EQ(b.find(k3).value(), 30);
		EXPECT_EQ(a.size(), 0);
		EXPECT_EQ(a.capacity(), 1);
	}
	EXPECT_TRUE(check_stats(9, 9, 0, 3, 6, 0, 0, 0, 0, 0, 8));
}

TEST_F(hxflat_map_test_f, move_constructor_transfers_elements) {
	{
		const hxtest_object k1(1), k2(2), k3(3);
		hxflat_multimap<hxtest_object, int> src{{k1, 10}, {k2, 20}, {k3, 30}};
		hxflat_multimap<hxtest_object, int> dst(hxmove(src));
		EXPECT_EQ(dst.size(), 3);
		EXPECT_EQ(dst.find(k1).value(), 10);
		EXPECT_EQ(dst.find(k2).value(), 20);
		EXPECT_EQ(dst.find(k3).value(), 30);
		EXPECT_EQ(src.size(), 0);
		EXPECT_EQ(src.capacity(), 0);
	}
	EXPECT_TRUE(check_stats(9, 9, 0, 3, 6, 0, 0, 0, 0, 0, 8));
}

TEST_F(hxflat_map_test_f, copy_constructor) {
	const hxtest_object k1(1), k2(2), k3(3);
	hxflat_map<hxtest_object, int, 3> src{
		{k1, 10}, {k2, 20}, {k3, 30}};
	const hxflat_map<hxtest_object, int, 3> dst(src);
	EXPECT_EQ(dst.size(), 3);
	EXPECT_EQ(dst.find(k1).value(), 10);
	EXPECT_EQ(dst.find(k2).value(), 20);
	EXPECT_EQ(dst.find(k3).value(), 30);
	EXPECT_EQ(src.size(), 3);
	src.find(k1).value() = 99;
	EXPECT_EQ(dst.find(k1).value(), 10);
	EXPECT_TRUE(check_stats(12, 3, 0, 3, 9, 0, 0, 0, 0, 0, 12));
}

TEST_F(hxflat_map_test_f, copy_constructor_empty) {
	const hxflat_map<hxtest_object, int, 4> src;
	const hxflat_map<hxtest_object, int, 4> dst(src); // NOLINT(performance-unnecessary-copy-initialization)
	EXPECT_TRUE(dst.empty());
	EXPECT_EQ(dst.size(), 0);
	EXPECT_TRUE(check_no_stats());
}

TEST_F(hxflat_map_test_f, copy_constructor_lifecycle) {
	{
		const hxtest_object k1(1), k2(2);
		const hxflat_map<hxtest_object, int, 3> src{
			{k1, 10}, {k2, 20}};
		{
			const hxflat_map<hxtest_object, int, 3> dst(src); // NOLINT(performance-unnecessary-copy-initialization)
			EXPECT_EQ(dst.size(), 2);
		}
	}
	EXPECT_TRUE(check_stats(8, 8, 0, 2, 6, 0, 0, 0, 0, 0, 1));
}

TEST(hxflat_map_test, implements_rand_iterator_api) {
	hxflat_map<int, int, 4> m{
		{1, 10}, {2, 20}, {3, 30}};
	const hxflat_map<int, int, 4>& cm = m;
	EXPECT_TRUE(hxtest_check_rand_iterator_api(m.begin(), m.end()));
	EXPECT_TRUE(hxtest_check_rand_iterator_api(cm.begin(), cm.end()));
}

#if HX_CPLUSPLUS >= 202002L
TEST_F(hxflat_map_test_f, hxkey_equal) {
	typedef hxflat_map<hxtest_object, int, 2> map_t;
	map_t a;
	hxflat_map<hxtest_object, int, 4> b;
	const hxtest_object k1(1), k2(2);
	a.insert(k1, 10);
	b.insert(k1, 10);
	EXPECT_TRUE(a == b);
	b.insert(k2, 20);
	a.insert(k2, 20);
	EXPECT_TRUE(a == b);
	map_t c;
	c.insert(k1, 20);
	EXPECT_FALSE(a == c);
	EXPECT_TRUE(check_stats(7, 0, 0, 2, 5, 0, 0, 0, 3, 0, 2));
}

TEST_F(hxflat_map_test_f, hxkey_less) {
	typedef hxflat_map<hxtest_object, int, 4> map_t;
	hxflat_map<hxtest_object, int, 2> a;
	map_t b;
	const hxtest_object k1(1), k2(2);
	a.insert(k1, 10);
	b.insert(k2, 10);
	EXPECT_TRUE(a < b);
	EXPECT_FALSE(b < a);
	hxflat_map<hxtest_object, int, 2> c;
	c.insert(k2, 10);
	EXPECT_FALSE(b < c);
	EXPECT_FALSE(c < b);
	EXPECT_TRUE(check_stats(5, 0, 0, 2, 3, 0, 0, 0, 6, 2, 0));
}

TEST_F(hxflat_map_test_f, hxswap_exchanges_contents) {
	{
		hxflat_multimap<hxtest_object, int> a;
		a.reserve(2);
		hxflat_multimap<hxtest_object, int> b;
		b.reserve(2);
		const hxtest_object k1(1), k2(2);
		a.insert(k1, 10);
		b.insert(k2, 20);
		hxswap(a, b);
		EXPECT_EQ(a.size(), 1);
		EXPECT_EQ(a.find(k2).value(), 20);
		EXPECT_EQ(b.size(), 1);
		EXPECT_EQ(b.find(k1).value(), 10);
	}
	EXPECT_TRUE(check_stats(4, 4, 0, 2, 2, 0, 0, 0, 0, 0, 2));
}

TEST_F(hxflat_map_test_f, hxswap_empty_and_nonempty) {
	{
		hxflat_multimap<hxtest_object, int> a;
		a.reserve(2);
		hxflat_multimap<hxtest_object, int> b;
		b.reserve(1);
		const hxtest_object k1(1), k2(2);
		a.insert(k1, 10);
		a.insert(k2, 20);
		hxswap(a, b);
		EXPECT_TRUE(a.empty());
		EXPECT_EQ(b.size(), 2);
		EXPECT_EQ(b.find(k1).value(), 10);
		EXPECT_EQ(b.find(k2).value(), 20);
	}
	EXPECT_TRUE(check_stats(4, 4, 0, 2, 2, 0, 0, 0, 0, 0, 5));
}
#endif // HX_CPLUSPLUS >= 202002L

TEST_F(hxflat_map_test_f, three_way_find_hit_costs_one_comparison) {
	hxflat_map<hxtest_object, int, hxallocator_dynamic_capacity, hxthree_way_t<hxtest_object>, hxtrait_three_way> m;
	m.reserve(4);
	const hxtest_object k10(10), k20(20), k30(30);
	m.insert(k10, 100);
	m.insert(k20, 200);
	m.insert(k30, 300);
	const auto p = m.find(hxtest_object(20));
	EXPECT_NE(p, m.end());
	EXPECT_EQ(p.value(), 200);
	EXPECT_EQ(m.find(hxtest_object(1)), m.end());
	EXPECT_TRUE(check_stats(8, 2, 0, 5, 3, 0, 0, 0, 0, 0, 6));
}

TEST_F(hxflat_map_test_f, three_way_count_insert_erase_unique) {
	hxflat_map<hxtest_object, int, hxallocator_dynamic_capacity, hxthree_way_t<hxtest_object>, hxtrait_three_way> m;
	m.reserve(4);
	const hxtest_object k5(5);
	m.insert(k5, 50);
	const hxtest_object k5b(5);
	const auto dup = m.insert(k5b, 51);
	EXPECT_EQ(dup.value(), 50);
	EXPECT_EQ(m.size(), 1);
	EXPECT_EQ(m.count(hxtest_object(5)), 1);
	EXPECT_EQ(m.count(hxtest_object(9)), 0);
	EXPECT_EQ(m.count(hxtest_object(1)), 0);
	EXPECT_EQ(m.erase(hxtest_object(5)), 1);
	EXPECT_EQ(m.erase(hxtest_object(5)), 0);
	EXPECT_TRUE(check_stats(8, 6, 0, 7, 1, 0, 0, 0, 0, 0, 5));
}

TEST_F(hxflat_map_test_f, three_way_multi_count_and_erase) {
	hxflat_multimap<hxtest_object, int, hxallocator_dynamic_capacity, hxthree_way_t<hxtest_object>, hxtrait_three_way> m;
	m.reserve(4);
	const hxtest_object k7a(7), k7b(7), k3(3);
	m.insert(k7a, 70);
	m.insert(k7b, 71);
	m.insert(k3, 30);
	EXPECT_EQ(m.count(hxtest_object(7)), 2);
	EXPECT_EQ(m.erase(hxtest_object(7)), 2);
	EXPECT_EQ(m.size(), 1);
	EXPECT_TRUE(check_stats(8, 4, 0, 5, 1, 2, 2, 1, 0, 0, 9));
}

TEST(hxflat_map_test, three_way_int_key_uses_subtraction_fallback) {
	hxflat_map<int, int, hxallocator_dynamic_capacity, hxthree_way_t<int>, hxtrait_three_way> m;
	m.reserve(4);
	m.insert(31, 310);
	m.insert(32, 320);
	m.insert(33, 330);
	EXPECT_EQ(m.size(), 3);
	EXPECT_NE(m.find(32), m.end());
	EXPECT_EQ(m.find(32).value(), 320);
	EXPECT_EQ(m.find(1), m.end());
	EXPECT_EQ(m.count(32), 1);
	EXPECT_EQ(m.erase(32), 1);
	EXPECT_EQ(m.erase(32), 0);
	EXPECT_EQ(m.size(), 2);
}
