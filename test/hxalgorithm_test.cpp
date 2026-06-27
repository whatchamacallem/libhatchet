// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

#include <hx/hxalgorithm.hpp>
#include <hx/hxrandom.hpp>
#include <hx/hxvector.hpp>
#include <hx/hxtest.hpp>
#include "test_trackers.hpp"

HX_NS_USE

TEST(hxalgorithm_test, hxmerge_iterator_support) {
	hxtest_ref_tracker_t left[3] = { hxtest_ref_tracker_t(1), hxtest_ref_tracker_t(3), hxtest_ref_tracker_t(5) };
	hxtest_ref_tracker_t right[3] = { hxtest_ref_tracker_t(2), hxtest_ref_tracker_t(4), hxtest_ref_tracker_t(6) };
	hxtest_ref_tracker_t dest[6] = {
		hxtest_ref_tracker_t(0), hxtest_ref_tracker_t(0), hxtest_ref_tracker_t(0),
		hxtest_ref_tracker_t(0), hxtest_ref_tracker_t(0), hxtest_ref_tracker_t(0)
	};
	const hxtest_iter_api_t merge_end =
		hxmerge(hxtest_iter_api_t(left), hxtest_iter_api_t(left + 3),
			hxtest_iter_api_t(right), hxtest_iter_api_t(right + 3),
			hxtest_iter_api_t(dest), hxtest_value_less);
	EXPECT_EQ(merge_end - hxtest_iter_api_t(dest), ptrdiff_t{6});
	const int expected_sorted[6] = { 1, 2, 3, 4, 5, 6 };
	for(hxsize_t i = 0; i < 6; ++i) {
		EXPECT_EQ(dest[i].value, expected_sorted[i]);
	}
	hxtest_ref_tracker_t left_desc[3] = { hxtest_ref_tracker_t(5), hxtest_ref_tracker_t(3), hxtest_ref_tracker_t(1) };
	hxtest_ref_tracker_t right_desc[3] = { hxtest_ref_tracker_t(6), hxtest_ref_tracker_t(4), hxtest_ref_tracker_t(2) };
	hxtest_ref_tracker_t dest_desc[6] = {
		hxtest_ref_tracker_t(0), hxtest_ref_tracker_t(0), hxtest_ref_tracker_t(0),
		hxtest_ref_tracker_t(0), hxtest_ref_tracker_t(0), hxtest_ref_tracker_t(0)
	};
	const hxtest_iter_api_t merge_desc_end =
		hxmerge(hxtest_iter_api_t(left_desc), hxtest_iter_api_t(left_desc + 3),
			hxtest_iter_api_t(right_desc), hxtest_iter_api_t(right_desc + 3),
			hxtest_iter_api_t(dest_desc), hxtest_value_greater);
	EXPECT_EQ(merge_desc_end - hxtest_iter_api_t(dest_desc), ptrdiff_t{6});
	const int expected_desc[6] = { 6, 5, 4, 3, 2, 1 };
	for(hxsize_t i = 0; i < 6; ++i) {
		EXPECT_EQ(dest_desc[i].value, expected_desc[i]);
	}
}

TEST(hxalgorithm_test, hxbinary_search_iterator_support) {
	hxtest_ref_tracker_t values[7] = {
		hxtest_ref_tracker_t(-5), hxtest_ref_tracker_t(-1), hxtest_ref_tracker_t(0), hxtest_ref_tracker_t(3),
		hxtest_ref_tracker_t(5), hxtest_ref_tracker_t(8), hxtest_ref_tracker_t(12)
	};
	const hxtest_iter_api_t begin(values);
	const hxtest_iter_api_t end(values + 7);
	const hxtest_ref_tracker_t key_three(3);
	hxtest_iter_api_t result = hxbinary_search(begin, end, key_three, hxtest_value_less);
	EXPECT_NE(result, end);
	EXPECT_EQ((*result).value, 3);
	const hxtest_ref_tracker_t key_high(12);
	result = hxbinary_search(begin, end, key_high, hxtest_value_less);
	EXPECT_NE(result, end);
	EXPECT_EQ((*result).value, 12);
	const hxtest_ref_tracker_t missing(7);
	result = hxbinary_search(begin, end, missing, hxtest_value_less);
	EXPECT_EQ(result, end);
	result = hxbinary_search(begin, begin, key_three, hxtest_value_less);
	EXPECT_EQ(result, begin);
}

