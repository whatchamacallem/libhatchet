// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

#include <hx/hxtest.hpp>
#include "./hxtest_util.hpp"

#if HX_CPLUSPLUS >= 202302L
#include <hx/hxexpected.hpp>

HX_NS_USE

#if !defined _MSC_VER && !defined __wasm__
static_assert(sizeof(size_t) != 4 || (
		sizeof(hxexpected<char>) == 2u),
	"hxexpected<char> must pack its one byte value and one byte non-null flag"
	" with no padding");
static_assert(sizeof(size_t) != 8 || (
		sizeof(hxexpected<char>) == 2u),
	"hxexpected<char> must pack its one byte value and one byte non-null flag"
	" with no padding");
#endif

using hxexpected_test_f = hxtest_object_fixture;

hxattr_noinline static void hxtest_gdb_break_hxexpected(void) { }

TEST(hxexpected_test, construction_disengaged) {
	const hxexpected<int> a = hxnil;
	EXPECT_FALSE((bool)a);
	EXPECT_TRUE(a == hxnil);
	const hxexpected<int> b;
	EXPECT_FALSE((bool)b);
	EXPECT_FALSE(b.has_value());
	EXPECT_TRUE(b == hxnil);
	EXPECT_FALSE(b != hxnil);
}

TEST(hxexpected_test, construction_engaged) {
	const hxexpected<int> a(false, 34);
	EXPECT_TRUE((bool)a);
	EXPECT_TRUE(a.has_value());
	EXPECT_EQ(*a, 34);
	const hxexpected<int> b = hxnil;
	EXPECT_FALSE((bool)b);
	EXPECT_FALSE(b.has_value());
	const hxexpected<int, int> c(31);
	EXPECT_FALSE((bool)c);
	EXPECT_EQ(c.error(), 31);
	hxtest_gdb_break_hxexpected();
}

TEST_F(hxexpected_test_f, error_truth_controls_engagement) {
	const hxexpected<int, int> c_success(0);
	EXPECT_TRUE(c_success.has_value());
	EXPECT_EQ(c_success.value(), 0);

	const hxexpected<int, int> c_error(1);
	EXPECT_FALSE(c_error.has_value());
	EXPECT_EQ(c_error.error(), 1);

	const hxexpected<int, hxtest_object> class_success(false);
	EXPECT_TRUE(class_success.has_value());
	EXPECT_EQ(class_success.value(), 0);

	const hxexpected<int, hxtest_object> class_error(2);
	EXPECT_FALSE(class_error.has_value());
	EXPECT_EQ(class_error.error().value(), 2);

	const hxtest_object lvalue_success_error(0);
	const hxexpected<int, hxtest_object> lvalue_success(lvalue_success_error);
	EXPECT_TRUE(lvalue_success.has_value());
	EXPECT_EQ(lvalue_success.value(), 0);

	EXPECT_TRUE(check_stats(4, 0, 0, 3, 1, 0, 0, 0, 0, 0));
}

TEST(hxexpected_test, int_error_bool_conversions_do_not_warn) {
	const hxexpected<int, int> from_false(false, 31);
	EXPECT_TRUE(from_false.has_value());
	EXPECT_EQ(from_false.value(), 31);
	EXPECT_TRUE((bool)from_false);

	const hxexpected<int, int> from_true(true);
	EXPECT_FALSE(from_true.has_value());
	EXPECT_FALSE((bool)from_true);
	EXPECT_EQ(from_true.error(), 1);

	hxexpected<int, int> mutable_expected(false, 5);
	mutable_expected.reset();
	EXPECT_FALSE((bool)mutable_expected);
	EXPECT_EQ(mutable_expected.error(), 1);

	mutable_expected.reset(static_cast<int>(false));
	EXPECT_TRUE((bool)mutable_expected);
	EXPECT_EQ(mutable_expected.value(), 0);

	mutable_expected.reset(static_cast<int>(true));
	EXPECT_FALSE((bool)mutable_expected);
	EXPECT_EQ(mutable_expected.error(), 1);

	mutable_expected = hxnil;
	EXPECT_FALSE((bool)mutable_expected);
	EXPECT_EQ(mutable_expected.error(), 1);
}

