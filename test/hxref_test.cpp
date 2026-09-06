// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

#include <hx/hxtest.hpp>
#include "./hxtest_util.hpp"

#if HX_CPLUSPLUS >= 202302L
#include <hx/hxref.hpp>
#include <hx/hxexpected.hpp>

HX_NS_USE

#if !defined _MSC_VER && !defined __wasm__
static_assert(sizeof(size_t) != 4 || (
		sizeof(hxref<int>) == 4u),
	"hxref<T> must pack a single pointer with no padding regardless of T");
static_assert(sizeof(size_t) != 8 || (
		sizeof(hxref<int>) == 8u),
	"hxref<T> must pack a single pointer with no padding regardless of T");
#endif

using hxref_test_f = hxtest_object_fixture;

hxattr_noinline static void hxtest_gdb_break_hxref(void) { }

TEST(hxref_test, construction_disengaged) {
	const hxref<int> disengaged;
	EXPECT_FALSE((bool)disengaged);
	EXPECT_FALSE(disengaged.has_value());
	EXPECT_TRUE(disengaged == hxnil);
	EXPECT_FALSE(disengaged != hxnil);
	const hxref<int> from_null = hxnil;
	EXPECT_FALSE((bool)from_null);
}

TEST_F(hxref_test_f, construction_engaged) {
	int v = 34;
	const hxref<int> non_null = v;
	EXPECT_TRUE((bool)non_null);
	EXPECT_TRUE(non_null.has_value());
	EXPECT_FALSE(non_null == hxnil);
	EXPECT_TRUE(non_null != hxnil);
	EXPECT_EQ(&*non_null, &v);
	EXPECT_EQ(&non_null.value(), &v);
	*non_null = 99;
	EXPECT_EQ(v, 99);
	hxtest_object s(7);
	const hxref<hxtest_object> a = s;
	EXPECT_EQ(a->value(), 7);
	EXPECT_EQ(&a->value(), &s.value());
	const hxref<hxtest_object> b;
	hxtest_gdb_break_hxref();
	EXPECT_TRUE(check_stats(1, 0, 0, 1, 0, 0, 0, 0, 0, 0));
}

TEST_F(hxref_test_f, shallow_const_writes_through) {
	int v = 5;
	const hxref<int> non_null = v;
	*non_null = 7;
	EXPECT_EQ(v, 7);
	non_null.value() = 8;
	EXPECT_EQ(v, 8);
	hxtest_object s(3);
	const hxref<hxtest_object> engaged_struct = s;
	engaged_struct->value() = 9;
	EXPECT_EQ(s.value(), 9);
	EXPECT_TRUE(check_stats(1, 0, 0, 1, 0, 0, 0, 0, 0, 0));
}

TEST(hxref_test, copy_construction_copies_binding) {
	int v = 3;
	int w = 4;
	const hxref<int> non_null = v;
	hxref<int> copy_engaged(non_null);
	EXPECT_TRUE((bool)copy_engaged);
	EXPECT_EQ(&*copy_engaged, &v);
	copy_engaged = w;
	EXPECT_EQ(&*non_null, &v);
	const hxref<int> copy_disengaged{hxref<int>()};
	EXPECT_FALSE((bool)copy_disengaged);
}

TEST(hxref_test, copy_assign_rebinds) {
	int a = 1;
	int b = 2;
	hxref<int> o = a;
	const hxref<int> to_b = b;
	o = to_b;
	EXPECT_EQ(&*o, &b);
	EXPECT_EQ(a, 1);
	const hxref<int> disengaged;
	o = disengaged;
	EXPECT_FALSE((bool)o);
}

TEST(hxref_test, value_assign_rebinds_without_assigning_through) {
	int a = 1;
	int b = 2;
	hxref<int> o = a;
	o = b;
	EXPECT_EQ(&*o, &b);
	EXPECT_EQ(a, 1);
	EXPECT_EQ(*o, 2);
	*o = 5;
	EXPECT_EQ(b, 5);
	EXPECT_EQ(a, 1);
}

TEST(hxref_test, assign_nullopt_disengages) {
	int v = 4;
	hxref<int> o = v;
	o = hxnil;
	EXPECT_FALSE((bool)o);
	EXPECT_EQ(v, 4);
}

TEST(hxref_test, assign_and_reset_rebind) {
	int a = 1;
	int b = 2;
	hxref<int> o;
	o = a;
	int& r = *o;
	EXPECT_EQ(&r, &a);
	EXPECT_EQ(&*o, &a);
	r = 7;
	EXPECT_EQ(a, 7);
	o = b;
	EXPECT_EQ(&*o, &b);
	o.reset();
	EXPECT_FALSE((bool)o);
	EXPECT_EQ(a, 7);
	EXPECT_EQ(b, 2);
	o.reset();
	EXPECT_FALSE((bool)o);
}

