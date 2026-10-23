// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

#include <hx/hxsort.hpp>
#include <hx/hxrandom.hpp>
#include <hx/hxvector.hpp>
#include <hx/hxtest.hpp>
#include "./hxtest_util.hpp"

HX_NS_USE

using hxsort_test_f = hxtest_object_fixture;

#if HX_CPLUSPLUS >= 201402L
namespace {

template<typename iterator_t, typename sort_callback_t>
void test_sort_cases(const sort_callback_t& sort_callback) {
	const int initial_values[5] = { 2, 1, 0, 4, -5 };
	const int expected_two[5] = { 1, 2, 0, 4, -5 };
	const int expected_sorted[5] = { -5, 0, 1, 2, 4 };
	const int expected_descending[5] = { 4, 2, 1, 0, -5 };
	hxvector<hxtest_object, 5> values(initial_values);
	auto expect_values = [&](const int (&expected)[5]) {
		for(hxsize_t i = 0; i < 5; ++i) {
			EXPECT_EQ(values[i].value(), expected[i]);
		}
	};
	sort_callback(iterator_t(values.data()), iterator_t(values.data()), hxtest_value_less);
	expect_values(initial_values);
	sort_callback(iterator_t(values.data()), iterator_t(values.data() + 1), hxtest_value_less);
	expect_values(initial_values);
	sort_callback(iterator_t(values.data()), iterator_t(values.data() + 2), hxtest_value_less);
	expect_values(expected_two);
	sort_callback(iterator_t(values.data()), iterator_t(values.data() + 5), hxtest_value_less);
	expect_values(expected_sorted);
	sort_callback(iterator_t(values.data()), iterator_t(values.data() + 5), hxtest_value_greater);
	expect_values(expected_descending);
	sort_callback(iterator_t(values.data()), iterator_t(values.data() + 5), hxtest_value_less);
	expect_values(expected_sorted);
}

void test_partition_sort_case(const int (&initial_values)[11], const int (&expected_sorted)[11]) {
	hxvector<hxtest_object, 11> values(initial_values);
	hxsort(values.begin(), values.end());
	for(hxsize_t i = 0; i < 11; ++i) {
		EXPECT_EQ(values[i].value(), expected_sorted[i]);
	}
	hxvector<hxtest_object, 11> iterator_values(initial_values);
	hxsort(hxtest_rand_iterator_api_t(iterator_values.data()),
		hxtest_rand_iterator_api_t(iterator_values.data() + 11), hxtest_value_less);
	for(hxsize_t i = 0; i < 11; ++i) {
		EXPECT_EQ(iterator_values[i].value(), expected_sorted[i]);
	}
}

} // namespace

TEST_F(hxsort_test_f, sort_cases) {
	test_sort_cases<hxtest_object*>(
		hxinsertion_sort<hxtest_object*, bool (*)(const hxtest_object&, const hxtest_object&)>);
	test_sort_cases<hxtest_object*>(
		hxheapsort<hxtest_object*, bool (*)(const hxtest_object&, const hxtest_object&)>);
	test_sort_cases<hxtest_object*>(
		hxsort<hxtest_object*, bool (*)(const hxtest_object&, const hxtest_object&)>);
	EXPECT_TRUE(check_stats(48, 0, 15, 0, 33, 0, 114, 0, 87, 0));
}

