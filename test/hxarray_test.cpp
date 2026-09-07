// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

#include <hx/hxarray.hpp>
#include <hx/hxrange.hpp>
#include "./hxtest_util.hpp"
#if HX_USE_LIBCXX
#include <utility>
#endif

HX_NS_USE

#if !defined _MSC_VER && !defined __wasm__
static_assert(sizeof(size_t) != 4 || (
		sizeof(hxarray<int32_t, hxallocator_dynamic_capacity>) == 8u
		&& sizeof(hxarray<int32_t, 4>) == 16u),
	"hxarray must pack dynamic storage as a hxsize_t capacity and a T* data"
	" pointer with no padding, and fixed storage as capacity * sizeof(T) with"
	" no padding");
static_assert(sizeof(size_t) != 8 || (
		sizeof(hxarray<int32_t, hxallocator_dynamic_capacity>) == 16u
		&& sizeof(hxarray<int32_t, 4>) == 16u),
	"hxarray must pack dynamic storage as a hxsize_t capacity and a T* data"
	" pointer with no padding, and fixed storage as capacity * sizeof(T) with"
	" no padding");
#endif

hxattr_noinline static void hxtest_gdb_break_hxarray_static(void) { }
hxattr_noinline static void hxtest_gdb_break_hxarray_dynamic(void) { }
hxattr_noinline static void hxtest_gdb_break_hxarray_unallocated(void) { }

using hxarray_test_f = hxtest_object_fixture;

#if HX_CPLUSPLUS >= 202302L
TEST_F(hxarray_test_f, value_or_emplaces_fallback) {
	hxarray<hxtest_object, 1> a;
	EXPECT_EQ(a.value_or(hxsize_t{0}, 14, 17).value(), 0);
	EXPECT_EQ(a.value_or(hxsize_t{1}, 14, 17).value(), 31);
	EXPECT_EQ(a.value_or(a.begin(), 20, 23).value(), 0);
	EXPECT_EQ(a.value_or(a.end(), 20, 23).value(), 43);
	EXPECT_TRUE(check_stats(5, 4, 1, 2, 2, 0, 0, 0, 0, 0));
}
#endif // HX_CPLUSPLUS >= 202302L

TEST_F(hxarray_test_f, default_constructs_all_elements) {
	{
		hxarray<hxtest_object, 3> a;
		EXPECT_EQ(a.size(), 3);
		EXPECT_EQ(a.max_size(), 3);
		EXPECT_EQ(a.size_bytes(), 3 * hxsizeof<hxtest_object>());
		for(hxsize_t i = 0; i < a.size(); ++i) {
			EXPECT_EQ(a[i].state(), hxtest_object_state::valid);
		}
	}
	EXPECT_TRUE(check_stats(3, 3, 3, 0, 0, 0, 0, 0, 0, 0));
}

TEST_F(hxarray_test_f, fill_constructor) {
	{
		const hxtest_object fill(99);
		hxarray<hxtest_object, 4> a(fill);
		EXPECT_EQ(a.size(), 4);
		for(hxsize_t i = 0; i < a.size(); ++i) {
			EXPECT_EQ(a[i].value(), 99);
			EXPECT_EQ(a[i].state(), hxtest_object_state::valid);
		}
	}
	EXPECT_TRUE(check_stats(5, 5, 0, 1, 4, 0, 0, 0, 0, 0));
}

TEST_F(hxarray_test_f, copy_constructor) {
	{
		const hxarray<hxtest_object, 3> src{
			hxtest_object(10), hxtest_object(20), hxtest_object(30)};
		const hxarray<hxtest_object, 3> dst(src); // NOLINT(performance-unnecessary-copy-initialization)
		EXPECT_EQ(dst[0].value(), 10);
		EXPECT_EQ(dst[1].value(), 20);
		EXPECT_EQ(dst[2].value(), 30);
		for(hxsize_t i = 0; i < dst.size(); ++i) {
			EXPECT_EQ(dst[i].state(), hxtest_object_state::valid);
			EXPECT_EQ(src[i].state(), hxtest_object_state::valid);
		}
	}
	EXPECT_TRUE(check_stats(9, 9, 0, 3, 6, 0, 0, 0, 0, 0));
}

