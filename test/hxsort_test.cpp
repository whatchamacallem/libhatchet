// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

#include <hx/hxsort.hpp>
#include <hx/hxrandom.hpp>
#include <hx/hxvector.hpp>
#include <hx/hxtest.hpp>
#include "test_api_trackers.hpp"

HX_NS_USE

#if HX_CPLUSPLUS >= 201402L // C++14 only.

// ==> TEST(hxsort_test, sort_int_case).
namespace {

// Run some simple integer tests first.
bool test_compare_int(int a, int b) { return a < b; }

bool test_compare_int_reverse(int a, int b) { return a > b; }

template<typename sort_callback_t>
void test_sort_int_cases(const sort_callback_t& sort_callback) {
	int ints[5] = { 2, 1, 0, 4, -5 };

	// "The end parameter points just past the end of the array." Confirm size 0
	// and size 1 ranges keep { 2, 1, 0, 4, -5 } untouched.
	sort_callback(ints, ints, test_compare_int);
	const int ints1[5] = { 2, 1, 0, 4, -5 };
	EXPECT_EQ(::memcmp(ints, ints1, sizeof ints), 0); // Nothing changed.

	// Sort 1 element.
	sort_callback(ints, ints + 1, test_compare_int);
	EXPECT_EQ(::memcmp(ints, ints1, sizeof ints), 0); // Still nothing changed.

	// "Sorts the elements in the range [begin, end) in comparison order using the insertion sort algorithm."
	// Expect head slice -> { 1, 2 } while tail remains { 0, 4, -5 }.
	sort_callback(ints, ints + 2, test_compare_int);
	const int ints2[5] = { 1, 2, 0, 4, -5 };
	EXPECT_EQ(::memcmp(ints, ints2, sizeof ints), 0);

	// "This version is intended for sorting large numbers of small objects."
	// Whole array ascends into { -5, 0, 1, 2, 4 }. Sorts all elements.
	sort_callback(ints, ints + 5, test_compare_int);
	const int ints3[5] = { -5, 0, 1, 2, 4 };
	EXPECT_EQ(::memcmp(ints, ints3, sizeof ints), 0);

	// Ensure reversed comparator yields { 4, 2, 1, 0, -5 } before restoring order.
	sort_callback(ints, ints + 5, test_compare_int_reverse);
	const int ints4[5] = { 4, 2, 1, 0, -5 };
	EXPECT_EQ(::memcmp(ints, ints4, sizeof ints), 0);

	// Run one more ascending pass to confirm stability: { -5, 0, 1, 2, 4 }.
	sort_callback(ints, ints + 5, test_compare_int);
	EXPECT_EQ(::memcmp(ints, ints3, sizeof ints), 0);
}

} // namespace {

TEST(hxsort_test, sort_int_cases) {
	// Instantiate and pass the sort templates as function pointers.
	test_sort_int_cases(hxinsertion_sort<int*, bool (*)(int, int)>);
	test_sort_int_cases(hxheapsort<int*, bool (*)(int, int)>);
	test_sort_int_cases(hxsort<int*, bool (*)(int, int)>);
}

TEST(hxbinary_search_test, simple_case) {
	int ints[5] = { 2, 5, 6, 88, 99 };
	int* ints_end = ints+5;

	// "Performs a binary search in the range [first, last)." Expect hits { 88, 2, 99 }
	// across const and mutable pointers, then confirm misses fall to the end iterator.
	// hxbinary_search returns end when not found.
	int* result = hxbinary_search(ints, ints+5, 88, hxkey_less_t<int>{});
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

	// Size 0 range: begin == end so the return points at the sentinel.
	// Empty range returns end.
	result = hxbinary_search(ints, ints, 11, hxkey_less_t<int>{}); // Zero size.
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
	// "Unsorted data will lead to errors." Force ascending order before the grinder.
	hxsort(sorted.begin(), sorted.end());

	// Every resident value should be rediscovered: pointer equality relaxed to
	// value comparison avoids aliasing when duplicates exist.
		for(size_t i=100u; i-- != 0u; ) {
		const hxtest_ref_tracker_t t = hxmove(sorted[i]); // Don't pass an address that is in the array.
		hxtest_ref_tracker_t* ptr = hxbinary_search(sorted.begin(), sorted.end(), t);
		// Assert logical equivalence without using ==. The hxtest_ref_tracker_t* may point
		// elsewhere.
		EXPECT_TRUE(!(*ptr < t) && !(t < *ptr));
	}
}

TEST(hxsort_test, sort_grinder) {
	const hxsystem_allocator_scope temporary_stack_scope(hxsystem_allocator_temporary_stack);
	hxrandom rng(2);
	const size_t max_size_mask = 0x7f;
	hxvector<hxtest_ref_tracker_t> insertion_sorted; insertion_sorted.reserve(max_size_mask);
	hxvector<hxtest_ref_tracker_t> heap_sorted; heap_sorted.reserve(max_size_mask);
	hxvector<hxtest_ref_tracker_t> generic_sorted; generic_sorted.reserve(max_size_mask);

	for(int i=12; i-- != 0; ) {
		// Set up the arrays to be sorted.
		const size_t size = (max_size_mask >> i) & rng.u32();
		for(size_t j = size; j-- != 0u;) {
			insertion_sorted.push_back(hxtest_ref_tracker_t(rng.range(100, 200)));
			// Use the && constructor and not the const& one.
			heap_sorted.push_back(hxtest_ref_tracker_t(0));
			generic_sorted.push_back(hxtest_ref_tracker_t(0));
		}

		const size_t element_count = insertion_sorted.size();
		for(size_t j = 0u; j < element_count; ++j) {
			heap_sorted[j] = hxtest_ref_tracker_t(insertion_sorted[j].value);
			generic_sorted[j] = hxtest_ref_tracker_t(insertion_sorted[j].value);
		}

		hxinsertion_sort(insertion_sorted.begin(), insertion_sorted.end());
		hxheapsort(heap_sorted.begin(), heap_sorted.end());
		hxsort(generic_sorted.begin(), generic_sorted.end());

		// Compare the three results to confirm they are sorted.
		EXPECT_EQ(::memcmp(insertion_sorted.data(), heap_sorted.data(), insertion_sorted.size_bytes()), 0);
		EXPECT_EQ(::memcmp(insertion_sorted.data(), generic_sorted.data(), insertion_sorted.size_bytes()), 0);

		insertion_sorted.clear();
		heap_sorted.clear();
		generic_sorted.clear();
	}
}

