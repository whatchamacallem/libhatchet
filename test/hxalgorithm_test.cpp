// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

#include <hx/hxalgorithm.hpp>
#include <hx/hxvector.hpp>
#include <hx/hxflat_map.hpp>
#include <hx/hxrandom.hpp>
#include <hx/hxtest.hpp>
#include "./hxtest_util.hpp"

HX_NS_USE

using hxbinary_search_test_f = hxtest_object_fixture;
using hxcount_if_test_f = hxtest_object_fixture;
using hxexchange_test_f = hxtest_object_fixture;
using hxfind_if_test_f = hxtest_object_fixture;
using hxmerge_test_f = hxtest_object_fixture;
using hxminmax_test_f = hxtest_object_fixture;
using hxquantifier_test_f = hxtest_object_fixture;
using hxset_algorithms_test_f = hxtest_object_fixture;
using hxtest_test_f = hxtest_object_fixture;
using hxunique_test_f = hxtest_object_fixture;

TEST_F(hxbinary_search_test_f, iterator_support) {
	hxtest_object values[7] = {
		hxtest_object(-5), hxtest_object(-1), hxtest_object(0), hxtest_object(3),
		hxtest_object(5), hxtest_object(8), hxtest_object(12)
	};
	const hxtest_rand_iter_api_t begin(values);
	const hxtest_rand_iter_api_t end(values + 7);
	const hxtest_object key_three(3);
	hxtest_rand_iter_api_t result = hxbinary_search(begin, end, key_three, hxtest_value_less);
	EXPECT_NE(result, end);
	EXPECT_EQ((*result).value, 3);
	const hxtest_object key_high(12);
	result = hxbinary_search(begin, end, key_high, hxtest_value_less);
	EXPECT_NE(result, end);
	EXPECT_EQ((*result).value, 12);
	const hxtest_object missing(7);
	result = hxbinary_search(begin, end, missing, hxtest_value_less);
	EXPECT_EQ(result, end);
	result = hxbinary_search(begin, begin, key_three, hxtest_value_less);
	EXPECT_EQ(result, begin);
	EXPECT_TRUE(check_stats(10, 0, 0, 0, 0, 0, 0, 0));
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

TEST_F(hxcount_if_test_f, simple_case) {
	hxtest_object values[6] = {
		hxtest_object(1), hxtest_object(2), hxtest_object(3),
		hxtest_object(4), hxtest_object(5), hxtest_object(6)
	};
	const hxtest_forward_iter_api_t begin(values);
	const hxtest_forward_iter_api_t end(values + 6);
	const hxsize_t count = hxcount_if(begin, end, [](const hxtest_object& x) { return x.value % 2 == 0; });
	EXPECT_EQ(count, hxsize_t{3});
	EXPECT_EQ(hxcount_if(begin, end, [](const hxtest_object& x) { return x.value > 0; }), hxsize_t{6});
	EXPECT_EQ(hxcount_if(begin, end, [](const hxtest_object& x) { return x.value > 10; }), hxsize_t{0});
	EXPECT_EQ(hxcount_if(begin, begin, [](const hxtest_object& x) { return x.value == 1; }), hxsize_t{0});
	EXPECT_TRUE(check_stats(6, 0, 0, 0, 0, 0, 0, 0));
}

TEST(hxcount_if_test, boundary_matches) {
	const int last_matches[4] = { 1, 3, 5, 6 };
	EXPECT_EQ(hxcount_if(last_matches, last_matches + 4, [](const int& x) { return x % 2 == 0; }), hxsize_t{1});
	const int first_matches[4] = { 6, 1, 3, 5 };
	EXPECT_EQ(hxcount_if(first_matches, first_matches + 4, [](const int& x) { return x % 2 == 0; }), hxsize_t{1});
}

TEST_F(hxexchange_test_f, move_only_type) {
	hxtest_object a(34);
	const hxtest_object old = hxexchange(a, hxtest_object(99));
	EXPECT_EQ(old.value, 34);
	EXPECT_EQ(a.value, 99);
	// -fno-elide-constructors in the coverage test inflates this.
	EXPECT_TRUE(check_stats(4, 2, 0, 2, 0, 1, 0, 0));
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

TEST_F(hxfind_if_test_f, iterator_support) {
	hxtest_object values[4] = { hxtest_object(10), hxtest_object(20), hxtest_object(30), hxtest_object(40) };
	const hxtest_forward_iter_api_t begin(values);
	const hxtest_forward_iter_api_t end(values + 4);
	const hxtest_forward_iter_api_t result =
		hxfind_if(begin, end, [](const hxtest_object& x) { return x.value > 15; });
	EXPECT_NE(result, end);
	EXPECT_EQ((*result).value, 20);
	const hxtest_forward_iter_api_t no_match =
		hxfind_if(begin, end, [](const hxtest_object& x) { return x.value > 100; });
	EXPECT_EQ(no_match, end);
	const hxtest_forward_iter_api_t empty_result =
		hxfind_if(begin, begin, [](const hxtest_object& x) { return x.value == 10; });
	EXPECT_EQ(empty_result, begin);
	EXPECT_TRUE(check_stats(4, 0, 0, 0, 0, 0, 0, 0));
}

TEST(hxfind_if_test, flat_map_iterator) {
	using map_t = hxflat_map<int, int, hxkey_less_t<int>, false, 4>;
	map_t m;
	m.insert(1, 10);
	m.insert(2, 20);
	m.insert(3, 30);
	map_t::const_iterator result = hxfind_if(m.begin(), m.end(),
		[](const map_t::const_iterator& it) { return it.value() >= 20; });
	EXPECT_NE(result, m.end());
	EXPECT_EQ(result.key(), 2);
	result = hxfind_if(m.begin(), m.end(),
		[](const map_t::const_iterator& it) { return it.key() == 1; });
	EXPECT_EQ(result, m.begin());
	result = hxfind_if(m.begin(), m.end(),
		[](const map_t::const_iterator& it) { return it.value() == 30; });
	EXPECT_NE(result, m.end());
	EXPECT_EQ(result.key(), 3);
	result = hxfind_if(m.begin(), m.end(),
		[](const map_t::const_iterator& it) { return it.value() == 99; });
	EXPECT_EQ(result, m.end());
	result = hxfind_if(m.begin(), m.begin(),
		[](const map_t::const_iterator& it) { return it.value() == 10; });
	EXPECT_EQ(result, m.begin());
}

TEST(hxmerge_test, pointer_range_tails) {
	const int left1[1] = { 1 };
	const int right2[2] = { 2, 3 };
	int dest3a[3] = { };
	const int* merge_end_a = hxmerge(left1 + 0, left1 + 1, right2 + 0, right2 + 2, dest3a + 0);
	EXPECT_EQ(merge_end_a - dest3a, ptrdiff_t{3});
	EXPECT_EQ(dest3a[0], 1);
	EXPECT_EQ(dest3a[1], 2);
	EXPECT_EQ(dest3a[2], 3);
	const int left2[2] = { 1, 3 };
	const int right1[1] = { 2 };
	int dest3b[3] = { };
	const int* merge_end_b = hxmerge(left2 + 0, left2 + 2, right1 + 0, right1 + 1, dest3b + 0);
	EXPECT_EQ(merge_end_b - dest3b, ptrdiff_t{3});
	EXPECT_EQ(dest3b[0], 1);
	EXPECT_EQ(dest3b[1], 2);
	EXPECT_EQ(dest3b[2], 3);
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

TEST_F(hxmerge_test_f, iterator_support) {
	hxtest_object left[3] = { hxtest_object(1), hxtest_object(3), hxtest_object(5) };
	hxtest_object right[3] = { hxtest_object(2), hxtest_object(4), hxtest_object(6) };
	hxtest_object dest[6] = {
		hxtest_object(0), hxtest_object(0), hxtest_object(0),
		hxtest_object(0), hxtest_object(0), hxtest_object(0)
	};
	const hxtest_rand_iter_api_t merge_end =
		hxmerge(hxtest_rand_iter_api_t(left), hxtest_rand_iter_api_t(left + 3),
			hxtest_rand_iter_api_t(right), hxtest_rand_iter_api_t(right + 3),
			hxtest_rand_iter_api_t(dest), hxtest_value_less);
	EXPECT_EQ(merge_end, hxtest_rand_iter_api_t(dest) + ptrdiff_t{6});
	const int expected_sorted[6] = { 1, 2, 3, 4, 5, 6 };
	for(hxsize_t i = 0; i < 6; ++i) {
		EXPECT_EQ(dest[i].value, expected_sorted[i]);
	}
	hxtest_object left_desc[3] = { hxtest_object(5), hxtest_object(3), hxtest_object(1) };
	hxtest_object right_desc[3] = { hxtest_object(6), hxtest_object(4), hxtest_object(2) };
	hxtest_object dest_desc[6] = {
		hxtest_object(0), hxtest_object(0), hxtest_object(0),
		hxtest_object(0), hxtest_object(0), hxtest_object(0)
	};
	const hxtest_rand_iter_api_t merge_desc_end =
		hxmerge(hxtest_rand_iter_api_t(left_desc), hxtest_rand_iter_api_t(left_desc + 3),
			hxtest_rand_iter_api_t(right_desc), hxtest_rand_iter_api_t(right_desc + 3),
			hxtest_rand_iter_api_t(dest_desc), hxtest_value_greater);
	EXPECT_EQ(merge_desc_end, hxtest_rand_iter_api_t(dest_desc) + ptrdiff_t{6});
	const int expected_desc[6] = { 6, 5, 4, 3, 2, 1 };
	for(hxsize_t i = 0; i < 6; ++i) {
		EXPECT_EQ(dest_desc[i].value, expected_desc[i]);
	}
	EXPECT_TRUE(check_stats(24, 0, 0, 0, 0, 12, 0, 0));
}

TEST(hxminmax_test, small_ranges) {
	const int one[1] = { 34 };
	hxminmax_result<const int*> result = hxminmax(one, one + 1);
	EXPECT_EQ(result.min, one);
	EXPECT_EQ(result.max, one);
	const int ascending[2] = { 7, 11 };
	result = hxminmax(ascending, ascending + 2);
	EXPECT_EQ(result.min, ascending);
	EXPECT_EQ(result.max, ascending + 1);
	const int descending[2] = { 11, 7 };
	result = hxminmax(descending, descending + 2);
	EXPECT_EQ(result.min, descending + 1);
	EXPECT_EQ(result.max, descending);
	const int ties[6] = { 3, 1, 2, 1, 5, 5 };
	result = hxminmax(ties, ties + 6);
	EXPECT_EQ(result.min, ties + 1);
	EXPECT_EQ(result.max, ties + 4);
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

TEST_F(hxminmax_test_f, iterator_support) {
	hxtest_object values[4] = { hxtest_object(30), hxtest_object(10), hxtest_object(40), hxtest_object(20) };
	const hxtest_forward_iter_api_t begin(values);
	const hxtest_forward_iter_api_t end(values + 4);
	const hxminmax_result<hxtest_forward_iter_api_t> result =
		hxminmax(begin, end, hxtest_value_less);
	EXPECT_NE(result.min, end);
	EXPECT_EQ((*result.min).value, 10);
	EXPECT_NE(result.max, end);
	EXPECT_EQ((*result.max).value, 40);
	const hxminmax_result<hxtest_forward_iter_api_t> empty_result =
		hxminmax(begin, begin, hxtest_value_less);
	EXPECT_EQ(empty_result.min, begin);
	EXPECT_EQ(empty_result.max, begin);
	EXPECT_TRUE(check_stats(4, 0, 0, 0, 0, 0, 0, 0));
}

TEST_F(hxquantifier_test_f, all_of_any_of_and_for_each) {
	hxtest_object values[3] = { hxtest_object(10), hxtest_object(20), hxtest_object(30) };
	const hxtest_forward_iter_api_t begin(values);
	const hxtest_forward_iter_api_t end(values + 3);
	EXPECT_TRUE(hxall_of(begin, end, [](const hxtest_object& x) { return x.value >= 10; }));
	EXPECT_FALSE(hxall_of(begin, end, [](const hxtest_object& x) { return x.value >= 30; }));
	EXPECT_TRUE(hxall_of(begin, begin, [](const hxtest_object& x) { return x.value >= 100; }));
	EXPECT_TRUE(hxany_of(begin, end, [](const hxtest_object& x) { return x.value == 30; }));
	EXPECT_FALSE(hxany_of(begin, end, [](const hxtest_object& x) { return x.value == 99; }));
	EXPECT_FALSE(hxany_of(begin, begin, [](const hxtest_object& x) { return x.value == 10; }));
	struct hxfor_each_test_accumulator_t {
		int total;
		void operator()(const hxtest_object& x) { total += x.value; }
	};
	const hxfor_each_test_accumulator_t result =
		hxfor_each(begin, end, hxfor_each_test_accumulator_t{0});
	EXPECT_EQ(result.total, 60);
	const hxfor_each_test_accumulator_t empty_result =
		hxfor_each(begin, begin, hxfor_each_test_accumulator_t{7});
	EXPECT_EQ(empty_result.total, 7);
	EXPECT_TRUE(check_stats(3, 0, 0, 0, 0, 0, 0, 0));
}

#if HX_CPLUSPLUS >= 201402L
TEST(hxset_algorithms_test, edge_cases) {
	const int single_left[1] = { 2 };
	const int single_right[1] = { 3 };
	int dest_union[2] = { };
	int dest_intersection[1] = { };
	int dest_difference[1] = { };
	const int* union_end = hxset_union(single_left + 0, single_left + 1, single_right + 0, single_right + 1, dest_union + 0);
	EXPECT_EQ(union_end - dest_union, ptrdiff_t{2});
	EXPECT_EQ(dest_union[0], 2);
	EXPECT_EQ(dest_union[1], 3);
	const int* intersection_end = hxset_intersection(single_left + 0, single_left + 1, single_right + 0, single_right + 1, dest_intersection + 0);
	EXPECT_EQ(intersection_end - dest_intersection, ptrdiff_t{0});
	const int* difference_end = hxset_difference(single_left + 0, single_left + 1, single_right + 0, single_right + 1, dest_difference + 0);
	EXPECT_EQ(difference_end - dest_difference, ptrdiff_t{1});
	EXPECT_EQ(dest_difference[0], 2);
	const int left3[3] = { 1, 5, 9 };
	const int right1[1] = { 5 };
	int dest3[3] = { };
	const int* diff_end = hxset_difference(left3 + 0, left3 + 3, right1 + 0, right1 + 1, dest3 + 0);
	EXPECT_EQ(diff_end - dest3, ptrdiff_t{2});
	EXPECT_EQ(dest3[0], 1);
	EXPECT_EQ(dest3[1], 9);
	int dest4[1] = { };
	const int* inter_end = hxset_intersection(left3 + 0, left3 + 3, right1 + 0, right1 + 1, dest4 + 0);
	EXPECT_EQ(inter_end - dest4, ptrdiff_t{1});
	EXPECT_EQ(dest4[0], 5);
}

TEST(hxset_algorithms_test, largest_element_at_tail) {
	const int left_a[3] = { 1, 3, 9 };
	const int right_a[2] = { 2, 4 };
	int dest_a[5] = { };
	const int* union_end_a = hxset_union(left_a + 0, left_a + 3, right_a + 0, right_a + 2, dest_a + 0);
	EXPECT_EQ(union_end_a - dest_a, ptrdiff_t{5});
	EXPECT_EQ(dest_a[4], 9);
	const int left_b[2] = { 1, 3 };
	const int right_b[3] = { 2, 4, 9 };
	int dest_b[5] = { };
	const int* union_end_b = hxset_union(left_b + 0, left_b + 2, right_b + 0, right_b + 3, dest_b + 0);
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

TEST_F(hxset_algorithms_test_f, hxarray_output_iterator_merge) {
	const hxsystem_allocator_scope temporary_stack_scope(hxsystem_allocator_stack_0);
	auto expect_hxarray = [](const hxvector<hxtest_object>& actual, const int* expected, hxsize_t count) {
		ASSERT_EQ(actual.size(), count);
		for(hxsize_t i = 0; i < count; ++i) {
			EXPECT_EQ(actual[i].value, expected[i]);
		}
	};
	hxtest_object left[] = { hxtest_object(1), hxtest_object(2), hxtest_object(4) };
	hxtest_object right[] = { hxtest_object(2), hxtest_object(4), hxtest_object(5) };
	hxvector<hxtest_object> merge_output;
	merge_output.reserve(hxsize(left) + hxsize(right) + 1);
	merge_output.push_back(hxtest_object(0));
	{
		const hxvector<hxtest_object>& merge_ret = hxmerge(
			hxtest_rand_iter_api_t(left), hxtest_rand_iter_api_t(left + hxsize(left)),
			hxtest_rand_iter_api_t(right), hxtest_rand_iter_api_t(right + hxsize(right)), merge_output);
		EXPECT_EQ(&merge_ret, &merge_output);
	}
	const int expected_merge[] = { 0, 1, 2, 2, 4, 4, 5 };
	expect_hxarray(merge_output, expected_merge, hxsize(expected_merge));
	EXPECT_TRUE(check_stats(14, 1, 0, 7, 0, 0, 0, 4));
}

TEST_F(hxset_algorithms_test_f, hxarray_output_iterator_union) {
	const hxsystem_allocator_scope temporary_stack_scope(hxsystem_allocator_stack_0);
	auto expect_hxarray = [](const hxvector<hxtest_object>& actual, const int* expected, hxsize_t count) {
		ASSERT_EQ(actual.size(), count);
		for(hxsize_t i = 0; i < count; ++i) {
			EXPECT_EQ(actual[i].value, expected[i]);
		}
	};
	hxtest_object left[] = { hxtest_object(1), hxtest_object(2), hxtest_object(4) };
	hxtest_object right[] = { hxtest_object(2), hxtest_object(4), hxtest_object(5) };
	hxvector<hxtest_object> union_output;
	union_output.reserve(hxsize(left) + hxsize(right) + 1);
	union_output.push_back(hxtest_object(0));
	{
		const hxvector<hxtest_object>& union_ret = hxset_union(
			hxtest_rand_iter_api_t(left), hxtest_rand_iter_api_t(left + hxsize(left)),
			hxtest_rand_iter_api_t(right), hxtest_rand_iter_api_t(right + hxsize(right)), union_output);
		EXPECT_EQ(&union_ret, &union_output);
	}
	const int expected_union[] = { 0, 1, 2, 4, 5 };
	expect_hxarray(union_output, expected_union, hxsize(expected_union));
	EXPECT_TRUE(check_stats(12, 1, 0, 5, 0, 0, 0, 6));
}

TEST_F(hxset_algorithms_test_f, hxarray_output_iterator_intersection) {
	const hxsystem_allocator_scope temporary_stack_scope(hxsystem_allocator_stack_0);
	auto expect_hxarray = [](const hxvector<hxtest_object>& actual, const int* expected, hxsize_t count) {
		ASSERT_EQ(actual.size(), count);
		for(hxsize_t i = 0; i < count; ++i) {
			EXPECT_EQ(actual[i].value, expected[i]);
		}
	};
	hxtest_object left[] = { hxtest_object(1), hxtest_object(2), hxtest_object(4) };
	hxtest_object right[] = { hxtest_object(2), hxtest_object(4), hxtest_object(5) };
	hxvector<hxtest_object> intersection_output;
	intersection_output.reserve(hxsize(left) + 1);
	intersection_output.push_back(hxtest_object(0));
	{
		const hxvector<hxtest_object>& intersection_ret = hxset_intersection(
			hxtest_rand_iter_api_t(left), hxtest_rand_iter_api_t(left + hxsize(left)),
			hxtest_rand_iter_api_t(right), hxtest_rand_iter_api_t(right + hxsize(right)), intersection_output);
		EXPECT_EQ(&intersection_ret, &intersection_output);
	}
	const int expected_intersection[] = { 0, 2, 4 };
	expect_hxarray(intersection_output, expected_intersection, hxsize(expected_intersection));
	EXPECT_TRUE(check_stats(10, 1, 0, 3, 0, 0, 0, 6));
}

TEST_F(hxset_algorithms_test_f, hxarray_output_iterator_difference) {
	const hxsystem_allocator_scope temporary_stack_scope(hxsystem_allocator_stack_0);
	auto expect_hxarray = [](const hxvector<hxtest_object>& actual, const int* expected, hxsize_t count) {
		ASSERT_EQ(actual.size(), count);
		for(hxsize_t i = 0; i < count; ++i) {
			EXPECT_EQ(actual[i].value, expected[i]);
		}
	};
	hxtest_object left[] = { hxtest_object(1), hxtest_object(2), hxtest_object(4) };
	hxtest_object right[] = { hxtest_object(2), hxtest_object(4), hxtest_object(5) };
	hxvector<hxtest_object> difference_output;
	difference_output.reserve(hxsize(left) + 1);
	difference_output.push_back(hxtest_object(0));
	{
		const hxvector<hxtest_object>& difference_ret = hxset_difference(
			hxtest_rand_iter_api_t(left), hxtest_rand_iter_api_t(left + hxsize(left)),
			hxtest_rand_iter_api_t(right), hxtest_rand_iter_api_t(right + hxsize(right)), difference_output);
		EXPECT_EQ(&difference_ret, &difference_output);
	}
	const int expected_difference[] = { 0, 1 };
	expect_hxarray(difference_output, expected_difference, hxsize(expected_difference));
	EXPECT_TRUE(check_stats(9, 1, 0, 2, 0, 0, 0, 6));
}

TEST_F(hxset_algorithms_test_f, hxarray_output_iterator_right_exhausted_first_union) {
	const hxsystem_allocator_scope temporary_stack_scope(hxsystem_allocator_stack_0);
	auto expect_hxarray = [](const hxvector<hxtest_object>& actual, const int* expected, hxsize_t count) {
		ASSERT_EQ(actual.size(), count);
		for(hxsize_t i = 0; i < count; ++i) {
			EXPECT_EQ(actual[i].value, expected[i]);
		}
	};
	hxtest_object left[] = { hxtest_object(1), hxtest_object(2), hxtest_object(4) };
	hxtest_object right[] = { hxtest_object(3) };
	hxvector<hxtest_object> union_output;
	union_output.reserve(hxsize(left) + hxsize(right));
	{
		const hxvector<hxtest_object>& union_ret = hxset_union(
			hxtest_rand_iter_api_t(left), hxtest_rand_iter_api_t(left + hxsize(left)),
			hxtest_rand_iter_api_t(right), hxtest_rand_iter_api_t(right + hxsize(right)), union_output);
		EXPECT_EQ(&union_ret, &union_output);
	}
	const int expected_union[] = { 1, 2, 3, 4 };
	expect_hxarray(union_output, expected_union, hxsize(expected_union));
	EXPECT_TRUE(check_stats(8, 0, 0, 4, 0, 0, 0, 6));
}

TEST_F(hxset_algorithms_test_f, hxarray_output_iterator_right_exhausted_first_intersection) {
	const hxsystem_allocator_scope temporary_stack_scope(hxsystem_allocator_stack_0);
	hxtest_object left[] = { hxtest_object(1), hxtest_object(2), hxtest_object(4) };
	hxtest_object right[] = { hxtest_object(3) };
	hxvector<hxtest_object> intersection_output;
	intersection_output.reserve(hxsize(left));
	{
		const hxvector<hxtest_object>& intersection_ret = hxset_intersection(
			hxtest_rand_iter_api_t(left), hxtest_rand_iter_api_t(left + hxsize(left)),
			hxtest_rand_iter_api_t(right), hxtest_rand_iter_api_t(right + hxsize(right)), intersection_output);
		EXPECT_EQ(&intersection_ret, &intersection_output);
	}
	EXPECT_EQ(intersection_output.size(), hxsize_t{0});
	EXPECT_TRUE(check_stats(4, 0, 0, 0, 0, 0, 0, 6));
}

TEST_F(hxset_algorithms_test_f, hxarray_output_iterator_right_exhausted_first_difference) {
	const hxsystem_allocator_scope temporary_stack_scope(hxsystem_allocator_stack_0);
	auto expect_hxarray = [](const hxvector<hxtest_object>& actual, const int* expected, hxsize_t count) {
		ASSERT_EQ(actual.size(), count);
		for(hxsize_t i = 0; i < count; ++i) {
			EXPECT_EQ(actual[i].value, expected[i]);
		}
	};
	hxtest_object left[] = { hxtest_object(1), hxtest_object(2), hxtest_object(4) };
	hxtest_object right[] = { hxtest_object(3) };
	hxvector<hxtest_object> difference_output;
	difference_output.reserve(hxsize(left));
	{
		const hxvector<hxtest_object>& difference_ret = hxset_difference(
			hxtest_rand_iter_api_t(left), hxtest_rand_iter_api_t(left + hxsize(left)),
			hxtest_rand_iter_api_t(right), hxtest_rand_iter_api_t(right + hxsize(right)), difference_output);
		EXPECT_EQ(&difference_ret, &difference_output);
	}
	const int expected_difference[] = { 1, 2, 4 };
	expect_hxarray(difference_output, expected_difference, hxsize(expected_difference));
	EXPECT_TRUE(check_stats(7, 0, 0, 3, 0, 0, 0, 6));
}
#endif // HX_CPLUSPLUS >= 201402L

TEST_F(hxtest_test_f, iter_api_types) {
	hxtest_object values[2] = {
		hxtest_object(11),
		hxtest_object(22)
	};
	EXPECT_TRUE(hxtest_check_forward_iter_api(
		hxtest_forward_iter_api_t(values), hxtest_forward_iter_api_t(values + 2)));
	EXPECT_TRUE(hxtest_check_forward_iter_api(
		hxtest_bidirectional_iter_api_t(values), hxtest_bidirectional_iter_api_t(values + 2)));
	EXPECT_TRUE(hxtest_check_bidirectional_iter_api(
		hxtest_bidirectional_iter_api_t(values), hxtest_bidirectional_iter_api_t(values + 2)));
	EXPECT_TRUE(hxtest_check_forward_iter_api(
		hxtest_rand_iter_api_t(values), hxtest_rand_iter_api_t(values + 2)));
	EXPECT_TRUE(hxtest_check_bidirectional_iter_api(
		hxtest_rand_iter_api_t(values), hxtest_rand_iter_api_t(values + 2)));
	EXPECT_TRUE(hxtest_check_rand_iter_api(
		hxtest_rand_iter_api_t(values), hxtest_rand_iter_api_t(values + 2)));
	EXPECT_TRUE(check_stats(2, 0, 0, 0, 0, 0, 2, 0));
}

TEST_F(hxunique_test_f, simple_case) {
	hxtest_object values[7] = {
		hxtest_object(1), hxtest_object(1), hxtest_object(2), hxtest_object(3),
		hxtest_object(3), hxtest_object(3), hxtest_object(4)
	};
	const hxtest_forward_iter_api_t begin(values);
	const hxtest_forward_iter_api_t end(values + 7);
	const hxtest_forward_iter_api_t new_end =
		hxunique(begin, end, hxtest_value_equal);
	EXPECT_EQ(new_end, hxtest_forward_iter_api_t(values + 4));
	EXPECT_EQ(values[0].value, 1);
	EXPECT_EQ(values[1].value, 2);
	EXPECT_EQ(values[2].value, 3);
	EXPECT_EQ(values[3].value, 4);
	hxtest_object no_dup_values[3] = {
		hxtest_object(1), hxtest_object(2), hxtest_object(3)
	};
	const hxtest_forward_iter_api_t no_dup_begin(no_dup_values);
	const hxtest_forward_iter_api_t no_dup_end(no_dup_values + 3);
	const hxtest_forward_iter_api_t no_dup_new_end =
		hxunique(no_dup_begin, no_dup_end, hxtest_value_equal);
	EXPECT_EQ(no_dup_new_end, no_dup_end);
	EXPECT_TRUE(check_stats(10, 0, 0, 0, 0, 3, 0, 0));
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
	hxtest_object value[1] = { hxtest_object(1) };
	const hxtest_forward_iter_api_t begin(value);
	const hxtest_forward_iter_api_t empty_end =
		hxunique(begin, begin, hxtest_value_equal);
	EXPECT_EQ(empty_end, begin);
	EXPECT_TRUE(check_stats(1, 0, 0, 0, 0, 0, 0, 0));
}
