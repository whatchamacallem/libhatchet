// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

#include <hx/hxsort.hpp>
#include <hx/hxrandom.hpp>
#include <hx/hxvector.hpp>
#include <hx/hxtest.hpp>
#include "./hxtest_util.hpp"

HX_NS_USE

#if HX_CPLUSPLUS >= 201402L
namespace {

bool test_compare_int(int a, int b) { return a < b; }
bool test_compare_int_reverse(int a, int b) { return a > b; }
template<typename sort_callback_t>
void test_sort_int_cases(const sort_callback_t& sort_callback) {
	int ints[5] = { 2, 1, 0, 4, -5 };
	sort_callback(ints, ints, test_compare_int);
	const int ints1[5] = { 2, 1, 0, 4, -5 };
	EXPECT_EQ(::memcmp(ints, ints1, sizeof ints), 0);
	sort_callback(ints, ints + 1, test_compare_int);
	EXPECT_EQ(::memcmp(ints, ints1, sizeof ints), 0);
	sort_callback(ints, ints + 2, test_compare_int);
	const int ints2[5] = { 1, 2, 0, 4, -5 };
	EXPECT_EQ(::memcmp(ints, ints2, sizeof ints), 0);
	sort_callback(ints, ints + 5, test_compare_int);
	const int ints3[5] = { -5, 0, 1, 2, 4 };
	EXPECT_EQ(::memcmp(ints, ints3, sizeof ints), 0);
	sort_callback(ints, ints + 5, test_compare_int_reverse);
	const int ints4[5] = { 4, 2, 1, 0, -5 };
	EXPECT_EQ(::memcmp(ints, ints4, sizeof ints), 0);
	sort_callback(ints, ints + 5, test_compare_int);
	EXPECT_EQ(::memcmp(ints, ints3, sizeof ints), 0);
}
} // namespace

TEST(hxsort_test, sort_int_cases) {
	test_sort_int_cases(hxinsertion_sort<int*, bool (*)(int, int)>);
	test_sort_int_cases(hxheapsort<int*, bool (*)(int, int)>);
	test_sort_int_cases(hxsort<int*, bool (*)(int, int)>);
}

TEST(hxbinary_search_test, simple_case) {
	int ints[5] = { 2, 5, 6, 88, 99 };
	const int* const ints_end = ints+5;
	const int* result = hxbinary_search(ints, ints+5, 88, hxkey_less_t<int>{});
	EXPECT_TRUE(result != ints_end && *result == 88);
	const int* const_ints = ints;
	const int* cresult = hxbinary_search(const_ints, const_ints + 5, 2, hxkey_less_t<int>{});
	EXPECT_TRUE(cresult != ints_end && *cresult == 2);
	cresult = hxbinary_search(const_ints, const_ints + 5, 99);
	EXPECT_TRUE(cresult != ints_end && *cresult == 99);
	result = hxbinary_search(ints, ints+5, 0);
	EXPECT_EQ(result, ints_end);
	result = hxbinary_search(ints, ints+5, 100);
	EXPECT_EQ(result, ints_end);
	result = hxbinary_search(ints, ints+5, 7);
	EXPECT_EQ(result, ints_end);
	result = hxbinary_search(ints, ints, 11, hxkey_less_t<int>{});
	EXPECT_EQ(result, ints);
}

TEST(hxbinary_search_test, binary_search_grinder) {
	const hxsystem_allocator_scope temporary_stack_scope(hxsystem_allocator_stack_0);
	hxrandom rng(4);
	hxvector<hxtest_ref_tracker_t> sorted; sorted.reserve(100);
		for(int i=100; i-- != 0; ) {
			const int x = rng.range(0, 100);
		sorted.push_back(hxtest_ref_tracker_t(x));
	}
	hxsort(sorted.begin(), sorted.end());
		for(hxsize_t i=100; i-- != 0; ) {
		const hxtest_ref_tracker_t t = hxmove(sorted[i]);
		const hxtest_ref_tracker_t* const ptr = hxbinary_search(sorted.begin(), sorted.end(), t);
		EXPECT_TRUE(!(*ptr < t) && !(t < *ptr));
	}
}

