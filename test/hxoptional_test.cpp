// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

#include <hx/hxoptional.hpp>
#include <hx/hxtest.hpp>

namespace {

// Tracks construction and destruction for lifetime tests.
int s_hxtest_ctor_count = 0;
int s_hxtest_dtor_count = 0;

struct hxtest_optional_counted_t {
	explicit hxtest_optional_counted_t(int v) : value(v) { ++s_hxtest_ctor_count; }
	hxtest_optional_counted_t(const hxtest_optional_counted_t& o)
		: value(o.value) { ++s_hxtest_ctor_count; }
	hxtest_optional_counted_t(hxtest_optional_counted_t&& o)
		: value(o.value) { ++s_hxtest_ctor_count; }
	~hxtest_optional_counted_t(void) { ++s_hxtest_dtor_count; }
	hxtest_optional_counted_t& operator=(const hxtest_optional_counted_t& o) {
		value = o.value; return *this;
	}
	hxtest_optional_counted_t& operator=(hxtest_optional_counted_t&& o) {
		value = o.value; return *this;
	}
	bool operator==(const hxtest_optional_counted_t& o) const {
		return value == o.value;
	}
	bool operator!=(const hxtest_optional_counted_t& o) const {
		return value != o.value;
	}
	bool operator<(const hxtest_optional_counted_t& o) const {
		return value < o.value;
	}
	int value;
};

// Returns an engaged or disengaged optional based on flag, as a function would.
hxoptional<int> hxtest_make_int(bool engaged, int v = 0) {
	if (engaged) { return v; }
	return hxnullopt;
}

hxoptional<hxtest_optional_counted_t> hxtest_make_counted(bool engaged, int v = 0) {
	if (engaged) { return hxtest_optional_counted_t(v); }
	return hxnullopt;
}

} // namespace

// Default construction is disengaged.
TEST(hxoptional_test, default_construction_is_disengaged) {
	const hxoptional<int> o;
	EXPECT_FALSE((bool)o);
	EXPECT_FALSE(o.has_value());
	EXPECT_TRUE(o == hxnullopt);
	EXPECT_FALSE(o != hxnullopt);
}

// Construction from a value engages the optional with that value.
TEST(hxoptional_test, value_construction_is_engaged) {
	const hxoptional<int> o = hxtest_make_int(true, 42);
	EXPECT_TRUE((bool)o);
	EXPECT_TRUE(o.has_value());
	EXPECT_EQ(*o, 42);
}

// Copy construction from a disengaged optional is disengaged.
TEST(hxoptional_test, copy_construction_from_disengaged) {
	const hxoptional<int> a = hxtest_make_int(false);
	EXPECT_FALSE((bool)hxoptional<int>(a));
}

// Copy construction from an engaged optional copies the value.
TEST(hxoptional_test, copy_construction_from_engaged) {
	const hxoptional<int> a = hxtest_make_int(true, 99);
	EXPECT_TRUE((bool)hxoptional<int>(a));
	EXPECT_EQ(*hxoptional<int>(a), 99);
}

// Move construction from an engaged optional transfers value and disengages source.
TEST(hxoptional_test, move_construction_from_engaged) {
	hxoptional<int> a = hxtest_make_int(true, 5);
	const hxoptional<int> b(hxmove(a));
	EXPECT_TRUE((bool)b);
	EXPECT_EQ(*b, 5);
	EXPECT_FALSE((bool)a); // NOLINT
}

// Destructor destroys the contained object exactly once.
TEST(hxoptional_test, destructor_destroys_value) {
	s_hxtest_ctor_count = 0;
	s_hxtest_dtor_count = 0;
	{
		const hxoptional<hxtest_optional_counted_t> o = hxtest_make_counted(true, 1);
	}
	// hxtest_optional_counted_t(v) may be moved into optional storage.
	EXPECT_TRUE(s_hxtest_ctor_count == 1 || s_hxtest_ctor_count == 2);
	EXPECT_EQ(s_hxtest_dtor_count, s_hxtest_ctor_count);
}

// Destructor on a disengaged optional does not destroy anything.
TEST(hxoptional_test, destructor_disengaged_no_destroy) {
	s_hxtest_dtor_count = 0;
	{
		const hxoptional<hxtest_optional_counted_t> o = hxtest_make_counted(false);
	}
	EXPECT_EQ(s_hxtest_dtor_count, 0);
}

