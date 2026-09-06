// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

#include <hx/hxset_operations.hpp>
#include <hx/hxvector.hpp>
#include <hx/hxrandom.hpp>
#include <hx/hxtest.hpp>
#include "./hxtest_util.hpp"

HX_NS_USE

using hxmerge_test_f = hxtest_object_fixture;
using hxminmax_test_f = hxtest_object_fixture;
using hxset_algorithms_test_f = hxtest_object_fixture;
using hxunique_test_f = hxtest_object_fixture;

TEST(hxmerge_test, pointer_range_tails) {
	const int left1[1] = { 1 };
	const int right2[2] = { 2, 3 };
	int dest3a[3] = { };
	const int* merge_end_a = hxmerge(hxmake_range(left1 + 0, left1 + 1), hxmake_range(right2 + 0, right2 + 2), dest3a + 0);
	EXPECT_EQ(merge_end_a - dest3a, ptrdiff_t{3});
	EXPECT_EQ(dest3a[0], 1);
	EXPECT_EQ(dest3a[1], 2);
	EXPECT_EQ(dest3a[2], 3);
	const int left2[2] = { 1, 3 };
	const int right1[1] = { 2 };
	int dest3b[3] = { };
	const int* merge_end_b = hxmerge(hxmake_range(left2 + 0, left2 + 2), hxmake_range(right1 + 0, right1 + 1), dest3b + 0);
	EXPECT_EQ(merge_end_b - dest3b, ptrdiff_t{3});
	EXPECT_EQ(dest3b[0], 1);
	EXPECT_EQ(dest3b[1], 2);
	EXPECT_EQ(dest3b[2], 3);
}

