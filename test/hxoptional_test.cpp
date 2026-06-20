// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

#include <hx/hxoptional.hpp>
#include <hx/hxtest.hpp>

HX_NS_USE

namespace {

int hxs_test_ctor_count = 0;
int hxs_test_dtor_count = 0;
struct hxtest_optional_counted_t {
	explicit hxtest_optional_counted_t(int v) : value(v) { ++hxs_test_ctor_count; }
	hxtest_optional_counted_t(const hxtest_optional_counted_t& o)
		: value(o.value) { ++hxs_test_ctor_count; }
	hxtest_optional_counted_t(hxtest_optional_counted_t&& o) noexcept
		: value(o.value) { ++hxs_test_ctor_count; }
	~hxtest_optional_counted_t(void) { ++hxs_test_dtor_count; }
	hxtest_optional_counted_t& operator=(const hxtest_optional_counted_t& o) {
		value = o.value; return *this;
	}
	hxtest_optional_counted_t& operator=(hxtest_optional_counted_t&& o) noexcept {
		value = o.value; return *this;
	}
	bool operator==(const hxtest_optional_counted_t& o) const {
		return value == o.value;
	}
#if HX_CPLUSPLUS < 202002L
	bool operator!=(const hxtest_optional_counted_t& o) const {
		return value != o.value;
	}
#endif
	int value;
};
hxoptional<int> hxtest_make_int(bool engaged, int v = 0) {
	if (engaged) { return v; }
	return hxnullopt;
}
hxoptional<hxtest_optional_counted_t> hxtest_make_counted(bool engaged, int v = 0) {
	if (engaged) { return hxtest_optional_counted_t(v); }
	return hxnullopt;
}
} // namespace

TEST(hxoptional_test, default_construction_is_disengaged) {
	const hxoptional<int> o;
	EXPECT_FALSE((bool)o);
	EXPECT_FALSE(o.has_value());
	EXPECT_TRUE(o == hxnullopt);
	EXPECT_FALSE(o != hxnullopt);
}

TEST(hxoptional_test, value_construction_is_engaged) {
	const hxoptional<int> o = hxtest_make_int(true, 42);
	EXPECT_TRUE((bool)o);
	EXPECT_TRUE(o.has_value());
	EXPECT_EQ(*o, 42);
}

TEST(hxoptional_test, copy_construction_from_disengaged) {
	const hxoptional<int> a = hxtest_make_int(false);
	EXPECT_FALSE((bool)hxoptional<int>(a));
}

TEST(hxoptional_test, copy_construction_from_engaged) {
	const hxoptional<int> a = hxtest_make_int(true, 99);
	EXPECT_TRUE((bool)hxoptional<int>(a));
	EXPECT_EQ(*hxoptional<int>(a), 99);
}

TEST(hxoptional_test, move_construction_from_engaged) {
	hxoptional<int> a = hxtest_make_int(true, 5);
	const hxoptional<int> b(hxmove(a));
	EXPECT_TRUE((bool)b);
	EXPECT_EQ(*b, 5);
	EXPECT_FALSE((bool)a);
}

TEST(hxoptional_test, destructor_destroys_value) {
	hxs_test_ctor_count = 0;
	hxs_test_dtor_count = 0;
	{
		const hxoptional<hxtest_optional_counted_t> o = hxtest_make_counted(true, 1);
	}
	EXPECT_TRUE(hxs_test_ctor_count == 1 || hxs_test_ctor_count == 2);
	EXPECT_EQ(hxs_test_dtor_count, hxs_test_ctor_count);
}

TEST(hxoptional_test, destructor_disengaged_no_destroy) {
	hxs_test_dtor_count = 0;
	{
		const hxoptional<hxtest_optional_counted_t> o = hxtest_make_counted(false);
	}
	EXPECT_EQ(hxs_test_dtor_count, 0);
}

TEST(hxoptional_test, deref_operator_returns_reference) {
	hxoptional<int> o = hxtest_make_int(true, 10);
	*o = 20;
	EXPECT_EQ(*o, 20);
}

TEST(hxoptional_test, arrow_operator_accesses_methods) {
	hxoptional<hxtest_optional_counted_t> o = hxtest_make_counted(true, 4);
	EXPECT_EQ(o->value, 4);
	o->value = 8;
	EXPECT_EQ(o->value, 8);
}

TEST(hxoptional_test, copy_assign_engaged_to_disengaged) {
	const hxoptional<int> a = hxtest_make_int(true, 55);
	hxoptional<int> b = hxtest_make_int(false);
	b = a;
	EXPECT_TRUE((bool)b);
	EXPECT_EQ(*b, 55);
}

TEST(hxoptional_test, copy_assign_disengaged_to_engaged) {
	const hxoptional<int> a = hxtest_make_int(false);
	hxoptional<int> b = hxtest_make_int(true, 77);
	b = a;
	EXPECT_FALSE((bool)b);
}

TEST(hxoptional_test, copy_assign_engaged_to_engaged) {
	const hxoptional<int> a = hxtest_make_int(true, 11);
	hxoptional<int> b = hxtest_make_int(true, 22);
	b = a;
	EXPECT_TRUE((bool)b);
	EXPECT_EQ(*b, 11);
}