TEST_F(hxarray_test_f, copy_assignment) {
	{
		const hxarray<hxtest_object, 3> src{
			hxtest_object(11), hxtest_object(22), hxtest_object(33)};
		hxarray<hxtest_object, 3> dst;
		dst = src;
		EXPECT_EQ(dst[0].value(), 11);
		EXPECT_EQ(dst[1].value(), 22);
		EXPECT_EQ(dst[2].value(), 33);
		for(hxsize_t i = 0; i < dst.size(); ++i) {
			EXPECT_EQ(dst[i].state(), hxtest_object_state::valid);
		}
	}
	EXPECT_TRUE(check_stats(9, 9, 3, 3, 3, 0, 3, 0, 0, 0));
}

TEST_F(hxarray_test_f, static_c_array_constructor) {
	const hxtest_object values[4] = {
		hxtest_object(7), hxtest_object(14), hxtest_object(21), hxtest_object(28)};
	hxarray<hxtest_object, 4> a(values);
	EXPECT_EQ(a[0].value(), 7);
	EXPECT_EQ(a[1].value(), 14);
	EXPECT_EQ(a[2].value(), 21);
	EXPECT_EQ(a[3].value(), 28);
	hxtest_gdb_break_hxarray_static();
	EXPECT_TRUE(check_stats(8, 0, 0, 4, 4, 0, 0, 0, 0, 0));
}

#if HX_CPLUSPLUS >= 202002L
TEST(hxarray_test, initializer_list_constructor) {
	hxarray<int, 4> a { 3, 6, 9, 12 };
	EXPECT_EQ(a[0], 3);
	EXPECT_EQ(a[1], 6);
	EXPECT_EQ(a[2], 9);
	EXPECT_EQ(a[3], 12);
}
#endif

TEST(hxarray_test, iteration) {
	static const int values[5] = { 1, 2, 3, 4, 5 };
	hxarray<int, 5> a(values);
	const hxarray<int, 5>& ca = a;
	hxsize_t index = 0;
	for(hxarray<int, 5>::iterator it = a.begin(); it != a.end(); ++it) {
		EXPECT_EQ(*it, values[index++]);
	}
	EXPECT_EQ(index, 5);
	index = 0;
	for(hxarray<int, 5>::const_iterator it = ca.cbegin(); it != ca.cend(); ++it) {
		EXPECT_EQ(*it, values[index++]);
	}
	EXPECT_EQ(index, 5);
	EXPECT_EQ(ca.cbegin(), ca.begin());
	EXPECT_EQ(ca.cend(), ca.end());
	EXPECT_EQ(ca[0], 1);
	EXPECT_EQ(ca[4], 5);
}

TEST(hxarray_test, find) {
	hxarray<int, 5> a{ 2, 4, 4, 8, 16 };
	const hxarray<int, 5>& ca = a;
	EXPECT_EQ(ca.find(4), a.begin() + 1);
	EXPECT_EQ(ca.find(32), a.end());
	EXPECT_EQ(a.find(8), a.begin() + 3);
	const int* fp = ca.find_if([](int value) { return value >= 8; });
	EXPECT_EQ(fp, a.begin() + 3);
	const int* miss = ca.find_if([](int value) { return value < 0; });
	EXPECT_EQ(miss, a.end());
	const int* mp = a.find_if([](int& value) { return (value & 1) == 0 && value > 4; });
	EXPECT_EQ(mp, a.begin() + 3);
	const int* mp_miss = a.find_if([](int& value) { return value < 0; });
	EXPECT_EQ(mp_miss, a.end());
}

