// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

#include <hx/hxarray.hpp>
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

namespace {
class hxarray_test_f : public hxtest_object_fixture { };
} // namespace

TEST_F(hxarray_test_f, hxarray_default_constructs_all_elements) {
	{
		hxarray<hxtest_object, 3> a;
		EXPECT_EQ(a.size(), 3);
		EXPECT_EQ(a.max_size(), 3);
		EXPECT_EQ(a.size_bytes(), 3 * hxsizeof<hxtest_object>());
		for(hxsize_t i = 0; i < a.size(); ++i) {
			EXPECT_FALSE(a[i].moved_from);
		}
	}
	EXPECT_TRUE(check_totals(3));
}

TEST_F(hxarray_test_f, hxarray_fill_constructor) {
	{
		const hxtest_object fill(99);
		hxarray<hxtest_object, 4> a(fill);
		EXPECT_EQ(a.size(), 4);
		for(hxsize_t i = 0; i < a.size(); ++i) {
			EXPECT_EQ(a[i].id, 99);
			EXPECT_FALSE(a[i].moved_from);
		}
	}
	EXPECT_TRUE(check_totals(5));
}

TEST_F(hxarray_test_f, hxarray_copy_constructor) {
	{
		hxarray<hxtest_object, 3> src;
		src[0] = hxtest_object(10);
		src[1] = hxtest_object(20);
		src[2] = hxtest_object(30);
		hxarray<hxtest_object, 3> dst(src);
		EXPECT_EQ(dst[0].id, 10);
		EXPECT_EQ(dst[1].id, 20);
		EXPECT_EQ(dst[2].id, 30);
		for(hxsize_t i = 0; i < dst.size(); ++i) {
			EXPECT_FALSE(dst[i].moved_from);
			EXPECT_FALSE(src[i].moved_from);
		}
	}
	EXPECT_TRUE(check_totals(9));
}

TEST_F(hxarray_test_f, hxarray_copy_assignment) {
	{
		hxarray<hxtest_object, 3> src;
		src[0] = hxtest_object(11);
		src[1] = hxtest_object(22);
		src[2] = hxtest_object(33);
		hxarray<hxtest_object, 3> dst;
		dst = src;
		EXPECT_EQ(dst[0].id, 11);
		EXPECT_EQ(dst[1].id, 22);
		EXPECT_EQ(dst[2].id, 33);
		for(hxsize_t i = 0; i < dst.size(); ++i) {
			EXPECT_FALSE(dst[i].moved_from);
		}
	}
	EXPECT_TRUE(check_totals(9));
}

TEST(hxarray_test, hxarray_c_array_constructor) {
	static const int values[4] = { 7, 14, 21, 28 };
	hxarray<int, 4> a(values);
	EXPECT_EQ(a[0], 7);
	EXPECT_EQ(a[1], 14);
	EXPECT_EQ(a[2], 21);
	EXPECT_EQ(a[3], 28);
	hxtest_gdb_break_hxarray_static();
}

TEST(hxarray_test, hxarray_c_array_assignment) {
	static const int values[3] = { 5, 10, 15 };
	hxarray<int, 3> a;
	a = values;
	EXPECT_EQ(a[0], 5);
	EXPECT_EQ(a[1], 10);
	EXPECT_EQ(a[2], 15);
}

#if HX_CPLUSPLUS >= 202002L
TEST(hxarray_test, hxarray_initializer_list_constructor) {
	hxarray<int, 4> a { 3, 6, 9, 12 };
	EXPECT_EQ(a[0], 3);
	EXPECT_EQ(a[1], 6);
	EXPECT_EQ(a[2], 9);
	EXPECT_EQ(a[3], 12);
}
#endif