TEST(hxoptional_test, move_assign_engaged_to_disengaged) {
	hxoptional<int> a = hxtest_make_int(true, 9);
	hxoptional<int> b = hxtest_make_int(false);
	b = hxmove(a);
	EXPECT_TRUE((bool)b);
	EXPECT_EQ(*b, 9);
	EXPECT_FALSE((bool)a);
}

TEST(hxoptional_test, move_assign_disengaged_to_engaged) {
	hxoptional<int> a = hxtest_make_int(false);
	hxoptional<int> b = hxtest_make_int(true, 44);
	b = hxmove(a);
	EXPECT_FALSE((bool)b);
}

TEST(hxoptional_test, move_assign_engaged_to_engaged) {
	hxoptional<int> a = hxtest_make_int(true, 13);
	hxoptional<int> b = hxtest_make_int(true, 26);
	b = hxmove(a);
	EXPECT_TRUE((bool)b);
	EXPECT_EQ(*b, 13);
	EXPECT_FALSE((bool)a);
}

TEST(hxoptional_test, assign_nullopt_disengages) {
	hxoptional<int> o = hxtest_make_int(true, 5);
	o = hxnullopt;
	EXPECT_FALSE((bool)o);
}

TEST(hxoptional_test, value_copy_assign_to_disengaged) {
	hxoptional<int> o = hxtest_make_int(false);
	const int v = 17;
	o = v;
	EXPECT_TRUE((bool)o);
	EXPECT_EQ(*o, 17);
}

TEST(hxoptional_test, value_copy_assign_to_engaged) {
	hxoptional<int> o = hxtest_make_int(true, 1);
	const int v = 2;
	o = v;
	EXPECT_EQ(*o, 2);
}

TEST(hxoptional_test, value_move_assign_to_disengaged) {
	hxoptional<int> o = hxtest_make_int(false);
	int v = 19;
	o = hxmove(v);
	EXPECT_TRUE((bool)o);
	EXPECT_EQ(*o, 19);
}

TEST(hxoptional_test, value_move_assign_to_engaged) {
	hxoptional<int> o = hxtest_make_int(true, 3);
	int v = 4;
	o = hxmove(v);
	EXPECT_EQ(*o, 4);
}

TEST(hxoptional_test, eq_optional) {
	const hxoptional<int> empty = hxtest_make_int(false);
	const hxoptional<int> one = hxtest_make_int(true, 1);
	const hxoptional<int> one_b = hxtest_make_int(true, 1);
	const hxoptional<int> two = hxtest_make_int(true, 2);
	EXPECT_TRUE(empty == empty);
	EXPECT_FALSE(empty != empty);
	EXPECT_FALSE(empty == one);
	EXPECT_TRUE(empty != one);
	EXPECT_TRUE(one == one_b);
	EXPECT_FALSE(one != one_b);
	EXPECT_FALSE(one == two);
	EXPECT_TRUE(one != two);
}

TEST(hxoptional_test, eq_nullopt_and_value) {
	const hxoptional<int> empty = hxtest_make_int(false);
	const hxoptional<int> seven = hxtest_make_int(true, 7);
	EXPECT_TRUE(empty == hxnullopt);
	EXPECT_FALSE(empty != hxnullopt);
	EXPECT_FALSE(seven == hxnullopt);
	EXPECT_TRUE(seven != hxnullopt);
	EXPECT_TRUE(seven == 7);
	EXPECT_FALSE(seven != 7);
	EXPECT_FALSE(seven == 8);
	EXPECT_TRUE(seven != 8);
	EXPECT_FALSE(empty == 0);
	EXPECT_TRUE(empty != 0);
}

TEST(hxoptional_test, reset_engaged_disengages) {
	hxoptional<hxtest_optional_counted_t> o = hxtest_make_counted(true, 1);
	hxs_test_dtor_count = 0;
	o.reset();
	EXPECT_FALSE((bool)o);
	EXPECT_EQ(hxs_test_dtor_count, 1);
}

TEST(hxoptional_test, reset_disengaged_noop) {
	hxs_test_dtor_count = 0;
	hxoptional<hxtest_optional_counted_t> o = hxtest_make_counted(false);
	o.reset();
	EXPECT_FALSE((bool)o);
	EXPECT_EQ(hxs_test_dtor_count, 0);
}

TEST(hxoptional_test, swap) {
	hxoptional<int> a = hxtest_make_int(true, 1);
	hxoptional<int> b = hxtest_make_int(true, 2);
	a.swap(b);
	EXPECT_EQ(*a, 2);
	EXPECT_EQ(*b, 1);
	hxoptional<int> c = hxtest_make_int(false);
	a.swap(c);
	EXPECT_FALSE((bool)a);
	EXPECT_EQ(*c, 2);
}

TEST(hxoptional_test, value_returns_reference) {
	hxoptional<int> o = hxtest_make_int(true, 42);
	EXPECT_EQ(o.value(), 42);
	o.value() = 99;
	EXPECT_EQ(o.value(), 99);
	const hxoptional<int> co = hxtest_make_int(true, 10);
	EXPECT_EQ(co.value(), 10);
}