TEST(hxsort_test, sort_grinder) {
	const hxsystem_allocator_scope temporary_stack_scope(hxsystem_allocator_stack_0);
	hxrandom rng(2);
	const hxsize_t max_size_mask = 0x7f;
	hxvector<hxtest_ref_tracker_t> insertion_sorted; insertion_sorted.reserve(max_size_mask);
	hxvector<hxtest_ref_tracker_t> heap_sorted; heap_sorted.reserve(max_size_mask);
	hxvector<hxtest_ref_tracker_t> generic_sorted; generic_sorted.reserve(max_size_mask);
	for(int i=12; i-- != 0; ) {
		const hxsize_t size = (max_size_mask >> i) & rng.u32();
		for(hxsize_t j = size; j-- != 0;) {
			insertion_sorted.push_back(hxtest_ref_tracker_t(rng.range(100, 200)));
			heap_sorted.push_back(hxtest_ref_tracker_t(0));
			generic_sorted.push_back(hxtest_ref_tracker_t(0));
		}
		const hxsize_t element_count = insertion_sorted.size();
		for(hxsize_t j = 0; j < element_count; ++j) {
			heap_sorted[j] = hxtest_ref_tracker_t(insertion_sorted[j].value);
			generic_sorted[j] = hxtest_ref_tracker_t(insertion_sorted[j].value);
		}
		hxinsertion_sort(insertion_sorted.begin(), insertion_sorted.end());
		hxheapsort(heap_sorted.begin(), heap_sorted.end());
		hxsort(generic_sorted.begin(), generic_sorted.end());
		EXPECT_EQ(::memcmp(insertion_sorted.data(), heap_sorted.data(), static_cast<size_t>(insertion_sorted.size_bytes())), 0);
		EXPECT_EQ(::memcmp(insertion_sorted.data(), generic_sorted.data(), static_cast<size_t>(insertion_sorted.size_bytes())), 0);
		insertion_sorted.clear();
		heap_sorted.clear();
		generic_sorted.clear();
	}
}

TEST(hxsort_test, intro_sort_depth_limit_falls_back_to_heapsort) {
	const hxsystem_allocator_scope temporary_stack_scope(hxsystem_allocator_stack_0);
	const hxsize_t count = 2048;
	hxvector<int> equal_values(count, 7);
	hxsort(equal_values.begin(), equal_values.end());
	for(hxsize_t i = 0; i < count; ++i) {
		EXPECT_EQ(equal_values[i], 7);
	}
}

TEST(hxsort_test, sort_grinder_generic) {
	const hxsystem_allocator_scope temporary_stack_scope(hxsystem_allocator_stack_0);
	hxrandom rng(3);
	const hxsize_t max_size_mask = 0xfff;
	hxvector<hxtest_ref_tracker_t> sorted; sorted.reserve(max_size_mask);
	hxvector<int> histogram(20000, 0);
	for(int i=10; i-- != 0; ) {
		const hxsize_t size = (max_size_mask >> i) & rng.u32();
		if(size <= 16) {
			continue;
		}
		for(hxsize_t j = size; j-- != 0;) {
			const int x = rng.range(10000, 10000);
			sorted.push_back(hxtest_ref_tracker_t(x));
			++histogram[static_cast<hxsize_t>(x)];
		}
		hxsort(sorted.begin(), sorted.end());
		--histogram[static_cast<hxsize_t>(sorted[size - 1].value)];
		for(hxsize_t j=size - 1; j-- != 0;) {
			--histogram[static_cast<hxsize_t>(sorted[j].value)];
			EXPECT_FALSE(hxkey_less(*sorted.get(j + 1), *sorted.get(j)));
		}
		for(hxsize_t j=20000; j-- > 10000;) {
			EXPECT_EQ(histogram[j], 0);
		}
		sorted.clear();
	}
}

TEST(hxsort_test, partition_sort_network_all_ascending_takes_no_swaps) {
	int arr[33] = {
		1000, 1001, 1002, 1003, 1004, 1005, 10, 1007, 1008, 1009, 1010,
		20, 1012, 1013, 1014, 1015, 30, 1017, 1018, 1019, 1020, 40, 1022,
		1023, 1024, 1025, 50, 1027, 1028, 1029, 1030, 1031, 1032
	};
	const int expected[33] = {
		10, 20, 30, 40, 50, 1000, 1001, 1002, 1003, 1004, 1005, 1007,
		1008, 1009, 1010, 1012, 1013, 1014, 1015, 1017, 1018, 1019, 1020,
		1022, 1023, 1024, 1025, 1027, 1028, 1029, 1030, 1031, 1032
	};
	hxsort(arr, arr + 33, test_compare_int);
	EXPECT_EQ(::memcmp(arr, expected, sizeof arr), 0);
}

