// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

#include <hx/hxalgorithm.hpp>
#include <hx/hxrandom.hpp>
#include <hx/hxarray.hpp>
#include <hx/hxtest.hpp>
#include "test_api_trackers.hpp"

TEST(hxhxalgorithm_test, hxmerge_iterator_support) {
	hxtest_ref_tracker_t left[3] = { hxtest_ref_tracker_t(1), hxtest_ref_tracker_t(3), hxtest_ref_tracker_t(5) };
	hxtest_ref_tracker_t right[3] = { hxtest_ref_tracker_t(2), hxtest_ref_tracker_t(4), hxtest_ref_tracker_t(6) };
	hxtest_ref_tracker_t dest[6] = {
		hxtest_ref_tracker_t(0), hxtest_ref_tracker_t(0), hxtest_ref_tracker_t(0),
		hxtest_ref_tracker_t(0), hxtest_ref_tracker_t(0), hxtest_ref_tracker_t(0)
	};

	// "Performs a stable merge sort of two ordered ranges [begin0, end0) and"
	// "[begin1, end1) -> output."
	//   /-\ ascending merge: { 1, 3, 5 } + { 2, 4, 6 } => { 1, 2, 3, 4, 5, 6 }
	const hxtest_iter_api_t merge_end =
		hxmerge(hxtest_iter_api_t(left), hxtest_iter_api_t(left + 3),
			hxtest_iter_api_t(right), hxtest_iter_api_t(right + 3),
			hxtest_iter_api_t(dest), hxtest_value_less);

	// "Returns an output iterator positioned one past the last element written."
	//   6 elements written so offset must be 6.
	EXPECT_EQ(merge_end - hxtest_iter_api_t(dest), ptrdiff_t{6});

	const int expected_sorted[6] = { 1, 2, 3, 4, 5, 6 };
	// Confirm merged buffer now tracks { 1, 2, 3, 4, 5, 6 } without disturbing tickets.
	for(size_t i = 0; i < 6; ++i) {
		EXPECT_EQ(dest[i].value, expected_sorted[i]);
	}

	// Do it all over again with a GE callable and the parameters reversed.
	hxtest_ref_tracker_t left_desc[3] = { hxtest_ref_tracker_t(5), hxtest_ref_tracker_t(3), hxtest_ref_tracker_t(1) };
	hxtest_ref_tracker_t right_desc[3] = { hxtest_ref_tracker_t(6), hxtest_ref_tracker_t(4), hxtest_ref_tracker_t(2) };
	hxtest_ref_tracker_t dest_desc[6] = {
		hxtest_ref_tracker_t(0), hxtest_ref_tracker_t(0), hxtest_ref_tracker_t(0),
		hxtest_ref_tracker_t(0), hxtest_ref_tracker_t(0), hxtest_ref_tracker_t(0)
	};

	// "Assumes both [begin0, end0) and [begin1, end1) are ordered by the less callable."
	//   \-/ descending merge inputs { 5, 3, 1 } & { 6, 4, 2 } with greater-than
	//   ensure stable output { 6, 5, 4, 3, 2, 1 }
	const hxtest_iter_api_t merge_desc_end =
		hxmerge(hxtest_iter_api_t(left_desc), hxtest_iter_api_t(left_desc + 3),
			hxtest_iter_api_t(right_desc), hxtest_iter_api_t(right_desc + 3),
			hxtest_iter_api_t(dest_desc), hxtest_value_greater);

	// 6 elements written so offset must be 6.
	EXPECT_EQ(merge_desc_end - hxtest_iter_api_t(dest_desc), ptrdiff_t{6});

	const int expected_desc[6] = { 6, 5, 4, 3, 2, 1 };
	// Ensure reverse-ordered comparison places tickets into { 6, 5, 4, 3, 2, 1 } without swaps.
	for(size_t i = 0; i < 6; ++i) {
		EXPECT_EQ(dest_desc[i].value, expected_desc[i]);
	}
}