#if HX_CPLUSPLUS >= 201402L
TEST(hxset_algorithms_test, single_element_ranges) {
	const int left[1] = { 2 };
	const int right[1] = { 3 };
	int dest_union[2] = { };
	int dest_intersection[1] = { };
	int dest_difference[1] = { };
	const int* union_end = hxset_union(left + 0, left + 1, right + 0, right + 1, dest_union + 0);
	EXPECT_EQ(union_end - dest_union, ptrdiff_t{2});
	EXPECT_EQ(dest_union[0], 2);
	EXPECT_EQ(dest_union[1], 3);
	const int* intersection_end = hxset_intersection(left + 0, left + 1, right + 0, right + 1, dest_intersection + 0);
	EXPECT_EQ(intersection_end - dest_intersection, ptrdiff_t{0});
	const int* difference_end = hxset_difference(left + 0, left + 1, right + 0, right + 1, dest_difference + 0);
	EXPECT_EQ(difference_end - dest_difference, ptrdiff_t{1});
	EXPECT_EQ(dest_difference[0], 2);
}

TEST(hxset_algorithms_test, last_left_element_is_largest) {
	const int left[3] = { 1, 3, 9 };
	const int right[2] = { 2, 4 };
	int dest[5] = { };
	const int* union_end = hxset_union(left + 0, left + 3, right + 0, right + 2, dest + 0);
	EXPECT_EQ(union_end - dest, ptrdiff_t{5});
	EXPECT_EQ(dest[4], 9);
}

TEST(hxset_algorithms_test, last_right_element_is_largest) {
	const int left[2] = { 1, 3 };
	const int right[3] = { 2, 4, 9 };
	int dest[5] = { };
	const int* union_end = hxset_union(left + 0, left + 2, right + 0, right + 3, dest + 0);
	EXPECT_EQ(union_end - dest, ptrdiff_t{5});
	EXPECT_EQ(dest[4], 9);
}

TEST(hxset_algorithms_test, int_pointer_ranges) {
	const int left[] = { 1, 3, 5, 7 };
	const int right[] = { 3, 4, 7, 9 };
	int dest_union[8] = { };
	int dest_intersection[4] = { };
	int dest_difference[4] = { };
	auto expect_range = [](const int* begin, const int* end, const int* expected) {
		for(const int* it = begin; it != end; ++it, ++expected) {
			EXPECT_EQ(*it, *expected);
		}
	};
	const int* union_end = hxset_union(left+0, left + hxsize(left), right+0, right + hxsize(right), dest_union+0);
	const int expected_union[] = { 1, 3, 4, 5, 7, 9 };
	EXPECT_EQ(union_end - dest_union, hxsize(expected_union));
	expect_range(dest_union, union_end, expected_union);
	const int* intersection_end = hxset_intersection(left+0, left + hxsize(left), right+0, right + hxsize(right), dest_intersection+0);
	const int expected_intersection[] = { 3, 7 };
	EXPECT_EQ(intersection_end - dest_intersection, hxsize(expected_intersection));
	expect_range(dest_intersection, intersection_end, expected_intersection);
	const int* difference_end = hxset_difference(left+0, left + hxsize(left), right+0, right + hxsize(right), dest_difference+0);
	const int expected_difference[] = { 1, 5 };
	EXPECT_EQ(difference_end - dest_difference, hxsize(expected_difference));
	expect_range(dest_difference, difference_end, expected_difference);
}