// operator* returns a mutable reference to the contained value.
TEST(hxoptional_test, deref_operator_returns_reference) {
	hxoptional<int> o = hxtest_make_int(true, 10);
	*o = 20;
	EXPECT_EQ(*o, 20);
}

// operator-> gives mutable access to members.
TEST(hxoptional_test, arrow_operator_accesses_members) {
	hxoptional<hxtest_optional_counted_t> o = hxtest_make_counted(true, 4);
	EXPECT_EQ(o->value, 4);
	o->value = 8;
	EXPECT_EQ(o->value, 8);
}

// Copy assignment from engaged to disengaged engages with copied value.
TEST(hxoptional_test, copy_assign_engaged_to_disengaged) {
	const hxoptional<int> a = hxtest_make_int(true, 55);
	hxoptional<int> b = hxtest_make_int(false);
	b = a;
	EXPECT_TRUE((bool)b);
	EXPECT_EQ(*b, 55);
}

// Copy assignment from disengaged to engaged disengages.
TEST(hxoptional_test, copy_assign_disengaged_to_engaged) {
	const hxoptional<int> a = hxtest_make_int(false);
	hxoptional<int> b = hxtest_make_int(true, 77);
	b = a;
	EXPECT_FALSE((bool)b);
}

// Copy assignment from engaged to engaged updates the value.
TEST(hxoptional_test, copy_assign_engaged_to_engaged) {
	const hxoptional<int> a = hxtest_make_int(true, 11);
	hxoptional<int> b = hxtest_make_int(true, 22);
	b = a;
	EXPECT_TRUE((bool)b);
	EXPECT_EQ(*b, 11);
}

// Move assignment from engaged to disengaged transfers and disengages source.
TEST(hxoptional_test, move_assign_engaged_to_disengaged) {
	hxoptional<int> a = hxtest_make_int(true, 9);
	hxoptional<int> b = hxtest_make_int(false);
	b = hxmove(a);
	EXPECT_TRUE((bool)b);
	EXPECT_EQ(*b, 9);
	EXPECT_FALSE((bool)a); // NOLINT
}

// Move assignment from disengaged to engaged disengages.
TEST(hxoptional_test, move_assign_disengaged_to_engaged) {
	hxoptional<int> a = hxtest_make_int(false);
	hxoptional<int> b = hxtest_make_int(true, 44);
	b = hxmove(a);
	EXPECT_FALSE((bool)b);
}

// Move assignment from engaged to engaged moves the value and disengages source.
TEST(hxoptional_test, move_assign_engaged_to_engaged) {
	hxoptional<int> a = hxtest_make_int(true, 13);
	hxoptional<int> b = hxtest_make_int(true, 26);
	b = hxmove(a);
	EXPECT_TRUE((bool)b);
	EXPECT_EQ(*b, 13);
	EXPECT_FALSE((bool)a); // NOLINT
}

// Assignment of hxnullopt disengages an engaged optional.
TEST(hxoptional_test, assign_nullopt_disengages) {
	hxoptional<int> o = hxtest_make_int(true, 5);
	o = hxnullopt;
	EXPECT_FALSE((bool)o);
}

// Assignment of a value to a disengaged optional engages it.
TEST(hxoptional_test, value_copy_assign_to_disengaged) {
	hxoptional<int> o = hxtest_make_int(false);
	const int v = 17;
	o = v;
	EXPECT_TRUE((bool)o);
	EXPECT_EQ(*o, 17);
}

// Assignment of a value to an engaged optional updates the value.
TEST(hxoptional_test, value_copy_assign_to_engaged) {
	hxoptional<int> o = hxtest_make_int(true, 1);
	const int v = 2;
	o = v;
	EXPECT_EQ(*o, 2);
}

// Move assignment of a value to a disengaged optional engages it.
TEST(hxoptional_test, value_move_assign_to_disengaged) {
	hxoptional<int> o = hxtest_make_int(false);
	int v = 19;
	o = hxmove(v);
	EXPECT_TRUE((bool)o);
	EXPECT_EQ(*o, 19);
}