TEST(hxarray_test, find_last_element) {
	hxarray<int, 4> a{ 1, 2, 3, 99 };
	const hxarray<int, 4>& ca = a;
	EXPECT_EQ(ca.find(99), a.begin() + 3);
	EXPECT_EQ(*ca.find(99), 99);
	const int* fp = ca.find_if([](int v) { return v == 99; });
	EXPECT_EQ(fp, a.begin() + 3);
	hxarray<int, 1> b{ 7 };
	EXPECT_EQ(b.find(7), b.begin());
	EXPECT_EQ(b.find(8), b.end());
}

TEST(hxarray_test, all_of_any_of) {
	hxarray<int, 5> a{ 91, 92, 93, 94, 95 };
	EXPECT_TRUE(a.all_of([](const int& x) { return x > 0; }));
	EXPECT_FALSE(a.all_of([](const int& x) { return x < 95; }));
	int all_calls = 0;
	EXPECT_FALSE(a.all_of([&](const int& value) -> bool {
		++all_calls;
		return value < 93;
	}));
	EXPECT_EQ(all_calls, 3);
	int any_calls = 0;
	EXPECT_TRUE(a.any_of([&](const int& value) -> bool {
		++any_calls;
		return value == 94;
	}));
	EXPECT_EQ(any_calls, 4);
	EXPECT_FALSE(a.any_of([](const int& value) { return value < 0; }));
	const hxarray<int, 5>& ca = a;
	EXPECT_TRUE(ca.all_of([](const int& x) { return x > 0; }));
	EXPECT_FALSE(ca.all_of([](const int& x) { return x < 95; }));
	EXPECT_TRUE(ca.any_of([](const int& x) { return x == 94; }));
	EXPECT_FALSE(ca.any_of([](const int& value) { return value < 0; }));
}

TEST(hxarray_test, all_of_any_of_last_element) {
	hxarray<int, 3> a{ 1, 1, 2 };
	EXPECT_FALSE(a.all_of([](const int& x) { return x == 1; }));
	EXPECT_TRUE(a.any_of([](const int& x) { return x == 2; }));
}

TEST(hxarray_test, all_of_any_of_first_element) {
	hxarray<int, 3> a{ 2, 1, 1 };
	EXPECT_FALSE(a.all_of([](const int& x) { return x == 1; }));
	EXPECT_TRUE(a.any_of([](const int& x) { return x == 2; }));
}

TEST(hxarray_test, for_each) {
	hxarray<int, 4> a{ 10, 20, 30, 40 };
	a.for_each([](int& x) { x += 1; });
	EXPECT_EQ(a[0], 11);
	EXPECT_EQ(a[1], 21);
	EXPECT_EQ(a[2], 31);
	EXPECT_EQ(a[3], 41);
	int count = 0;
	const hxarray<int, 4>& ca = a;
	ca.for_each([&](const int&) { ++count; });
	EXPECT_EQ(count, 4);
}

TEST(hxarray_test, for_each_visits_all_including_last) {
	hxarray<int, 3> a{ 1, 2, 3 };
	int sum = 0;
	a.for_each([&](const int& x) { sum += x; });
	EXPECT_EQ(sum, 6);
}

#if HX_CPLUSPLUS >= 202002L
TEST_F(hxarray_test_f, range_constructor_from_rvalue) {
	hxtest_object source_elements[] = {
		hxtest_object(31),
		hxtest_object(32),
		hxtest_object(33)
	};
	hxarray<hxtest_object, 3> elements(hxmake_range(
		source_elements, source_elements + 3));
	EXPECT_EQ(elements[0].value(), 31);
	EXPECT_EQ(elements[1].value(), 32);
	EXPECT_EQ(elements[2].value(), 33);
	for(hxsize_t i = 0; i < 3; ++i) {
		EXPECT_EQ(elements[i].state(), hxtest_object_state::valid);
		EXPECT_EQ(source_elements[i].state(), hxtest_object_state::moved);
	}
	EXPECT_TRUE(check_stats(6, 0, 0, 3, 0, 3, 0, 0, 0, 0));
}

