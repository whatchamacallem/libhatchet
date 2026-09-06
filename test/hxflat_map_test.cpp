// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

#include <hx/hxflat_map.hpp>
#include <hx/hxmemory_manager.h>
#if HX_CPLUSPLUS >= 202302L
#include <hx/hxexpected.hpp>
#endif // HX_CPLUSPLUS >= 202302L
#include "./hxtest_util.hpp"

HX_NS_USE

hxattr_noinline static void hxtest_gdb_break_hxflat_map_static(void) { }
hxattr_noinline static void hxtest_gdb_break_hxflat_map_dynamic(void) { }

#if !defined _MSC_VER && !defined __wasm__
static_assert(sizeof(size_t) != 4 || (
		sizeof(hxflat_map<int32_t, int32_t, hxkey_less_t<int32_t>, false, 4>) == 36u
		&& sizeof(hxflat_map<int32_t, int32_t, hxkey_less_t<int32_t>, false>) == 20u),
	"hxflat_map must pack fixed storage as a hxsize_t size plus two capacity *"
	" sizeof(T) key and value arrays and dynamic storage as a hxsize_t size"
	" plus two hxsize_t/T* allocator pairs with no padding");
static_assert(sizeof(size_t) != 8 || (
		sizeof(hxflat_map<int32_t, int32_t, hxkey_less_t<int32_t>, false, 4>) == 40u
		&& sizeof(hxflat_map<int32_t, int32_t, hxkey_less_t<int32_t>, false>) == 40u),
	"hxflat_map must pack fixed storage as a hxsize_t size plus two capacity *"
	" sizeof(T) key and value arrays and dynamic storage as a hxsize_t size"
	" plus two hxsize_t/T* allocator pairs with no padding");
#endif

using hxflat_map_test_f = hxtest_object_fixture;

TEST(hxflat_map_test, static_initializer_list_ctor_sorts_and_rejects_duplicates) {
	const hxflat_map<int, int, hxkey_less_t<int>, false, 4> m{
		{3, 30}, {1, 10}, {2, 20}, {1, 11}};
	EXPECT_EQ(m.size(), 3);
	EXPECT_EQ(m.find(1).value(), 10);
	EXPECT_EQ(m.find(2).value(), 20);
	EXPECT_EQ(m.find(3).value(), 30);
}

