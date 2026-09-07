// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

#include <hx/hxtest.hpp>
#include "./hxtest_util.hpp"

#if HX_CPLUSPLUS >= 202302L
#include <hx/hxref.hpp>
#include <hx/hxexpected.hpp>
#include <hx/hxptr.hpp>
#include <hx/hxarray.hpp>
#include <hx/hxvector.hpp>
#include <hx/hxflat_set.hpp>
#include <hx/hxflat_map.hpp>
#include <hx/hxhash_table_nodes.hpp>
#include <hx/hxhash_table.hpp>
#include <hx/hxhandle_table.hpp>
#include <hx/hxslot_map.hpp>

HX_NS_USE

using hxmonadic_test_f = hxtest_object_fixture;

TEST_F(hxmonadic_test_f, hxarray_and_then) {
	hxarray<hxtest_object, 3> a;
	a[0] = hxtest_object(31);
	EXPECT_TRUE((bool)a.and_then(hxsize_t{0}, hxmake_expected<hxtest_object>));
	EXPECT_EQ(a.and_then(hxsize_t{0}, hxmake_expected<hxtest_object>), hxtest_object(31));
	EXPECT_EQ(*a.and_then(hxsize_t{0}, hxmake_expected<hxtest_object>), hxtest_object(31));
	EXPECT_EQ(*a.and_then(hxsize_t{0}, hxmake_ref<hxtest_object>), hxtest_object(31));
	EXPECT_EQ(*a.and_then(hxsize_t{0}, hxmake_ptr<hxtest_object>), hxtest_object(31));
	EXPECT_FALSE((bool)a.and_then(hxsize_t{3}, hxmake_expected<hxtest_object>));
	EXPECT_EQ(a.and_then(hxsize_t{3}, hxmake_expected<hxtest_object>).value_or(hxtest_object(32)), hxtest_object(32));
	EXPECT_TRUE(check_stats(15, 12, 3, 7, 4, 1, 0, 1, 5, 0));
}

TEST_F(hxmonadic_test_f, hxarray_or_else) {
	hxarray<hxtest_object, 3> a;
	a[0] = hxtest_object(31);
	EXPECT_EQ(*a.or_else(hxsize_t{0}, [&]{ return a.end(); }), hxtest_object(31));
	EXPECT_EQ(*a.or_else(hxsize_t{3}, [&]{ return a.data(); }), hxtest_object(31));
	EXPECT_TRUE(check_stats(6, 3, 3, 3, 0, 0, 0, 1, 2, 0));
}

TEST_F(hxmonadic_test_f, hxarray_value_or) {
	hxarray<hxtest_object, 3> a;
	a[0] = hxtest_object(31);
	EXPECT_EQ(a.value_or(hxsize_t{0}, hxtest_object(32)), hxtest_object(31));
	EXPECT_EQ(a.value_or(hxsize_t{3}, hxtest_object(32)), hxtest_object(32));
	EXPECT_TRUE(check_stats(10, 7, 3, 5, 1, 1, 0, 1, 2, 0));
}

TEST_F(hxmonadic_test_f, hxvector_and_then) {
	hxvector<hxtest_object> v;
	v.reserve(1);
	v.emplace_back(31);
	EXPECT_TRUE((bool)v.and_then(hxsize_t{0}, hxmake_expected<hxtest_object>));
	EXPECT_EQ(v.and_then(hxsize_t{0}, hxmake_expected<hxtest_object>), hxtest_object(31));
	EXPECT_EQ(*v.and_then(hxsize_t{0}, hxmake_expected<hxtest_object>), 31);
	EXPECT_EQ(*v.and_then(hxsize_t{0}, hxmake_ref<hxtest_object>), 31);
	EXPECT_EQ(*v.and_then(hxsize_t{0}, hxmake_ptr<hxtest_object>), 31);
	EXPECT_FALSE((bool)v.and_then(hxsize_t{1}, hxmake_expected<hxtest_object>));
	EXPECT_EQ(v.and_then(hxsize_t{1}, hxmake_expected<hxtest_object>).value_or(hxtest_object(32)), 32);
	EXPECT_TRUE(check_stats(8, 7, 0, 3, 4, 1, 0, 0, 1, 0));
}

TEST_F(hxmonadic_test_f, hxvector_or_else) {
	hxvector<hxtest_object> v;
	v.reserve(1);
	v.emplace_back(31);
	EXPECT_EQ(*v.or_else(hxsize_t{0}, [&]{ return v.end(); }), 31);
	EXPECT_EQ(*v.or_else(hxsize_t{1}, [&]{ return v.data(); }), 31);
	EXPECT_TRUE(check_stats(1, 0, 0, 1, 0, 0, 0, 0, 0, 0));
}