TEST_F(hxsort_test_f, sort_grinder) {
	hxrandom rng(2);
	const hxsize_t max_size_mask = 0x7f;
	hxvector<hxtest_object> insertion_sorted; insertion_sorted.reserve(max_size_mask);
	hxvector<hxtest_object> heap_sorted; heap_sorted.reserve(max_size_mask);
	hxvector<hxtest_object> generic_sorted; generic_sorted.reserve(max_size_mask);
	for(int i=12; i-- != 0; ) {
		const hxsize_t size = (max_size_mask >> i) & rng.u32();
		for(hxsize_t j = size; j-- != 0;) {
			const int x = rng.range(100, 200);
			insertion_sorted.push_back(hxtest_object(x));
			heap_sorted.push_back(hxtest_object(0));
			generic_sorted.push_back(hxtest_object(0));
		}
		const hxsize_t element_count = insertion_sorted.size();
		for(hxsize_t j = 0; j < element_count; ++j) {
			heap_sorted[j] = hxtest_object(insertion_sorted[j].value());
			generic_sorted[j] = hxtest_object(insertion_sorted[j].value());
		}
		hxinsertion_sort(insertion_sorted.begin(), insertion_sorted.end());
		hxheapsort(heap_sorted.begin(), heap_sorted.end());
		hxsort(generic_sorted.begin(), generic_sorted.end());
		EXPECT_TRUE(hxequal_range(insertion_sorted, heap_sorted));
		EXPECT_TRUE(hxequal_range(insertion_sorted, generic_sorted));
		insertion_sorted.clear();
		heap_sorted.clear();
		generic_sorted.clear();
	}
	EXPECT_TRUE(check_stats(1698, 0, 675, 0, 1023, 0, 4743, 270, 4938, 0));
}

TEST_F(hxsort_test_f, sort_grinder_values_match_with_duplicate_keys) {
	hxrandom rng(2);
	const hxsize_t max_size_mask = 0x7f;
	hxvector<hxtest_object> insertion_sorted; insertion_sorted.reserve(max_size_mask);
	hxvector<hxtest_object> heap_sorted; heap_sorted.reserve(max_size_mask);
	hxvector<hxtest_object> generic_sorted; generic_sorted.reserve(max_size_mask);
	for(int i=12; i-- != 0; ) {
		const hxsize_t size = (max_size_mask >> i) & rng.u32();
		for(hxsize_t j = size; j-- != 0;) {
			const int x = rng.range(100, 200);
			insertion_sorted.push_back(hxtest_object(x));
			heap_sorted.push_back(hxtest_object(x));
			generic_sorted.push_back(hxtest_object(x));
		}
		hxinsertion_sort(insertion_sorted.begin(), insertion_sorted.end());
		hxheapsort(heap_sorted.begin(), heap_sorted.end());
		hxsort(generic_sorted.begin(), generic_sorted.end());
		const hxsize_t element_count = insertion_sorted.size();
		for(hxsize_t j = 0; j < element_count; ++j) {
			EXPECT_EQ(insertion_sorted[j].value(), heap_sorted[j].value());
			EXPECT_EQ(insertion_sorted[j].value(), generic_sorted[j].value());
		}
		insertion_sorted.clear();
		heap_sorted.clear();
		generic_sorted.clear();
	}
	EXPECT_TRUE(check_stats(1428, 0, 405, 0, 1023, 0, 4473, 0, 4938, 0));
}

TEST_F(hxsort_test_f, intro_sort_pivot_killer_reaches_depth_limit_and_heapsorts) {
	const int initial_values[69] = {
		8, 35, 46, 44, 61, 12, 59, 65, 28, 37, 16, 58, 24, 45, 0, 20, 39, 4, 29, 41, 60, 43, 25,
		64, 1, 48, 55, 5, 56, 42, 51, 54, 9, 49, 2, 13, 47, 6, 50, 17, 10, 52, 21, 14, 57, 40, 18,
		66, 32, 22, 68, 36, 26, 33, 3, 30, 11, 7, 34, 15, 19, 38, 67, 23, 27, 63, 62, 31, 53 };
	{
		hxvector<hxtest_object, 69> values(initial_values);
		hxsort(values.begin(), values.end());
		for(hxsize_t i = 0; i < 69; ++i) {
			EXPECT_EQ(values[i].value(), static_cast<int32_t>(i));
		}
	}
	EXPECT_TRUE(check_stats(639, 0, 69, 0, 570, 0, 1197, 0, 1299, 0));
}

TEST_F(hxsort_test_f, sort_all_equal_skips_middle_partition) {
	const hxsize_t count = 2048;
	{
		hxvector<hxtest_object> values; values.reserve(count);
		values.generate_n(count, []() { return hxtest_object(7); });
		hxsort(values.begin(), values.end());
		for(hxsize_t i = 0; i < count; ++i) {
			EXPECT_EQ(values[i].value(), 7);
		}
	}
	EXPECT_TRUE(check_stats(2050, 0, 2048, 0, 2, 0, 4, 0, 4102, 0));
}