TEST(hxsort_test, partition_sort_network_p3_p0_p4_p1_p2_p1_p4_p3_swap) {
	int arr[33] = {
		1000, 1001, 1002, 1003, 1004, 1005, 50, 1007, 1008, 1009, 1010,
		20, 1012, 1013, 1014, 1015, 30, 1017, 1018, 1019, 1020, 40, 1022,
		1023, 1024, 1025, 10, 1027, 1028, 1029, 1030, 1031, 1032
	};
	const int expected[33] = {
		10, 20, 30, 40, 50, 1000, 1001, 1002, 1003, 1004, 1005, 1007,
		1008, 1009, 1010, 1012, 1013, 1014, 1015, 1017, 1018, 1019, 1020,
		1022, 1023, 1024, 1025, 1027, 1028, 1029, 1030, 1031, 1032
	};
	hxsort(arr, arr + 33, test_compare_int);
	EXPECT_EQ(::memcmp(arr, expected, sizeof arr), 0);
}

TEST(hxsort_test, partition_sort_network_p3_p1_and_p3_p2_swap) {
	int arr[33] = {
		1000, 1001, 1002, 1003, 1004, 1005, 10, 1007, 1008, 1009, 1010,
		30, 1012, 1013, 1014, 1015, 40, 1017, 1018, 1019, 1020, 20, 1022,
		1023, 1024, 1025, 50, 1027, 1028, 1029, 1030, 1031, 1032
	};
	const int expected[33] = {
		10, 20, 30, 40, 50, 1000, 1001, 1002, 1003, 1004, 1005, 1007,
		1008, 1009, 1010, 1012, 1013, 1014, 1015, 1017, 1018, 1019, 1020,
		1022, 1023, 1024, 1025, 1027, 1028, 1029, 1030, 1031, 1032
	};
	hxsort(arr, arr + 33, test_compare_int);
	EXPECT_EQ(::memcmp(arr, expected, sizeof arr), 0);
}

TEST(hxsort_test, partition_sort_loop_lt_swap_taken_and_skipped) {
	int arr[33] = {
		100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112,
		113, 114, 1, 116, 117, 118, 119, 2, 121, 122, 123, 124, 3, 126,
		127, 128, 129, 130, 131, 132
	};
	const int expected[33] = {
		1, 2, 3, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110,
		111, 112, 113, 114, 116, 117, 118, 119, 121, 122, 123, 124, 126,
		127, 128, 129, 130, 131, 132
	};
	hxsort(arr, arr + 33, test_compare_int);
	EXPECT_EQ(::memcmp(arr, expected, sizeof arr), 0);
}

TEST(hxsort_test, partition_sort_all_equal_takes_no_pivot_swaps) {
	int arr[33];
	for(int i = 0; i < 33; ++i) {
		arr[i] = 7;
	}
	hxsort(arr, arr + 33, test_compare_int);
	for(int i = 0; i < 33; ++i) {
		EXPECT_EQ(arr[i], 7);
	}
}

TEST(hxsort_test, partition_sort_function_pointer_comparator_distinct_and_equal) {
	int arr[33];
	for(int i = 0; i < 33; ++i) {
		arr[i] = 32 - i;
	}
	hxsort<int*, bool (*)(int, int)>(arr, arr + 33, test_compare_int);
	for(int i = 0; i < 33; ++i) {
		EXPECT_EQ(arr[i], i);
	}
	for(int i = 0; i < 33; ++i) {
		arr[i] = 7;
	}
	hxsort<int*, bool (*)(int, int)>(arr, arr + 33, test_compare_int);
	for(int i = 0; i < 33; ++i) {
		EXPECT_EQ(arr[i], 7);
	}
}

TEST(hxsort_test, partition_sort_all_equal_ref_tracker_takes_no_pivot_swaps) {
	const hxsystem_allocator_scope temporary_stack_scope(hxsystem_allocator_stack_0);
	hxvector<hxtest_ref_tracker_t> values; values.reserve(33);
	for(int i = 33; i-- != 0; ) {
		values.push_back(hxtest_ref_tracker_t(7));
	}
	hxsort(values.begin(), values.end());
	for(hxsize_t i = 0; i < 33; ++i) {
		EXPECT_EQ(values[i].value, 7);
	}
}