TEST(hxmerge_test, preserves_stable_ordering) {
	struct hxmerge_test_record_t {
		int key;
		int ticket;
		bool operator<(const hxmerge_test_record_t& x) const {
			return key < x.key;
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
	const hxmerge_test_record_t* merge_end = hxmerge(
		hxmake_range(left + 0, left + left_count), hxmake_range(right + 0, right + right_count), dest + 0);
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

TEST_F(hxmerge_test_f, iterator_support) {
	hxvector<hxtest_object, 3> left{ 1, 3, 5 };
	hxvector<hxtest_object, 3> right{ 2, 4, 6 };
	hxvector<hxtest_object, 6> dest{ 0, 0, 0, 0, 0, 0 };
	const hxtest_rand_iterator_api_t merge_end =
		hxmerge(hxmake_range(hxtest_rand_iterator_api_t(left.data()), hxtest_rand_iterator_api_t(left.data() + 3)),
			hxmake_range(hxtest_rand_iterator_api_t(right.data()), hxtest_rand_iterator_api_t(right.data() + 3)),
			hxtest_rand_iterator_api_t(dest.data()), hxtest_value_less);
	EXPECT_EQ(merge_end, hxtest_rand_iterator_api_t(dest.data()) + ptrdiff_t{6});
	const int expected_sorted[6] = { 1, 2, 3, 4, 5, 6 };
	for(hxsize_t i = 0; i < 6; ++i) {
		EXPECT_EQ(dest[i].value(), expected_sorted[i]);
	}
	hxvector<hxtest_object, 3> left_desc{ 5, 3, 1 };
	hxvector<hxtest_object, 3> right_desc{ 6, 4, 2 };
	hxvector<hxtest_object, 6> dest_desc{ 0, 0, 0, 0, 0, 0 };
	const hxtest_rand_iterator_api_t merge_desc_end =
		hxmerge(hxmake_range(hxtest_rand_iterator_api_t(left_desc.data()), hxtest_rand_iterator_api_t(left_desc.data() + 3)),
			hxmake_range(hxtest_rand_iterator_api_t(right_desc.data()), hxtest_rand_iterator_api_t(right_desc.data() + 3)),
			hxtest_rand_iterator_api_t(dest_desc.data()), hxtest_value_greater);
	EXPECT_EQ(merge_desc_end, hxtest_rand_iterator_api_t(dest_desc.data()) + ptrdiff_t{6});
	const int expected_desc[6] = { 6, 5, 4, 3, 2, 1 };
	for(hxsize_t i = 0; i < 6; ++i) {
		EXPECT_EQ(dest_desc[i].value(), expected_desc[i]);
	}
	EXPECT_TRUE(check_stats(24, 0, 0, 24, 0, 0, 0, 12, 0, 10));
}

TEST_F(hxmerge_test_f, forwards_by_value_category) {
	hxvector<hxtest_object, 3> left{ 1, 3, 5 };
	hxvector<hxtest_object, 3> right{ 2, 4, 6 };
	hxvector<hxtest_object, 6> dest{ 0, 0, 0, 0, 0, 0 };
	const auto left_range =
		hxmake_range(hxtest_rand_iterator_api_t(left.data()), hxtest_rand_iterator_api_t(left.data() + 3));
	const hxtest_rand_iterator_api_t merge_end = hxmerge(left_range,
		hxmake_range(hxtest_rand_iterator_api_t(right.data()), hxtest_rand_iterator_api_t(right.data() + 3)),
		hxtest_rand_iterator_api_t(dest.data()), hxtest_value_less);
	EXPECT_EQ(merge_end, hxtest_rand_iterator_api_t(dest.data()) + ptrdiff_t{6});
	const int expected_sorted[6] = { 1, 2, 3, 4, 5, 6 };
	for(hxsize_t i = 0; i < 6; ++i) {
		EXPECT_EQ(dest[i].value(), expected_sorted[i]);
	}
	EXPECT_EQ(left[0].value(), 1);
	EXPECT_EQ(left[1].value(), 3);
	EXPECT_EQ(left[2].value(), 5);
	EXPECT_TRUE(check_stats(12, 0, 0, 12, 0, 0, 3, 3, 0, 5));
}

TEST_F(hxset_algorithms_test_f, hxset_union_forwards_by_value_category) {
	hxvector<hxtest_object, 3> left{ 1, 2, 4 };
	hxvector<hxtest_object, 3> right{ 2, 4, 5 };
	hxvector<hxtest_object, 5> dest{ 0, 0, 0, 0, 0 };
	const auto left_range =
		hxmake_range(hxtest_rand_iterator_api_t(left.data()), hxtest_rand_iterator_api_t(left.data() + left.size()));
	const hxtest_rand_iterator_api_t union_end = hxset_union(left_range,
		hxmake_range(hxtest_rand_iterator_api_t(right.data()), hxtest_rand_iterator_api_t(right.data() + right.size())),
		hxtest_rand_iterator_api_t(dest.data()));
	EXPECT_EQ(union_end, hxtest_rand_iterator_api_t(dest.data()) + ptrdiff_t{4});
	const int expected_union[] = { 1, 2, 4, 5 };
	for(hxsize_t i = 0; i < 4; ++i) {
		EXPECT_EQ(dest[i].value(), expected_union[i]);
	}
	EXPECT_EQ(left[0].value(), 1);
	EXPECT_EQ(left[1].value(), 2);
	EXPECT_EQ(left[2].value(), 4);
	EXPECT_TRUE(check_stats(11, 0, 0, 11, 0, 0, 3, 1, 0, 6));
}

TEST(hxminmax_test, small_ranges) {
	const int one[1] = { 34 };
	hxminmax_result<const int*> result = hxminmax(hxmake_range(one, one + 1));
	EXPECT_EQ(result.min, one);
	EXPECT_EQ(result.max, one);
	const int ascending[2] = { 7, 11 };
	result = hxminmax(hxmake_range(ascending, ascending + 2));
	EXPECT_EQ(result.min, ascending);
	EXPECT_EQ(result.max, ascending + 1);
	const int descending[2] = { 11, 7 };
	result = hxminmax(hxmake_range(descending, descending + 2));
	EXPECT_EQ(result.min, descending + 1);
	EXPECT_EQ(result.max, descending);
	const int ties[6] = { 3, 1, 2, 1, 5, 5 };
	result = hxminmax(hxmake_range(ties, ties + 6));
	EXPECT_EQ(result.min, ties + 1);
	EXPECT_EQ(result.max, ties + 4);
}

TEST(hxminmax_test, simple_case) {
	const int ints[5] = { 5, 3, 1, 4, 2 };
	const int* ints_end = ints + 5;
	hxminmax_result<const int*> result = hxminmax(hxmake_range(ints, ints_end));
	EXPECT_NE(result.min, ints_end);
	EXPECT_EQ(*result.min, 1);
	EXPECT_NE(result.max, ints_end);
	EXPECT_EQ(*result.max, 5);
	result = hxminmax(hxmake_range(ints, ints_end), hxkey_less_t<int>{});
	EXPECT_EQ(*result.min, 1);
	EXPECT_EQ(*result.max, 5);
	result = hxminmax(hxmake_range(ints, ints + 1));
	EXPECT_EQ(result.min, ints);
	EXPECT_EQ(result.max, ints);
	result = hxminmax(hxmake_range(ints, ints));
	EXPECT_EQ(result.min, ints);
	EXPECT_EQ(result.max, ints);
}

TEST_F(hxminmax_test_f, iterator_support) {
	hxvector<hxtest_object, 4> values{ 30, 10, 40, 20 };
	const hxtest_forward_iterator_api_t begin(values.data());
	const hxtest_forward_iterator_api_t end(values.data() + 4);
	const hxminmax_result<hxtest_forward_iterator_api_t> result =
		hxminmax(hxmake_range(begin, end), hxtest_value_less);
	EXPECT_NE(result.min, end);
	EXPECT_EQ((*result.min).value(), 10);
	EXPECT_NE(result.max, end);
	EXPECT_EQ((*result.max).value(), 40);
	const hxminmax_result<hxtest_forward_iterator_api_t> empty_result =
		hxminmax(hxmake_range(begin, begin), hxtest_value_less);
	EXPECT_EQ(empty_result.min, begin);
	EXPECT_EQ(empty_result.max, begin);
	EXPECT_TRUE(check_stats(4, 0, 0, 4, 0, 0, 0, 0, 0, 5));
}

#if HX_CPLUSPLUS >= 201402L
TEST(hxset_algorithms_test, edge_cases) {
	const int single_left[1] = { 2 };
	const int single_right[1] = { 3 };
	int dest_union[2] = { };
	int dest_intersection[1] = { };
	int dest_difference[1] = { };
	const int* union_end = hxset_union(
		hxmake_range(single_left + 0, single_left + 1), hxmake_range(single_right + 0, single_right + 1), dest_union + 0);
	EXPECT_EQ(union_end - dest_union, ptrdiff_t{2});
	EXPECT_EQ(dest_union[0], 2);
	EXPECT_EQ(dest_union[1], 3);
	const int* intersection_end = hxset_intersection(
		hxmake_range(single_left + 0, single_left + 1), hxmake_range(single_right + 0, single_right + 1), dest_intersection + 0);
	EXPECT_EQ(intersection_end - dest_intersection, ptrdiff_t{0});
	const int* difference_end = hxset_difference(
		hxmake_range(single_left + 0, single_left + 1), hxmake_range(single_right + 0, single_right + 1), dest_difference + 0);
	EXPECT_EQ(difference_end - dest_difference, ptrdiff_t{1});
	EXPECT_EQ(dest_difference[0], 2);
	const int left3[3] = { 1, 5, 9 };
	const int right1[1] = { 5 };
	int dest3[3] = { };
	const int* diff_end = hxset_difference(hxmake_range(left3 + 0, left3 + 3), hxmake_range(right1 + 0, right1 + 1), dest3 + 0);
	EXPECT_EQ(diff_end - dest3, ptrdiff_t{2});
	EXPECT_EQ(dest3[0], 1);
	EXPECT_EQ(dest3[1], 9);
	int dest4[1] = { };
	const int* inter_end = hxset_intersection(hxmake_range(left3 + 0, left3 + 3), hxmake_range(right1 + 0, right1 + 1), dest4 + 0);
	EXPECT_EQ(inter_end - dest4, ptrdiff_t{1});
	EXPECT_EQ(dest4[0], 5);
}

TEST(hxset_algorithms_test, largest_element_at_tail) {
	const int left_a[3] = { 1, 3, 9 };
	const int right_a[2] = { 2, 4 };
	int dest_a[5] = { };
	const int* union_end_a = hxset_union(hxmake_range(left_a + 0, left_a + 3), hxmake_range(right_a + 0, right_a + 2), dest_a + 0);
	EXPECT_EQ(union_end_a - dest_a, ptrdiff_t{5});
	EXPECT_EQ(dest_a[4], 9);
	const int left_b[2] = { 1, 3 };
	const int right_b[3] = { 2, 4, 9 };
	int dest_b[5] = { };
	const int* union_end_b = hxset_union(hxmake_range(left_b + 0, left_b + 2), hxmake_range(right_b + 0, right_b + 3), dest_b + 0);
	EXPECT_EQ(union_end_b - dest_b, ptrdiff_t{5});
	EXPECT_EQ(dest_b[4], 9);
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
	const int* union_end = hxset_union(
		hxmake_range(left + 0, left + hxsize(left)), hxmake_range(right + 0, right + hxsize(right)), dest_union+0);
	const int expected_union[] = { 1, 3, 4, 5, 7, 9 };
	EXPECT_EQ(union_end - dest_union, hxsize(expected_union));
	expect_range(dest_union, union_end, expected_union);
	const int* intersection_end = hxset_intersection(
		hxmake_range(left + 0, left + hxsize(left)), hxmake_range(right + 0, right + hxsize(right)), dest_intersection+0);
	const int expected_intersection[] = { 3, 7 };
	EXPECT_EQ(intersection_end - dest_intersection, hxsize(expected_intersection));
	expect_range(dest_intersection, intersection_end, expected_intersection);
	const int* difference_end = hxset_difference(
		hxmake_range(left + 0, left + hxsize(left)), hxmake_range(right + 0, right + hxsize(right)), dest_difference+0);
	const int expected_difference[] = { 1, 5 };
	EXPECT_EQ(difference_end - dest_difference, hxsize(expected_difference));
	expect_range(dest_difference, difference_end, expected_difference);
}

TEST_F(hxset_algorithms_test_f, preserves_left_provenance_on_ties) {
	hxvector<hxtest_object, 4> left{ 1, 3, 3, 5 };
	hxvector<hxtest_object, 4> right{ 2, 3, 5, 7 };
	const auto left_range = hxmake_range(left.data(), left.data() + left.size());
	const auto right_range = hxmake_range(right.data(), right.data() + right.size());
	hxvector<hxtest_object, 6> dest_union{ 0, 0, 0, 0, 0, 0 };
	const hxtest_object* union_end = hxset_union(left_range, right_range, dest_union.data());
	const int expected_union_value[] = { 1, 2, 3, 3, 5, 7 };
	const uint16_t expected_union_ticket[] = { 100u, 104u, 101u, 102u, 103u, 107u };
	EXPECT_EQ(union_end - dest_union.data(), hxsize(expected_union_value));
	for(hxsize_t i = 0; i < hxsize(expected_union_value); ++i) {
		EXPECT_EQ(dest_union[i].value(), expected_union_value[i]);
		EXPECT_EQ(dest_union[i].ticket(), expected_union_ticket[i]);
	}
	hxvector<hxtest_object, 2> dest_intersection{ 0, 0 };
	const hxtest_object* intersection_end = hxset_intersection(left_range, right_range, dest_intersection.data());
	const int expected_intersection_value[] = { 3, 5 };
	const uint16_t expected_intersection_ticket[] = { 101u, 103u };
	EXPECT_EQ(intersection_end - dest_intersection.data(), hxsize(expected_intersection_value));
	for(hxsize_t i = 0; i < hxsize(expected_intersection_value); ++i) {
		EXPECT_EQ(dest_intersection[i].value(), expected_intersection_value[i]);
		EXPECT_EQ(dest_intersection[i].ticket(), expected_intersection_ticket[i]);
	}
	hxvector<hxtest_object, 2> dest_difference{ 0, 0 };
	const hxtest_object* difference_end = hxset_difference(left_range, right_range, dest_difference.data());
	const int expected_difference_value[] = { 1, 3 };
	const uint16_t expected_difference_ticket[] = { 100u, 102u };
	EXPECT_EQ(difference_end - dest_difference.data(), hxsize(expected_difference_value));
	for(hxsize_t i = 0; i < hxsize(expected_difference_value); ++i) {
		EXPECT_EQ(dest_difference[i].value(), expected_difference_value[i]);
		EXPECT_EQ(dest_difference[i].ticket(), expected_difference_ticket[i]);
	}
	EXPECT_TRUE(check_stats(18, 0, 0, 18, 0, 0, 10, 0, 0, 30));
}

TEST_F(hxset_algorithms_test_f, hxarray_output_iterator_merge) {
	auto expect_hxarray = [](const hxvector<hxtest_object>& actual, const int* expected, hxsize_t count) {
		ASSERT_EQ(actual.size(), count);
		for(hxsize_t i = 0; i < count; ++i) {
			EXPECT_EQ(actual[i].value(), expected[i]);
		}
	};
	hxvector<hxtest_object, 3> left{ 1, 2, 4 };
	hxvector<hxtest_object, 3> right{ 2, 4, 5 };
	hxvector<hxtest_object> merge_output;
	merge_output.reserve(left.size() + right.size() + 1);
	merge_output.push_back(hxtest_object(0));
	{
		auto& merge_ret = hxmerge(
			hxmake_range(hxtest_rand_iterator_api_t(left.data()), hxtest_rand_iterator_api_t(left.data() + left.size())),
			hxmake_range(hxtest_rand_iterator_api_t(right.data()), hxtest_rand_iterator_api_t(right.data() + right.size())),
			merge_output);
		EXPECT_EQ(&merge_ret, &merge_output);
	}
	const int expected_merge[] = { 0, 1, 2, 2, 4, 4, 5 };
	const uint16_t expected_ticket[] = { 106u, 100u, 101u, 103u, 102u, 104u, 105u };
	expect_hxarray(merge_output, expected_merge, hxsize(expected_merge));
	for(hxsize_t i = 0; i < hxsize(expected_ticket); ++i) {
		EXPECT_EQ(merge_output[i].ticket(), expected_ticket[i]);
	}
	EXPECT_TRUE(check_stats(14, 1, 0, 7, 0, 7, 0, 0, 0, 4));
}

TEST_F(hxset_algorithms_test_f, hxarray_output_iterator_union) {
	auto expect_hxarray = [](const hxvector<hxtest_object>& actual, const int* expected, hxsize_t count) {
		ASSERT_EQ(actual.size(), count);
		for(hxsize_t i = 0; i < count; ++i) {
			EXPECT_EQ(actual[i].value(), expected[i]);
		}
	};
	hxvector<hxtest_object, 3> left{ 1, 2, 4 };
	hxvector<hxtest_object, 3> right{ 2, 4, 5 };
	hxvector<hxtest_object> union_output;
	union_output.reserve(left.size() + right.size() + 1);
	union_output.push_back(hxtest_object(0));
	{
		auto& union_ret = hxset_union(
			hxmake_range(hxtest_rand_iterator_api_t(left.data()), hxtest_rand_iterator_api_t(left.data() + left.size())),
			hxmake_range(hxtest_rand_iterator_api_t(right.data()), hxtest_rand_iterator_api_t(right.data() + right.size())),
			union_output);
		EXPECT_EQ(&union_ret, &union_output);
	}
	const int expected_union[] = { 0, 1, 2, 4, 5 };
	const uint16_t expected_ticket[] = { 106u, 100u, 101u, 102u, 105u };
	expect_hxarray(union_output, expected_union, hxsize(expected_union));
	for(hxsize_t i = 0; i < hxsize(expected_ticket); ++i) {
		EXPECT_EQ(union_output[i].ticket(), expected_ticket[i]);
	}
	EXPECT_TRUE(check_stats(12, 1, 0, 7, 0, 5, 0, 0, 0, 6));
}

TEST_F(hxset_algorithms_test_f, hxarray_output_iterator_intersection) {
	auto expect_hxarray = [](const hxvector<hxtest_object>& actual, const int* expected, hxsize_t count) {
		ASSERT_EQ(actual.size(), count);
		for(hxsize_t i = 0; i < count; ++i) {
			EXPECT_EQ(actual[i].value(), expected[i]);
		}
	};
	hxvector<hxtest_object, 3> left{ 1, 2, 4 };
	hxvector<hxtest_object, 3> right{ 2, 4, 5 };
	hxvector<hxtest_object> intersection_output;
	intersection_output.reserve(left.size() + 1);
	intersection_output.push_back(hxtest_object(0));
	{
		auto& intersection_ret = hxset_intersection(
			hxmake_range(hxtest_rand_iterator_api_t(left.data()), hxtest_rand_iterator_api_t(left.data() + left.size())),
			hxmake_range(hxtest_rand_iterator_api_t(right.data()), hxtest_rand_iterator_api_t(right.data() + right.size())),
			intersection_output);
		EXPECT_EQ(&intersection_ret, &intersection_output);
	}
	const int expected_intersection[] = { 0, 2, 4 };
	const uint16_t expected_ticket[] = { 106u, 101u, 102u };
	expect_hxarray(intersection_output, expected_intersection, hxsize(expected_intersection));
	for(hxsize_t i = 0; i < hxsize(expected_ticket); ++i) {
		EXPECT_EQ(intersection_output[i].ticket(), expected_ticket[i]);
	}
	EXPECT_TRUE(check_stats(10, 1, 0, 7, 0, 3, 0, 0, 0, 6));
}

TEST_F(hxset_algorithms_test_f, hxarray_output_iterator_difference) {
	auto expect_hxarray = [](const hxvector<hxtest_object>& actual, const int* expected, hxsize_t count) {
		ASSERT_EQ(actual.size(), count);
		for(hxsize_t i = 0; i < count; ++i) {
			EXPECT_EQ(actual[i].value(), expected[i]);
		}
	};
	hxvector<hxtest_object, 3> left{ 1, 2, 4 };
	hxvector<hxtest_object, 3> right{ 2, 4, 5 };
	hxvector<hxtest_object> difference_output;
	difference_output.reserve(left.size() + 1);
	difference_output.push_back(hxtest_object(0));
	{
		auto& difference_ret = hxset_difference(
			hxmake_range(hxtest_rand_iterator_api_t(left.data()), hxtest_rand_iterator_api_t(left.data() + left.size())),
			hxmake_range(hxtest_rand_iterator_api_t(right.data()), hxtest_rand_iterator_api_t(right.data() + right.size())),
			difference_output);
		EXPECT_EQ(&difference_ret, &difference_output);
	}
	const int expected_difference[] = { 0, 1 };
	const uint16_t expected_ticket[] = { 106u, 100u };
	expect_hxarray(difference_output, expected_difference, hxsize(expected_difference));
	for(hxsize_t i = 0; i < hxsize(expected_ticket); ++i) {
		EXPECT_EQ(difference_output[i].ticket(), expected_ticket[i]);
	}
	EXPECT_TRUE(check_stats(9, 1, 0, 7, 0, 2, 0, 0, 0, 6));
}

TEST_F(hxset_algorithms_test_f, hxarray_output_iterator_right_exhausted_first_union) {
	auto expect_hxarray = [](const hxvector<hxtest_object>& actual, const int* expected, hxsize_t count) {
		ASSERT_EQ(actual.size(), count);
		for(hxsize_t i = 0; i < count; ++i) {
			EXPECT_EQ(actual[i].value(), expected[i]);
		}
	};
	hxvector<hxtest_object, 3> left{ 1, 2, 4 };
	hxvector<hxtest_object, 1> right{ 3 };
	hxvector<hxtest_object> union_output;
	union_output.reserve(left.size() + right.size());
	{
		auto& union_ret = hxset_union(
			hxmake_range(hxtest_rand_iterator_api_t(left.data()), hxtest_rand_iterator_api_t(left.data() + left.size())),
			hxmake_range(hxtest_rand_iterator_api_t(right.data()), hxtest_rand_iterator_api_t(right.data() + right.size())),
			union_output);
		EXPECT_EQ(&union_ret, &union_output);
	}
	const int expected_union[] = { 1, 2, 3, 4 };
	expect_hxarray(union_output, expected_union, hxsize(expected_union));
	EXPECT_TRUE(check_stats(8, 0, 0, 4, 0, 4, 0, 0, 0, 6));
}

TEST_F(hxset_algorithms_test_f, hxarray_output_iterator_right_exhausted_first_intersection) {
	hxvector<hxtest_object, 3> left{ 1, 2, 4 };
	hxvector<hxtest_object, 1> right{ 3 };
	hxvector<hxtest_object> intersection_output;
	intersection_output.reserve(left.size());
	{
		auto& intersection_ret = hxset_intersection(
			hxmake_range(hxtest_rand_iterator_api_t(left.data()), hxtest_rand_iterator_api_t(left.data() + left.size())),
			hxmake_range(hxtest_rand_iterator_api_t(right.data()), hxtest_rand_iterator_api_t(right.data() + right.size())),
			intersection_output);
		EXPECT_EQ(&intersection_ret, &intersection_output);
	}
	EXPECT_EQ(intersection_output.size(), hxsize_t{0});
	EXPECT_TRUE(check_stats(4, 0, 0, 4, 0, 0, 0, 0, 0, 6));
}

TEST_F(hxset_algorithms_test_f, hxarray_output_iterator_right_exhausted_first_difference) {
	auto expect_hxarray = [](const hxvector<hxtest_object>& actual, const int* expected, hxsize_t count) {
		ASSERT_EQ(actual.size(), count);
		for(hxsize_t i = 0; i < count; ++i) {
			EXPECT_EQ(actual[i].value(), expected[i]);
		}
	};
	hxvector<hxtest_object, 3> left{ 1, 2, 4 };
	hxvector<hxtest_object, 1> right{ 3 };
	hxvector<hxtest_object> difference_output;
	difference_output.reserve(left.size());
	{
		auto& difference_ret = hxset_difference(
			hxmake_range(hxtest_rand_iterator_api_t(left.data()), hxtest_rand_iterator_api_t(left.data() + left.size())),
			hxmake_range(hxtest_rand_iterator_api_t(right.data()), hxtest_rand_iterator_api_t(right.data() + right.size())),
			difference_output);
		EXPECT_EQ(&difference_ret, &difference_output);
	}
	const int expected_difference[] = { 1, 2, 4 };
	expect_hxarray(difference_output, expected_difference, hxsize(expected_difference));
	EXPECT_TRUE(check_stats(7, 0, 0, 4, 0, 3, 0, 0, 0, 6));
}
#endif // HX_CPLUSPLUS >= 201402L

TEST_F(hxunique_test_f, simple_case) {
	hxvector<hxtest_object, 7> values{ 1, 1, 2, 3, 3, 3, 4 };
	const hxtest_rand_iterator_api_t begin(values.data());
	const hxtest_rand_iterator_api_t end(values.data() + 7);
	const hxtest_rand_iterator_api_t new_end =
		hxunique(begin, end, hxtest_value_equal);
	EXPECT_EQ(new_end, hxtest_rand_iterator_api_t(values.data() + 4));
	EXPECT_EQ(values[0].value(), 1);
	EXPECT_EQ(values[1].value(), 2);
	EXPECT_EQ(values[2].value(), 3);
	EXPECT_EQ(values[3].value(), 4);
	hxvector<hxtest_object, 3> no_dup_values{ 1, 2, 3 };
	const hxtest_rand_iterator_api_t no_dup_begin(no_dup_values.data());
	const hxtest_rand_iterator_api_t no_dup_end(no_dup_values.data() + 3);
	const hxtest_rand_iterator_api_t no_dup_new_end =
		hxunique(no_dup_begin, no_dup_end, hxtest_value_equal);
	EXPECT_EQ(no_dup_new_end, no_dup_end);
	EXPECT_TRUE(check_stats(10, 0, 0, 10, 0, 0, 0, 3, 8, 0));
}

TEST(hxunique_test, boundary_counts) {
	int all_dup[5] = { 7, 7, 7, 7, 7 };
	const int* new_end = hxunique(all_dup, all_dup + 5);
	EXPECT_EQ(new_end - all_dup, ptrdiff_t{1});
	EXPECT_EQ(all_dup[0], 7);
	new_end = hxunique(all_dup, all_dup + 1);
	EXPECT_EQ(new_end, all_dup + 1);
	int no_dup[3] = { 1, 2, 3 };
	new_end = hxunique(no_dup, no_dup + 3);
	EXPECT_EQ(new_end - no_dup, ptrdiff_t{3});
	int two_none[2] = { 4, 5 };
	new_end = hxunique(two_none, two_none + 2);
	EXPECT_EQ(new_end - two_none, ptrdiff_t{2});
	EXPECT_EQ(two_none[0], 4);
	EXPECT_EQ(two_none[1], 5);
	int two_dup[2] = { 9, 9 };
	new_end = hxunique(two_dup, two_dup + 2);
	EXPECT_EQ(new_end - two_dup, ptrdiff_t{1});
	int mixed[7] = { 1, 1, 2, 3, 3, 3, 4 };
	new_end = hxunique(mixed, mixed + 7);
	EXPECT_EQ(new_end - mixed, ptrdiff_t{4});
	EXPECT_EQ(mixed[0], 1);
	EXPECT_EQ(mixed[1], 2);
	EXPECT_EQ(mixed[2], 3);
	EXPECT_EQ(mixed[3], 4);
	int* const empty_end = hxunique(mixed + 0, mixed + 0);
	EXPECT_EQ(empty_end, mixed + 0);
}

TEST_F(hxunique_test_f, empty_range_explicit_equal) {
	hxvector<hxtest_object, 1> value{ 1 };
	const hxtest_rand_iterator_api_t begin(value.data());
	const hxtest_rand_iterator_api_t empty_end =
		hxunique(begin, begin, hxtest_value_equal);
	EXPECT_EQ(empty_end, begin);
	EXPECT_TRUE(check_stats(1, 0, 0, 1, 0, 0, 0, 0, 0, 0));
}