TEST_F(hxsort_test_f, sort_grinder_generic) {
	hxrandom rng(3);
	const hxsize_t max_size_mask = 0xfff;
	hxvector<hxtest_object> sorted; sorted.reserve(max_size_mask);
	hxvector<int> histogram(20000, 0);
	for(int i=10; i-- != 0; ) {
		const hxsize_t size = (max_size_mask >> i) & rng.u32();
		if(size <= 16) {
			continue;
		}
		for(hxsize_t j = size; j-- != 0;) {
			const int x = rng.range(10000, 10000);
			sorted.push_back(hxtest_object(x));
			++histogram[static_cast<hxsize_t>(x)];
		}
		hxsort(sorted.begin(), sorted.end());
		--histogram[static_cast<hxsize_t>(sorted[size - 1].value())];
		for(hxsize_t j=size - 1; j-- != 0;) {
			--histogram[static_cast<hxsize_t>(sorted[j].value())];
			EXPECT_FALSE(hxkey_less(sorted[j + 1], sorted[j]));
		}
		for(hxsize_t j=20000; j-- > 10000;) {
			EXPECT_EQ(histogram[j], 0);
		}
		sorted.clear();
	}
	EXPECT_TRUE(check_stats(18089, 0, 3208, 0, 14881, 0, 25762, 0, 38010, 0));
}

TEST_F(hxsort_test_f, partition_sort_network_all_ascending_takes_no_swaps) {
	const int initial_values[11] = { 1000, 10, 1002, 20, 1004, 30, 1006, 40, 1008, 50, 1010 };
	const int expected_sorted[11] = { 10, 20, 30, 40, 50, 1000, 1002, 1004, 1006, 1008, 1010 };
	test_partition_sort_case(initial_values, expected_sorted);
	EXPECT_TRUE(check_stats(46, 0, 22, 0, 24, 0, 66, 0, 86, 0));
}

TEST_F(hxsort_test_f, partition_sort_network_all_but_p3_p1_and_p3_p2_swap) {
	const int initial_values[11] = { 1000, 50, 1002, 20, 1004, 30, 1006, 40, 1008, 10, 1010 };
	const int expected_sorted[11] = { 10, 20, 30, 40, 50, 1000, 1002, 1004, 1006, 1008, 1010 };
	test_partition_sort_case(initial_values, expected_sorted);
	EXPECT_TRUE(check_stats(46, 0, 22, 0, 24, 0, 68, 0, 88, 0));
}

TEST_F(hxsort_test_f, partition_sort_network_p3_p1_and_p3_p2_swap) {
	const int initial_values[11] = { 1000, 10, 1002, 30, 1004, 40, 1006, 20, 1008, 50, 1010 };
	const int expected_sorted[11] = { 10, 20, 30, 40, 50, 1000, 1002, 1004, 1006, 1008, 1010 };
	test_partition_sort_case(initial_values, expected_sorted);
	EXPECT_TRUE(check_stats(52, 0, 22, 0, 30, 0, 80, 0, 88, 0));
}

TEST_F(hxsort_test_f, partition_sort_loop_lt_and_gt_swaps_taken_and_skipped) {
	const int initial_values[11] = { 100, 101, 1, 103, 2, 105, 3, 107, 108, 109, 110 };
	const int expected_sorted[11] = { 1, 2, 3, 100, 101, 103, 105, 107, 108, 109, 110 };
	test_partition_sort_case(initial_values, expected_sorted);
	EXPECT_TRUE(check_stats(40, 0, 22, 0, 18, 0, 48, 0, 70, 0));
}

TEST_F(hxsort_test_f, partition_sort_all_equal_takes_no_pivot_swaps) {
	const int all_equal[11] = { 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7 };
	test_partition_sort_case(all_equal, all_equal);
	EXPECT_TRUE(check_stats(26, 0, 22, 0, 4, 0, 8, 0, 56, 0));
}

