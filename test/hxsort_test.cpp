// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

#include <hx/hxsort.hpp>
#include <hx/hxrandom.hpp>
#include <hx/hxvector.hpp>
#include <hx/hxtest.hpp>
#include "test_trackers.hpp"

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
	const hxsystem_allocator_scope temporary_stack_scope(hxsystem_allocator_temporary_stack);
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
	const hxsystem_allocator_scope temporary_stack_scope(hxsystem_allocator_temporary_stack);
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

TEST(hxsort_test, sort_grinder_generic) {
	const hxsystem_allocator_scope temporary_stack_scope(hxsystem_allocator_temporary_stack);
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

TEST(hxsort_test, hxsort_empty_range) {
	int ints[3] = { 3, 1, 2 };
	hxsort(ints, ints, test_compare_int);
	const int ints_unchanged[3] = { 3, 1, 2 };
	EXPECT_EQ(::memcmp(ints, ints_unchanged, sizeof ints), 0);
	hxsort(ints, ints + 1, test_compare_int);
	EXPECT_EQ(::memcmp(ints, ints_unchanged, sizeof ints), 0);
}

TEST(hxbinary_search_test, single_element_hit_and_miss) {
	const int arr[1] = { 42 };
	const int* result = hxbinary_search(arr, arr + 1, 42);
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
	sort_callback(hxtest_iter_api_t(values), hxtest_iter_api_t(values), hxtest_value_less);
	expect_values(initial_values);
	reset();
	sort_callback(hxtest_iter_api_t(values), hxtest_iter_api_t(values + 1), hxtest_value_less);
	expect_values(initial_values);
	reset();
	sort_callback(hxtest_iter_api_t(values), hxtest_iter_api_t(values + 2), hxtest_value_less);
	expect_values(expected_two);
	reset();
	sort_callback(hxtest_iter_api_t(values), hxtest_iter_api_t(values + 5), hxtest_value_less);
	expect_values(expected_sorted);
	reset();
	sort_callback(hxtest_iter_api_t(values), hxtest_iter_api_t(values + 5), hxtest_value_greater);
	expect_values(expected_descending);
	sort_callback(hxtest_iter_api_t(values), hxtest_iter_api_t(values + 5), hxtest_value_less);
	expect_values(expected_sorted);
}

TEST(hxsort_test, iterator_support) {
	do_sort_iter_case([](hxtest_iter_api_t begin, hxtest_iter_api_t end, const auto& less) {
		hxinsertion_sort(begin, end, less);
	});
	do_sort_iter_case([](hxtest_iter_api_t begin, hxtest_iter_api_t end, const auto& less) {
		hxheapsort(begin, end, less);
	});
	do_sort_iter_case([](hxtest_iter_api_t begin, hxtest_iter_api_t end, const auto& less) {
		hxsort(begin, end, less);
	});
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