TEST_F(hxexpected_test_f, reset_with_class_type_error) {
	{
		hxexpected<int, hxtest_object> a(1);
		EXPECT_FALSE(a.has_value());
		a.reset(hxtest_object(2));
		EXPECT_FALSE(a.has_value());
		EXPECT_EQ(a.error().value(), 2);
		a.reset(hxtest_object(0));
		EXPECT_TRUE(a.has_value());
		EXPECT_EQ(*a, 0);
		a.reset(hxtest_object(3));
		EXPECT_FALSE(a.has_value());
		EXPECT_EQ(a.error().value(), 3);
	}
	EXPECT_TRUE(check_stats(4, 4, 0, 4, 0, 0, 3, 0, 0, 0));
}

TEST_F(hxexpected_test_f, copy_and_move_assign_class_type_error) {
	{
		hxexpected<int, hxtest_object> b(3);
		const hxexpected<int, hxtest_object> c(4);
		b = c;
		EXPECT_FALSE(b.has_value());
		EXPECT_EQ(b.error().value(), 4);
		hxexpected<int, hxtest_object> d(5);
		b = hxmove(d);
		EXPECT_FALSE(b.has_value());
		EXPECT_EQ(b.error().value(), 5);
		EXPECT_FALSE(d.has_value());
	}
	EXPECT_TRUE(check_stats(3, 3, 0, 3, 0, 0, 1, 1, 0, 0));
}

TEST_F(hxexpected_test_f, swap_class_type_error) {
	{
		hxexpected<int, hxtest_object> e(6);
		hxexpected<int, hxtest_object> f(7);
		e.swap(f);
		EXPECT_EQ(e.error().value(), 7);
		EXPECT_EQ(f.error().value(), 6);
	}
	EXPECT_TRUE(check_stats(3, 3, 0, 2, 0, 1, 0, 2, 0, 0));
}

TEST_F(hxexpected_test_f, emplace_and_or_else_class_type_error) {
	{
		hxexpected<int, hxtest_object> a(1);
		EXPECT_FALSE(a.has_value());
		a.emplace(31);
		EXPECT_TRUE(a.has_value());
		EXPECT_EQ(*a, 31);

		const hxexpected<int, hxtest_object> g(8);
		bool called = false;
		const hxexpected<int, hxtest_object> h = g.or_else([&called]() {
			called = true;
			return hxmake_expected<int, hxtest_object>(9);
		});
		EXPECT_TRUE(called);
		EXPECT_TRUE(h.has_value());
		EXPECT_EQ(*h, 9);
	}
	EXPECT_TRUE(check_stats(5, 5, 0, 5, 0, 0, 0, 2, 0, 0));
}

TEST(hxexpected_test, error_transitions_and_propagation) {
	hxexpected<int, int> value = hxmake_expected<int, int>(31);
	EXPECT_EQ(value.value(), 31);
	value.reset(0);
	EXPECT_TRUE(value.has_value());
	EXPECT_EQ(value.value(), 31);
	value.reset(3);
	EXPECT_FALSE(value.has_value());
	EXPECT_EQ(value.error(), 3);
	value.reset(4);
	EXPECT_FALSE(value.has_value());
	EXPECT_EQ(value.error(), 4);
	value.reset(0);
	EXPECT_TRUE(value.has_value());
	EXPECT_EQ(value.value(), 0);

	const hxexpected<int, int> error(7);
	const hxexpected<long, int> propagated = error.and_then([](const int& x) {
		// GCOVR_EXCL_START
		return hxexpected<long, int>(x);
		// GCOVR_EXCL_STOP
	});
	EXPECT_FALSE(propagated.has_value());
	EXPECT_EQ(propagated, hxnil);
}

TEST(hxexpected_test, copy_construction) {
	const hxexpected<int> disengaged = hxnil;
	EXPECT_FALSE((bool)hxexpected<int>(disengaged));
	const hxexpected<int> non_null(false, 99);
	EXPECT_TRUE((bool)hxexpected<int>(non_null));
	EXPECT_EQ(*hxexpected<int>(non_null), 99);
}

