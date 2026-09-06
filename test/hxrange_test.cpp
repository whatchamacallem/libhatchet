// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

#include <hx/hxrange.hpp>
#include <hx/hxflat_map.hpp>
#include <hx/hxrandom.hpp>
#include <hx/hxsort.hpp>
#include <hx/hxtest.hpp>
#include <hx/hxvector.hpp>
#include "./hxtest_util.hpp"

HX_NS_USE

using hxbinary_search_test_f = hxtest_object_fixture;
using hxcount_if_test_f = hxtest_object_fixture;
using hxequal_range_test_f = hxtest_object_fixture;
using hxexchange_test_f = hxtest_object_fixture;
using hxfind_if_test_f = hxtest_object_fixture;
using hxquantifier_test_f = hxtest_object_fixture;
using hxtest_test_f = hxtest_object_fixture;

TEST_F(hxbinary_search_test_f, iterator_support) {
	hxvector<hxtest_object, 7> values{ -5, -1, 0, 3, 5, 8, 12 };
	const hxtest_rand_iterator_api_t begin(values.data());
	const hxtest_rand_iterator_api_t end(values.data() + 7);
	const hxtest_object key_three(3);
	hxtest_rand_iterator_api_t result = hxbinary_search(begin, end, key_three, hxtest_value_less);
	EXPECT_NE(result, end);
	EXPECT_EQ((*result).value(), 3);
	const hxtest_object key_high(12);
	result = hxbinary_search(begin, end, key_high, hxtest_value_less);
	EXPECT_NE(result, end);
	EXPECT_EQ((*result).value(), 12);
	const hxtest_object missing(7);
	result = hxbinary_search(begin, end, missing, hxtest_value_less);
	EXPECT_EQ(result, end);
	result = hxbinary_search(begin, begin, key_three, hxtest_value_less);
	EXPECT_EQ(result, begin);
	EXPECT_TRUE(check_stats(10, 0, 0, 10, 0, 0, 0, 0, 0, 12));
}

TEST(hxbinary_search_test, two_element_boundary) {
	const int values[2] = { 3, 7 };
	const int* found = hxbinary_search(values + 0, values + 2, 3, hxkey_less_t<int>{});
	EXPECT_NE(found, values + 2);
	EXPECT_EQ(*found, 3);
	found = hxbinary_search(values + 0, values + 2, 7, hxkey_less_t<int>{});
	EXPECT_NE(found, values + 2);
	EXPECT_EQ(*found, 7);
	found = hxbinary_search(values + 0, values + 2, 5, hxkey_less_t<int>{});
	EXPECT_EQ(found, values + 2);
}

TEST(hxbinary_search_test, range_overload_with_less) {
	const int values[3] = { 1, 3, 5 };
	const int* found = hxbinary_search(hxmake_range(values + 0, values + 3), 3, hxkey_less_t<int>{});
	EXPECT_NE(found, values + 3);
	EXPECT_EQ(*found, 3);
	found = hxbinary_search(hxmake_range(values + 0, values + 3), 4, hxkey_less_t<int>{});
	EXPECT_EQ(found, values + 3);
}

TEST(hxbinary_search_test, simple_case) {
	int ints[5] = { 2, 5, 6, 88, 99 };
	const int* const ints_end = ints+5;
	const int* result = hxbinary_search(hxmake_range(ints, ints+5), 88, hxkey_less_t<int>{});
	EXPECT_TRUE(result != ints_end && *result == 88);
	const int* const_ints = ints;
	const int* cresult = hxbinary_search(hxmake_range(const_ints, const_ints + 5), 2, hxkey_less_t<int>{});
	EXPECT_TRUE(cresult != ints_end && *cresult == 2);
	cresult = hxbinary_search(hxmake_range(const_ints, const_ints + 5), 99);
	EXPECT_TRUE(cresult != ints_end && *cresult == 99);
	result = hxbinary_search(hxmake_range(ints, ints+5), 0);
	EXPECT_EQ(result, ints_end);
	result = hxbinary_search(hxmake_range(ints, ints+5), 100);
	EXPECT_EQ(result, ints_end);
	result = hxbinary_search(hxmake_range(ints, ints+5), 7);
	EXPECT_EQ(result, ints_end);
	result = hxbinary_search(hxmake_range(ints, ints), 11, hxkey_less_t<int>{});
	EXPECT_EQ(result, ints);
}