TEST_F(hxarray_test_f, range_constructor_from_const_lvalue) {
	const hxarray<hxtest_object, 4> src{ 31, 32, 33, 34 };
	hxrange<const hxtest_object*> range(src.begin(), src.end());
	const hxarray<hxtest_object, 4> elements(range);
	for(hxsize_t i = 0; i < 4; ++i) {
		EXPECT_EQ(elements[i].value(), src[i].value());
		EXPECT_EQ(elements[i].state(), hxtest_object_state::valid);
		EXPECT_EQ(src[i].state(), hxtest_object_state::valid);
	}
	EXPECT_TRUE(check_stats(8, 0, 0, 4, 4, 0, 0, 0, 0, 0));
}
#endif

TEST(hxarray_test, binary_search) {
	hxarray<int, 5> a{ 1, 3, 5, 7, 9 };
	const hxarray<int, 5>& ca = a;
	const int* found = ca.binary_search(7);
	EXPECT_EQ(found, a.begin() + 3);
	const int* missing = ca.binary_search(4);
	EXPECT_EQ(missing, a.end());
	const int* mfound = a.binary_search(5);
	EXPECT_EQ(mfound, a.begin() + 2);
}

TEST(hxarray_test, binary_search_first_and_last) {
	hxarray<int, 4> a{ 10, 20, 30, 40 };
	const hxarray<int, 4>& ca = a;
	EXPECT_EQ(ca.binary_search(10), a.begin());
	EXPECT_EQ(ca.binary_search(40), a.begin() + 3);
	EXPECT_EQ(ca.binary_search(9), a.end());
	EXPECT_EQ(ca.binary_search(41), a.end());
}

TEST(hxarray_test, sort) {
	hxarray<int, 6> a{ 13, -5, 7, 0, 13, 2 };
	a.sort();
	static const int expected[6] = { -5, 0, 2, 7, 13, 13 };
	for(hxsize_t i = 0; i < 6; ++i) {
		EXPECT_EQ(a[i], expected[i]);
	}
}

TEST(hxarray_test, insertion_sort) {
	hxarray<int, 6> a{ 9, 2, 7, 4, 4, 1 };
	a.insertion_sort();
	static const int expected[6] = { 1, 2, 4, 4, 7, 9 };
	for(hxsize_t i = 0; i < 6; ++i) {
		EXPECT_EQ(a[i], expected[i]);
	}
}

#if HX_CPLUSPLUS >= 202002L
TEST(hxarray_test, memcpy_and_memset) {
	static const unsigned char src[4] = { 10, 20, 30, 40 };
	const hxarray<unsigned char, 4> a(hxmake_range(src));
	hxarray<unsigned char, 4> b;
	b.memcpy(a);
	for(hxsize_t i = 0; i < 4; ++i) {
		EXPECT_EQ(b[i], src[i]);
	}
	b.memset(0xff);
	for(hxsize_t i = 0; i < 4; ++i) {
		EXPECT_EQ(static_cast<int>(b[i]), 0xff);
	}
}
#endif

#if HX_CPLUSPLUS >= 202002L
TEST(hxarray_test, equal_and_less) {
	static const int v1[3] = { 1, 2, 3 };
	const hxarray<int, 3> a(hxmake_range(v1));
	const hxarray<int, 3> b(hxmake_range(v1));
	const hxarray<int, 3> c{ 1, 2, 4 };
	EXPECT_TRUE(a.equal(b));
	EXPECT_FALSE(a.equal(c));
	EXPECT_FALSE(a.less(b));
	EXPECT_TRUE(a.less(c));
	EXPECT_FALSE(c.less(a));
	EXPECT_TRUE(hxkey_equal(a, b));
	EXPECT_FALSE(hxkey_equal(a, c));
	EXPECT_FALSE(hxkey_less(a, b));
	EXPECT_TRUE(hxkey_less(a, c));
}