TEST(hxarray_test, hxarray_iteration) {
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

TEST(hxarray_test, hxarray_get) {
	static const int values[3] = { 10, 20, 30 };
	hxarray<int, 3> a(values);
	const hxarray<int, 3>& ca = a;
	EXPECT_EQ(a.get(0), a.begin());
	EXPECT_EQ(a.get(1), a.begin() + 1);
	EXPECT_EQ(a.get(3), hxnullptr);
	EXPECT_EQ(ca.get(0), ca.begin());
	EXPECT_EQ(ca.get(2), ca.begin() + 2);
	EXPECT_EQ(ca.get(3), hxnullptr);
}

TEST(hxarray_test, hxarray_get_boundary) {
	static const int values[4] = { 1, 2, 3, 4 };
	hxarray<int, 4> a(values);
	const hxarray<int, 4>& ca = a;
	EXPECT_EQ(a.get(3), a.begin() + 3);
	EXPECT_EQ(ca.get(3), ca.begin() + 3);
	EXPECT_EQ(a.get(4), hxnullptr);
	EXPECT_EQ(ca.get(4), hxnullptr);
}

TEST(hxarray_test, hxarray_find) {
	static const int values[5] = { 2, 4, 4, 8, 16 };
	hxarray<int, 5> a(values);
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

TEST(hxarray_test, hxarray_find_last_element) {
	static const int values[4] = { 1, 2, 3, 99 };
	hxarray<int, 4> a(values);
	const hxarray<int, 4>& ca = a;
	EXPECT_EQ(ca.find(99), a.begin() + 3);
	EXPECT_EQ(*ca.find(99), 99);
	const int* fp = ca.find_if([](int v) { return v == 99; });
	EXPECT_EQ(fp, a.begin() + 3);
	static const int one[1] = { 7 };
	hxarray<int, 1> b(one);
	EXPECT_EQ(b.find(7), b.begin());
	EXPECT_EQ(b.find(8), b.end());
}

TEST(hxarray_test, hxarray_all_of_any_of) {
	static const int values[5] = { 91, 92, 93, 94, 95 };
	hxarray<int, 5> a(values);
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

TEST(hxarray_test, hxarray_all_of_any_of_last_element) {
	static const int values[3] = { 1, 1, 2 };
	hxarray<int, 3> a(values);
	EXPECT_FALSE(a.all_of([](const int& x) { return x == 1; }));
	EXPECT_TRUE(a.any_of([](const int& x) { return x == 2; }));
}

TEST(hxarray_test, hxarray_all_of_any_of_first_element) {
	static const int values[3] = { 2, 1, 1 };
	hxarray<int, 3> a(values);
	EXPECT_FALSE(a.all_of([](const int& x) { return x == 1; }));
	EXPECT_TRUE(a.any_of([](const int& x) { return x == 2; }));
}

TEST(hxarray_test, hxarray_for_each) {
	static const int values[4] = { 10, 20, 30, 40 };
	hxarray<int, 4> a(values);
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

TEST(hxarray_test, hxarray_for_each_visits_all_including_last) {
	static const int values[3] = { 1, 2, 3 };
	hxarray<int, 3> a(values);
	int sum = 0;
	a.for_each([&](const int& x) { sum += x; });
	EXPECT_EQ(sum, 6);
}

TEST(hxarray_test, hxarray_assign_from_iterators) {
	static const int src[4] = { 3, 1, 4, 1 };
	hxarray<int, 4> a;
	a.assign(src + 0, src + 4);
	EXPECT_EQ(a[0], 3);
	EXPECT_EQ(a[1], 1);
	EXPECT_EQ(a[2], 4);
	EXPECT_EQ(a[3], 1);
}

#if HX_CPLUSPLUS >= 202002L
TEST_F(hxarray_test_f, hxarray_assign_range_from_rvalue) {
	hxtest_object source_elements[] = {
		hxtest_object(5),
		hxtest_object(9),
		hxtest_object(13)
	};
	hxarray<hxtest_object, 3> elements;
	elements.assign_range(hxtest_pointer_range(
		source_elements, source_elements + 3));
	EXPECT_EQ(elements[0].id, 5);
	EXPECT_EQ(elements[1].id, 9);
	EXPECT_EQ(elements[2].id, 13);
	for(hxsize_t i = 0; i < 3; ++i) {
		EXPECT_FALSE(elements[i].moved_from);
		EXPECT_TRUE(source_elements[i].moved_from);
	}
}

TEST_F(hxarray_test_f, hxarray_assign_range_from_const) {
	static const int32_t src_ints[4] = { 4, 7, 11, 18 };
	const hxarray<hxtest_object, 4> src(src_ints);
	hxarray<hxtest_object, 4> elements;
	hxtest_pointer_range<const hxtest_object> range(src.begin(), src.end());
	elements.assign_range(range);
	for(hxsize_t i = 0; i < 4; ++i) {
		EXPECT_EQ(elements[i].id, src[i].id);
		EXPECT_FALSE(elements[i].moved_from);
		EXPECT_FALSE(src[i].moved_from);
	}
}
#endif

TEST(hxarray_test, hxarray_binary_search) {
	static const int sorted[5] = { 1, 3, 5, 7, 9 };
	hxarray<int, 5> a(sorted);
	const hxarray<int, 5>& ca = a;
	const int* found = ca.binary_search(7);
	EXPECT_EQ(found, a.begin() + 3);
	const int* missing = ca.binary_search(4);
	EXPECT_EQ(missing, a.end());
	const int* mfound = a.binary_search(5);
	EXPECT_EQ(mfound, a.begin() + 2);
}

TEST(hxarray_test, hxarray_binary_search_first_and_last) {
	static const int sorted[4] = { 10, 20, 30, 40 };
	hxarray<int, 4> a(sorted);
	const hxarray<int, 4>& ca = a;
	EXPECT_EQ(ca.binary_search(10), a.begin());
	EXPECT_EQ(ca.binary_search(40), a.begin() + 3);
	EXPECT_EQ(ca.binary_search(9), a.end());
	EXPECT_EQ(ca.binary_search(41), a.end());
}

TEST(hxarray_test, hxarray_sort) {
	static const int unsorted[6] = { 13, -5, 7, 0, 13, 2 };
	hxarray<int, 6> a(unsorted);
	a.sort();
	static const int expected[6] = { -5, 0, 2, 7, 13, 13 };
	for(hxsize_t i = 0; i < 6; ++i) {
		EXPECT_EQ(a[i], expected[i]);
	}
}

TEST(hxarray_test, hxarray_insertion_sort) {
	static const int unsorted[6] = { 9, 2, 7, 4, 4, 1 };
	hxarray<int, 6> a(unsorted);
	a.insertion_sort();
	static const int expected[6] = { 1, 2, 4, 4, 7, 9 };
	for(hxsize_t i = 0; i < 6; ++i) {
		EXPECT_EQ(a[i], expected[i]);
	}
}

TEST(hxarray_test, hxarray_memcpy_and_memset) {
	static const unsigned char src[4] = { 10, 20, 30, 40 };
	const hxarray<unsigned char, 4> a(src);
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

#if HX_CPLUSPLUS >= 202002L
TEST(hxarray_test, hxarray_equal_and_less) {
	static const int v1[3] = { 1, 2, 3 };
	static const int v2[3] = { 1, 2, 4 };
	const hxarray<int, 3> a(v1);
	const hxarray<int, 3> b(v1);
	const hxarray<int, 3> c(v2);
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
#endif

TEST_F(hxarray_test_f, hxarray_reserve_dynamic) {
	const hxsystem_allocator_scope temporary_stack_scope(hxsystem_allocator_stack_0);
	{
		hxarray<hxtest_object, hxallocator_dynamic_capacity> a;
		EXPECT_EQ(a.size(), 0);
		EXPECT_EQ(a.capacity(), 0);
		a.reserve(3);
		EXPECT_EQ(a.size(), 3);
		EXPECT_EQ(a.capacity(), 3);
		for(hxsize_t i = 0; i < a.size(); ++i) {
			EXPECT_FALSE(a[i].moved_from);
		}
		a.reserve(3);
		EXPECT_EQ(a.size(), 3);
		hxtest_gdb_break_hxarray_dynamic();
	}
	EXPECT_TRUE(check_totals(3));
}

TEST(hxarray_test, hxarray_reserve_static_noop) {
	hxarray<int, 4> a;
	a.reserve(4);
	EXPECT_EQ(a.size(), 4);
}

TEST_F(hxarray_test_f, hxarray_dynamic_copy_constructor) {
	const hxsystem_allocator_scope temporary_stack_scope(hxsystem_allocator_stack_0);
	{
		hxarray<hxtest_object, hxallocator_dynamic_capacity> src;
		src.reserve(2);
		src[0] = hxtest_object(7);
		src[1] = hxtest_object(8);
		hxarray<hxtest_object, hxallocator_dynamic_capacity> dst(src);
		EXPECT_EQ(dst.size(), 2);
		EXPECT_EQ(dst[0].id, 7);
		EXPECT_EQ(dst[1].id, 8);
		for(hxsize_t i = 0; i < 2; ++i) {
			EXPECT_FALSE(dst[i].moved_from);
			EXPECT_FALSE(src[i].moved_from);
		}
	}
}

TEST_F(hxarray_test_f, hxarray_dynamic_c_array_constructor) {
	const hxsystem_allocator_scope temporary_stack_scope(hxsystem_allocator_stack_0);
	{
		static const int32_t values[3] = { 2, 4, 6 };
		hxarray<hxtest_object, hxallocator_dynamic_capacity> a(values);
		EXPECT_EQ(a.size(), 3);
		EXPECT_EQ(a[0].id, 2);
		EXPECT_EQ(a[1].id, 4);
		EXPECT_EQ(a[2].id, 6);
	}
}

#if HX_CPLUSPLUS >= 202002L
TEST_F(hxarray_test_f, hxarray_dynamic_initializer_list_constructor) {
	const hxsystem_allocator_scope temporary_stack_scope(hxsystem_allocator_stack_0);
	{
		hxarray<int, hxallocator_dynamic_capacity> a { 10, 20, 30 };
		EXPECT_EQ(a.size(), 3);
		EXPECT_EQ(a[0], 10);
		EXPECT_EQ(a[1], 20);
		EXPECT_EQ(a[2], 30);
	}
}
#endif

TEST_F(hxarray_test_f, hxarray_dynamic_assign_from_iterators) {
	const hxsystem_allocator_scope temporary_stack_scope(hxsystem_allocator_stack_0);
	{
		hxtest_object src[3] = { hxtest_object(3), hxtest_object(6), hxtest_object(9) };
		hxarray<hxtest_object, hxallocator_dynamic_capacity> a;
		a.assign(src + 0, src + 3);
		EXPECT_EQ(a.size(), 3);
		EXPECT_EQ(a[0].id, 3);
		EXPECT_EQ(a[1].id, 6);
		EXPECT_EQ(a[2].id, 9);
		hxtest_object src2[3] = { hxtest_object(30), hxtest_object(60), hxtest_object(90) };
		a.assign(src2 + 0, src2 + 3);
		EXPECT_EQ(a.size(), 3);
		EXPECT_EQ(a[0].id, 30);
		EXPECT_EQ(a[1].id, 60);
		EXPECT_EQ(a[2].id, 90);
	}
}

#if HX_CPLUSPLUS >= 202002L
TEST_F(hxarray_test_f, hxarray_dynamic_assign_range_rvalue) {
	const hxsystem_allocator_scope temporary_stack_scope(hxsystem_allocator_stack_0);
	{
		hxtest_object source_elements[] = {
			hxtest_object(1),
			hxtest_object(2)
		};
		hxarray<hxtest_object, hxallocator_dynamic_capacity> a;
		a.assign_range(hxtest_pointer_range(
			source_elements, source_elements + 2));
		EXPECT_EQ(a.size(), 2);
		EXPECT_EQ(a[0].id, 1);
		EXPECT_EQ(a[1].id, 2);
		for(hxsize_t i = 0; i < 2; ++i) {
			EXPECT_FALSE(a[i].moved_from);
			EXPECT_TRUE(source_elements[i].moved_from);
		}
		hxtest_object more_elements[] = {
			hxtest_object(10),
			hxtest_object(20)
		};
		a.assign_range(hxtest_pointer_range(
			more_elements, more_elements + 2));
		EXPECT_EQ(a.size(), 2);
		EXPECT_EQ(a[0].id, 10);
		EXPECT_EQ(a[1].id, 20);
		for(hxsize_t i = 0; i < 2; ++i) {
			EXPECT_FALSE(a[i].moved_from);
			EXPECT_TRUE(more_elements[i].moved_from);
		}
	}
}
#endif

TEST(hxarray_test, hxarray_equal_mismatched_capacity) {
	const hxsystem_allocator_scope temporary_stack_scope(hxsystem_allocator_stack_0);
	{
		hxarray<int, hxallocator_dynamic_capacity> a;
		hxarray<int, hxallocator_dynamic_capacity> b;
		a.reserve(2);
		b.reserve(3);
		a[0] = 1; a[1] = 2;
		b[0] = 1; b[1] = 2; b[2] = 3;
		EXPECT_FALSE(a.equal(b));
	}
}

TEST(hxarray_test, hxarray_less_different_sizes) {
	const hxsystem_allocator_scope temporary_stack_scope(hxsystem_allocator_stack_0);
	{
		hxarray<int, hxallocator_dynamic_capacity> shorter;
		hxarray<int, hxallocator_dynamic_capacity> longer;
		shorter.reserve(2);
		longer.reserve(3);
		shorter[0] = 1; shorter[1] = 2;
		longer[0] = 1; longer[1] = 2; longer[2] = 3;
		EXPECT_TRUE(shorter.less(longer));
		EXPECT_FALSE(longer.less(shorter));
		EXPECT_FALSE(shorter.less(shorter));
	}
}

TEST(hxarray_test, hxarray_equal_detects_last_element_difference) {
	static const int v1[3] = { 1, 2, 3 };
	static const int v2[3] = { 1, 2, 4 };
	const hxarray<int, 3> a(v1);
	const hxarray<int, 3> b(v2);
	EXPECT_FALSE(a.equal(b));
	EXPECT_FALSE(b.equal(a));
}

TEST(hxarray_test, hxarray_less_detects_last_element_difference) {
	static const int v1[3] = { 1, 2, 3 };
	static const int v2[3] = { 1, 2, 4 };
	const hxarray<int, 3> a(v1);
	const hxarray<int, 3> b(v2);
	EXPECT_TRUE(a.less(b));
	EXPECT_FALSE(b.less(a));
}

TEST_F(hxarray_test_f, hxarray_dynamic_move_constructor) {
	const hxsystem_allocator_scope temporary_stack_scope(hxsystem_allocator_stack_0);
	{
		hxarray<hxtest_object, hxallocator_dynamic_capacity> src;
		src.reserve(3);
		src[0] = hxtest_object(11);
		src[1] = hxtest_object(22);
		src[2] = hxtest_object(33);
		hxarray<hxtest_object, hxallocator_dynamic_capacity> dst(hxmove(src));
		EXPECT_EQ(dst.size(), 3);
		EXPECT_EQ(dst[0].id, 11);
		EXPECT_EQ(dst[1].id, 22);
		EXPECT_EQ(dst[2].id, 33);
		for(hxsize_t i = 0; i < 3; ++i) {
			EXPECT_FALSE(dst[i].moved_from);
		}
		EXPECT_EQ(src.size(), 0);
		EXPECT_EQ(src.capacity(), 0);
	}
	EXPECT_EQ(m_constructed, m_destructed);
}

#if HX_CPLUSPLUS >= 202002L
TEST_F(hxarray_test_f, hxswap_exchanges_dynamic_contents) {
	const hxsystem_allocator_scope temporary_stack_scope(hxsystem_allocator_stack_0);
	{
		hxarray<hxtest_object, hxallocator_dynamic_capacity> a;
		a.reserve(2);
		a[0] = hxtest_object(1);
		a[1] = hxtest_object(2);
		hxarray<hxtest_object, hxallocator_dynamic_capacity> b;
		b.reserve(3);
		b[0] = hxtest_object(3);
		b[1] = hxtest_object(4);
		b[2] = hxtest_object(5);
		hxswap(a, b);
		EXPECT_EQ(a.size(), 3);
		EXPECT_EQ(a[0].id, 3);
		EXPECT_EQ(a[2].id, 5);
		EXPECT_EQ(b.size(), 2);
		EXPECT_EQ(b[0].id, 1);
		EXPECT_EQ(b[1].id, 2);
	}
	EXPECT_EQ(m_constructed, m_destructed);
}
#endif // HX_CPLUSPLUS >= 202002L