TEST(hxhxalgorithm_test, hxbinary_search_iterator_support) {
	hxtest_ref_tracker_t values[7] = {
		hxtest_ref_tracker_t(-5), hxtest_ref_tracker_t(-1), hxtest_ref_tracker_t(0), hxtest_ref_tracker_t(3),
		hxtest_ref_tracker_t(5), hxtest_ref_tracker_t(8), hxtest_ref_tracker_t(12)
	};

	const hxtest_iter_api_t begin(values);
	const hxtest_iter_api_t end(values + 7);

	// "Performs a binary search in the range [first, last)." Confirm hits stay
	// in-bounds for { 3, 12 } without aliasing iterators.
	const hxtest_ref_tracker_t key_three(3);
	hxtest_iter_api_t result = hxbinary_search(begin, end, key_three, hxtest_value_less);
	EXPECT_NE(result, end);
	EXPECT_EQ((*result).value, 3);

	const hxtest_ref_tracker_t key_high(12);
	result = hxbinary_search(begin, end, key_high, hxtest_value_less);
	EXPECT_NE(result, end);
	EXPECT_EQ((*result).value, 12);

	// "Returns end if the value is not found." Validate misses { 7 } including
	// the degenerate case -> empty range.
	const hxtest_ref_tracker_t missing(7);
	result = hxbinary_search(begin, end, missing, hxtest_value_less);
	EXPECT_EQ(result, end);

	// Empty list.
	result = hxbinary_search(begin, begin, key_three, hxtest_value_less);
	EXPECT_EQ(result, begin);
}

#if HX_CPLUSPLUS >= 201402L // C++14 only.

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

	// "Forms the union of two ordered ranges [begin0, end0) and [begin1, end1) into output."
	//   [1 3 5 7] | [3 4 7 9] => { 1, 3, 4, 5, 7, 9 }
	int* union_end = hxset_union(left+0, left + hxsize(left), right+0, right + hxsize(right), dest_union+0);
	const int expected_union[] = { 1, 3, 4, 5, 7, 9 };
	EXPECT_EQ(union_end - dest_union, (ptrdiff_t)hxsize(expected_union));
	expect_range(dest_union, union_end, expected_union);

	// "Only keys present in both ranges appear in the output."
	//   [1 3 5 7] & [3 4 7 9] => { 3, 7 }
	int* intersection_end = hxset_intersection(left+0, left + hxsize(left), right+0, right + hxsize(right), dest_intersection+0);
	const int expected_intersection[] = { 3, 7 };
	EXPECT_EQ(intersection_end - dest_intersection, (ptrdiff_t)hxsize(expected_intersection));
	expect_range(dest_intersection, intersection_end, expected_intersection);

	// "The output contains keys that appear in the first range but not the second."
	//   [1 3 5 7] - [3 4 7 9] => { 1, 5 }
	int* difference_end = hxset_difference(left+0, left + hxsize(left), right+0, right + hxsize(right), dest_difference+0);
	const int expected_difference[] = { 1, 5 };
	EXPECT_EQ(difference_end - dest_difference, (ptrdiff_t)hxsize(expected_difference));
	expect_range(dest_difference, difference_end, expected_difference);
}

