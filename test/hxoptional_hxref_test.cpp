// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

#include <hx/hxoptional.hpp>
#include <hx/hxref.hpp>
#include <hx/hxtest.hpp>

HX_NS_USE

#if !defined _MSC_VER && !defined __wasm__
static_assert(sizeof(size_t) != 4 || (
		sizeof(hxoptional<char>) == 2u
		&& sizeof(hxref<int>) == 4u),
	"hxoptional<char> must pack its one byte value and one byte engaged flag"
	" with no padding, and hxref<T> must pack a single pointer with no"
	" padding regardless of T");
static_assert(sizeof(size_t) != 8 || (
		sizeof(hxoptional<char>) == 2u
		&& sizeof(hxref<int>) == 8u),
	"hxoptional<char> must pack its one byte value and one byte engaged flag"
	" with no padding, and hxref<T> must pack a single pointer with no"
	" padding regardless of T");
#endif

namespace {

int hxs_test_ctor_count = 0;
int hxs_test_dtor_count = 0;
struct hxtest_optional_counted_t {
	explicit hxtest_optional_counted_t(int v) : value(v) { ++hxs_test_ctor_count; }
	hxtest_optional_counted_t(const hxtest_optional_counted_t& o) noexcept = delete;
	hxtest_optional_counted_t(hxtest_optional_counted_t&& o) noexcept
		: value(o.value) { ++hxs_test_ctor_count; }
	~hxtest_optional_counted_t(void) { ++hxs_test_dtor_count; }
	hxtest_optional_counted_t& operator=(const hxtest_optional_counted_t& o) = delete;
	// GCOVR_EXCL_START
	hxtest_optional_counted_t& operator=(hxtest_optional_counted_t&& o) noexcept {
		hxassert_hard(false, "unused_function");
		value = o.value; return *this;
	}
	// GCOVR_EXCL_STOP
	bool operator==(const hxtest_optional_counted_t& o) = delete;
	bool operator!=(const hxtest_optional_counted_t& o) const = delete;
	int value;
};

} // namespace

TEST(hxoptional_test, construction_disengaged) {
	const hxoptional<int> a = hxnullopt;
	EXPECT_FALSE((bool)a);
	EXPECT_TRUE(a == hxnullopt);
	const hxoptional<int> b;
	EXPECT_FALSE((bool)b);
	EXPECT_FALSE(b.has_value());
	EXPECT_TRUE(b == hxnullopt);
	EXPECT_FALSE(b != hxnullopt);
}

TEST(hxoptional_test, construction_engaged) {
	const hxoptional<int> a = 34;
	EXPECT_TRUE((bool)a);
	EXPECT_TRUE(a.has_value());
	EXPECT_EQ(*a, 34);
}

TEST(hxoptional_test, copy_construction) {
	const hxoptional<int> disengaged = hxnullopt;
	EXPECT_FALSE((bool)hxoptional<int>(disengaged));
	const hxoptional<int> engaged = 99;
	EXPECT_TRUE((bool)hxoptional<int>(engaged));
	EXPECT_EQ(*hxoptional<int>(engaged), 99);
}

TEST(hxoptional_test, move_construction) {
	hxoptional<int> a = 5;
	const hxoptional<int> b(hxmove(a));
	EXPECT_TRUE((bool)b);
	EXPECT_EQ(*b, 5);
	EXPECT_FALSE((bool)a);
}

TEST(hxoptional_test, move_construction_disengaged) {
	hxoptional<int> a;
	const hxoptional<int> b(hxmove(a));
	EXPECT_FALSE((bool)b);
	EXPECT_FALSE((bool)a);
}

TEST(hxoptional_test, destructor_engaged_and_disengaged) {
	hxs_test_ctor_count = 0;
	hxs_test_dtor_count = 0;
	{
		const hxoptional<hxtest_optional_counted_t> o = hxtest_optional_counted_t(1);
	}
	EXPECT_TRUE(hxs_test_ctor_count == 1 || hxs_test_ctor_count == 2);
	EXPECT_EQ(hxs_test_dtor_count, hxs_test_ctor_count);
	EXPECT_GE(hxs_test_dtor_count, 1);
	hxs_test_dtor_count = 0;
	{
		const hxoptional<hxtest_optional_counted_t> o = hxnullopt;
	}
	EXPECT_EQ(hxs_test_dtor_count, 0);
}

TEST(hxoptional_test, deref_operator_returns_reference) {
	hxoptional<int> o = 10;
	*o = 20;
	EXPECT_EQ(*o, 20);
	const hxoptional<int> co = 30;
	EXPECT_EQ(*co, 30);
}