TEST(hxset_algorithms_test, hxarray_output_iterator_support) {
	const hxsystem_allocator_scope temporary_stack_scope(hxsystem_allocator_stack_0);
	auto expect_hxarray = [](const hxvector<hxtest_ref_tracker_t>& actual, const int* expected, hxsize_t count) {
		ASSERT_EQ(actual.size(), count);
		for(hxsize_t i = 0; i < count; ++i) {
			EXPECT_EQ(actual[i].value, expected[i]);
		}
	};
	hxtest_ref_tracker_t left[] = { hxtest_ref_tracker_t(1), hxtest_ref_tracker_t(2), hxtest_ref_tracker_t(4) };
	hxtest_ref_tracker_t right[] = { hxtest_ref_tracker_t(2), hxtest_ref_tracker_t(4), hxtest_ref_tracker_t(5) };
	hxvector<hxtest_ref_tracker_t> merge_output;
	merge_output.reserve(hxsize(left) + hxsize(right) + 1);
	merge_output.push_back(hxtest_ref_tracker_t(0));
	{
		const hxvector<hxtest_ref_tracker_t>& merge_ret = hxmerge(left+0, left + hxsize(left),
			right+0, right + hxsize(right), merge_output);
		EXPECT_EQ(&merge_ret, &merge_output);
	}
	const int expected_merge[] = { 0, 1, 2, 2, 4, 4, 5 };
	expect_hxarray(merge_output, expected_merge, hxsize(expected_merge));
	hxvector<hxtest_ref_tracker_t> union_output;
	union_output.reserve(hxsize(left) + hxsize(right) + 1);
	union_output.push_back(hxtest_ref_tracker_t(0));
	{
		const hxvector<hxtest_ref_tracker_t>& union_ret = hxset_union(left+0, left + hxsize(left),
			right+0, right + hxsize(right), union_output);
		EXPECT_EQ(&union_ret, &union_output);
	}
	const int expected_union[] = { 0, 1, 2, 4, 5 };
	expect_hxarray(union_output, expected_union, hxsize(expected_union));
	hxvector<hxtest_ref_tracker_t> intersection_output;
	intersection_output.reserve(hxsize(left) + 1);
	intersection_output.push_back(hxtest_ref_tracker_t(0));
	{
		const hxvector<hxtest_ref_tracker_t>& intersection_ret = hxset_intersection(left+0, left + hxsize(left),
			right+0, right + hxsize(right), intersection_output);
		EXPECT_EQ(&intersection_ret, &intersection_output);
	}
	const int expected_intersection[] = { 0, 2, 4 };
	expect_hxarray(intersection_output, expected_intersection, hxsize(expected_intersection));
	hxvector<hxtest_ref_tracker_t> difference_output;
	difference_output.reserve(hxsize(left) + 1);
	difference_output.push_back(hxtest_ref_tracker_t(0));
	{
		const hxvector<hxtest_ref_tracker_t>& difference_ret = hxset_difference(left+0, left + hxsize(left),
			right+0, right + hxsize(right), difference_output);
		EXPECT_EQ(&difference_ret, &difference_output);
	}
	const int expected_difference[] = { 0, 1 };
	expect_hxarray(difference_output, expected_difference, hxsize(expected_difference));
}

TEST(hxset_algorithms_test, difference_left_tail_after_right_exhausted) {
	const int left[3] = { 1, 5, 9 };
	const int right[1] = { 5 };
	int dest[3] = { };
	const int* difference_end = hxset_difference(left + 0, left + 3, right + 0, right + 1, dest + 0);
	EXPECT_EQ(difference_end - dest, ptrdiff_t{2});
	EXPECT_EQ(dest[0], 1);
	EXPECT_EQ(dest[1], 9);
}

TEST(hxmerge_test, right_tail_appended_after_left_exhausted) {
	const int left[1] = { 1 };
	const int right[2] = { 2, 3 };
	int dest[3] = { };
	const int* merge_end = hxmerge(left + 0, left + 1, right + 0, right + 2, dest + 0);
	EXPECT_EQ(merge_end - dest, ptrdiff_t{3});
	EXPECT_EQ(dest[0], 1);
	EXPECT_EQ(dest[1], 2);
	EXPECT_EQ(dest[2], 3);
}

TEST(hxmerge_test, left_tail_appended_after_right_exhausted) {
	const int left[2] = { 1, 3 };
	const int right[1] = { 2 };
	int dest[3] = { };
	const int* merge_end = hxmerge(left + 0, left + 2, right + 0, right + 1, dest + 0);
	EXPECT_EQ(merge_end - dest, ptrdiff_t{3});
	EXPECT_EQ(dest[0], 1);
	EXPECT_EQ(dest[1], 2);
	EXPECT_EQ(dest[2], 3);
}

TEST(hxmerge_test, preserves_stable_ordering) {
	struct hxmerge_test_record_t {
		int key;
		int ticket;
		bool operator<(const hxmerge_test_record_t& other) const {
			return key < other.key;
		}
	};
	hxmerge_test_record_t left[] = {
		{ 1, 0 }, { 3, 0 }, { 5, 0 }, { 5, 1 }
	};
	hxmerge_test_record_t right[] = {
		{ 1, 1 }, { 3, 1 }, { 5, 2 }, { 7, 0 }
	};
	hxmerge_test_record_t dest[8] = { };
	const hxsize_t left_count = hxsize(left);
	const hxsize_t right_count = hxsize(right);
	const hxmerge_test_record_t* merge_end = hxmerge(left+0, left + left_count, right+0, right + right_count, dest+0);
	EXPECT_EQ(merge_end - dest, hxsize_t{8});
	const hxmerge_test_record_t expected[] = {
		{ 1, 0 }, { 1, 1 }, { 3, 0 }, { 3, 1 },
		{ 5, 0 }, { 5, 1 }, { 5, 2 }, { 7, 0 }
	};
	for(hxsize_t i = 0; i < left_count + right_count; ++i) {
		EXPECT_EQ(dest[i].key, expected[i].key);
		EXPECT_EQ(dest[i].ticket, expected[i].ticket);
	}
}