TEST_F(hxsort_test_f, partition_sort_function_pointer_comparator_distinct_and_equal) {
	const hxsize_t count = 11;
	{
		hxvector<hxtest_object> values; values.reserve(count);
		hxsize_t index = count;
		values.generate_n(count, [&index]() { return hxtest_object(static_cast<int32_t>(--index)); });
		hxsort<hxtest_object*, bool (*)(const hxtest_object&, const hxtest_object&)>(
			values.begin(), values.end(), hxtest_value_less);
		for(hxsize_t i = 0; i < count; ++i) {
			EXPECT_EQ(values[i].value(), static_cast<int32_t>(i));
		}
		values.clear();
		values.generate_n(count, []() { return hxtest_object(7); });
		hxsort<hxtest_object*, bool (*)(const hxtest_object&, const hxtest_object&)>(
			values.begin(), values.end(), hxtest_value_less);
		for(hxsize_t i = 0; i < count; ++i) {
			EXPECT_EQ(values[i].value(), 7);
		}
	}
	EXPECT_TRUE(check_stats(30, 0, 22, 0, 8, 0, 19, 0, 60, 0));
}

TEST_F(hxsort_test_f, sort_thousand_random_vectors_stays_within_work_budget) {
	hxsize_t elements = 0;
	{
		const hxsize_t trials = 1000;
		hxrandom rng(31u);
		hxvector<hxtest_object> values; values.reserve(1000);
		for(hxsize_t t = 0; t < trials; ++t) {
			const hxsize_t size = static_cast<hxsize_t>(rng.range(int64_t{100}, int64_t{801}));
			values.clear();
			values.generate_n(size, [&rng]() {
				return hxtest_object(static_cast<int32_t>(rng.u32() >> 1));
			});
			elements += size;
			hxsort(values.begin(), values.end());
			for(hxsize_t i = 1; i < values.size(); ++i) {
				ASSERT_FALSE(values[i] < values[i - hxsize_t{1}]);
			}
		}
	}
	hxlog("sort_survey elements %zd less_than %d moves %d\n", elements,
		m_less_than, m_move_assign + m_move_construct);
	EXPECT_EQ(elements, hxsize_t{493537});
	EXPECT_TRUE(check_stats(2176957, 0, 493537, 0, 1683420, 0, 3742158, 0, 5266078, 0));
}

TEST_F(hxsort_test_f, heapsort_above_cutoff_builds_heap_and_drains) {
	const hxsize_t count = 21;
	{
		hxrandom rng(31u);
		hxvector<hxtest_object> values; values.reserve(count);
		values.generate_n(count, [&rng]() {
			return hxtest_object(static_cast<int32_t>(rng.range(int64_t{0}, int64_t{1000})));
		});
		hxvector<hxtest_object> less_values(values);
		hxheapsort(values.begin(), values.end());
		hxheapsort(less_values.begin(), less_values.end(), hxtest_value_less);
		for(hxsize_t i = 1; i < count; ++i) {
			EXPECT_FALSE(values[i] < values[i - hxsize_t{1}]);
			EXPECT_FALSE(less_values[i] < less_values[i - hxsize_t{1}]);
		}
	}
	EXPECT_TRUE(check_stats(102, 0, 21, 21, 60, 0, 224, 0, 296, 0));
}

TEST_F(hxsort_test_f, intro_sort_cutoff_boundary_at_cutoff_uses_insertion_sort) {
	const hxsize_t count = 10;
	{
		hxvector<hxtest_object> values; values.reserve(count);
		hxsize_t index = count;
		values.generate_n(count, [&index]() { return hxtest_object(static_cast<int32_t>(--index)); });
		hxsort(values.begin(), values.end());
		for(hxsize_t i = 0; i < count; ++i) {
			EXPECT_EQ(values[i].value(), static_cast<int32_t>(i));
		}
	}
	EXPECT_TRUE(check_stats(19, 0, 10, 0, 9, 0, 54, 0, 45, 0));
}

TEST_F(hxsort_test_f, intro_sort_cutoff_boundary_past_cutoff_uses_partition_sort) {
	const hxsize_t count = 11;
	{
		hxvector<hxtest_object> values; values.reserve(count);
		hxsize_t index = count;
		values.generate_n(count, [&index]() { return hxtest_object(static_cast<int32_t>(--index)); });
		hxsort(values.begin(), values.end());
		for(hxsize_t i = 0; i < count; ++i) {
			EXPECT_EQ(values[i].value(), static_cast<int32_t>(i));
		}
	}
	EXPECT_TRUE(check_stats(17, 0, 11, 0, 6, 0, 15, 0, 32, 0));
}