TEST(hxoptional_test, arrow_operator_accesses_fields) {
	hxoptional<hxtest_optional_counted_t> o = hxtest_optional_counted_t(4);
	EXPECT_EQ(o->value, 4);
	o->value = 8;
	EXPECT_EQ(o->value, 8);
	const hxoptional<hxtest_optional_counted_t> co = hxtest_optional_counted_t(4);
	EXPECT_EQ(co->value, 4);
	EXPECT_EQ(&co->value, &(*co).value);
}

TEST(hxoptional_test, copy_assign) {
	const hxoptional<int> engaged = 55;
	hxoptional<int> to_disengaged = hxnullopt;
	to_disengaged = engaged;
	EXPECT_TRUE((bool)to_disengaged);
	EXPECT_EQ(*to_disengaged, 55);
	const hxoptional<int> disengaged = hxnullopt;
	hxoptional<int> to_engaged = 77;
	to_engaged = disengaged;
	EXPECT_FALSE((bool)to_engaged);
	const hxoptional<int> eleven = 11;
	hxoptional<int> twentytwo = 22;
	twentytwo = eleven;
	EXPECT_TRUE((bool)twentytwo);
	EXPECT_EQ(*twentytwo, 11);
}

TEST(hxoptional_test, move_assign) {
	hxoptional<int> engaged = 9;
	hxoptional<int> to_disengaged = hxnullopt;
	to_disengaged = hxmove(engaged);
	EXPECT_TRUE((bool)to_disengaged);
	EXPECT_EQ(*to_disengaged, 9);
	EXPECT_FALSE((bool)engaged);
	hxoptional<int> disengaged = hxnullopt;
	hxoptional<int> to_engaged = 44;
	to_engaged = hxmove(disengaged);
	EXPECT_FALSE((bool)to_engaged);
	hxoptional<int> thirteen = 13;
	hxoptional<int> twentysix = 26;
	twentysix = hxmove(thirteen);
	EXPECT_TRUE((bool)twentysix);
	EXPECT_EQ(*twentysix, 13);
	EXPECT_FALSE((bool)thirteen);
}

TEST(hxoptional_test, assign_nullopt_disengages) {
	hxoptional<int> o = 5;
	o = hxnullopt;
	EXPECT_FALSE((bool)o);
}

TEST(hxoptional_test, value_assign) {
	hxoptional<int> to_disengaged = hxnullopt;
	const int v = 17;
	to_disengaged = v;
	EXPECT_TRUE((bool)to_disengaged);
	EXPECT_EQ(*to_disengaged, 17);
	hxoptional<int> to_engaged = 1;
	const int w = 2;
	to_engaged = w;
	EXPECT_EQ(*to_engaged, 2);
	hxoptional<int> moved_to_disengaged = hxnullopt;
	int mv = 19;
	moved_to_disengaged = hxmove(mv);
	EXPECT_TRUE((bool)moved_to_disengaged);
	EXPECT_EQ(*moved_to_disengaged, 19);
	hxoptional<int> moved_to_engaged = 3;
	int mw = 4;
	moved_to_engaged = hxmove(mw);
	EXPECT_EQ(*moved_to_engaged, 4);
}

TEST(hxoptional_test, eq_optional) {
	const hxoptional<int> empty = hxnullopt;
	const hxoptional<int> one = 1;
	const hxoptional<int> one_b = 1;
	const hxoptional<int> two = 2;
	EXPECT_TRUE(empty == empty);
	EXPECT_FALSE(empty != empty);
	EXPECT_FALSE(empty == one);
	EXPECT_TRUE(empty != one);
	EXPECT_TRUE(one == one_b);
	EXPECT_FALSE(one != one_b);
	EXPECT_FALSE(one == two);
	EXPECT_TRUE(one != two);
	const hxoptional<int> zero = 0;
	const hxoptional<int> zero_b = 0;
	EXPECT_TRUE(zero == zero_b);
	EXPECT_FALSE(zero != zero_b);
	EXPECT_FALSE(zero == one);
	EXPECT_TRUE(zero != one);
}

TEST(hxoptional_test, eq_nullopt_and_value) {
	const hxoptional<int> empty = hxnullopt;
	const hxoptional<int> seven = 7;
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
	const hxoptional<int> zero = 0;
	EXPECT_TRUE(zero == 0);
	EXPECT_FALSE(zero != 0);
}