TEST_F(hxmonadic_test_f, hxvector_value_or) {
	hxvector<hxtest_object> v;
	v.reserve(1);
	v.emplace_back(31);
	EXPECT_EQ(v.value_or(hxsize_t{0}, hxtest_object(32)), 31);
	EXPECT_EQ(v.value_or(hxsize_t{1}, hxtest_object(32)), 32);
	EXPECT_TRUE(check_stats(5, 4, 0, 3, 1, 1, 0, 0, 0, 0));
}

TEST_F(hxmonadic_test_f, hxflat_set_and_then) {
	hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 3> s;
	s.emplace(31);
	const hxtest_object missing(32);
	EXPECT_TRUE((bool)s.and_then(hxtest_object(31), hxmake_expected<hxtest_object>));
	EXPECT_EQ(s.and_then(hxtest_object(31), hxmake_expected<hxtest_object>), hxtest_object(31));
	EXPECT_EQ(*s.and_then(hxtest_object(31), hxmake_expected<hxtest_object>), 31);
	EXPECT_EQ(*s.and_then(hxtest_object(31), hxmake_ref<const hxtest_object>), 31);
	EXPECT_EQ(*s.and_then(hxtest_object(31), hxmake_ptr<const hxtest_object>), 31);
	EXPECT_FALSE((bool)s.and_then(hxtest_object(32), hxmake_expected<hxtest_object>));
	EXPECT_EQ(s.and_then(hxtest_object(32), hxmake_expected<hxtest_object>).value_or(missing), 32);
	EXPECT_TRUE(check_stats(16, 14, 0, 10, 5, 1, 0, 0, 1, 12));
}

TEST_F(hxmonadic_test_f, hxflat_set_or_else) {
	hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 3> s;
	s.emplace(31);
	EXPECT_EQ(*s.or_else(hxtest_object(31), [&]{ return s.end(); }), 31);
	EXPECT_EQ(*s.or_else(hxtest_object(32), [&]{ return s.keys(); }), 31);
	EXPECT_TRUE(check_stats(4, 3, 0, 3, 0, 1, 0, 0, 0, 3));
}

TEST_F(hxmonadic_test_f, hxflat_set_value_or) {
	hxflat_set<hxtest_object, hxkey_less_t<hxtest_object>, false, 3> s;
	s.emplace(31);
	const hxtest_object missing(32);
	EXPECT_EQ(s.value_or(hxtest_object(31), missing), 31);
	EXPECT_EQ(s.value_or(hxtest_object(32), missing), 32);
	EXPECT_TRUE(check_stats(7, 5, 0, 4, 2, 1, 0, 0, 0, 3));
}

TEST_F(hxmonadic_test_f, hxflat_map_and_then) {
	hxflat_map<hxtest_object, hxtest_object, hxkey_less_t<hxtest_object>, false, 3> m;
	m.emplace(hxtest_object(1), hxtest_object(31));
	EXPECT_TRUE((bool)m.and_then(hxtest_object(1), hxmake_expected<hxtest_object>));
	EXPECT_EQ(m.and_then(hxtest_object(1), hxmake_expected<hxtest_object>), hxtest_object(31));
	EXPECT_EQ(*m.and_then(hxtest_object(1), hxmake_expected<hxtest_object>), 31);
	EXPECT_EQ(*m.and_then(hxtest_object(1), hxmake_ref<hxtest_object>), 31);
	EXPECT_EQ(*m.and_then(hxtest_object(1), hxmake_ptr<hxtest_object>), 31);
	EXPECT_FALSE((bool)m.and_then(hxtest_object(2), hxmake_expected<hxtest_object>));
	EXPECT_EQ(m.and_then(hxtest_object(2), hxmake_expected<hxtest_object>).value_or(hxtest_object(32)), 32);
	EXPECT_TRUE(check_stats(19, 17, 0, 11, 5, 3, 0, 0, 1, 12));
}

TEST_F(hxmonadic_test_f, hxflat_map_or_else) {
	hxflat_map<hxtest_object, hxtest_object, hxkey_less_t<hxtest_object>, false, 3> m;
	m.emplace(hxtest_object(1), hxtest_object(31));
	EXPECT_EQ((*m.or_else(hxtest_object(1), [&]{ return m.end(); })).value, 31);
	EXPECT_EQ((*m.or_else(hxtest_object(2), [&]{ return m.begin(); })).value, 31);
	EXPECT_TRUE(check_stats(7, 5, 0, 4, 1, 2, 0, 0, 0, 3));
}

TEST_F(hxmonadic_test_f, hxflat_map_value_or) {
	hxflat_map<hxtest_object, hxtest_object, hxkey_less_t<hxtest_object>, false, 3> m;
	m.emplace(hxtest_object(1), hxtest_object(31));
	EXPECT_EQ(m.value_or(hxtest_object(1), hxtest_object(32)), 31);
	EXPECT_EQ(m.value_or(hxtest_object(2), hxtest_object(32)), 32);
	EXPECT_TRUE(check_stats(11, 9, 0, 6, 2, 3, 0, 0, 0, 3));
}