TEST(hxsort_test, intro_sort_cutoff_boundary_thirty_two_uses_insertion_sort) {
	hxvector<int> values; values.reserve(32);
	for(int i = 32; i-- != 0; ) {
		values.push_back(31 - i);
	}
	hxsort(values.begin(), values.end());
	for(hxsize_t i = 0; i < 32; ++i) {
		EXPECT_EQ(values[i], static_cast<int>(i));
	}
}

TEST(hxsort_test, intro_sort_cutoff_boundary_thirty_three_uses_partition_sort) {
	hxvector<int> values; values.reserve(33);
	for(int i = 33; i-- != 0; ) {
		values.push_back(32 - i);
	}
	hxsort(values.begin(), values.end());
	for(hxsize_t i = 0; i < 33; ++i) {
		EXPECT_EQ(values[i], static_cast<int>(i));
	}
}

TEST(hxsort_test, hxsort_empty_range) {
	int ints[3] = { 3, 1, 2 };
	hxsort(ints, ints, test_compare_int);
	const int ints_unchanged[3] = { 3, 1, 2 };
	EXPECT_EQ(::memcmp(ints, ints_unchanged, sizeof ints), 0);
	hxsort(ints, ints + 1, test_compare_int);
	EXPECT_EQ(::memcmp(ints, ints_unchanged, sizeof ints), 0);
}

TEST(hxbinary_search_test, single_element_hit_and_miss) {
	const int arr[1] = { 34 };
	const int* result = hxbinary_search(arr, arr + 1, 34);
	EXPECT_EQ(result, arr);
	result = hxbinary_search(arr, arr + 1, 40);
	EXPECT_EQ(result, arr + 1);
	result = hxbinary_search(arr, arr + 1, 50);
	EXPECT_EQ(result, arr + 1);
}

TEST(hxbinary_search_test, two_element_boundaries) {
	const int arr[2] = { 10, 20 };
	const int* result = hxbinary_search(arr, arr + 2, 10);
	EXPECT_EQ(result, arr);
	result = hxbinary_search(arr, arr + 2, 20);
	EXPECT_EQ(result, arr + 1);
	result = hxbinary_search(arr, arr + 2, 15);
	EXPECT_EQ(result, arr + 2);
}

TEST(hxsort_test, insertion_sort_two_elements_reversed) {
	int arr[2] = { 2, 1 };
	hxinsertion_sort(arr, arr + 2, test_compare_int);
	EXPECT_EQ(arr[0], 1);
	EXPECT_EQ(arr[1], 2);
}

TEST(hxsort_test, insertion_sort_three_elements_descending) {
	int arr[3] = { 3, 2, 1 };
	hxinsertion_sort(arr, arr + 3, test_compare_int);
	EXPECT_EQ(arr[0], 1);
	EXPECT_EQ(arr[1], 2);
	EXPECT_EQ(arr[2], 3);
}

TEST(hxsort_test, heapsort_two_elements) {
	int arr[2] = { 2, 1 };
	hxheapsort(arr, arr + 2, test_compare_int);
	EXPECT_EQ(arr[0], 1);
	EXPECT_EQ(arr[1], 2);
}

TEST(hxsort_test, heapsort_three_elements_right_child_boundary) {
	int arr[3] = { 3, 1, 2 };
	hxheapsort(arr, arr + 3, test_compare_int);
	EXPECT_EQ(arr[0], 1);
	EXPECT_EQ(arr[1], 2);
	EXPECT_EQ(arr[2], 3);
}