TEST(hxoptional_test, reset_engaged_and_disengaged) {
	hxoptional<hxtest_optional_counted_t> engaged = hxtest_optional_counted_t(1);
	hxs_test_dtor_count = 0;
	engaged.reset();
	EXPECT_FALSE((bool)engaged);
	EXPECT_EQ(hxs_test_dtor_count, 1);
	engaged.reset();
	EXPECT_EQ(hxs_test_dtor_count, 1);
	hxs_test_dtor_count = 0;
	hxoptional<hxtest_optional_counted_t> disengaged = hxnullopt;
	disengaged.reset();
	EXPECT_FALSE((bool)disengaged);
	EXPECT_EQ(hxs_test_dtor_count, 0);
}

TEST(hxoptional_test, swap_both_engaged) {
	hxoptional<int> a = 1;
	hxoptional<int> b = 2;
	a.swap(b);
	EXPECT_EQ(*a, 2);
	EXPECT_EQ(*b, 1);
}

TEST(hxoptional_test, swap_with_disengaged) {
	hxoptional<int> a = 1;
	hxoptional<int> c = hxnullopt;
	a.swap(c);
	EXPECT_FALSE((bool)a);
	EXPECT_EQ(*c, 1);
	c.swap(a);
	EXPECT_FALSE((bool)c);
	EXPECT_TRUE((bool)a);
	EXPECT_EQ(*a, 1);
	hxoptional<int> d = hxnullopt;
	hxoptional<int> e = 88;
	d.swap(e);
	EXPECT_TRUE((bool)d);
	EXPECT_EQ(*d, 88);
	EXPECT_FALSE((bool)e);
}

TEST(hxoptional_test, swap_both_disengaged) {
	hxs_test_dtor_count = 0;
	hxoptional<hxtest_optional_counted_t> f = hxnullopt;
	hxoptional<hxtest_optional_counted_t> g = hxnullopt;
	f.swap(g);
	EXPECT_FALSE((bool)f);
	EXPECT_FALSE((bool)g);
	EXPECT_EQ(hxs_test_dtor_count, 0);
}

TEST(hxoptional_test, value_returns_reference) {
	hxoptional<int> o = 34;
	EXPECT_EQ(o.value(), 34);
	o.value() = 99;
	EXPECT_EQ(o.value(), 99);
	const hxoptional<int> co = 10;
	EXPECT_EQ(co.value(), 10);
}

TEST(hxoptional_test, value_or) {
	const hxoptional<int> engaged = 3;
	const hxoptional<int> empty = hxnullopt;
	EXPECT_EQ(engaged.value_or(99), 3);
	EXPECT_EQ(empty.value_or(7), 7);
	const hxoptional<int> zero = 0;
	EXPECT_EQ(zero.value_or(99), 0);
}

TEST(hxoptional_test, converting_copy_construction) {
	const hxoptional<int> disengaged = hxnullopt;
	EXPECT_FALSE((bool)hxoptional<long>(disengaged));
	const hxoptional<int> engaged = 34;
	const hxoptional<long> converted(engaged);
	EXPECT_TRUE((bool)converted);
	EXPECT_EQ(*converted, 34);
}

TEST(hxoptional_test, converting_move_construction) {
	hxoptional<int> disengaged = hxnullopt;
	EXPECT_FALSE((bool)hxoptional<long>(hxmove(disengaged)));
	hxoptional<int> engaged = 7;
	const hxoptional<long> converted(hxmove(engaged));
	EXPECT_TRUE((bool)converted);
	EXPECT_EQ(*converted, 7);
	EXPECT_FALSE((bool)engaged);
}

TEST(hxoptional_test, converting_copy_assign) {
	const hxoptional<int> engaged = 55;
	hxoptional<long> to_disengaged = hxnullopt;
	to_disengaged = engaged;
	EXPECT_TRUE((bool)to_disengaged);
	EXPECT_EQ(*to_disengaged, 55);
	const hxoptional<int> disengaged = hxnullopt;
	hxoptional<long> to_engaged = 77;
	to_engaged = disengaged;
	EXPECT_FALSE((bool)to_engaged);
	const hxoptional<int> eleven = 11;
	hxoptional<long> twentytwo = 22;
	twentytwo = eleven;
	EXPECT_TRUE((bool)twentytwo);
	EXPECT_EQ(*twentytwo, 11);
	EXPECT_TRUE((bool)eleven);
	EXPECT_EQ(*eleven, 11);
}