TEST(hxset_algorithms_test, hxarray_output_iterator_support) {
	const hxsystem_allocator_scope temporary_stack_scope(hxsystem_allocator_temporary_stack);

	auto expect_hxarray = [](const hxarray<hxtest_ref_tracker_t>& actual, const int* expected, size_t count) {
		ASSERT_EQ(actual.size(), count);
		for(size_t i = 0; i < count; ++i) {
			EXPECT_EQ(actual[i].value, expected[i]);
		}
	};

	hxtest_ref_tracker_t left[] = { hxtest_ref_tracker_t(1), hxtest_ref_tracker_t(2), hxtest_ref_tracker_t(4) };
	hxtest_ref_tracker_t right[] = { hxtest_ref_tracker_t(2), hxtest_ref_tracker_t(4), hxtest_ref_tracker_t(5) };

	// hxmerge
	hxarray<hxtest_ref_tracker_t> merge_output;
	merge_output.reserve(hxsize(left) + hxsize(right) + 1u);
	merge_output.push_back(hxtest_ref_tracker_t(0));
	// "Passing a hxarray as an output iterator ... will append to the array."
	//   seed { 0 } then merge -> { 0, 1, 2, 2, 4, 4, 5 }
	{
		const hxarray<hxtest_ref_tracker_t>& merge_ret = hxmerge(left+0, left + hxsize(left),
			right+0, right + hxsize(right), merge_output);
		// "Returns an output iterator positioned one past the last element written."
		//   Returned reference must be the same array that was passed in.
		EXPECT_EQ(&merge_ret, &merge_output);
	}
	const int expected_merge[] = { 0, 1, 2, 2, 4, 4, 5 };
	expect_hxarray(merge_output, expected_merge, hxsize(expected_merge));

	// hxset_union
	hxarray<hxtest_ref_tracker_t> union_output;
	union_output.reserve(hxsize(left) + hxsize(right) + 1u);
	union_output.push_back(hxtest_ref_tracker_t(0));
	// "Passing a hxarray as an output iterator ... will append to the array."
	//   union payload extends { 0 } => { 0, 1, 2, 4, 5 }
	{
		const hxarray<hxtest_ref_tracker_t>& union_ret = hxset_union(left+0, left + hxsize(left),
			right+0, right + hxsize(right), union_output);
		// "Returns an output iterator positioned one past the last element written."
		//   Returned reference must be the same array that was passed in.
		EXPECT_EQ(&union_ret, &union_output);
	}
	const int expected_union[] = { 0, 1, 2, 4, 5 };
	expect_hxarray(union_output, expected_union, hxsize(expected_union));

	// hxset_intersection
	hxarray<hxtest_ref_tracker_t> intersection_output;
	intersection_output.reserve(hxsize(left) + 1u);
	intersection_output.push_back(hxtest_ref_tracker_t(0));
	// "Only keys present in both ranges appear in the output."
	//   sentinel { 0 } + overlap { 2, 4 } => { 0, 2, 4 }
	{
		const hxarray<hxtest_ref_tracker_t>& intersection_ret = hxset_intersection(left+0, left + hxsize(left),
			right+0, right + hxsize(right), intersection_output);
		// "Returns an output iterator positioned one past the last element written."
		//   Returned reference must be the same array that was passed in.
		EXPECT_EQ(&intersection_ret, &intersection_output);
	}
	const int expected_intersection[] = { 0, 2, 4 };
	expect_hxarray(intersection_output, expected_intersection, hxsize(expected_intersection));

	// hxset_difference
	hxarray<hxtest_ref_tracker_t> difference_output;
	difference_output.reserve(hxsize(left) + 1u);
	difference_output.push_back(hxtest_ref_tracker_t(0));
	// "The output contains keys that appear in the first range but not the second."
	//   sentinel { 0 } + diff { 1 } => { 0, 1 }
	{
		const hxarray<hxtest_ref_tracker_t>& difference_ret = hxset_difference(left+0, left + hxsize(left),
			right+0, right + hxsize(right), difference_output);
		// "Returns an output iterator positioned one past the last element written."
		//   Returned reference must be the same array that was passed in.
		EXPECT_EQ(&difference_ret, &difference_output);
	}
	const int expected_difference[] = { 0, 1 };
	expect_hxarray(difference_output, expected_difference, hxsize(expected_difference));
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

	const size_t left_count = hxsize(left);
	const size_t right_count = hxsize(right);

	// "Performs a stable merge sort of two ordered ranges [begin0, end0) and"
	// "[begin1, end1) -> output." Verify equal keys keep ticket order.
	hxmerge_test_record_t* merge_end = hxmerge(left+0, left + left_count, right+0, right + right_count, dest+0);

	// "Returns an output iterator positioned one past the last element written."
	//   4 + 4 elements written so offset must be 8.
	EXPECT_EQ(merge_end - dest, ptrdiff_t{8});

	const hxmerge_test_record_t expected[] = {
		{ 1, 0 }, { 1, 1 }, { 3, 0 }, { 3, 1 },
		{ 5, 0 }, { 5, 1 }, { 5, 2 }, { 7, 0 }
	};
	// Stable result should read {(1,0), (1,1), (3,0), (3,1), (5,0), (5,1), (5,2), (7,0)} top-to-bottom.
	for(size_t i = 0; i < left_count + right_count; ++i) {
		EXPECT_EQ(dest[i].key, expected[i].key);
		EXPECT_EQ(dest[i].ticket, expected[i].ticket);
	}
}