TEST(hxexpected_test, move_construction) {
	hxexpected<int> a(false, 5);
	const hxexpected<int> b(hxmove(a));
	EXPECT_TRUE((bool)b);
	EXPECT_EQ(*b, 5);
	EXPECT_FALSE((bool)a);
}

TEST(hxexpected_test, move_construction_disengaged) {
	hxexpected<int> a;
	const hxexpected<int> b(hxmove(a));
	EXPECT_FALSE((bool)b);
	EXPECT_FALSE((bool)a);
}

TEST_F(hxexpected_test_f, move_construction_reads_source_before_error_moved) {
	{
		hxexpected<int, hxtest_object> a = hxmake_expected<int, hxtest_object>(31);
		const hxexpected<int, hxtest_object> b(hxmove(a));
		EXPECT_TRUE((bool)b);
		EXPECT_EQ(*b, 31);
	}
	EXPECT_TRUE(check_stats(4, 4, 0, 3, 0, 1, 0, 2, 0, 0));
}

TEST_F(hxexpected_test_f, destructor_engaged_and_disengaged) {
	{
		const hxexpected<hxtest_object> o(false, hxtest_object(1));
	}
	EXPECT_TRUE(check_stats(2, 2, 0, 1, 0, 1, 0, 0, 0, 0));
	{
		const hxexpected<hxtest_object> o = hxnil;
	}
	EXPECT_TRUE(check_stats(2, 2, 0, 1, 0, 1, 0, 0, 0, 0));
}

TEST_F(hxexpected_test_f, deref_operator_returns_reference) {
	{
		hxexpected<hxtest_object> o(false, 1);
		(*o).value() = 2;
		EXPECT_EQ((*o).value(), 2);
		const hxexpected<hxtest_object> co(false, 3);
		EXPECT_EQ((*co).value(), 3);
	}
	EXPECT_TRUE(check_stats(2, 2, 0, 2, 0, 0, 0, 0, 0, 0));
}

TEST_F(hxexpected_test_f, arrow_operator_accesses_fields) {
	{
		hxexpected<hxtest_object> o(false, 4);
		EXPECT_EQ(o->value(), 4);
		o->value() = 8;
		EXPECT_EQ(o->value(), 8);
		const hxexpected<hxtest_object> co(false, 31);
		EXPECT_EQ(co->value(), 31);
		EXPECT_EQ(&co->value(), &(*co).value());
	}
	EXPECT_TRUE(check_stats(2, 2, 0, 2, 0, 0, 0, 0, 0, 0));
}

TEST(hxexpected_test, copy_assign) {
	const hxexpected<int> non_null(false, 55);
	hxexpected<int> to_disengaged = hxnil;
	to_disengaged = non_null;
	EXPECT_TRUE((bool)to_disengaged);
	EXPECT_EQ(*to_disengaged, 55);
	const hxexpected<int> disengaged = hxnil;
	hxexpected<int> to_engaged(false, 77);
	to_engaged = disengaged;
	EXPECT_FALSE((bool)to_engaged);
	const hxexpected<int> eleven(false, 11);
	hxexpected<int> twentytwo(false, 22);
	twentytwo = eleven;
	EXPECT_TRUE((bool)twentytwo);
	EXPECT_EQ(*twentytwo, 11);
	hxexpected<int> still_disengaged = hxnil;
	still_disengaged = disengaged;
	EXPECT_FALSE((bool)still_disengaged);
}

TEST(hxexpected_test, move_assign) {
	hxexpected<int> non_null(false, 9);
	hxexpected<int> to_disengaged = hxnil;
	to_disengaged = hxmove(non_null);
	EXPECT_TRUE((bool)to_disengaged);
	EXPECT_EQ(*to_disengaged, 9);
	EXPECT_FALSE((bool)non_null);
	hxexpected<int> disengaged = hxnil;
	hxexpected<int> to_engaged(false, 44);
	to_engaged = hxmove(disengaged);
	EXPECT_FALSE((bool)to_engaged);
	hxexpected<int> thirteen(false, 13);
	hxexpected<int> twentysix(false, 26);
	twentysix = hxmove(thirteen);
	EXPECT_TRUE((bool)twentysix);
	EXPECT_EQ(*twentysix, 13);
	EXPECT_FALSE((bool)thirteen);
	hxexpected<int> disengaged_source = hxnil;
	hxexpected<int> still_disengaged = hxnil;
	still_disengaged = hxmove(disengaged_source);
	EXPECT_FALSE((bool)still_disengaged);
	EXPECT_FALSE((bool)disengaged_source);
}