TEST(hxbinary_search_test, single_element_hit_and_miss) {
	const int arr[1] = { 34 };
	const int* result = hxbinary_search(hxmake_range(arr, arr + 1), 34);
	EXPECT_EQ(result, arr);
	result = hxbinary_search(hxmake_range(arr, arr + 1), 40);
	EXPECT_EQ(result, arr + 1);
	result = hxbinary_search(hxmake_range(arr, arr + 1), 50);
	EXPECT_EQ(result, arr + 1);
}

TEST(hxbinary_search_test, two_element_boundaries) {
	const int arr[2] = { 10, 20 };
	const int* result = hxbinary_search(hxmake_range(arr, arr + 2), 10);
	EXPECT_EQ(result, arr);
	result = hxbinary_search(hxmake_range(arr, arr + 2), 20);
	EXPECT_EQ(result, arr + 1);
	result = hxbinary_search(hxmake_range(arr, arr + 2), 15);
	EXPECT_EQ(result, arr + 2);
}

TEST(hxbinary_search_test, last_element_in_five_element_array) {
	const int arr[5] = { 1, 3, 5, 7, 9 };
	const int* result = hxbinary_search(hxmake_range(arr, arr + 5), 9);
	EXPECT_EQ(result, arr + 4);
	result = hxbinary_search(hxmake_range(arr, arr + 5), 1);
	EXPECT_EQ(result, arr);
}

TEST_F(hxbinary_search_test_f, binary_search_grinder) {
	{
		hxrandom rng(4);
		hxvector<hxtest_object> sorted; sorted.reserve(100);
			for(int i=100; i-- != 0; ) {
				const int x = rng.range(0, 100);
			sorted.push_back(hxtest_object(x));
		}
		hxsort(sorted.begin(), sorted.end());
			for(hxsize_t i=100; i-- != 0; ) {
			const hxtest_object t = sorted[i];
			const hxtest_object* const ptr = hxbinary_search(sorted, t);
			EXPECT_TRUE(!(*ptr < t) && !(t < *ptr));
		}
	}
	EXPECT_TRUE(check_stats(485, 485, 0, 100, 100, 285, 0, 627, 0, 1729));
}

TEST_F(hxcount_if_test_f, simple_case) {
	hxvector<hxtest_object, 6> values{ 1, 2, 3, 4, 5, 6 };
	const hxtest_forward_iterator_api_t begin(values.data());
	const hxtest_forward_iterator_api_t end(values.data() + 6);
	const hxsize_t count = hxcount_if(hxmake_range(begin, end),
		[](const hxtest_object& x) { return x.value() % 2 == 0; });
	EXPECT_EQ(count, hxsize_t{3});
	EXPECT_EQ(hxcount_if(hxmake_range(begin, end),
		[](const hxtest_object& x) { return x.value() > 0; }), hxsize_t{6});
	EXPECT_EQ(hxcount_if(hxmake_range(begin, end),
		[](const hxtest_object& x) { return x.value() > 10; }), hxsize_t{0});
	EXPECT_EQ(hxcount_if(hxmake_range(begin, begin),
		[](const hxtest_object& x) { return x.value() == 1; }), hxsize_t{0});
	EXPECT_TRUE(check_stats(6, 0, 0, 6, 0, 0, 0, 0, 0, 0));
}

TEST(hxcount_if_test, boundary_matches) {
	const int last_matches[4] = { 1, 3, 5, 6 };
	EXPECT_EQ(hxcount_if(hxmake_range(last_matches, last_matches + 4),
		[](const int& x) { return x % 2 == 0; }), hxsize_t{1});
	const int first_matches[4] = { 6, 1, 3, 5 };
	EXPECT_EQ(hxcount_if(hxmake_range(first_matches, first_matches + 4),
		[](const int& x) { return x % 2 == 0; }), hxsize_t{1});
}