TEST(hxoptional_test, value_or) {
	const hxoptional<int> engaged = hxtest_make_int(true, 3);
	const hxoptional<int> empty = hxtest_make_int(false);
	EXPECT_EQ(engaged.value_or(99), 3);
	EXPECT_EQ(empty.value_or(7), 7);
}

TEST(hxoptional_test, converting_copy_construction_from_disengaged) {
	const hxoptional<int> a = hxtest_make_int(false);
	const hxoptional<long> b(a);
	EXPECT_FALSE((bool)b);
}

TEST(hxoptional_test, converting_copy_construction_from_engaged) {
	const hxoptional<int> a = hxtest_make_int(true, 42);
	const hxoptional<long> b(a);
	EXPECT_TRUE((bool)b);
	EXPECT_EQ(*b, 42);
}

TEST(hxoptional_test, converting_move_construction_from_disengaged) {
	hxoptional<int> a = hxtest_make_int(false);
	const hxoptional<long> b(hxmove(a));
	EXPECT_FALSE((bool)b);
}

TEST(hxoptional_test, converting_move_construction_from_engaged) {
	hxoptional<int> a = hxtest_make_int(true, 7);
	const hxoptional<long> b(hxmove(a));
	EXPECT_TRUE((bool)b);
	EXPECT_EQ(*b, 7);
	EXPECT_FALSE((bool)a);
}

TEST(hxoptional_test, emplace) {
	hxoptional<hxtest_optional_counted_t> o = hxtest_make_counted(false);
	o.emplace(42);
	EXPECT_TRUE((bool)o);
	EXPECT_EQ(o->value, 42);
	hxs_test_dtor_count = 0;
	o.emplace(2);
	EXPECT_EQ(hxs_test_dtor_count, 1);
	EXPECT_EQ(o->value, 2);
	hxoptional<int> n = hxtest_make_int(false);
	int& ref = n.emplace(55);
	EXPECT_EQ(ref, 55);
	ref = 66;
	EXPECT_EQ(*n, 66);
}

TEST(hxoptional_test, eq_boundary_equal_values) {
	const hxoptional<int> a = hxtest_make_int(true, 0);
	const hxoptional<int> b = hxtest_make_int(true, 0);
	const hxoptional<int> c = hxtest_make_int(true, 1);
	EXPECT_TRUE(a == b);
	EXPECT_FALSE(a != b);
	EXPECT_FALSE(a == c);
	EXPECT_TRUE(a != c);
}

TEST(hxoptional_test, eq_boundary_both_disengaged) {
	const hxoptional<int> a = hxtest_make_int(false);
	const hxoptional<int> b = hxtest_make_int(false);
	EXPECT_TRUE(a == b);
	EXPECT_FALSE(a != b);
}

TEST(hxoptional_test, eq_value_boundary_disengaged_not_zero) {
	const hxoptional<int> empty = hxtest_make_int(false);
	EXPECT_FALSE(empty == 0);
	EXPECT_TRUE(empty != 0);
	const hxoptional<int> zero = hxtest_make_int(true, 0);
	EXPECT_TRUE(zero == 0);
	EXPECT_FALSE(zero != 0);
}

TEST(hxoptional_test, reset_destroys_exactly_once) {
	hxoptional<hxtest_optional_counted_t> o = hxtest_make_counted(true, 7);
	hxs_test_dtor_count = 0;
	o.reset();
	EXPECT_EQ(hxs_test_dtor_count, 1);
	o.reset();
	EXPECT_EQ(hxs_test_dtor_count, 1);
}

TEST(hxoptional_test, swap_both_disengaged_noop) {
	hxs_test_dtor_count = 0;
	hxoptional<hxtest_optional_counted_t> a = hxtest_make_counted(false);
	hxoptional<hxtest_optional_counted_t> b = hxtest_make_counted(false);
	a.swap(b);
	EXPECT_FALSE((bool)a);
	EXPECT_FALSE((bool)b);
	EXPECT_EQ(hxs_test_dtor_count, 0);
}

TEST(hxoptional_test, swap_engaged_with_disengaged_boundary) {
	hxoptional<int> a = hxtest_make_int(true, 42);
	hxoptional<int> b = hxtest_make_int(false);
	b.swap(a);
	EXPECT_FALSE((bool)a);
	EXPECT_TRUE((bool)b);
	EXPECT_EQ(*b, 42);
}

TEST(hxoptional_test, value_or_engaged_zero) {
	const hxoptional<int> zero = hxtest_make_int(true, 0);
	EXPECT_EQ(zero.value_or(99), 0);
}

TEST(hxoptional_test, destructor_exactly_one_call) {
	hxs_test_ctor_count = 0;
	hxs_test_dtor_count = 0;
	{ hxoptional<hxtest_optional_counted_t> o(hxtest_optional_counted_t(1)); }
	EXPECT_EQ(hxs_test_dtor_count, hxs_test_ctor_count);
	EXPECT_GE(hxs_test_dtor_count, 1);
}