TEST(hxexpected_test, assign_nullopt_disengages) {
	hxexpected<int> o(false, 5);
	o = hxnil;
	EXPECT_FALSE((bool)o);
}

TEST(hxexpected_test, value_assign) {
	hxexpected<int> to_disengaged = hxnil;
	const int v = 17;
	to_disengaged = v;
	EXPECT_TRUE((bool)to_disengaged);
	EXPECT_EQ(*to_disengaged, 17);
	hxexpected<int> to_engaged(false, 1);
	const int w = 2;
	to_engaged = w;
	EXPECT_EQ(*to_engaged, 2);
	hxexpected<int> moved_to_disengaged = hxnil;
	int mv = 19;
	moved_to_disengaged = hxmove(mv);
	EXPECT_TRUE((bool)moved_to_disengaged);
	EXPECT_EQ(*moved_to_disengaged, 19);
	hxexpected<int> moved_to_engaged(false, 3);
	int mw = 4;
	moved_to_engaged = hxmove(mw);
	EXPECT_EQ(*moved_to_engaged, 4);
}

TEST_F(hxexpected_test_f, eq_expected) {
	const hxexpected<int> empty = hxnil;
	const hxexpected<int> one(false, 1);
	const hxexpected<int> one_b(false, 1);
	const hxexpected<int> two(false, 2);
	EXPECT_TRUE(empty == empty);
	EXPECT_FALSE(empty != empty);
	EXPECT_FALSE(empty == one);
	EXPECT_TRUE(empty != one);
	EXPECT_TRUE(one == one_b);
	EXPECT_FALSE(one != one_b);
	EXPECT_FALSE(one == two);
	EXPECT_TRUE(one != two);
	const hxexpected<int> zero(false, 0);
	const hxexpected<int> zero_b(false, 0);
	EXPECT_TRUE(zero == zero_b);
	EXPECT_FALSE(zero != zero_b);
	EXPECT_FALSE(zero == one);
	EXPECT_TRUE(zero != one);
	const hxexpected<int, hxtest_object> error_two(hxtest_object(2));
	const hxexpected<int, hxtest_object> error_two_b(hxtest_object(2));
	const hxexpected<int, hxtest_object> error_three(hxtest_object(3));
	EXPECT_TRUE(error_two == error_two_b);
	EXPECT_FALSE(error_two == error_three);
	EXPECT_TRUE(check_stats(6, 3, 0, 3, 0, 3, 0, 0, 2, 0));
}

TEST(hxexpected_test, hxkey_equal) {
	const hxexpected<int> empty = hxnil;
	const hxexpected<int> empty_b = hxnil;
	const hxexpected<int> one(false, 1);
	const hxexpected<int> one_b(false, 1);
	const hxexpected<int> two(false, 2);
	EXPECT_TRUE(hxkey_equal(empty, empty_b));
	EXPECT_FALSE(hxkey_equal(empty, one));
	EXPECT_TRUE(hxkey_equal(one, one_b));
	EXPECT_FALSE(hxkey_equal(one, two));
}

TEST_F(hxexpected_test_f, hxkey_hash) {
	const hxexpected<hxtest_object> empty = hxnil;
	const hxexpected<hxtest_object> empty_b = hxnil;
	const hxexpected<hxtest_object> a(false, 31);
	const hxexpected<hxtest_object> b(false, 31);
	const hxexpected<hxtest_object> c(false, 32);
	EXPECT_EQ(hxkey_hash(empty), hxkey_hash(empty_b));
	EXPECT_EQ(hxkey_hash(empty), hxhash_t{31u});
	EXPECT_EQ(hxkey_hash(a), hxkey_hash(b));
	EXPECT_NE(hxkey_hash(a), hxkey_hash(c));
	EXPECT_TRUE(check_stats(3, 0, 0, 3, 0, 0, 0, 0, 0, 0));
}