TEST(hxbinary_search_test, last_element_in_five_element_array) {
	const int arr[5] = { 1, 3, 5, 7, 9 };
	const int* result = hxbinary_search(arr, arr + 5, 9);
	EXPECT_EQ(result, arr + 4);
	result = hxbinary_search(arr, arr + 5, 1);
	EXPECT_EQ(result, arr);
}
template<typename sort_callback_t>
static void do_sort_iter_case(const sort_callback_t& sort_callback) {
	const int initial_values[5] = { 2, 1, 0, 4, -5 };
	const int expected_two[5] = { 1, 2, 0, 4, -5 };
	const int expected_sorted[5] = { -5, 0, 1, 2, 4 };
	const int expected_descending[5] = { 4, 2, 1, 0, -5 };
	hxtest_ref_tracker_t values[5] = {
		hxtest_ref_tracker_t(initial_values[0]),
		hxtest_ref_tracker_t(initial_values[1]),
		hxtest_ref_tracker_t(initial_values[2]),
		hxtest_ref_tracker_t(initial_values[3]),
		hxtest_ref_tracker_t(initial_values[4])
	};
	auto reset = [&]() {
		for(hxsize_t i = 0; i < 5; ++i) {
			values[i] = hxtest_ref_tracker_t(initial_values[i]);
		}
	};
	auto expect_values = [&](const int (&expected)[5]) {
		for(hxsize_t i = 0; i < 5; ++i) {
			EXPECT_EQ(values[i].value, expected[i]);
		}
	};
	reset();
	sort_callback(hxtest_rand_iter_api_t(values), hxtest_rand_iter_api_t(values), hxtest_value_less);
	expect_values(initial_values);
	reset();
	sort_callback(hxtest_rand_iter_api_t(values), hxtest_rand_iter_api_t(values + 1), hxtest_value_less);
	expect_values(initial_values);
	reset();
	sort_callback(hxtest_rand_iter_api_t(values), hxtest_rand_iter_api_t(values + 2), hxtest_value_less);
	expect_values(expected_two);
	reset();
	sort_callback(hxtest_rand_iter_api_t(values), hxtest_rand_iter_api_t(values + 5), hxtest_value_less);
	expect_values(expected_sorted);
	reset();
	sort_callback(hxtest_rand_iter_api_t(values), hxtest_rand_iter_api_t(values + 5), hxtest_value_greater);
	expect_values(expected_descending);
	sort_callback(hxtest_rand_iter_api_t(values), hxtest_rand_iter_api_t(values + 5), hxtest_value_less);
	expect_values(expected_sorted);
}

TEST(hxsort_test, iterator_support) {
	do_sort_iter_case([](hxtest_rand_iter_api_t begin, hxtest_rand_iter_api_t end, const auto& less) {
		hxinsertion_sort(begin, end, less);
	});
	do_sort_iter_case([](hxtest_rand_iter_api_t begin, hxtest_rand_iter_api_t end, const auto& less) {
		hxheapsort(begin, end, less);
	});
	do_sort_iter_case([](hxtest_rand_iter_api_t begin, hxtest_rand_iter_api_t end, const auto& less) {
		hxsort(begin, end, less);
	});
}

static void do_sort_iter_partition_case(const int (&initial_values)[33], const int (&expected_sorted)[33]) {
	hxtest_ref_tracker_t values[33] = {
		hxtest_ref_tracker_t(initial_values[0]), hxtest_ref_tracker_t(initial_values[1]),
		hxtest_ref_tracker_t(initial_values[2]), hxtest_ref_tracker_t(initial_values[3]),
		hxtest_ref_tracker_t(initial_values[4]), hxtest_ref_tracker_t(initial_values[5]),
		hxtest_ref_tracker_t(initial_values[6]), hxtest_ref_tracker_t(initial_values[7]),
		hxtest_ref_tracker_t(initial_values[8]), hxtest_ref_tracker_t(initial_values[9]),
		hxtest_ref_tracker_t(initial_values[10]), hxtest_ref_tracker_t(initial_values[11]),
		hxtest_ref_tracker_t(initial_values[12]), hxtest_ref_tracker_t(initial_values[13]),
		hxtest_ref_tracker_t(initial_values[14]), hxtest_ref_tracker_t(initial_values[15]),
		hxtest_ref_tracker_t(initial_values[16]), hxtest_ref_tracker_t(initial_values[17]),
		hxtest_ref_tracker_t(initial_values[18]), hxtest_ref_tracker_t(initial_values[19]),
		hxtest_ref_tracker_t(initial_values[20]), hxtest_ref_tracker_t(initial_values[21]),
		hxtest_ref_tracker_t(initial_values[22]), hxtest_ref_tracker_t(initial_values[23]),
		hxtest_ref_tracker_t(initial_values[24]), hxtest_ref_tracker_t(initial_values[25]),
		hxtest_ref_tracker_t(initial_values[26]), hxtest_ref_tracker_t(initial_values[27]),
		hxtest_ref_tracker_t(initial_values[28]), hxtest_ref_tracker_t(initial_values[29]),
		hxtest_ref_tracker_t(initial_values[30]), hxtest_ref_tracker_t(initial_values[31]),
		hxtest_ref_tracker_t(initial_values[32])
	};
	hxsort(hxtest_rand_iter_api_t(values), hxtest_rand_iter_api_t(values + 33), hxtest_value_less);
	for(hxsize_t i = 0; i < 33; ++i) {
		EXPECT_EQ(values[i].value, expected_sorted[i]);
	}
}