TEST(hxarray_test, hxkey_hash) {
	const hxarray<int, 3> a{ 31, 32, 33 };
	const hxarray<int, 3> b{ 31, 32, 33 };
	const hxarray<int, 3> c{ 31, 33, 32 };
	const hxarray<int, 3> d{ 33, 32, 31 };
	EXPECT_EQ(a.hash(), b.hash());
	EXPECT_NE(a.hash(), c.hash());
	EXPECT_NE(a.hash(), d.hash());
	EXPECT_EQ(hxkey_hash(a), a.hash());
}
#endif

TEST_F(hxarray_test_f, reserve_dynamic) {
	{
		hxarray<hxtest_object, hxallocator_dynamic_capacity> a;
		EXPECT_EQ(a.size(), 0);
		EXPECT_EQ(a.capacity(), 0);
		a.reserve(3);
		EXPECT_EQ(a.size(), 3);
		EXPECT_EQ(a.capacity(), 3);
		for(hxsize_t i = 0; i < a.size(); ++i) {
			EXPECT_EQ(a[i].state(), hxtest_object_state::valid);
		}
		a.reserve(3);
		EXPECT_EQ(a.size(), 3);
		a[0].value() = 71;
		a[1].value() = 72;
		a[2].value() = 73;
		hxtest_gdb_break_hxarray_dynamic();
	}
	EXPECT_TRUE(check_stats(3, 3, 3, 0, 0, 0, 0, 0, 0, 0));
}

TEST(hxarray_test, unallocated_dynamic) {
	const hxarray<int, hxallocator_dynamic_capacity> a;
	EXPECT_EQ(a.size(), 0);
	EXPECT_EQ(a.capacity(), 0);
	hxtest_gdb_break_hxarray_unallocated();
}

TEST(hxarray_test, reserve_static_noop) {
	hxarray<int, 4> a;
	a.reserve(4);
	EXPECT_EQ(a.size(), 4);
}

TEST_F(hxarray_test_f, dynamic_copy_constructor) {
	{
		const hxarray<hxtest_object, hxallocator_dynamic_capacity> src{
			hxtest_object(7), hxtest_object(8)};
		const hxarray<hxtest_object, hxallocator_dynamic_capacity> dst(src); // NOLINT(performance-unnecessary-copy-initialization)
		EXPECT_EQ(dst.size(), 2);
		EXPECT_EQ(dst[0].value(), 7);
		EXPECT_EQ(dst[1].value(), 8);
		for(hxsize_t i = 0; i < 2; ++i) {
			EXPECT_EQ(dst[i].state(), hxtest_object_state::valid);
			EXPECT_EQ(src[i].state(), hxtest_object_state::valid);
		}
	}
	EXPECT_TRUE(check_stats(6, 6, 0, 2, 4, 0, 0, 0, 0, 0));
}

#if HX_CPLUSPLUS >= 202002L
TEST_F(hxarray_test_f, dynamic_c_array_constructor) {
	{
		static const int32_t values[3] = { 2, 4, 6 };
		hxarray<hxtest_object, hxallocator_dynamic_capacity> a(hxmake_range(values));
		EXPECT_EQ(a.size(), 3);
		EXPECT_EQ(a[0].value(), 2);
		EXPECT_EQ(a[1].value(), 4);
		EXPECT_EQ(a[2].value(), 6);
	}
	EXPECT_TRUE(check_stats(3, 3, 0, 3, 0, 0, 0, 0, 0, 0));
}
#endif

#if HX_CPLUSPLUS >= 202002L
TEST(hxarray_test, dynamic_initializer_list_constructor) {
	{
		hxarray<int, hxallocator_dynamic_capacity> a { 10, 20, 30 };
		EXPECT_EQ(a.size(), 3);
		EXPECT_EQ(a[0], 10);
		EXPECT_EQ(a[1], 20);
		EXPECT_EQ(a[2], 30);
	}
}
#endif

TEST(hxarray_test, equal_mismatched_capacity) {
	{
		const hxarray<int, hxallocator_dynamic_capacity> a{1, 2};
		const hxarray<int, hxallocator_dynamic_capacity> b{1, 2, 3};
		EXPECT_FALSE(a.equal(b));
	}
}