TEST(hxoptional_test, converting_move_assign) {
	hxoptional<int> engaged = 9;
	hxoptional<long> to_disengaged = hxnullopt;
	to_disengaged = hxmove(engaged);
	EXPECT_TRUE((bool)to_disengaged);
	EXPECT_EQ(*to_disengaged, 9);
	EXPECT_FALSE((bool)engaged);
	hxoptional<int> disengaged = hxnullopt;
	hxoptional<long> to_engaged = 44;
	to_engaged = hxmove(disengaged);
	EXPECT_FALSE((bool)to_engaged);
	EXPECT_FALSE((bool)disengaged);
	hxoptional<int> thirteen = 13;
	hxoptional<long> twentysix = 26;
	twentysix = hxmove(thirteen);
	EXPECT_TRUE((bool)twentysix);
	EXPECT_EQ(*twentysix, 13);
	EXPECT_FALSE((bool)thirteen);
}

TEST(hxoptional_test, emplace) {
	hxoptional<hxtest_optional_counted_t> o = hxnullopt;
	o.emplace(34);
	EXPECT_TRUE((bool)o);
	EXPECT_EQ(o->value, 34);
	hxs_test_dtor_count = 0;
	o.emplace(2);
	EXPECT_EQ(hxs_test_dtor_count, 1);
	EXPECT_EQ(o->value, 2);
	hxoptional<int> n = hxnullopt;
	int& ref = n.emplace(55);
	EXPECT_EQ(ref, 55);
	ref = 66;
	EXPECT_EQ(*n, 66);
}

TEST(hxoptional_test, destructor_exactly_one_call) {
	hxs_test_ctor_count = 0;
	hxs_test_dtor_count = 0;
	{ const hxoptional<hxtest_optional_counted_t> o(hxtest_optional_counted_t(1)); }
	EXPECT_EQ(hxs_test_dtor_count, hxs_test_ctor_count);
	EXPECT_GE(hxs_test_dtor_count, 1);
}

TEST(hxoptional_test, and_then_engaged) {
	hxoptional<int> engaged = 3;
	const hxoptional<long> r = engaged.and_then([](int& v) { return hxoptional<long>(v + 1); });
	EXPECT_TRUE((bool)r);
	EXPECT_EQ(*r, 4);
	EXPECT_FALSE((bool)engaged.and_then([](int&) { return hxoptional<int>(hxnullopt); }));
	const hxoptional<int> const_engaged = 5;
	const hxoptional<int> const_r =
		const_engaged.and_then([](const int& v) { return hxoptional<int>(v * 2); });
	EXPECT_TRUE((bool)const_r);
	EXPECT_EQ(*const_r, 10);
}

TEST(hxoptional_test, and_then_disengaged) {
	hxoptional<int> disengaged = hxnullopt;
	bool called = false;
	const hxoptional<long> empty = disengaged.and_then([&called](int& v) {
		// GCOVR_EXCL_START
		called = true; return hxoptional<long>(v);
		// GCOVR_EXCL_STOP
	});
	EXPECT_FALSE((bool)empty);
	EXPECT_FALSE(called);
	const hxoptional<int> const_disengaged = hxnullopt;
	EXPECT_FALSE((bool)const_disengaged.and_then([](const int& v) { return hxoptional<int>(v); }));
}

TEST(hxoptional_test, or_else_engaged_and_disengaged) {
	const hxoptional<int> engaged = 8;
	bool called = false;
	const hxoptional<int> self = engaged.or_else([&called]() {
		// GCOVR_EXCL_START
		called = true; return hxoptional<int>(0);
		// GCOVR_EXCL_STOP
	});
	EXPECT_TRUE((bool)self);
	EXPECT_EQ(*self, 8);
	EXPECT_FALSE(called);
	const hxoptional<int> disengaged = hxnullopt;
	const hxoptional<int> r = disengaged.or_else([&called]() {
		called = true; return hxoptional<int>(42);
	});
	EXPECT_TRUE(called);
	EXPECT_TRUE((bool)r);
	EXPECT_EQ(*r, 42);
	EXPECT_FALSE((bool)disengaged.or_else([]() { return hxoptional<int>(hxnullopt); }));
}

TEST(hxref_test, construction_disengaged) {
	const hxref<int> disengaged;
	EXPECT_FALSE((bool)disengaged);
	EXPECT_FALSE(disengaged.has_value());
	EXPECT_TRUE(disengaged == hxnullopt);
	EXPECT_FALSE(disengaged != hxnullopt);
	const hxref<int> from_null = hxnullopt;
	EXPECT_FALSE((bool)from_null);
}