TEST(hxflat_map_test, dynamic_initializer_list_ctor_allows_duplicates) {
	const hxflat_map<int, int> m{
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
	const hxflat_map<int, int, hxkey_less_t<int>, false, 2> a{ {1, 10} };
	const hxflat_map<int, int, hxkey_less_t<int>, false, 2> b{ {1, 10} };
	EXPECT_TRUE(a == b);
	const hxflat_map<int, int, hxkey_less_t<int>, false, 2> c{ {1, 99} };
	EXPECT_FALSE(a == c);
	const hxflat_map<int, int, hxkey_less_t<int>, false, 2> d{ {2, 10} };
	EXPECT_FALSE(a == d);
	const hxflat_map<int, int, hxkey_less_t<int>, false, 2> e{ {1, 10}, {2, 20} };
	EXPECT_FALSE(a == e);
	EXPECT_FALSE(e == a);
	const hxflat_map<int, int, hxkey_less_t<int>, false, 2> f{ {1, 10}, {2, 99} };
	EXPECT_FALSE(e == f);
}

TEST(hxflat_map_test, mutable_value_proxy_operator_equal_detects_key_and_value_mismatch) {
	hxflat_map<int, int, hxkey_less_t<int>, false, 2> a{ {1, 10} };
	hxflat_map<int, int, hxkey_less_t<int>, false, 2> b{ {1, 10} };
	hxflat_map<int, int, hxkey_less_t<int>, false, 2> c{ {1, 99} };
	hxflat_map<int, int, hxkey_less_t<int>, false, 2> d{ {2, 10} };
	EXPECT_TRUE(*a.begin() == *b.begin());
	EXPECT_FALSE(*a.begin() == *c.begin());
	EXPECT_FALSE(*a.begin() == *d.begin());
}

#if HX_CPLUSPLUS >= 202302L
TEST(hxflat_map_test, expected_lookup_and_emplace) {
	hxflat_map<int, int, hxkey_less_t<int>, false, 3> m;
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
	const hxflat_map<int, int, hxkey_less_t<int>, false, 3>& cm = m;
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
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 1> m;
	m.emplace(1, 10);
	EXPECT_EQ(m.value_or(1, 14, 17).value(), 10);
	EXPECT_EQ(m.value_or(2, 14, 17).value(), 31);
	EXPECT_EQ(m.value_or(m.end(), 20, 23).value(), 43);
	EXPECT_TRUE(check_stats(5, 4, 0, 3, 1, 1, 0, 0, 0, 0));
}
#endif // HX_CPLUSPLUS >= 202302L

TEST_F(hxflat_map_test_f, gdb_break) {
	const hxtest_object k1(1), k2(2), k3(3);
	const hxtest_object v1(10), v2(20), v3(30);
	const hxflat_map<hxtest_object, hxtest_object, hxkey_less_t<hxtest_object>, false, 4> a{
		{k1, v1}, {k2, v2}, {k3, v3}};
	hxtest_gdb_break_hxflat_map_static();
	EXPECT_EQ(a.size(), 3);
	const hxflat_map<hxtest_object, hxtest_object, hxkey_less_t<hxtest_object>> b{
		{k1, v1}, {k2, v2}, {k3, v3}};
	const hxflat_map<hxtest_object, hxtest_object, hxkey_less_t<hxtest_object>> c;
	hxflat_map<hxtest_object, hxtest_object, hxkey_less_t<hxtest_object>> d;
	d.reserve(4);
	hxtest_gdb_break_hxflat_map_dynamic();
	EXPECT_EQ(b.size(), 3);
	EXPECT_TRUE(c.empty());
	EXPECT_TRUE(d.empty());
	EXPECT_TRUE(check_stats(30, 12, 0, 6, 24, 0, 0, 0, 0, 4));
}

TEST_F(hxflat_map_test_f, construct) {
	const hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 1> ms;
	EXPECT_TRUE(ms.empty());
	EXPECT_EQ(ms.size(), 0);
	EXPECT_EQ(ms.capacity(), 1);
	EXPECT_EQ(ms.max_size(), 1);
	EXPECT_FALSE(ms.full());
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false> md;
	EXPECT_TRUE(md.empty());
	EXPECT_EQ(md.capacity(), 0);
	EXPECT_EQ(md.max_size(), 0);
	md.reserve(8);
	EXPECT_EQ(md.capacity(), 8);
	EXPECT_EQ(md.max_size(), 8);
	EXPECT_TRUE(check_no_stats());
}

TEST_F(hxflat_map_test_f, reserve_static_exact) {
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 1> m;
	m.reserve(1);
	EXPECT_EQ(m.capacity(), 1);
	EXPECT_TRUE(check_no_stats());
}

TEST_F(hxflat_map_test_f, insert_unique_basic) {
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 2> m;
	const hxtest_object v1(10), v2(20);
	const hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 2>::iterator it1 = m.insert(1, v1);
	EXPECT_EQ(it1.key(), 1);
	EXPECT_EQ(it1.value().value(), 10);
	EXPECT_EQ(m.size(), 1);
	const hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 2>::iterator it2 = m.insert(1, v2);
	EXPECT_EQ(it2.key(), 1);
	EXPECT_EQ(it2.value().value(), 10);
	EXPECT_EQ(m.size(), 1);
	EXPECT_TRUE(check_stats(3, 0, 0, 2, 1, 0, 0, 0, 0, 0));
}

TEST_F(hxflat_map_test_f, insert_unique_sorted_order) {
	const hxtest_object v3(3), v1(1), v2(2);
	const hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 3> m{
		{30, v3}, {10, v1}, {20, v2}};
	EXPECT_EQ(m.size(), 3);
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 3>::const_iterator it = m.begin();
	EXPECT_EQ(it.key(), 10); ++it;
	EXPECT_EQ(it.key(), 20); ++it;
	EXPECT_EQ(it.key(), 30);
	EXPECT_TRUE(check_stats(9, 3, 0, 3, 4, 2, 2, 0, 0, 0));
}

TEST_F(hxflat_map_test_f, insert_unique_move) {
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 3> m;
	hxtest_object v(34);
	const hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 3>::iterator it =
		m.insert(5, hxmove(v));
	EXPECT_EQ(it.key(), 5);
	EXPECT_EQ(it.value().value(), 34);
	EXPECT_EQ(v.state(), hxtest_object_state::moved);
	const hxtest_object v1(10);
	m.insert(1, v1);
	hxtest_object v2(20);
	const hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 3>::iterator it2 =
		m.insert(1, hxmove(v2));
	EXPECT_EQ(it2.value().value(), 10);
	EXPECT_EQ(m.size(), 2);
	hxtest_object v3(15);
	const hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 3>::iterator it3 =
		m.insert(3, hxmove(v3));
	EXPECT_EQ(it3.key(), 3);
	EXPECT_EQ(it3.value().value(), 15);
	EXPECT_EQ(m.size(), 3);
	EXPECT_TRUE(check_stats(7, 0, 0, 4, 0, 3, 1, 1, 0, 0));
}

TEST_F(hxflat_map_test_f, insert_multi) {
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, true, 2> m;
	const hxtest_object v1(10);
	hxtest_object v2(20);
	m.insert(1, v1);
	m.insert(1, hxmove(v2));
	EXPECT_EQ(m.size(), 2);
	EXPECT_EQ(m.count(1), 2);
	EXPECT_EQ(v2.state(), hxtest_object_state::moved);
	EXPECT_TRUE(check_stats(4, 0, 0, 2, 1, 1, 0, 1, 0, 0));
}

TEST_F(hxflat_map_test_f, insert_shifts_elements) {
	const hxtest_object va(1), vb(2), vc(3);
	const hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 3> m{
		{10, va}, {30, vb}, {20, vc}};
	EXPECT_EQ(m.size(), 3);
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 3>::const_iterator it = m.begin();
	EXPECT_EQ(it.key(), 10); ++it;
	EXPECT_EQ(it.key(), 20); ++it;
	EXPECT_EQ(it.key(), 30);
	EXPECT_TRUE(check_stats(9, 3, 0, 3, 5, 1, 1, 0, 0, 0));
}

TEST_F(hxflat_map_test_f, insert_at_front_shifts_single_element) {
	const hxtest_object va(1), vb(2);
	const hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 2> m{
		{20, va}, {10, vb}};
	EXPECT_EQ(m.size(), 2);
	EXPECT_EQ(m.begin().key(), 10);
	EXPECT_EQ(m.begin().value().value(), 2);
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 2>::const_iterator it = m.begin();
	++it;
	EXPECT_EQ(it.key(), 20);
	EXPECT_EQ(it.value().value(), 1);
	EXPECT_TRUE(check_stats(6, 2, 0, 2, 3, 1, 1, 0, 0, 0));
}

TEST_F(hxflat_map_test_f, insert_dynamic) {
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false> m;
	m.reserve(1);
	const hxtest_object v(7);
	m.insert(3, v);
	EXPECT_EQ(m.size(), 1);
	EXPECT_EQ(m.find(3).value().value(), 7);
	EXPECT_TRUE(check_stats(2, 0, 0, 1, 1, 0, 0, 0, 0, 0));
}

TEST_F(hxflat_map_test_f, clear) {
	const hxtest_object v1(1), v2(2);
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 2> m{
		{1, v1}, {2, v2}};
	EXPECT_EQ(m.size(), 2);
	m.clear();
	EXPECT_EQ(m.size(), 0);
	EXPECT_TRUE(m.empty());
	m.clear();
	EXPECT_EQ(m.size(), 0);
	EXPECT_TRUE(check_stats(6, 4, 0, 2, 4, 0, 0, 0, 0, 0));
}

TEST_F(hxflat_map_test_f, find) {
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 2> m;
	const hxtest_object v(99);
	m.insert(7, v);
	const hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 2>::iterator p = m.find(7);
	EXPECT_NE(p, m.end());
	EXPECT_EQ(p.value().value(), 99);
	p.value().value() = 55;
	EXPECT_EQ(m.find(7).value().value(), 55);
	EXPECT_EQ(m.find(99), m.end());
	const hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 2>& cm = m;
	EXPECT_EQ(cm.find(7).value().value(), 55);
	EXPECT_EQ(cm.find(99), cm.end());
	const hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 2> empty;
	EXPECT_EQ(empty.find(1), empty.end());
	EXPECT_TRUE(check_stats(2, 0, 0, 1, 1, 0, 0, 0, 0, 0));
}

TEST_F(hxflat_map_test_f, count) {
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, true, 3> m;
	const hxtest_object v1(1), v2(2), v3(3);
	m.insert(4, v1);
	EXPECT_EQ(m.count(4), 1);
	EXPECT_EQ(m.count(9), 0);
	m.insert(4, v2);
	m.insert(4, v3);
	EXPECT_EQ(m.count(4), 3);
	EXPECT_TRUE(check_stats(6, 0, 0, 3, 1, 2, 2, 1, 0, 0));
}

TEST_F(hxflat_map_test_f, lower_bound) {
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 3> m;
	const hxtest_object v1(1), v2(2), v3(3);
	m.insert(10, v1);
	m.insert(30, v2);
	EXPECT_EQ(m.lower_bound(10).key(), 10);
	EXPECT_EQ(m.lower_bound(20).key(), 30);
	EXPECT_TRUE(m.lower_bound(99) == m.end());
	m.insert(20, v3);
	EXPECT_EQ(m.lower_bound(20).key(), 20);
	const hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 3>::iterator it = m.lower_bound(20);
	it.value().value() = 99;
	EXPECT_EQ(m.find(20).value().value(), 99);
	EXPECT_TRUE(check_stats(6, 0, 0, 3, 2, 1, 1, 0, 0, 0));
}

TEST_F(hxflat_map_test_f, upper_bound) {
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, true, 3> m;
	const hxtest_object v1(1), v2(2), v3(3);
	m.insert(10, v1);
	m.insert(20, v2);
	const hxflat_map<int, hxtest_object, hxkey_less_t<int>, true, 3>& cm = m;
	EXPECT_EQ(cm.upper_bound(10).key(), 20);
	EXPECT_EQ(m.upper_bound(15).key(), 20);
	EXPECT_TRUE(m.upper_bound(20) == m.end());
	m.insert(10, v3);
	EXPECT_EQ(m.upper_bound(10).key(), 20);
	EXPECT_TRUE(m.upper_bound(9999) == m.end());
	EXPECT_TRUE(check_stats(6, 0, 0, 3, 2, 1, 1, 1, 0, 0));
}

TEST_F(hxflat_map_test_f, erase_key_unique) {
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 1> m;
	const hxtest_object v(1);
	m.insert(5, v);
	EXPECT_EQ(m.erase(9), 0);
	EXPECT_EQ(m.size(), 1);
	EXPECT_EQ(m.erase(99), 0);
	EXPECT_EQ(m.erase(1), 0);
	EXPECT_EQ(m.size(), 1);
	EXPECT_EQ(m.erase(5), 1);
	EXPECT_TRUE(m.empty());
	EXPECT_TRUE(check_stats(2, 1, 0, 1, 1, 0, 0, 0, 0, 0));
}

TEST_F(hxflat_map_test_f, erase_key_multi) {
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, true, 4> m;
	const hxtest_object v1(1), v2(2), v3(3), v4(4);
	m.insert(5, v1);
	EXPECT_EQ(m.erase(9), 0);
	m.insert(5, v2);
	m.insert(10, v3);
	EXPECT_EQ(m.erase(5), 2);
	EXPECT_EQ(m.size(), 1);
	EXPECT_EQ(m.count(10), 1);
	m.insert(10, v4);
	m.insert(10, v1);
	EXPECT_EQ(m.erase(10), 3);
	EXPECT_EQ(m.size(), 0);
	EXPECT_TRUE(check_stats(9, 5, 0, 4, 2, 3, 3, 2, 0, 0));
}

TEST_F(hxflat_map_test_f, erase_key_multi_tail_relative_to_count) {
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, true, 4> ma;
	const hxtest_object v1(1), v2(2), v3(3), v4(4);
	ma.insert(5, v1);
	ma.insert(5, v2);
	ma.insert(5, v3);
	ma.insert(10, v4);
	EXPECT_EQ(ma.erase(5), 3);
	EXPECT_EQ(ma.size(), 1);
	EXPECT_EQ(ma.find(10).value().value(), 4);
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, true, 4> mb;
	mb.insert(5, v1);
	mb.insert(10, v2);
	mb.insert(10, v3);
	mb.insert(10, v4);
	EXPECT_EQ(mb.erase(5), 1);
	EXPECT_EQ(mb.size(), 3);
	EXPECT_EQ(mb.count(10), 3);
	EXPECT_TRUE(check_stats(12, 4, 0, 4, 4, 4, 4, 6, 0, 0));
}

TEST_F(hxflat_map_test_f, erase_iterator_only_and_pair) {
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 2> m;
	const hxtest_object v1(1), v2(2);
	m.insert(7, v1);
	const hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 2>::iterator next1 =
		m.erase(m.begin());
	EXPECT_TRUE(next1 == m.end());
	EXPECT_TRUE(m.empty());
	m.insert(10, v1);
	m.insert(20, v2);
	const hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 2>::iterator next2 =
		m.erase(m.begin());
	EXPECT_EQ(next2.key(), 20);
	EXPECT_EQ(m.size(), 1);
	const hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 2>::iterator last = m.begin();
	const hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 2>::iterator next3 = m.erase(last);
	EXPECT_TRUE(next3 == m.end());
	EXPECT_EQ(m.size(), 0);
	EXPECT_TRUE(check_stats(5, 3, 0, 2, 3, 0, 0, 1, 0, 0));
}

TEST_F(hxflat_map_test_f, erase_iterator_first_and_middle_of_three) {
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 3> m;
	const hxtest_object v1(1), v2(2), v3(3);
	m.insert(10, v1);
	m.insert(20, v2);
	m.insert(30, v3);
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 3>::iterator mid = m.begin();
	++mid;
	const hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 3>::iterator next1 = m.erase(mid);
	EXPECT_EQ(next1.key(), 30);
	EXPECT_EQ(m.size(), 2);
	const hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 3>::iterator next2 =
		m.erase(m.begin());
	EXPECT_EQ(next2.key(), 30);
	EXPECT_EQ(m.size(), 1);
	EXPECT_EQ(m.find(10), m.end());
	EXPECT_EQ(m.find(30).value().value(), 3);
	EXPECT_TRUE(check_stats(6, 2, 0, 3, 3, 0, 0, 2, 0, 0));
}

TEST_F(hxflat_map_test_f, const_iterator_arithmetic) {
	const hxtest_object v1(1), v2(2), v3(3);
	const hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 3> m{
		{10, v1}, {20, v2}, {30, v3}};
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 3>::const_iterator it = m.begin();
	it += 2;
	EXPECT_EQ(it.key(), 30);
	it -= 1;
	EXPECT_EQ(it.key(), 20);
	const hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 3>::const_iterator it2 = it + 1;
	EXPECT_EQ(it2.key(), 30);
	const hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 3>::const_iterator it3 = it2 - 1;
	EXPECT_EQ(it3.key(), 20);
	EXPECT_EQ(m.begin()[1].key, 20);
	EXPECT_EQ(m.end() - m.begin(), ptrdiff_t{3});
	EXPECT_TRUE(check_stats(9, 3, 0, 3, 6, 0, 0, 0, 0, 0));
}

TEST_F(hxflat_map_test_f, const_iterator_pre_post_increment_decrement) {
	const hxtest_object v1(1), v2(2);
	const hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 2> m{
		{10, v1}, {20, v2}};
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
	EXPECT_TRUE(check_stats(6, 2, 0, 2, 4, 0, 0, 0, 0, 0));
}

TEST_F(hxflat_map_test_f, const_iterator_equality_and_order) {
	const hxtest_object v1(1), v2(2);
	const hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 2> m{
		{10, v1}, {20, v2}};
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
	EXPECT_TRUE(check_stats(6, 2, 0, 2, 4, 0, 0, 0, 0, 0));
}

TEST_F(hxflat_map_test_f, const_iterator_dereference_proxy) {
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 1> m;
	const hxtest_object v(7);
	m.insert(3, v);
	const hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 1>::const_iterator it = m.begin();
	EXPECT_EQ((*it).key, 3);
	EXPECT_EQ((*it).value.value(), 7);
	EXPECT_TRUE(check_stats(2, 0, 0, 1, 1, 0, 0, 0, 0, 0));
}

TEST_F(hxflat_map_test_f, iterator_dereference_and_mutation) {
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 1> m;
	const hxtest_object v(7);
	m.insert(3, v);
	const hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 1>::iterator it = m.begin();
	EXPECT_EQ((*it).key, 3);
	(*it).value.value() = 99;
	EXPECT_EQ(m.find(3).value().value(), 99);
	EXPECT_TRUE(check_stats(2, 0, 0, 1, 1, 0, 0, 0, 0, 0));
}

TEST_F(hxflat_map_test_f, iterator_arithmetic) {
	const hxtest_object v1(1), v2(2), v3(3);
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 3> m{
		{10, v1}, {20, v2}, {30, v3}};
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 3>::iterator it = m.begin();
	it += 2;
	EXPECT_EQ(it.key(), 30);
	it -= 1;
	EXPECT_EQ(it.key(), 20);
	const hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 3>::iterator it2 = it + 1;
	EXPECT_EQ(it2.key(), 30);
	const hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 3>::iterator it3 = it2 - 1;
	EXPECT_EQ(it3.key(), 20);
	EXPECT_EQ(it[1].key, 30);
	EXPECT_EQ(it2 - it3, hxsize_t{1});
	EXPECT_TRUE(check_stats(9, 3, 0, 3, 6, 0, 0, 0, 0, 0));
}

TEST_F(hxflat_map_test_f, iterator_pre_post_increment_decrement) {
	const hxtest_object v1(1), v2(2);
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 2> m{
		{10, v1}, {20, v2}};
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
	EXPECT_TRUE(check_stats(6, 2, 0, 2, 4, 0, 0, 0, 0, 0));
}

TEST_F(hxflat_map_test_f, begin_end) {
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 1> m;
	EXPECT_TRUE(m.begin() == m.end());
	EXPECT_TRUE(m.cbegin() == m.cend());
	const hxtest_object v(1);
	m.insert(5, v);
	EXPECT_FALSE(m.begin() == m.end());
	EXPECT_EQ(m.begin().key(), 5);
	EXPECT_TRUE(check_stats(2, 0, 0, 1, 1, 0, 0, 0, 0, 0));
}

TEST_F(hxflat_map_test_f, full) {
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 2> m;
	const hxtest_object v1(1), v2(2);
	EXPECT_FALSE(m.full());
	m.insert(1, v1);
	EXPECT_FALSE(m.full());
	m.insert(2, v2);
	EXPECT_TRUE(m.full());
	EXPECT_TRUE(check_stats(4, 0, 0, 2, 2, 0, 0, 0, 0, 0));
}

TEST_F(hxflat_map_test_f, dynamic_multimap_insert_erase) {
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
	EXPECT_TRUE(check_stats(10, 4, 0, 5, 3, 2, 2, 3, 0, 0));
}

TEST_F(hxflat_map_test_f, destructor_destroys_elements) {
	{
		hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 2> m;
		{
			const hxtest_object v1(10), v2(20);
			m.insert(1, v1);
			m.insert(2, v2);
		}
	}
	EXPECT_TRUE(check_stats(4, 4, 0, 2, 2, 0, 0, 0, 0, 0));
}

TEST_F(hxflat_map_test_f, default_constructed_iterator_assignable) {
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 1> m;
	const hxtest_object v(7);
	m.insert(3, v);
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 1>::const_iterator cit;
	cit = m.begin();
	EXPECT_EQ(cit.key(), 3);
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 1>::iterator it;
	it = m.begin();
	EXPECT_EQ(it.key(), 3);
	it.value().value() = 99;
	EXPECT_EQ(m.find(3).value().value(), 99);
	EXPECT_TRUE(check_stats(2, 0, 0, 1, 1, 0, 0, 0, 0, 0));
}

TEST_F(hxflat_map_test_f, operator_equal) {
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 3> a;
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 4> b;
	const hxtest_object v1(10), v2(20), v3(10), v4(20);
	EXPECT_TRUE(a == b);
	a.insert(1, v1);
	a.insert(2, v2);
	b.insert(1, v3);
	EXPECT_FALSE(a == b);
	b.insert(2, v4);
	EXPECT_TRUE(a == b);
	EXPECT_TRUE(check_stats(8, 0, 0, 4, 4, 0, 0, 0, 2, 0));
}

TEST_F(hxflat_map_test_f, operator_equal_mismatched_keys_or_values) {
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 2> a;
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 2> b;
	const hxtest_object v1(10), v2(99);
	a.insert(1, v1);
	b.insert(1, v2);
	EXPECT_FALSE(a == b);
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 2> c;
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 2> d;
	const hxtest_object v3(10), v4(10);
	c.insert(1, v3);
	d.insert(2, v4);
	EXPECT_FALSE(c == d);
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 2> e;
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 2> f;
	const hxtest_object v5(10), v6(10), v7(20), v8(99);
	e.insert(1, v5);
	e.insert(2, v7);
	f.insert(1, v6);
	f.insert(2, v8);
	EXPECT_FALSE(e == f);
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 2> g;
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 2> h;
	const hxtest_object v9(10), v10(10), v11(20), v12(20);
	g.insert(1, v9);
	g.insert(2, v11);
	h.insert(1, v10);
	h.insert(2, v12);
	EXPECT_TRUE(g == h);
	EXPECT_TRUE(check_stats(24, 0, 0, 12, 12, 0, 0, 0, 5, 0));
}

TEST_F(hxflat_map_test_f, operator_less) {
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 3> a;
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 3> b;
	EXPECT_FALSE(a < b);
	const hxtest_object v(1);
	b.insert(1, v);
	EXPECT_TRUE(a < b);
	EXPECT_FALSE(b < a);
	a.insert(1, v);
	EXPECT_FALSE(a < b);
	EXPECT_FALSE(b < a);
	b.insert(2, v);
	EXPECT_TRUE(a < b);
	EXPECT_FALSE(b < a);
	EXPECT_TRUE(check_stats(4, 0, 0, 1, 3, 0, 0, 0, 4, 0));
}

TEST_F(hxflat_map_test_f, operator_less_smaller_key_or_value_is_less) {
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 2> a;
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 2> b;
	const hxtest_object v1(10), v2(10);
	a.insert(1, v1);
	b.insert(2, v2);
	EXPECT_TRUE(a < b);
	EXPECT_FALSE(b < a);
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 2> c;
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 2> d;
	const hxtest_object v3(5), v4(10);
	c.insert(1, v3);
	d.insert(1, v4);
	EXPECT_TRUE(c < d);
	EXPECT_FALSE(d < c);
	EXPECT_TRUE(check_stats(8, 0, 0, 4, 4, 0, 0, 0, 2, 2));
}

TEST_F(hxflat_map_test_f, subscript) {
	const hxtest_object v1(10), v2(20), v3(30);
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 3> m{
		{1, v1}, {2, v2}, {3, v3}};
	const hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 3>& cm = m;
	EXPECT_EQ(cm[0].key(), 1);
	EXPECT_EQ(cm[0].value().value(), 10);
	EXPECT_EQ(cm[2].key(), 3);
	m[0].value().value() = 99;
	EXPECT_EQ(m.find(1).value().value(), 99);
	EXPECT_EQ(m[m.size() - 1].key(), 3);
	EXPECT_TRUE(check_stats(9, 3, 0, 3, 6, 0, 0, 0, 0, 0));
}

TEST_F(hxflat_map_test_f, copy_assign) {
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 3> a;
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 4> b;
	const hxtest_object v1(10), v2(20), vb(99);
	b.insert(7, vb);
	a.insert(1, v1);
	a.insert(2, v2);
	b = a;
	EXPECT_EQ(b.size(), 2);
	EXPECT_EQ(b.find(1).value().value(), 10);
	EXPECT_EQ(b.find(2).value().value(), 20);
	EXPECT_EQ(b.find(7), b.end());
	const hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 3> empty;
	b = empty;
	EXPECT_TRUE(b.empty());
	EXPECT_TRUE(check_stats(8, 3, 0, 3, 5, 0, 0, 0, 0, 0));
}

TEST_F(hxflat_map_test_f, move_assign_transfers_elements) {
	{
		const hxtest_object v1(10), v2(20), v3(30);
		hxflat_map<int, hxtest_object> a{{1, v1}, {2, v2}, {3, v3}};
		hxflat_map<int, hxtest_object> b;
		b.reserve(1);
		b = hxmove(a);
		EXPECT_EQ(b.size(), 3);
		EXPECT_EQ(b.find(1).value().value(), 10);
		EXPECT_EQ(b.find(2).value().value(), 20);
		EXPECT_EQ(b.find(3).value().value(), 30);
		EXPECT_EQ(a.size(), 0);
		EXPECT_EQ(a.capacity(), 1);
	}
	EXPECT_TRUE(check_stats(9, 9, 0, 3, 6, 0, 0, 0, 0, 0));
}

TEST_F(hxflat_map_test_f, move_constructor_transfers_elements) {
	{
		const hxtest_object v1(10), v2(20), v3(30);
		hxflat_map<int, hxtest_object> src{{1, v1}, {2, v2}, {3, v3}};
		hxflat_map<int, hxtest_object> dst(hxmove(src));
		EXPECT_EQ(dst.size(), 3);
		EXPECT_EQ(dst.find(1).value().value(), 10);
		EXPECT_EQ(dst.find(2).value().value(), 20);
		EXPECT_EQ(dst.find(3).value().value(), 30);
		EXPECT_EQ(src.size(), 0);
		EXPECT_EQ(src.capacity(), 0);
	}
	EXPECT_TRUE(check_stats(9, 9, 0, 3, 6, 0, 0, 0, 0, 0));
}

TEST_F(hxflat_map_test_f, copy_constructor) {
	const hxtest_object v1(10), v2(20), v3(30);
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 3> src{
		{1, v1}, {2, v2}, {3, v3}};
	const hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 3> dst(src);
	EXPECT_EQ(dst.size(), 3);
	EXPECT_EQ(dst.find(1).value().value(), 10);
	EXPECT_EQ(dst.find(2).value().value(), 20);
	EXPECT_EQ(dst.find(3).value().value(), 30);
	EXPECT_EQ(src.size(), 3);
	src.find(1).value().value() = 99;
	EXPECT_EQ(dst.find(1).value().value(), 10);
	EXPECT_TRUE(check_stats(12, 3, 0, 3, 9, 0, 0, 0, 0, 0));
}

TEST_F(hxflat_map_test_f, copy_constructor_empty) {
	const hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 4> src;
	const hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 4> dst(src); // NOLINT(performance-unnecessary-copy-initialization)
	EXPECT_TRUE(dst.empty());
	EXPECT_EQ(dst.size(), 0);
	EXPECT_TRUE(check_no_stats());
}

TEST_F(hxflat_map_test_f, copy_constructor_lifecycle) {
	{
		const hxtest_object v1(10), v2(20);
		const hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 3> src{
			{1, v1}, {2, v2}};
		{
			const hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 3> dst(src); // NOLINT(performance-unnecessary-copy-initialization)
			EXPECT_EQ(dst.size(), 2);
		}
	}
	EXPECT_TRUE(check_stats(8, 8, 0, 2, 6, 0, 0, 0, 0, 0));
}

TEST(hxflat_map_test, implements_rand_iterator_api) {
	hxflat_map<int, int, hxkey_less_t<int>, false, 4> m{
		{1, 10}, {2, 20}, {3, 30}};
	const hxflat_map<int, int, hxkey_less_t<int>, false, 4>& cm = m;
	EXPECT_TRUE(hxtest_check_rand_iterator_api(m.begin(), m.end()));
	EXPECT_TRUE(hxtest_check_rand_iterator_api(cm.begin(), cm.end()));
}

#if HX_CPLUSPLUS >= 202002L
TEST_F(hxflat_map_test_f, hxkey_equal) {
	typedef hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 2> map_t;
	map_t a;
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 4> b;
	const hxtest_object v1(10), v2(20), v3(10);
	a.insert(1, v1);
	b.insert(1, v3);
	EXPECT_TRUE(a == b);
	b.insert(2, v2);
	a.insert(2, v2);
	EXPECT_TRUE(a == b);
	map_t c;
	c.insert(1, v2);
	EXPECT_FALSE(a == c);
	EXPECT_TRUE(check_stats(8, 0, 0, 3, 5, 0, 0, 0, 3, 0));
}

TEST_F(hxflat_map_test_f, hxkey_less) {
	typedef hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 4> map_t;
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 2> a;
	map_t b;
	const hxtest_object v1(10), v2(10);
	a.insert(1, v1);
	b.insert(2, v2);
	EXPECT_TRUE(a < b);
	EXPECT_FALSE(b < a);
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 2> c;
	c.insert(2, v2);
	EXPECT_FALSE(b < c);
	EXPECT_FALSE(c < b);
	EXPECT_TRUE(check_stats(5, 0, 0, 2, 3, 0, 0, 0, 2, 0));
}

TEST_F(hxflat_map_test_f, hxswap_exchanges_contents) {
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
		EXPECT_EQ(a.find(2).value().value(), 20);
		EXPECT_EQ(b.size(), 1);
		EXPECT_EQ(b.find(1).value().value(), 10);
	}
	EXPECT_TRUE(check_stats(4, 4, 0, 2, 2, 0, 0, 0, 0, 0));
}

TEST_F(hxflat_map_test_f, hxswap_empty_and_nonempty) {
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
		EXPECT_EQ(b.find(1).value().value(), 10);
		EXPECT_EQ(b.find(2).value().value(), 20);
	}
	EXPECT_TRUE(check_stats(4, 4, 0, 2, 2, 0, 0, 0, 0, 0));
}
#endif // HX_CPLUSPLUS >= 202002L