TEST_F(hxsort_test_f, insertion_sort_two_elements_reversed) {
	{
		hxvector<hxtest_object, 2> values{ 2, 1 };
		hxinsertion_sort(values.begin(), values.end());
		EXPECT_EQ(values[0].value(), 1);
		EXPECT_EQ(values[1].value(), 2);
	}
	EXPECT_TRUE(check_stats(3, 0, 2, 0, 1, 0, 2, 0, 1, 0));
}

TEST_F(hxsort_test_f, insertion_sort_three_elements_descending) {
	{
		hxvector<hxtest_object, 3> values{ 3, 2, 1 };
		hxinsertion_sort(values.begin(), values.end());
		EXPECT_EQ(values[0].value(), 1);
		EXPECT_EQ(values[1].value(), 2);
		EXPECT_EQ(values[2].value(), 3);
	}
	EXPECT_TRUE(check_stats(5, 0, 3, 0, 2, 0, 5, 0, 3, 0));
}

TEST_F(hxsort_test_f, insertion_sort_preserves_stable_ordering_of_equal_keys) {
	hxvector<hxtest_object, 6> values{ 5, 3, 5, 5, 1, 3 };
	hxinsertion_sort(values.begin(), values.end());
	const int32_t expected_value[6] = { 1, 3, 3, 5, 5, 5 };
	const uint16_t expected_ticket[6] = { 104u, 101u, 105u, 100u, 102u, 103u };
	for(hxsize_t i = 0; i < 6; ++i) {
		EXPECT_EQ(values[i].value(), expected_value[i]);
		EXPECT_EQ(values[i].ticket(), expected_ticket[i]);
	}
	EXPECT_TRUE(check_stats(9, 0, 6, 0, 3, 0, 11, 0, 11, 0));
}

TEST_F(hxsort_test_f, iterator_support) {
	test_sort_cases<hxtest_rand_iterator_api_t>([](hxtest_rand_iterator_api_t begin,
			hxtest_rand_iterator_api_t end, const auto& less) {
		hxinsertion_sort(begin, end, less);
	});
	test_sort_cases<hxtest_rand_iterator_api_t>([](hxtest_rand_iterator_api_t begin,
			hxtest_rand_iterator_api_t end, const auto& less) {
		hxheapsort(begin, end, less);
	});
	test_sort_cases<hxtest_rand_iterator_api_t>([](hxtest_rand_iterator_api_t begin,
			hxtest_rand_iterator_api_t end, const auto& less) {
		hxsort(begin, end, less);
	});
	EXPECT_TRUE(check_stats(48, 0, 15, 0, 33, 0, 114, 0, 87, 0));
}

TEST_F(hxsort_test_f, iterator_api_types) {
	{
		hxvector<hxtest_object, 2> values{ 0, 1 };
		EXPECT_TRUE(hxtest_check_forward_iterator_api(
			hxtest_forward_iterator_api_t(values.data()), hxtest_forward_iterator_api_t(values.data() + 2)));
		EXPECT_TRUE(hxtest_check_rand_iterator_api(
			hxtest_rand_iterator_api_t(values.data()), hxtest_rand_iterator_api_t(values.data() + 2)));
	}
	EXPECT_TRUE(check_stats(2, 0, 2, 0, 0, 0, 0, 2, 0, 0));
}
#endif // HX_CPLUSPLUS >= 201402L
#if HX_CPLUSPLUS >= 202302L
namespace {

consteval bool hxtest_hxinsertion_sort_consteval(void) {
	int arr[5] = { 33, 31, 34, 31, 35 };
	hxinsertion_sort(arr, arr + 5);
	return arr[0] == 31 && arr[1] == 31 && arr[2] == 33 && arr[3] == 34 && arr[4] == 35;
}
static_assert(hxtest_hxinsertion_sort_consteval(),
	"hxinsertion_sort consteval: must sort array correctly at compile time");
} // namespace
#endif // HX_CPLUSPLUS >= 202302L