TEST_F(hxequal_range_test_f, forward_iterator_equal_and_unequal) {
	hxvector<hxtest_object, 4> values0{ 1, 3, 5, 7 };
	hxvector<hxtest_object, 4> values1{ 1, 3, 5, 7 };
	const hxtest_forward_iterator_api_t begin0(values0.data());
	const hxtest_forward_iterator_api_t end0(values0.data() + 4);
	const hxtest_forward_iterator_api_t begin1(values1.data());
	const hxtest_forward_iterator_api_t end1(values1.data() + 4);
	EXPECT_TRUE(hxequal_range(hxmake_range(begin0, end0), hxmake_range(begin1, end1), hxtest_value_equal));

	hxvector<hxtest_object, 4> values2{ 1, 3, 6, 7 };
	const hxtest_forward_iterator_api_t begin2(values2.data());
	const hxtest_forward_iterator_api_t end2(values2.data() + 4);
	EXPECT_FALSE(hxequal_range(hxmake_range(begin0, end0), hxmake_range(begin2, end2), hxtest_value_equal));

	hxvector<hxtest_object, 3> values3{ 1, 3, 5 };
	const hxtest_forward_iterator_api_t begin3(values3.data());
	const hxtest_forward_iterator_api_t end3(values3.data() + 3);
	EXPECT_FALSE(hxequal_range(hxmake_range(begin0, end0), hxmake_range(begin3, end3), hxtest_value_equal));
	EXPECT_TRUE(check_stats(15, 0, 0, 15, 0, 0, 0, 0, 10, 0));
}

TEST_F(hxexchange_test_f, move_only_type) {
	hxtest_object a(34);
	const hxtest_object old = hxexchange(a, hxtest_object(99));
	EXPECT_EQ(old.value(), 34);
	EXPECT_EQ(a.value(), 99);
	EXPECT_TRUE(check_stats(4, 2, 0, 2, 0, 2, 0, 1, 0, 0));
}

TEST(hxfind_if_test, simple_case) {
	const int ints[5] = { 2, 5, 6, 88, 99 };
	const int* ints_end = ints + 5;
	const int* result = hxfind_if(hxmake_range(ints, ints_end), [](const int& x) { return x >= 6; });
	EXPECT_NE(result, ints_end);
	EXPECT_EQ(*result, 6);
	result = hxfind_if(hxmake_range(ints, ints_end), [](const int& x) { return x == 2; });
	EXPECT_EQ(result, ints);
	result = hxfind_if(hxmake_range(ints, ints_end), [](const int& x) { return x == 99; });
	EXPECT_NE(result, ints_end);
	EXPECT_EQ(*result, 99);
	result = hxfind_if(hxmake_range(ints, ints_end), [](const int& x) { return x == 0; });
	EXPECT_EQ(result, ints_end);
	result = hxfind_if(hxmake_range(ints, ints), [](const int& x) { return x == 2; });
	EXPECT_EQ(result, ints);
}

TEST_F(hxfind_if_test_f, iterator_support) {
	hxvector<hxtest_object, 4> values{ 10, 20, 30, 40 };
	const hxtest_forward_iterator_api_t begin(values.data());
	const hxtest_forward_iterator_api_t end(values.data() + 4);
	const hxtest_forward_iterator_api_t result =
		hxfind_if(hxmake_range(begin, end),
			[](const hxtest_object& x) { return x.value() > 15; });
	EXPECT_NE(result, end);
	EXPECT_EQ((*result).value(), 20);
	const hxtest_forward_iterator_api_t no_match =
		hxfind_if(hxmake_range(begin, end),
			[](const hxtest_object& x) { return x.value() > 100; });
	EXPECT_EQ(no_match, end);
	const hxtest_forward_iterator_api_t empty_result =
		hxfind_if(hxmake_range(begin, begin),
			[](const hxtest_object& x) { return x.value() == 10; });
	EXPECT_EQ(empty_result, begin);
	EXPECT_TRUE(check_stats(4, 0, 0, 4, 0, 0, 0, 0, 0, 0));
}

TEST(hxfind_if_test, flat_map_iterator) {
	using map_t = hxflat_map<int, int, hxkey_less_t<int>, false, 4>;
	map_t m;
	m.insert(1, 10);
	m.insert(2, 20);
	m.insert(3, 30);
	map_t::const_iterator result = hxfind_if(hxmake_range(m.cbegin(), m.cend()),
		[](const hxflat_map_const_value_t<int, int>& v) { return v.value >= 20; });
	EXPECT_NE(result, m.end());
	EXPECT_EQ(result.key(), 2);
	result = hxfind_if(hxmake_range(m.cbegin(), m.cend()),
		[](const hxflat_map_const_value_t<int, int>& v) { return v.key == 1; });
	EXPECT_EQ(result, m.begin());
	result = hxfind_if(hxmake_range(m.cbegin(), m.cend()),
		[](const hxflat_map_const_value_t<int, int>& v) { return v.value == 30; });
	EXPECT_NE(result, m.end());
	EXPECT_EQ(result.key(), 3);
	result = hxfind_if(hxmake_range(m.cbegin(), m.cend()),
		[](const hxflat_map_const_value_t<int, int>& v) { return v.value == 99; });
	EXPECT_EQ(result, m.end());
	result = hxfind_if(hxmake_range(m.cbegin(), m.cbegin()),
		[](const hxflat_map_const_value_t<int, int>& v) { return v.value == 10; });
	EXPECT_EQ(result, m.begin());
}