TEST(hxsort_test, iterator_support_partition_sort_network_all_ascending_takes_no_swaps) {
	const int initial_values[33] = {
		1000, 1001, 1002, 1003, 1004, 1005, 10, 1007, 1008, 1009, 1010,
		20, 1012, 1013, 1014, 1015, 30, 1017, 1018, 1019, 1020, 40, 1022,
		1023, 1024, 1025, 50, 1027, 1028, 1029, 1030, 1031, 1032
	};
	const int expected_sorted[33] = {
		10, 20, 30, 40, 50, 1000, 1001, 1002, 1003, 1004, 1005, 1007,
		1008, 1009, 1010, 1012, 1013, 1014, 1015, 1017, 1018, 1019, 1020,
		1022, 1023, 1024, 1025, 1027, 1028, 1029, 1030, 1031, 1032
	};
	do_sort_iter_partition_case(initial_values, expected_sorted);
}

TEST(hxsort_test, iterator_support_partition_sort_network_p3_p0_p4_p1_p2_p1_p4_p3_swap) {
	const int initial_values[33] = {
		1000, 1001, 1002, 1003, 1004, 1005, 50, 1007, 1008, 1009, 1010,
		20, 1012, 1013, 1014, 1015, 30, 1017, 1018, 1019, 1020, 40, 1022,
		1023, 1024, 1025, 10, 1027, 1028, 1029, 1030, 1031, 1032
	};
	const int expected_sorted[33] = {
		10, 20, 30, 40, 50, 1000, 1001, 1002, 1003, 1004, 1005, 1007,
		1008, 1009, 1010, 1012, 1013, 1014, 1015, 1017, 1018, 1019, 1020,
		1022, 1023, 1024, 1025, 1027, 1028, 1029, 1030, 1031, 1032
	};
	do_sort_iter_partition_case(initial_values, expected_sorted);
}

TEST(hxsort_test, iterator_support_partition_sort_network_p3_p1_and_p3_p2_swap) {
	const int initial_values[33] = {
		1000, 1001, 1002, 1003, 1004, 1005, 10, 1007, 1008, 1009, 1010,
		30, 1012, 1013, 1014, 1015, 40, 1017, 1018, 1019, 1020, 20, 1022,
		1023, 1024, 1025, 50, 1027, 1028, 1029, 1030, 1031, 1032
	};
	const int expected_sorted[33] = {
		10, 20, 30, 40, 50, 1000, 1001, 1002, 1003, 1004, 1005, 1007,
		1008, 1009, 1010, 1012, 1013, 1014, 1015, 1017, 1018, 1019, 1020,
		1022, 1023, 1024, 1025, 1027, 1028, 1029, 1030, 1031, 1032
	};
	do_sort_iter_partition_case(initial_values, expected_sorted);
}

TEST(hxsort_test, iterator_support_partition_sort_all_equal_takes_no_pivot_swaps) {
	const int all_equal[33] = {
		7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
		7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7
	};
	do_sort_iter_partition_case(all_equal, all_equal);
}

TEST(hxsort_test, iterator_api_types) {
	hxtest_ref_tracker_t values[2] = {
		hxtest_ref_tracker_t(0),
		hxtest_ref_tracker_t(1)
	};
	EXPECT_TRUE(hxtest_check_forward_iter_api(
		hxtest_forward_iter_api_t(values), hxtest_forward_iter_api_t(values + 2)));
	EXPECT_TRUE(hxtest_check_rand_iter_api(
		hxtest_rand_iter_api_t(values), hxtest_rand_iter_api_t(values + 2)));
}
#endif // HX_CPLUSPLUS >= 201402L
#if HX_CPLUSPLUS >= 202302L
namespace {

consteval bool hxtest_hxinsertion_sort_consteval(void) {
	int arr[5] = { 3, 1, 4, 1, 5 };
	hxinsertion_sort(arr, arr + 5);
	return arr[0] == 1 && arr[1] == 1 && arr[2] == 3 && arr[3] == 4 && arr[4] == 5;
}
static_assert(hxtest_hxinsertion_sort_consteval(),
	"hxinsertion_sort consteval: must sort array correctly at compile time");
} // namespace
#endif // HX_CPLUSPLUS >= 202302L