// Move assignment of a value to an engaged optional updates the value.
TEST(hxoptional_test, value_move_assign_to_engaged) {
	hxoptional<int> o = hxtest_make_int(true, 3);
	int v = 4;
	o = hxmove(v);
	EXPECT_EQ(*o, 4);
}

// operator== between two optionals: both disengaged, one engaged one not, both
// engaged equal, both engaged unequal.
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

// operator== with hxnullopt and with a value.
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

// operator< ordering covers all four engaged/disengaged combinations and equal.
TEST(hxoptional_test, ordering) {
	const hxoptional<int> empty = hxtest_make_int(false);
	const hxoptional<int> zero = hxtest_make_int(true, 0);
	const hxoptional<int> one = hxtest_make_int(true, 1);
	const hxoptional<int> two = hxtest_make_int(true, 2);
	// Both disengaged: not less, equal.
	EXPECT_FALSE(empty < empty);
	EXPECT_TRUE(empty <= empty);
	EXPECT_FALSE(empty > empty);
	EXPECT_TRUE(empty >= empty);
	// Disengaged < engaged.
	EXPECT_TRUE(empty < zero);
	EXPECT_FALSE(zero < empty);
	EXPECT_TRUE(empty <= zero);
	EXPECT_FALSE(empty >= zero);
	EXPECT_FALSE(empty > zero);
	EXPECT_TRUE(zero > empty);
	// Both engaged, less.
	EXPECT_TRUE(one < two);
	EXPECT_FALSE(two < one);
	EXPECT_TRUE(one <= two);
	EXPECT_FALSE(two <= one);
	EXPECT_FALSE(one > two);
	EXPECT_TRUE(two > one);
	EXPECT_FALSE(one >= two);
	EXPECT_TRUE(two >= one);
	// Both engaged, equal.
	EXPECT_FALSE(one < one);
	EXPECT_TRUE(one <= one);
	EXPECT_FALSE(one > one);
	EXPECT_TRUE(one >= one);
}

// reset on an engaged optional destroys the value and disengages.
TEST(hxoptional_test, reset_engaged_disengages) {
	hxoptional<hxtest_optional_counted_t> o = hxtest_make_counted(true, 1);
	s_hxtest_dtor_count = 0;
	o.reset();
	EXPECT_FALSE((bool)o);
	EXPECT_EQ(s_hxtest_dtor_count, 1);
}

// reset on a disengaged optional is a no-op.
TEST(hxoptional_test, reset_disengaged_noop) {
	s_hxtest_dtor_count = 0;
	hxoptional<hxtest_optional_counted_t> o = hxtest_make_counted(false);
	o.reset();
	EXPECT_FALSE((bool)o);
	EXPECT_EQ(s_hxtest_dtor_count, 0);
}

// swap: both engaged exchanges values; engaged with disengaged transfers value.
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

// value() returns a mutable reference; const overload returns const reference.
TEST(hxoptional_test, value_returns_reference) {
	hxoptional<int> o = hxtest_make_int(true, 42);
	EXPECT_EQ(o.value(), 42);
	o.value() = 99;
	EXPECT_EQ(o.value(), 99);
	const hxoptional<int> co = hxtest_make_int(true, 10);
	EXPECT_EQ(co.value(), 10);
}

// value_or returns contained value when engaged, default when disengaged.
TEST(hxoptional_test, value_or) {
	const hxoptional<int> engaged = hxtest_make_int(true, 3);
	const hxoptional<int> empty = hxtest_make_int(false);
	EXPECT_EQ(engaged.value_or(99), 3);
	EXPECT_EQ(empty.value_or(7), 7);
}

// emplace on disengaged engages; on engaged destroys old and constructs new;
// returns a reference to the constructed value.
TEST(hxoptional_test, emplace) {
	hxoptional<hxtest_optional_counted_t> o = hxtest_make_counted(false);
	o.emplace(42);
	EXPECT_TRUE((bool)o);
	EXPECT_EQ(o->value, 42);
	s_hxtest_dtor_count = 0;
	o.emplace(2);
	EXPECT_EQ(s_hxtest_dtor_count, 1);
	EXPECT_EQ(o->value, 2);
	hxoptional<int> n = hxtest_make_int(false);
	int& ref = n.emplace(55);
	EXPECT_EQ(ref, 55);
	ref = 66;
	EXPECT_EQ(*n, 66);
}