TEST(hxref_test, construction_engaged) {
	int v = 34;
	const hxref<int> engaged = v;
	EXPECT_TRUE((bool)engaged);
	EXPECT_TRUE(engaged.has_value());
	EXPECT_FALSE(engaged == hxnullopt);
	EXPECT_TRUE(engaged != hxnullopt);
	EXPECT_EQ(&*engaged, &v);
	EXPECT_EQ(&engaged.value(), &v);
	*engaged = 99;
	EXPECT_EQ(v, 99);
	hxtest_optional_counted_t s(7);
	const hxref<hxtest_optional_counted_t> engaged_struct = s;
	EXPECT_EQ(engaged_struct->value, 7);
	EXPECT_EQ(&engaged_struct->value, &s.value);
}

TEST(hxref_test, shallow_const_writes_through) {
	int v = 5;
	const hxref<int> engaged = v;
	*engaged = 7;
	EXPECT_EQ(v, 7);
	engaged.value() = 8;
	EXPECT_EQ(v, 8);
	hxtest_optional_counted_t s(3);
	const hxref<hxtest_optional_counted_t> engaged_struct = s;
	engaged_struct->value = 9;
	EXPECT_EQ(s.value, 9);
}

TEST(hxref_test, copy_construction_copies_binding) {
	int v = 3;
	int w = 4;
	const hxref<int> engaged = v;
	hxref<int> copy_engaged(engaged);
	EXPECT_TRUE((bool)copy_engaged);
	EXPECT_EQ(&*copy_engaged, &v);
	copy_engaged = w;
	EXPECT_EQ(&*engaged, &v);
	const hxref<int> copy_disengaged{hxref<int>()};
	EXPECT_FALSE((bool)copy_disengaged);
}

TEST(hxref_test, converting_construction_adds_const) {
	int v = 21;
	const hxref<int> engaged = v;
	const hxref<const int> converted(engaged);
	EXPECT_TRUE((bool)converted);
	EXPECT_EQ(&*converted, &v);
	EXPECT_EQ(*converted, 21);
	const hxref<int> disengaged;
	const hxref<const int> converted_empty(disengaged);
	EXPECT_FALSE((bool)converted_empty);
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
	o = hxnullopt;
	EXPECT_FALSE((bool)o);
	EXPECT_EQ(v, 4);
}

TEST(hxref_test, emplace_and_reset_rebind) {
	int a = 1;
	int b = 2;
	hxref<int> o;
	int& r = o.emplace(a);
	EXPECT_EQ(&r, &a);
	EXPECT_EQ(&*o, &a);
	r = 7;
	EXPECT_EQ(a, 7);
	o.emplace(b);
	EXPECT_EQ(&*o, &b);
	o.reset();
	EXPECT_FALSE((bool)o);
	EXPECT_EQ(a, 7);
	EXPECT_EQ(b, 2);
	o.reset();
	EXPECT_FALSE((bool)o);
}

TEST(hxref_test, eq_optional_and_value) {
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

TEST(hxref_test, value_or_returns_value) {
	int v = 3;
	const hxref<int> engaged = v;
	const hxref<int> empty;
	EXPECT_EQ(engaged.value_or(99), 3);
	EXPECT_EQ(empty.value_or(7), 7);
	int zero = 0;
	const hxref<int> o_zero = zero;
	EXPECT_EQ(o_zero.value_or(99), 0);
}

TEST(hxref_test, and_then_engaged_and_disengaged) {
	int v = 3;
	const hxref<int> engaged = v;
	const hxoptional<long> r = engaged.and_then([](int& x) { return hxoptional<long>(x + 1); });
	EXPECT_TRUE((bool)r);
	EXPECT_EQ(*r, 4);
	EXPECT_FALSE((bool)engaged.and_then([](int&) { return hxoptional<int>(hxnullopt); }));
	const hxref<int> disengaged;
	bool called = false;
	const hxoptional<long> empty = disengaged.and_then([&called](int& x) {
		// GCOVR_EXCL_START
		called = true; return hxoptional<long>(x);
		// GCOVR_EXCL_STOP
	});
	EXPECT_FALSE((bool)empty);
	EXPECT_FALSE(called);
}

TEST(hxref_test, or_else_engaged_and_disengaged) {
	int v = 8;
	int fallback = 42;
	const hxref<int> engaged = v;
	bool called = false;
	const hxref<int> self = engaged.or_else([&called, &fallback]() {
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