TEST(hxref_test, eq_expected_and_value) {
	int one_a = 1;
	int one_b = 1;
	int two = 2;
	const hxref<int> empty;
	const hxref<int> o_one_a = one_a;
	const hxref<int> o_one_b = one_b;
	const hxref<int> o_two = two;
	EXPECT_TRUE(empty == empty);
	EXPECT_FALSE(empty != empty);
	EXPECT_FALSE(empty == o_one_a);
	EXPECT_TRUE(empty != o_one_a);
	EXPECT_FALSE(o_one_a == empty);
	EXPECT_TRUE(o_one_a != empty);
	EXPECT_TRUE(o_one_a == o_one_b);
	EXPECT_FALSE(o_one_a != o_one_b);
	EXPECT_FALSE(o_one_a == o_two);
	EXPECT_TRUE(o_one_a != o_two);
	EXPECT_TRUE(o_one_a == 1);
	EXPECT_FALSE(o_one_a != 1);
	EXPECT_FALSE(o_one_a == 2);
	EXPECT_TRUE(o_one_a != 2);
	EXPECT_FALSE(empty == 1);
	EXPECT_TRUE(empty != 1);
}

TEST(hxref_test, hxkey_equal) {
	int one_a = 1;
	int one_b = 1;
	int two = 2;
	const hxref<int> empty;
	const hxref<int> empty_b;
	const hxref<int> o_one_a = one_a;
	const hxref<int> o_one_b = one_b;
	const hxref<int> o_two = two;
	EXPECT_TRUE(hxkey_equal(empty, empty_b));
	EXPECT_FALSE(hxkey_equal(empty, o_one_a));
	EXPECT_TRUE(hxkey_equal(o_one_a, o_one_b));
	EXPECT_FALSE(hxkey_equal(o_one_a, o_two));
}

TEST_F(hxref_test_f, hxkey_hash) {
	hxtest_object a(31);
	hxtest_object b(31);
	hxtest_object c(32);
	const hxref<hxtest_object> empty;
	const hxref<hxtest_object> empty_b;
	const hxref<hxtest_object> o_a = a;
	const hxref<hxtest_object> o_b = b;
	const hxref<hxtest_object> o_c = c;
	EXPECT_EQ(hxkey_hash(empty), hxkey_hash(empty_b));
	EXPECT_EQ(hxkey_hash(empty), hxhash_t{31u});
	EXPECT_EQ(hxkey_hash(o_a), hxkey_hash(o_b));
	EXPECT_NE(hxkey_hash(o_a), hxkey_hash(o_c));
	EXPECT_TRUE(check_stats(3, 0, 0, 3, 0, 0, 0, 0, 0, 0));
}

TEST(hxref_test, swap_rebinds) {
	int a = 1;
	int b = 2;
	hxref<int> o_a = a;
	hxref<int> o_b = b;
	o_a.swap(o_b);
	EXPECT_EQ(&*o_a, &b);
	EXPECT_EQ(&*o_b, &a);
	hxref<int> empty;
	o_a.swap(empty);
	EXPECT_FALSE((bool)o_a);
	EXPECT_EQ(&*empty, &b);
	empty.swap(o_a);
	EXPECT_FALSE((bool)empty);
	EXPECT_EQ(&*o_a, &b);
}

TEST_F(hxref_test_f, value_or_returns_value) {
	int v = 3;
	const hxref<int> non_null = v;
	const hxref<int> empty;
	EXPECT_EQ(non_null.value_or(99), 3);
	EXPECT_EQ(empty.value_or(7), 7);
	int zero = 0;
	const hxref<int> o_zero = zero;
	EXPECT_EQ(o_zero.value_or(31), 0);
	const hxref<hxtest_object> emplace_empty;
	EXPECT_EQ(emplace_empty.value_or(4, 5).value(), 9);
	EXPECT_TRUE(check_stats(1, 1, 0, 1, 0, 0, 0, 0, 0, 0));
}

TEST(hxref_test, and_then_engaged_and_disengaged) {
	int v = 3;
	const hxref<int> non_null = v;
	const hxexpected<long> r = non_null.and_then([](int& x) { return hxexpected<long>(false, x + 1); });
	EXPECT_TRUE((bool)r);
	EXPECT_EQ(*r, 4);
	EXPECT_FALSE((bool)non_null.and_then([](int&) { return hxexpected<int>(hxnil); }));
	const hxref<int> disengaged;
	bool called = false;
	const hxexpected<long> empty = disengaged.and_then([&called](int& x) {
		// GCOVR_EXCL_START
		called = true; return hxexpected<long>(false, x);
		// GCOVR_EXCL_STOP
	});
	EXPECT_FALSE((bool)empty);
	EXPECT_FALSE(called);
	EXPECT_TRUE((bool)hxmove(non_null).and_then([](int& x) {
		return hxexpected<int>(false, x + 2);
	}));
	EXPECT_EQ(hxmove(non_null).and_then([](int& x) {
		return hxexpected<int>(false, x + 2);
	}), 5);
}

TEST(hxref_test, or_else_engaged_and_disengaged) {
	int v = 8;
	int fallback = 31;
	const hxref<int> non_null = v;
	bool called = false;
	const hxref<int> self = non_null.or_else([&called, &fallback]() {
		// GCOVR_EXCL_START
		called = true; return hxref<int>(fallback);
		// GCOVR_EXCL_STOP
	});
	EXPECT_TRUE((bool)self);
	EXPECT_EQ(&*self, &v);
	EXPECT_FALSE(called);
	const hxref<int> disengaged;
	const hxref<int> r = disengaged.or_else([&called, &fallback]() {
		called = true; return hxref<int>(fallback);
	});
	EXPECT_TRUE(called);
	EXPECT_TRUE((bool)r);
	EXPECT_EQ(&*r, &fallback);
}

#endif // HX_CPLUSPLUS >= 202302L