TEST(hxminmax_test, simple_case) {
	const int ints[5] = { 5, 3, 1, 4, 2 };
	const int* ints_end = ints + 5;
	hxminmax_result<const int*> result = hxminmax(ints, ints_end);
	EXPECT_NE(result.min, ints_end);
	EXPECT_EQ(*result.min, 1);
	EXPECT_NE(result.max, ints_end);
	EXPECT_EQ(*result.max, 5);
	result = hxminmax(ints, ints_end, hxkey_less_t<int>{});
	EXPECT_EQ(*result.min, 1);
	EXPECT_EQ(*result.max, 5);
	result = hxminmax(ints, ints + 1);
	EXPECT_EQ(result.min, ints);
	EXPECT_EQ(result.max, ints);
	result = hxminmax(ints, ints);
	EXPECT_EQ(result.min, ints);
	EXPECT_EQ(result.max, ints);
}

TEST(hxminmax_test, first_of_equal_minima_and_maxima) {
	const int ints[6] = { 3, 1, 2, 1, 5, 5 };
	const hxminmax_result<const int*> result = hxminmax(ints, ints + 6);
	EXPECT_EQ(result.min, ints + 1);
	EXPECT_EQ(result.max, ints + 4);
}

TEST(hxminmax_test, single_element_returns_begin_for_both) {
	const int ints[1] = { 42 };
	const hxminmax_result<const int*> result = hxminmax(ints, ints + 1);
	EXPECT_EQ(result.min, ints);
	EXPECT_EQ(result.max, ints);
}

TEST(hxminmax_test, two_elements_ascending) {
	const int ints[2] = { 7, 11 };
	const hxminmax_result<const int*> result = hxminmax(ints, ints + 2);
	EXPECT_EQ(result.min, ints);
	EXPECT_EQ(result.max, ints + 1);
}