TEST(hxarray_test, less_different_sizes) {
	{
		const hxarray<int, hxallocator_dynamic_capacity> shorter{1, 2};
		const hxarray<int, hxallocator_dynamic_capacity> longer{1, 2, 3};
		EXPECT_TRUE(shorter.less(longer));
		EXPECT_FALSE(longer.less(shorter));
		EXPECT_FALSE(shorter.less(shorter));
	}
}

TEST(hxarray_test, equal_detects_last_element_difference) {
	const hxarray<int, 3> a{ 1, 2, 3 };
	const hxarray<int, 3> b{ 1, 2, 4 };
	EXPECT_FALSE(a.equal(b));
	EXPECT_FALSE(b.equal(a));
}

TEST(hxarray_test, less_detects_last_element_difference) {
	const hxarray<int, 3> a{ 1, 2, 3 };
	const hxarray<int, 3> b{ 1, 2, 4 };
	EXPECT_TRUE(a.less(b));
	EXPECT_FALSE(b.less(a));
}

TEST_F(hxarray_test_f, dynamic_move_constructor) {
	{
		hxarray<hxtest_object, hxallocator_dynamic_capacity> src{
			hxtest_object(11), hxtest_object(22), hxtest_object(33)};
		hxarray<hxtest_object, hxallocator_dynamic_capacity> dst(hxmove(src));
		EXPECT_EQ(dst.size(), 3);
		EXPECT_EQ(dst[0].value(), 11);
		EXPECT_EQ(dst[1].value(), 22);
		EXPECT_EQ(dst[2].value(), 33);
		for(hxsize_t i = 0; i < 3; ++i) {
			EXPECT_EQ(dst[i].state(), hxtest_object_state::valid);
		}
		EXPECT_EQ(src.size(), 0);
		EXPECT_EQ(src.capacity(), 0);
	}
	EXPECT_TRUE(check_stats(6, 6, 0, 3, 3, 0, 0, 0, 0, 0));
}

#if HX_CPLUSPLUS >= 202002L
TEST(hxarray_test, empty_and_full) {
	const hxarray<int, hxallocator_dynamic_capacity> a;
	EXPECT_TRUE(a.empty());

	const hxarray<int, 1> a1{ 1 };
	EXPECT_FALSE(a1.empty());
	const hxarray<int, 3> a3{ 1, 2, 3 };
	EXPECT_FALSE(a3.empty());
}

TEST(hxarray_test, swap_exchanges_dynamic_contents) {
	hxarray<int> a{ 2, 7 };
	hxarray<int> b{ 31, 32, 33 };
	a.swap(b);
	EXPECT_EQ(a.size(), 3);
	EXPECT_EQ(a[0], 31);
	EXPECT_EQ(a[1], 32);
	EXPECT_EQ(a[2], 33);
	EXPECT_EQ(b.size(), 2);
	EXPECT_EQ(b[0], 2);
	EXPECT_EQ(b[1], 7);
}

TEST_F(hxarray_test_f, hxswap_exchanges_dynamic_contents) {
	{
		hxarray<hxtest_object, hxallocator_dynamic_capacity> a{
			hxtest_object(1), hxtest_object(2)};
		hxarray<hxtest_object, hxallocator_dynamic_capacity> b{
			hxtest_object(3), hxtest_object(4), hxtest_object(5)};
		hxswap(a, b);
		EXPECT_EQ(a.size(), 3);
		EXPECT_EQ(a[0].value(), 3);
		EXPECT_EQ(a[2].value(), 5);
		EXPECT_EQ(b.size(), 2);
		EXPECT_EQ(b[0].value(), 1);
		EXPECT_EQ(b[1].value(), 2);
	}
	EXPECT_TRUE(check_stats(10, 10, 0, 5, 5, 0, 0, 0, 0, 0));
}
#endif // HX_CPLUSPLUS >= 202002L