TEST(hxminmax_test, simple_case) {
	const int ints[5] = { 5, 3, 1, 4, 2 };
	const int* ints_end = ints + 5;

	// "Returns an hxminmax_result with iterators to the smallest and largest
	// elements in [begin, end)."
	hxminmax_result<const int*> result = hxminmax(ints, ints_end);
	EXPECT_NE(result.min, ints_end);
	EXPECT_EQ(*result.min, 1);
	EXPECT_NE(result.max, ints_end);
	EXPECT_EQ(*result.max, 5);

	// With explicit less comparator.
	result = hxminmax(ints, ints_end, hxkey_less_t<int>{});
	EXPECT_EQ(*result.min, 1);
	EXPECT_EQ(*result.max, 5);

	// Single element: min and max both point at the only element.
	result = hxminmax(ints, ints + 1);
	EXPECT_EQ(result.min, ints);
	EXPECT_EQ(result.max, ints);

	// "Both min and max equal end for an empty range."
	result = hxminmax(ints, ints);
	EXPECT_EQ(result.min, ints);
	EXPECT_EQ(result.max, ints);
}

TEST(hxminmax_test, first_of_equal_minima_and_maxima) {
	const int ints[6] = { 3, 1, 2, 1, 5, 5 };

	// "When multiple elements compare equal and minimal, the first is returned."
	// "When multiple elements compare equal and maximal, the first is returned."
	const hxminmax_result<const int*> result = hxminmax(ints, ints + 6);
	EXPECT_EQ(result.min, ints + 1);
	EXPECT_EQ(result.max, ints + 4);
}

TEST(hxminmax_test, iterator_support) {
	hxtest_ref_tracker_t values[4] = { hxtest_ref_tracker_t(30), hxtest_ref_tracker_t(10), hxtest_ref_tracker_t(40), hxtest_ref_tracker_t(20) };
	const hxtest_iter_api_t begin(values);
	const hxtest_iter_api_t end(values + 4);

	// Minimum is 10 at index 1, maximum is 40 at index 2.
	const hxminmax_result<hxtest_iter_api_t> result =
		hxminmax(begin, end, hxtest_value_less);
	EXPECT_NE(result.min, end);
	EXPECT_EQ((*result.min).value, 10);
	EXPECT_NE(result.max, end);
	EXPECT_EQ((*result.max).value, 40);

	// "Both min and max equal end for an empty range."
	const hxminmax_result<hxtest_iter_api_t> empty_result =
		hxminmax(begin, begin, hxtest_value_less);
	EXPECT_EQ(empty_result.min, begin);
	EXPECT_EQ(empty_result.max, begin);
}

TEST(hxcount_if_test, simple_case) {
	const int ints[6] = { 1, 2, 3, 4, 5, 6 };

	// "Returns the number of elements in [begin, end) for which predicate returns true."
	//   Even numbers { 2, 4, 6 } => 3.
	const size_t count = hxcount_if(ints, ints + 6, [](const int& x) { return x % 2 == 0; });
	EXPECT_EQ(count, size_t{3});

	// All match.
	EXPECT_EQ(hxcount_if(ints, ints + 6, [](const int& x) { return x > 0; }), size_t{6});

	// None match.
	EXPECT_EQ(hxcount_if(ints, ints + 6, [](const int& x) { return x > 10; }), size_t{0});

	// Empty range returns zero.
	EXPECT_EQ(hxcount_if(ints, ints, [](const int& x) { return x == 1; }), size_t{0});
}