TEST_F(hxmonadic_test_f, hxhash_table_and_then) {
	using node_t = hxhash_table_map_node<hxtest_object, hxtest_object>;
	hxhash_table<node_t, hxdefault_delete, false, 4> table;
	table.try_emplace(hxtest_object(1), hxtest_object(1), hxtest_object(31));
	const node_t missing(hxtest_object(2), hxtest_object(32));
	EXPECT_TRUE((bool)table.and_then(hxtest_object(1), hxmake_ref<node_t>));
	EXPECT_EQ(table.and_then(hxtest_object(1), hxmake_ref<node_t>)->value(), 31);
	EXPECT_FALSE((bool)table.and_then(hxtest_object(2), hxmake_ref<node_t>));
	EXPECT_EQ(table.value_or(hxtest_object(2), &missing)->value(), 32);
	EXPECT_TRUE(check_stats(13, 9, 0, 9, 2, 2, 0, 0, 2, 0));
}

TEST_F(hxmonadic_test_f, hxhash_table_or_else) {
	using node_t = hxhash_table_map_node<hxtest_object, hxtest_object>;
	hxhash_table<node_t, hxdefault_delete, false, 4> table;
	table.try_emplace(hxtest_object(1), hxtest_object(1), hxtest_object(31));
	EXPECT_EQ(table.or_else(hxtest_object(1), [&]{ return table.end(); })->value(), 31);
	EXPECT_EQ(table.or_else(hxtest_object(2), [&]{ return table.begin(); })->value(), 31);
	EXPECT_TRUE(check_stats(7, 5, 0, 5, 1, 1, 0, 0, 1, 0));
}

TEST_F(hxmonadic_test_f, hxhash_table_value_or) {
	using node_t = hxhash_table_map_node<hxtest_object, hxtest_object>;
	hxhash_table<node_t, hxdefault_delete, false, 4> table;
	table.try_emplace(hxtest_object(1), hxtest_object(1), hxtest_object(31));
	const node_t missing(hxtest_object(2), hxtest_object(32));
	EXPECT_EQ(table.value_or(hxtest_object(1), &missing)->value(), 31);
	EXPECT_EQ(table.value_or(hxtest_object(2), &missing)->value(), 32);
	EXPECT_TRUE(check_stats(11, 7, 0, 7, 2, 2, 0, 0, 1, 0));
}

TEST_F(hxmonadic_test_f, hxhandle_table_and_then) {
	hxhandle_table<hxtest_object, hxdefault_delete, 2> t;
	const hxhandle_t handle = t.insert(hxnew<hxtest_object>(31));
	const hxhandle_t missing = handle + static_cast<hxhandle_t>(4);
	const hxtest_object expected(31);
	const hxtest_object fallback(32);
	EXPECT_TRUE((bool)t.and_then(handle, hxmake_expected<hxtest_object>));
	EXPECT_EQ(t.and_then(handle, hxmake_expected<hxtest_object>), expected);
	EXPECT_FALSE(t.and_then(handle, hxmake_expected<hxtest_object>) == fallback);
	EXPECT_EQ(*t.and_then(handle, hxmake_expected<hxtest_object>), expected);
	EXPECT_EQ(*t.and_then(handle, hxmake_ref<hxtest_object>), expected);
	EXPECT_EQ(*t.and_then(handle, hxmake_ptr<hxtest_object>), expected);
	EXPECT_FALSE((bool)t.and_then(missing, hxmake_expected<hxtest_object>));
	EXPECT_FALSE(t.and_then(missing, hxmake_expected<hxtest_object>) == fallback);
	EXPECT_EQ(t.value_or(missing, fallback), fallback);
	EXPECT_TRUE(check_stats(9, 6, 0, 3, 6, 0, 0, 0, 6, 0));
}

TEST_F(hxmonadic_test_f, hxhandle_table_or_else) {
	hxhandle_table<hxtest_object, hxdefault_delete, 2> t;
	const hxhandle_t handle = t.insert(hxnew<hxtest_object>(31));
	const hxhandle_t missing = handle + static_cast<hxhandle_t>(4);
	hxtest_object fallback(32);
	EXPECT_EQ(*t.or_else(handle, [&]{ return &fallback; }), hxtest_object(31));
	EXPECT_EQ(*t.or_else(missing, [&]{ return &fallback; }), hxtest_object(32));
	EXPECT_TRUE(check_stats(4, 2, 0, 4, 0, 0, 0, 0, 2, 0));
}