TEST(hxsort_test, sort_grinder_generic) {
	const hxsystem_allocator_scope temporary_stack_scope(hxsystem_allocator_temporary_stack);
	hxrandom rng(3);
	const size_t max_size_mask = 0xfff;
	hxvector<hxtest_ref_tracker_t> sorted; sorted.reserve(max_size_mask);
	hxvector<int> histogram(20000, 0);

	for(int i=10; i-- != 0; ) {
		// Pick random values of increasing maximum value up to 2^16 and keep a
		// count of them.
		const size_t size = (max_size_mask >> i) & rng.u32();
		if(size <= 16) {
			continue;
		}
		for(size_t j = size; j-- != 0u;) {
			const int x = rng.range(10000, 10000);
			sorted.push_back(hxtest_ref_tracker_t(x));
			++histogram[static_cast<size_t>(x)];
		}

		hxsort(sorted.begin(), sorted.end());

		// Check that all values are accounted for starting with the last one.
		// Confirm sort order with (j <= j+1) while walking down to the first
		// value. Note size > 16.
		--histogram[static_cast<size_t>(sorted[size - 1].value)];
		for(size_t j=size - 1u; j-- != 0u;) {
			--histogram[static_cast<size_t>(sorted[j].value)];
			EXPECT_FALSE(hxkey_less(*sorted.get(j + 1), *sorted.get(j)));
		}
		for(size_t j=20000u; j-- > 10000u;) {
			EXPECT_EQ(histogram[j], 0);
		}
		sorted.clear();
	}
}

TEST(hxsort_test, hxsort_empty_range) {
	int ints[3] = { 3, 1, 2 };

	// This code path invokes hxlog2i(0) which is currently -127 and is
	// undefined. Confirm the printer doesn't catch fire.
	hxsort(ints, ints, test_compare_int);
	const int ints_unchanged[3] = { 3, 1, 2 };
	EXPECT_EQ(::memcmp(ints, ints_unchanged, sizeof ints), 0);

	hxsort(ints, ints + 1, test_compare_int);
	EXPECT_EQ(::memcmp(ints, ints_unchanged, sizeof ints), 0);
}

// ==> TEST(hxsort_test, iterator_support).
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
		for(size_t i = 0; i < 5; ++i) {
			values[i] = hxtest_ref_tracker_t(initial_values[i]);
		}
	};

	auto expect_values = [&](const int (&expected)[5]) {
		for(size_t i = 0; i < 5; ++i) {
			EXPECT_EQ(values[i].value, expected[i]);
		}
	};

	// "The end parameter points just past the last element in the range to sort."
	// Iterator wrapper should keep { 2, 1, 0, 4, -5 } unchanged for sizes 0 and 1.
	reset();
	sort_callback(hxtest_iter_api_t(values), hxtest_iter_api_t(values), hxtest_value_less);
	expect_values(initial_values);

	reset();
	sort_callback(hxtest_iter_api_t(values), hxtest_iter_api_t(values + 1), hxtest_value_less);
	expect_values(initial_values);

	// "Requires only the standard pointer operations. No array notation."
	// Confirm head slice { 2, 1 } becomes { 1, 2 } with iterator arithmetic.
	reset();
	sort_callback(hxtest_iter_api_t(values), hxtest_iter_api_t(values + 2), hxtest_value_less);
	expect_values(expected_two);

	// "Sorts the elements in the range [begin, end) in comparison order using the insertion sort algorithm."
	// Iterator facade should deliver { -5, 0, 1, 2, 4 }.
	reset();
	sort_callback(hxtest_iter_api_t(values), hxtest_iter_api_t(values + 5), hxtest_value_less);
	expect_values(expected_sorted);

	// Ensure alternate comparator flips to { 4, 2, 1, 0, -5 } before restoring order.
	reset();
	sort_callback(hxtest_iter_api_t(values), hxtest_iter_api_t(values + 5), hxtest_value_greater);
	expect_values(expected_descending);

	// One more ascending pass confirms iterator facade remains stable.
	sort_callback(hxtest_iter_api_t(values), hxtest_iter_api_t(values + 5), hxtest_value_less);
	expect_values(expected_sorted);
}

TEST(hxsort_test, iterator_support) {
	// Exercise iterator facade with insertion, heap, and hybrid strategies to
	// guarantee the adapter satisfies each contract.
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

#if HX_CPLUSPLUS >= 202302L // C++23 only.
namespace {

// Returns true if hxinsertion_sort correctly sorts an array at compile time.
consteval bool hxtest_hxinsertion_sort_consteval(void) {
	int arr[5] = { 3, 1, 4, 1, 5 };
	hxinsertion_sort(arr, arr + 5);
	return arr[0] == 1 && arr[1] == 1 && arr[2] == 3 && arr[3] == 4 && arr[4] == 5;
}

static_assert(hxtest_hxinsertion_sort_consteval(),
	"hxinsertion_sort consteval: must sort array correctly at compile time");
} // namespace
#endif // HX_CPLUSPLUS >= 202302L