TEST(hxunique_test, simple_case) {
	int ints[7] = { 1, 1, 2, 3, 3, 3, 4 };

	// "Removes consecutive duplicate elements." { 1,1,2,3,3,3,4 } => { 1,2,3,4 }.
	int* new_end = hxunique(ints, ints + 7);
	EXPECT_EQ(new_end - ints, ptrdiff_t{4});
	EXPECT_EQ(ints[0], 1);
	EXPECT_EQ(ints[1], 2);
	EXPECT_EQ(ints[2], 3);
	EXPECT_EQ(ints[3], 4);
}

TEST(hxunique_test, all_duplicates) {
	int ints[5] = { 7, 7, 7, 7, 7 };

	// All equal: only one element survives.
	int* new_end = hxunique(ints, ints + 5);
	EXPECT_EQ(new_end - ints, ptrdiff_t{1});
	EXPECT_EQ(ints[0], 7);

	// Single element: end is one past the first.
	new_end = hxunique(ints, ints + 1);
	EXPECT_EQ(new_end, ints + 1);
}

TEST(hxfind_if_test, simple_case) {
	const int ints[5] = { 2, 5, 6, 88, 99 };
	const int* ints_end = ints + 5;

	// "Returns an iterator to the first element in [begin, end) for which
	// predicate returns true." Match the first element >= 6 -> points at 6.
	const int* result = hxfind_if(ints, ints_end, [](const int& x) { return x >= 6; });
	EXPECT_NE(result, ints_end);
	EXPECT_EQ(*result, 6);

	// Match the first element -> points at 2.
	result = hxfind_if(ints, ints_end, [](const int& x) { return x == 2; });
	EXPECT_EQ(result, ints);

	// Match the last element -> points at 99.
	result = hxfind_if(ints, ints_end, [](const int& x) { return x == 99; });
	EXPECT_NE(result, ints_end);
	EXPECT_EQ(*result, 99);

	// "Returns end if no element matches."
	result = hxfind_if(ints, ints_end, [](const int& x) { return x == 0; });
	EXPECT_EQ(result, ints_end);

	// Empty range returns end.
	result = hxfind_if(ints, ints, [](const int& x) { return x == 2; });
	EXPECT_EQ(result, ints);
}

TEST(hxfind_if_test, iterator_support) {
	hxtest_ref_tracker_t values[4] = { hxtest_ref_tracker_t(10), hxtest_ref_tracker_t(20), hxtest_ref_tracker_t(30), hxtest_ref_tracker_t(40) };
	const hxtest_iter_api_t begin(values);
	const hxtest_iter_api_t end(values + 4);

	// Find first element > 15 using iterator facade: should be 20 at index 1.
	const hxtest_iter_api_t result =
		hxfind_if(begin, end, [](const hxtest_ref_tracker_t& x) { return x.value > 15; });
	EXPECT_NE(result, end);
	EXPECT_EQ((*result).value, 20);

	// No match returns end.
	const hxtest_iter_api_t no_match =
		hxfind_if(begin, end, [](const hxtest_ref_tracker_t& x) { return x.value > 100; });
	EXPECT_EQ(no_match, end);

	// Empty range returns end.
	const hxtest_iter_api_t empty_result =
		hxfind_if(begin, begin, [](const hxtest_ref_tracker_t& x) { return x.value == 10; });
	EXPECT_EQ(empty_result, begin);
}

TEST(hxexchange_test, move_only_type) {
	hxtest_ref_tracker_t a(42);
	// "T must be move-constructible." The old value is move-constructed out of
	// obj. Assign a fresh tracker to obj and retrieve the old one.
	const hxtest_ref_tracker_t old = hxexchange(a, hxtest_ref_tracker_t(99));
	EXPECT_EQ(old.value, 42);
	EXPECT_EQ(a.value, 99);
}

#endif // HX_CPLUSPLUS >= 201402L