TEST(hxexpected_test, eq_nullopt_and_value) {
	const hxexpected<int> empty = hxnil;
	const hxexpected<int> seven(false, 7);
	EXPECT_TRUE(empty == hxnil);
	EXPECT_FALSE(empty != hxnil);
	EXPECT_FALSE(seven == hxnil);
	EXPECT_TRUE(seven != hxnil);
	EXPECT_TRUE(seven == 7);
	EXPECT_FALSE(seven != 7);
	EXPECT_FALSE(seven == 8);
	EXPECT_TRUE(seven != 8);
	EXPECT_FALSE(empty == 0);
	EXPECT_TRUE(empty != 0);
	const hxexpected<int> zero(false, 0);
	EXPECT_TRUE(zero == 0);
	EXPECT_FALSE(zero != 0);
}

TEST_F(hxexpected_test_f, reset_engaged_and_disengaged) {
	hxexpected<hxtest_object> non_null(false, 1);
	non_null.reset();
	EXPECT_FALSE((bool)non_null);
	non_null.reset();

	hxexpected<hxtest_object> disengaged = hxnil;
	disengaged.reset();
	EXPECT_FALSE((bool)disengaged);

	EXPECT_TRUE(check_stats(1, 1, 0, 1, 0, 0, 0, 0, 0, 0));
}

TEST(hxexpected_test, swap_both_engaged) {
	hxexpected<int> a(false, 1);
	hxexpected<int> b(false, 2);
	a.swap(b);
	EXPECT_EQ(*a, 2);
	EXPECT_EQ(*b, 1);
}

TEST(hxexpected_test, swap_with_disengaged) {
	hxexpected<int> a(false, 1);
	hxexpected<int> c = hxnil;
	a.swap(c);
	EXPECT_FALSE((bool)a);
	EXPECT_EQ(*c, 1);
	c.swap(a);
	EXPECT_FALSE((bool)c);
	EXPECT_TRUE((bool)a);
	EXPECT_EQ(*a, 1);
	hxexpected<int> d = hxnil;
	hxexpected<int> e(false, 88);
	d.swap(e);
	EXPECT_TRUE((bool)d);
	EXPECT_EQ(*d, 88);
	EXPECT_FALSE((bool)e);
}

TEST_F(hxexpected_test_f, swap_both_disengaged) {
	hxexpected<hxtest_object> f = hxnil;
	hxexpected<hxtest_object> g = hxnil;
	f.swap(g);
	EXPECT_FALSE((bool)f);
	EXPECT_FALSE((bool)g);
	EXPECT_TRUE(check_no_stats());
}

TEST(hxexpected_test, value_returns_reference) {
	hxexpected<int> o(false, 34);
	EXPECT_EQ(o.value(), 34);
	o.value() = 99;
	EXPECT_EQ(o.value(), 99);
	const hxexpected<int> co(false, 10);
	EXPECT_EQ(co.value(), 10);
}

TEST_F(hxexpected_test_f, value_or) {
	const hxexpected<int> non_null(false, 3);
	const hxexpected<int> empty = hxnil;
	EXPECT_EQ(non_null.value_or(99), 3);
	EXPECT_EQ(empty.value_or(7), 7);
	const hxexpected<int> zero(false, 0);
	EXPECT_EQ(zero.value_or(99), 0);
	hxexpected<hxtest_object> move_engaged(false, 4);
	const hxtest_object moved =
		hxmove(move_engaged).value_or(hxtest_object(5));
	EXPECT_EQ(moved.value(), 4);
	hxexpected<hxtest_object> move_empty = hxnil;
	const hxtest_object fallback =
		hxmove(move_empty).value_or(hxtest_object(6));
	EXPECT_EQ(fallback.value(), 6);
	const bool has_error = true;
	const hxexpected<hxtest_object> emplace_empty(has_error);
	EXPECT_EQ(emplace_empty.value_or(3, 4).value(), 7);
	const bool no_error = false;
	const hxexpected<hxtest_object> emplace_engaged(no_error);
	EXPECT_EQ(emplace_engaged.value_or(8, 9).value(), 0);
	EXPECT_TRUE(check_stats(8, 4, 1, 4, 1, 2, 0, 0, 0, 0));
}