TEST_F(hxmonadic_test_f, hxhandle_table_value_or) {
	hxhandle_table<hxtest_object, hxdefault_delete, 2> t;
	const hxhandle_t handle = t.insert(hxnew<hxtest_object>(31));
	const hxhandle_t missing = handle + static_cast<hxhandle_t>(4);
	EXPECT_EQ(t.value_or(handle, hxtest_object(32)), hxtest_object(31));
	EXPECT_EQ(t.value_or(missing, hxtest_object(32)), hxtest_object(32));
	EXPECT_TRUE(check_stats(7, 6, 0, 5, 1, 1, 0, 0, 2, 0));
}

TEST_F(hxmonadic_test_f, hxslot_map_and_then) {
	hxslot_map<hxtest_object, 2> m;
	const hxhandle_t handle = m.insert(hxtest_object(31));
	EXPECT_TRUE((bool)m.and_then(handle, hxmake_expected<hxtest_object>));
	EXPECT_EQ(m.and_then(handle, hxmake_expected<hxtest_object>), hxtest_object(31));
	EXPECT_EQ(*m.and_then(handle, hxmake_expected<hxtest_object>), 31);
	EXPECT_EQ(*m.and_then(handle, hxmake_ref<hxtest_object>), 31);
	EXPECT_EQ(*m.and_then(handle, hxmake_ptr<hxtest_object>), 31);
	EXPECT_FALSE((bool)m.and_then(static_cast<hxhandle_t>(456), hxmake_expected<hxtest_object>));
	EXPECT_EQ(m.and_then(static_cast<hxhandle_t>(456), hxmake_expected<hxtest_object>).value_or(hxtest_object(32)), 32);
	EXPECT_TRUE(check_stats(9, 8, 0, 3, 4, 2, 0, 0, 1, 0));
}

TEST_F(hxmonadic_test_f, hxslot_map_or_else) {
	hxslot_map<hxtest_object, 2> m;
	const hxhandle_t handle = m.insert(hxtest_object(31));
	hxtest_object fallback(32);
	EXPECT_EQ(*m.or_else(handle, [&]{ return &fallback; }), 31);
	EXPECT_EQ(*m.or_else(static_cast<hxhandle_t>(456), [&]{ return &fallback; }), 32);
	EXPECT_TRUE(check_stats(3, 1, 0, 2, 0, 1, 0, 0, 0, 0));
}

TEST_F(hxmonadic_test_f, hxslot_map_value_or) {
	hxslot_map<hxtest_object, 2> m;
	const hxhandle_t handle = m.insert(hxtest_object(31));
	EXPECT_EQ(m.value_or(handle, hxtest_object(32)), 31);
	EXPECT_EQ(m.value_or(static_cast<hxhandle_t>(456), hxtest_object(32)), 32);
	EXPECT_TRUE(check_stats(6, 5, 0, 3, 1, 2, 0, 0, 0, 0));
}

TEST_F(hxmonadic_test_f, hxptr_and_then) {
	const hxptr<hxtest_object> p = hxmake_ptr<hxtest_object>(hxtest_object(31));
	EXPECT_TRUE((bool)p.and_then(hxmake_expected<hxtest_object>));
	EXPECT_EQ(p.and_then(hxmake_expected<hxtest_object>), hxtest_object(31));
	EXPECT_EQ(*p.and_then(hxmake_expected<hxtest_object>), 31);
	EXPECT_EQ(*p.and_then(hxmake_ref<hxtest_object>), 31);
	EXPECT_EQ(*p.and_then(hxmake_ptr<hxtest_object>), 31);
	EXPECT_TRUE(check_stats(7, 6, 0, 2, 5, 0, 0, 0, 1, 0));
}

TEST_F(hxmonadic_test_f, hxptr_or_else) {
	hxptr<hxtest_object> p = hxmake_ptr<hxtest_object>(hxtest_object(31));
	hxptr<hxtest_object> empty;
	EXPECT_EQ(*hxmove(p).or_else([]{ return hxmake_ptr<hxtest_object>(hxtest_object(32)); }), 31);
	EXPECT_EQ(*hxmove(empty).or_else([]{ return hxmake_ptr<hxtest_object>(hxtest_object(32)); }), 32);
	EXPECT_TRUE(check_stats(4, 4, 0, 2, 2, 0, 0, 0, 0, 0));
}

TEST_F(hxmonadic_test_f, hxptr_value_or) {
	const hxptr<hxtest_object> p = hxmake_ptr<hxtest_object>(hxtest_object(31));
	const hxptr<hxtest_object> empty;
	EXPECT_EQ(p.value_or(hxtest_object(32)), 31);
	EXPECT_EQ(empty.value_or(hxtest_object(32)), 32);
	EXPECT_TRUE(check_stats(6, 5, 0, 3, 2, 1, 0, 0, 0, 0));
}

#endif // HX_CPLUSPLUS >= 202302L