TEST_F(hxquantifier_test_f, all_of_any_of_and_for_each) {
	hxvector<hxtest_object, 3> values{ 10, 20, 30 };
	const hxtest_forward_iterator_api_t begin(values.data());
	const hxtest_forward_iterator_api_t end(values.data() + 3);
	EXPECT_TRUE(hxall_of(hxmake_range(begin, end), [](const hxtest_object& x) { return x.value() >= 10; }));
	EXPECT_FALSE(hxall_of(hxmake_range(begin, end), [](const hxtest_object& x) { return x.value() >= 30; }));
	EXPECT_TRUE(hxall_of(hxmake_range(begin, begin), [](const hxtest_object& x) { return x.value() >= 100; }));
	EXPECT_TRUE(hxany_of(hxmake_range(begin, end), [](const hxtest_object& x) { return x.value() == 30; }));
	EXPECT_FALSE(hxany_of(hxmake_range(begin, end), [](const hxtest_object& x) { return x.value() == 99; }));
	EXPECT_FALSE(hxany_of(hxmake_range(begin, begin), [](const hxtest_object& x) { return x.value() == 10; }));
	struct hxfor_each_test_accumulator_t {
		int total;
		void operator()(const hxtest_object& x) { total += x.value(); }
	};
	const hxfor_each_test_accumulator_t result =
		hxfor_each(hxmake_range(begin, end), hxfor_each_test_accumulator_t{0});
	EXPECT_EQ(result.total, 60);
	const hxfor_each_test_accumulator_t empty_result =
		hxfor_each(hxmake_range(begin, begin), hxfor_each_test_accumulator_t{7});
	EXPECT_EQ(empty_result.total, 7);
	EXPECT_TRUE(check_stats(3, 0, 0, 3, 0, 0, 0, 0, 0, 0));
}

TEST_F(hxtest_test_f, iter_api_types) {
	hxvector<hxtest_object, 2> values{ 11, 22 };
	EXPECT_TRUE(hxtest_check_forward_iterator_api(
		hxtest_forward_iterator_api_t(values.data()), hxtest_forward_iterator_api_t(values.data() + 2)));
	EXPECT_TRUE(hxtest_check_forward_iterator_api(
		hxtest_bidirectional_iterator_api_t(values.data()), hxtest_bidirectional_iterator_api_t(values.data() + 2)));
	EXPECT_TRUE(hxtest_check_bidirectional_iterator_api(
		hxtest_bidirectional_iterator_api_t(values.data()), hxtest_bidirectional_iterator_api_t(values.data() + 2)));
	EXPECT_TRUE(hxtest_check_forward_iterator_api(
		hxtest_rand_iterator_api_t(values.data()), hxtest_rand_iterator_api_t(values.data() + 2)));
	EXPECT_TRUE(hxtest_check_bidirectional_iterator_api(
		hxtest_rand_iterator_api_t(values.data()), hxtest_rand_iterator_api_t(values.data() + 2)));
	EXPECT_TRUE(hxtest_check_rand_iterator_api(
		hxtest_rand_iterator_api_t(values.data()), hxtest_rand_iterator_api_t(values.data() + 2)));
	EXPECT_TRUE(check_stats(2, 0, 0, 2, 0, 0, 0, 0, 2, 0));
}

TEST(hxrange_test, constructors) {
	int array[3] = { 31, 32, 33 };
	const hxrange<int*> from_array(array);
	EXPECT_EQ(from_array.begin(), array + 0);
	EXPECT_EQ(from_array.end(), array + 3);
	const hxrange<int*> from_signed_length(array, hxsize_t{3});
	EXPECT_EQ(from_signed_length.begin(), array + 0);
	EXPECT_EQ(from_signed_length.end(), array + 3);
	const hxrange<int*> from_unsigned_length(array, size_t{3});
	EXPECT_EQ(from_unsigned_length.begin(), array + 0);
	EXPECT_EQ(from_unsigned_length.end(), array + 3);
}