TEST_F(hxexpected_test_f, emplace) {
	hxexpected<hxtest_object> o = hxnil;
	o.emplace(34);
	EXPECT_TRUE((bool)o);
	EXPECT_EQ(o->value(), 34);
	o.emplace(2);
	EXPECT_EQ(o->value(), 2);
	EXPECT_TRUE(check_stats(2, 1, 0, 2, 0, 0, 0, 0, 0, 0));

	hxexpected<int> n = hxnil;
	int& ref = n.emplace(55);
	EXPECT_EQ(ref, 55);
	ref = 66;
	EXPECT_EQ(*n, 66);
}

TEST_F(hxexpected_test_f, destructor_exactly_one_call) {
	{ const hxexpected<hxtest_object> o(false, 1); }
	EXPECT_TRUE(check_stats(1, 1, 0, 1, 0, 0, 0, 0, 0, 0));
}

TEST(hxexpected_test, and_then_engaged) {
	hxexpected<int> non_null(false, 3);
	const hxexpected<long> r = non_null.and_then([](int& v) { return hxexpected<long>(false, v + 1); });
	EXPECT_TRUE((bool)r);
	EXPECT_EQ(*r, 4);
	EXPECT_FALSE((bool)non_null.and_then([](int&) { return hxexpected<int>(hxnil); }));
	const hxexpected<int> const_engaged(false, 5);
	const hxexpected<int> const_r =
		const_engaged.and_then([](const int& v) { return hxexpected<int>(false, v * 2); });
	EXPECT_TRUE((bool)const_r);
	EXPECT_EQ(*const_r, 10);
	hxexpected<int> rvalue(false, 6);
	EXPECT_EQ(hxmove(rvalue).and_then([](int&& v) {
		return hxexpected<int>(false, v + 1);
	}), 7);
	const hxexpected<int> const_rvalue(false, 31);
	EXPECT_EQ(hxmove(const_rvalue).and_then([](const int&& v) {
		return hxexpected<int>(false, v + 1);
	}), 32);
}

TEST(hxexpected_test, and_then_disengaged) {
	hxexpected<int> disengaged = hxnil;
	bool called = false;
	const hxexpected<long> empty = disengaged.and_then([&called](int& v) {
		// GCOVR_EXCL_START
		called = true; return hxexpected<long>(false, v);
		// GCOVR_EXCL_STOP
	});
	EXPECT_FALSE((bool)empty);
	EXPECT_FALSE(called);
	const hxexpected<int> const_disengaged = hxnil;
	EXPECT_FALSE((bool)const_disengaged.and_then([](const int& v) { return hxexpected<int>(false, v); }));
}

TEST_F(hxexpected_test_f, or_else_engaged_and_disengaged) {
	const hxexpected<int> non_null(false, 8);
	bool called = false;
	const hxexpected<int> self = non_null.or_else([&called]() {
		// GCOVR_EXCL_START
		called = true; return hxexpected<int>(false, 0);
		// GCOVR_EXCL_STOP
	});
	EXPECT_TRUE((bool)self);
	EXPECT_EQ(*self, 8);
	EXPECT_FALSE(called);
	const hxexpected<int> disengaged = hxnil;
	const hxexpected<int> r = disengaged.or_else([&called]() {
		called = true; return hxexpected<int>(false, 31);
	});
	EXPECT_TRUE(called);
	EXPECT_TRUE((bool)r);
	EXPECT_EQ(*r, 31);
	EXPECT_FALSE((bool)disengaged.or_else([]() { return hxexpected<int>(hxnil); }));
	hxexpected<hxtest_object> move_engaged(false, 9);
	const hxexpected<hxtest_object> moved = hxmove(move_engaged).or_else([]() {
		// GCOVR_EXCL_START
		return hxexpected<hxtest_object>(hxnil);
		// GCOVR_EXCL_STOP
	});
	EXPECT_TRUE((bool)moved);
	EXPECT_EQ(moved->value(), 9);
	EXPECT_TRUE(check_stats(2, 1, 0, 1, 0, 1, 0, 0, 0, 0));
}

#endif // HX_CPLUSPLUS >= 202302L