TEST(hxminmax_test, two_elements_descending) {
	const int ints[2] = { 11, 7 };
	const hxminmax_result<const int*> result = hxminmax(ints, ints + 2);
	EXPECT_EQ(result.min, ints + 1);
	EXPECT_EQ(result.max, ints);
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

TEST(hxminmax_test, iterator_support) {
	hxtest_ref_tracker_t values[4] = { hxtest_ref_tracker_t(30), hxtest_ref_tracker_t(10), hxtest_ref_tracker_t(40), hxtest_ref_tracker_t(20) };
	const hxtest_iter_api_t begin(values);
	const hxtest_iter_api_t end(values + 4);
	const hxminmax_result<hxtest_iter_api_t> result =
		hxminmax(begin, end, hxtest_value_less);
	EXPECT_NE(result.min, end);
	EXPECT_EQ((*result.min).value, 10);
	EXPECT_NE(result.max, end);
	EXPECT_EQ((*result.max).value, 40);
	const hxminmax_result<hxtest_iter_api_t> empty_result =
		hxminmax(begin, begin, hxtest_value_less);
	EXPECT_EQ(empty_result.min, begin);
	EXPECT_EQ(empty_result.max, begin);
}

TEST(hxcount_if_test, simple_case) {
	const int ints[6] = { 1, 2, 3, 4, 5, 6 };
	const hxsize_t count = hxcount_if(ints, ints + 6, [](const int& x) { return x % 2 == 0; });
	EXPECT_EQ(count, hxsize_t{3});
	EXPECT_EQ(hxcount_if(ints, ints + 6, [](const int& x) { return x > 0; }), hxsize_t{6});
	EXPECT_EQ(hxcount_if(ints, ints + 6, [](const int& x) { return x > 10; }), hxsize_t{0});
	EXPECT_EQ(hxcount_if(ints, ints, [](const int& x) { return x == 1; }), hxsize_t{0});
}

TEST(hxcount_if_test, only_last_element_matches) {
	const int ints[4] = { 1, 3, 5, 6 };
	EXPECT_EQ(hxcount_if(ints, ints + 4, [](const int& x) { return x % 2 == 0; }), hxsize_t{1});
}

TEST(hxcount_if_test, only_first_element_matches) {
	const int ints[4] = { 6, 1, 3, 5 };
	EXPECT_EQ(hxcount_if(ints, ints + 4, [](const int& x) { return x % 2 == 0; }), hxsize_t{1});
}

TEST(hxunique_test, simple_case) {
	int ints[7] = { 1, 1, 2, 3, 3, 3, 4 };
	const int* new_end = hxunique(ints, ints + 7);
	EXPECT_EQ(new_end - ints, ptrdiff_t{4});
	EXPECT_EQ(ints[0], 1);
	EXPECT_EQ(ints[1], 2);
	EXPECT_EQ(ints[2], 3);
	EXPECT_EQ(ints[3], 4);
}

TEST(hxunique_test, all_duplicates) {
	int ints[5] = { 7, 7, 7, 7, 7 };
	const int* new_end = hxunique(ints, ints + 5);
	EXPECT_EQ(new_end - ints, ptrdiff_t{1});
	EXPECT_EQ(ints[0], 7);
	new_end = hxunique(ints, ints + 1);
	EXPECT_EQ(new_end, ints + 1);
}

TEST(hxunique_test, return_value_is_one_past_last_unique) {
	int ints[3] = { 1, 2, 3 };
	const int* new_end = hxunique(ints, ints + 3);
	EXPECT_EQ(new_end - ints, ptrdiff_t{3});
}

TEST(hxunique_test, two_element_no_duplicate) {
	int ints[2] = { 4, 5 };
	const int* new_end = hxunique(ints, ints + 2);
	EXPECT_EQ(new_end - ints, ptrdiff_t{2});
	EXPECT_EQ(ints[0], 4);
	EXPECT_EQ(ints[1], 5);
}

TEST(hxunique_test, two_element_duplicate) {
	int ints[2] = { 9, 9 };
	const int* new_end = hxunique(ints, ints + 2);
	EXPECT_EQ(new_end - ints, ptrdiff_t{1});
}

TEST(hxfind_if_test, simple_case) {
	const int ints[5] = { 2, 5, 6, 88, 99 };
	const int* ints_end = ints + 5;
	const int* result = hxfind_if(ints, ints_end, [](const int& x) { return x >= 6; });
	EXPECT_NE(result, ints_end);
	EXPECT_EQ(*result, 6);
	result = hxfind_if(ints, ints_end, [](const int& x) { return x == 2; });
	EXPECT_EQ(result, ints);
	result = hxfind_if(ints, ints_end, [](const int& x) { return x == 99; });
	EXPECT_NE(result, ints_end);
	EXPECT_EQ(*result, 99);
	result = hxfind_if(ints, ints_end, [](const int& x) { return x == 0; });
	EXPECT_EQ(result, ints_end);
	result = hxfind_if(ints, ints, [](const int& x) { return x == 2; });
	EXPECT_EQ(result, ints);
}

TEST(hxfind_if_test, iterator_support) {
	hxtest_ref_tracker_t values[4] = { hxtest_ref_tracker_t(10), hxtest_ref_tracker_t(20), hxtest_ref_tracker_t(30), hxtest_ref_tracker_t(40) };
	const hxtest_iter_api_t begin(values);
	const hxtest_iter_api_t end(values + 4);
	const hxtest_iter_api_t result =
		hxfind_if(begin, end, [](const hxtest_ref_tracker_t& x) { return x.value > 15; });
	EXPECT_NE(result, end);
	EXPECT_EQ((*result).value, 20);
	const hxtest_iter_api_t no_match =
		hxfind_if(begin, end, [](const hxtest_ref_tracker_t& x) { return x.value > 100; });
	EXPECT_EQ(no_match, end);
	const hxtest_iter_api_t empty_result =
		hxfind_if(begin, begin, [](const hxtest_ref_tracker_t& x) { return x.value == 10; });
	EXPECT_EQ(empty_result, begin);
}

TEST(hxexchange_test, move_only_type) {
	hxtest_ref_tracker_t a(42);
	const hxtest_ref_tracker_t old = hxexchange(a, hxtest_ref_tracker_t(99));
	EXPECT_EQ(old.value, 42);
	EXPECT_EQ(a.value, 99);
}

#endif // HX_CPLUSPLUS >= 201402L
