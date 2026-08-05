// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

#include <hx/hxvector.hpp>
#include <hx/hxsort.hpp>
#include "./hxtest_util.hpp"
#include <limits.h>
#if HX_USE_LIBCXX
#include <utility>
#endif

HX_NS_USE

hxattr_noinline static void hxtest_gdb_break_hxvector_static(void) { }
hxattr_noinline static void hxtest_gdb_break_hxvector_dynamic(void) { }

#if !defined _MSC_VER && !defined __wasm__
static_assert(sizeof(size_t) != 4 || (
		sizeof(hxvector<int32_t, hxallocator_dynamic_capacity>) == 12u
		&& sizeof(hxvector<int32_t, 4>) == 20u),
	"hxvector must pack dynamic storage as a hxsize_t capacity, a T*, and a"
	" T* end with no padding, and fixed storage as capacity * sizeof(T) plus"
	" a single T* end with no padding");
static_assert(sizeof(size_t) != 8 || (
		sizeof(hxvector<int32_t, hxallocator_dynamic_capacity>) == 24u
		&& sizeof(hxvector<int32_t, 4>) == 24u),
	"hxvector must pack dynamic storage as a hxsize_t capacity, a T*, and a"
	" T* end with no padding, and fixed storage as capacity * sizeof(T) plus"
	" a single T* end with no padding");
#endif

namespace {
using hxvector_test_f = hxtest_object_fixture;

template<typename array_t>
bool hxvector_test_is_max_heap(const array_t& heap) {
	const hxsize_t size = heap.size();
	for(hxsize_t parent = 0; parent != size; ++parent) {
		const hxsize_t left = (parent << 1) + 1;
		const hxsize_t right = left + 1;
		// GCOVR_EXCL_START
		if(left < size && hxkey_less(heap[parent], heap[left])) {
			return false;
		}
		if(right < size && hxkey_less(heap[parent], heap[right])) {
			return false;
		}
		// GCOVR_EXCL_STOP
	}
	return true;
}
} // namespace

TEST_F(hxvector_test_f, empty_full) {
	const hxsystem_allocator_scope temporary_stack_scope(hxsystem_allocator_stack_0);
	hxvector<hxtest_object, hxallocator_dynamic_capacity> a;
	EXPECT_TRUE(a.empty());
	EXPECT_TRUE(a.full());
	a.reserve(1);
	EXPECT_TRUE(a.empty());
	EXPECT_TRUE(!a.full());
	a.push_back(hxtest_object());
	EXPECT_TRUE(!a.empty());
	EXPECT_FALSE(a[0].moved_from);
	EXPECT_TRUE(a.full());
	a.pop_back();
	EXPECT_TRUE(a.empty());
	EXPECT_TRUE(!a.full());
	EXPECT_TRUE(check_stats(2, 2, 0, 1, 0, 0, 0, 0));
}

TEST(hxvector_test, empty_boundary) {
	hxvector<int, 2> v;
	EXPECT_TRUE(v.empty());
	v.push_back(0);
	EXPECT_FALSE(v.empty());
	v.pop_back();
	EXPECT_TRUE(v.empty());
}

TEST(hxvector_test, full_boundary) {
	hxvector<int, 1> v;
	EXPECT_FALSE(v.full());
	v.push_back(7);
	EXPECT_TRUE(v.full());
}

TEST_F(hxvector_test_f, iteration) {
	{
		static const int32_t nums[3] = { 21, 22, 23 };
		hxvector<hxtest_object, 10> objs;
		objs.push_back(hxtest_object(nums[0]));
		objs.push_back(hxtest_object(nums[1]));
		objs.push_back(hxtest_object(nums[2]));
		const hxvector<hxtest_object, 10>& cobjs = objs;
		int32_t counter = 0;
		for(hxvector<hxtest_object, 10>::iterator it = objs.begin(); it != objs.end(); ++it) {
			EXPECT_EQ(it->value, objs[counter].value);
			EXPECT_EQ(it->value, nums[hxmin<int32_t>(counter, 2)]);
			EXPECT_FALSE(objs[counter].moved_from);
			++counter;
		}
		counter = 0;
		for(hxvector<hxtest_object, 10>::const_iterator it = cobjs.begin();
				it != cobjs.end(); ++it) {
			EXPECT_EQ(it->value, objs[counter].value);
			EXPECT_EQ(it->value, nums[hxmin<int32_t>(counter, 2)]);
			EXPECT_FALSE(cobjs[counter].moved_from);
			++counter;
		}
		EXPECT_EQ(objs.front(), nums[0]);
		EXPECT_EQ(objs.back(), nums[2]);
		EXPECT_EQ(cobjs.front(), nums[0]);
		EXPECT_EQ(cobjs.back(), nums[2]);
	}
	EXPECT_TRUE(check_stats(6, 6, 0, 3, 0, 0, 0, 0));
}

TEST(hxvector_test, iteration_visits_first_and_last) {
	hxvector<int, 3> v;
	v.push_back(10);
	v.push_back(20);
	v.push_back(30);
	const int expected[] = { 10, 20, 30 };
	hxsize_t count = 0;
	for(const int* it = v.begin(); it != v.end(); ++it) {
		EXPECT_EQ(*it, expected[count]);
		++count;
	}
	EXPECT_EQ(count, 3);
}

TEST_F(hxvector_test_f, get) {
	{
		hxvector<hxtest_object, 4> objs; objs.reserve(4);
		objs.emplace_back(10);
		objs.emplace_back(20);
		const hxvector<hxtest_object, 4>& cobjs = objs;
		EXPECT_EQ(objs.get(0), objs.begin());
		EXPECT_EQ(objs.get(1), objs.begin() + 1);
		EXPECT_EQ(objs.get(2), hxnullptr);
		EXPECT_EQ(cobjs.get(0), cobjs.begin());
		EXPECT_EQ(cobjs.get(1), cobjs.begin() + 1);
		EXPECT_EQ(cobjs.get(2), hxnullptr);
		objs.pop_back();
		EXPECT_EQ(objs.get(1), hxnullptr);
	}
	EXPECT_TRUE(check_stats(2, 2, 0, 0, 0, 0, 0, 0));
}

TEST(hxvector_test, get_boundary_last_valid_and_past_end) {
	hxvector<int, 3> v;
	v.push_back(1);
	v.push_back(2);
	v.push_back(3);
	const hxvector<int, 3>& cv = v;
	EXPECT_EQ(v.get(2), v.begin() + 2);
	EXPECT_EQ(v.get(3), hxnullptr);
	EXPECT_EQ(cv.get(2), cv.begin() + 2);
	EXPECT_EQ(cv.get(3), hxnullptr);
}

TEST_F(hxvector_test_f, modification) {
	const hxsystem_allocator_scope temporary_stack_scope(hxsystem_allocator_stack_0);
	{
		hxvector<hxtest_object> objs { 91, 92, 93, 94, 95 };
		EXPECT_FALSE(objs.empty());
		EXPECT_EQ(objs.capacity(), 5);
		EXPECT_EQ(objs.size(), 5);
		objs.pop_back();
		objs.pop_back();
		objs.pop_back();
		hxtest_object to(97);
		objs.push_back(to);
		const hxtest_object& to_const_ref = to;
		objs.push_back(to_const_ref);
		objs.emplace_back();
		objs.erase_unordered(1);
		EXPECT_EQ(objs.size(), 4);
		const hxvector<hxtest_object> objs2 { 99 };
		objs += objs2;
		EXPECT_EQ(objs.size(), 5);
		EXPECT_EQ(objs[0].value, 91);
		EXPECT_EQ(objs[1].value, 0);
		EXPECT_EQ(objs[2].value, 97);
		EXPECT_EQ(objs[3].value, 97);
		EXPECT_EQ(objs[4].value, 99);
		for(hxsize_t i = 0; i < objs.size(); ++i) {
			EXPECT_FALSE(objs[i].moved_from);
		}
		hxtest_gdb_break_hxvector_dynamic();
	}
	EXPECT_TRUE(check_stats(11, 11, 3, 0, 0, 1, 0, 0));
}

TEST(hxvector_test, push_heap_preserves_heap_property) {
	static const int values[] = { 3, 7, 1, 9, 5, 8 };
	const hxsize_t value_count = hxsize(values);
	hxvector<int, 16> heap;
	int max_value = INT_MIN;
	for(hxsize_t index = 0; index < value_count; ++index) {
		const int value = values[index];
		heap.push_heap(value);
		if(value > max_value) {
			max_value = value;
		}
		EXPECT_TRUE(hxvector_test_is_max_heap(heap));
		EXPECT_EQ(heap.front(), max_value);
	}
	EXPECT_EQ(heap.size(), value_count);
}

TEST(hxvector_test, push_heap_sift_up_one_level) {
	hxvector<int, 4> heap;
	heap.push_heap(5);
	heap.push_heap(10);
	EXPECT_EQ(heap.front(), 10);
	EXPECT_TRUE(hxvector_test_is_max_heap(heap));
}

TEST(hxvector_test, push_heap_sifts_new_max_to_root) {
	hxvector<int, 4> heap;
	heap.push_heap(1);
	heap.push_heap(2);
	EXPECT_EQ(heap.front(), 2);
}

TEST(hxvector_test, pop_heap_preserves_heap_on_removal) {
	static const int values[] = { 5, 12, 3, 7, 9, 4, 15, 5 };
	const hxsize_t value_count = hxsize(values);
	hxvector<int, 16> heap;
	for(hxsize_t index = 0; index < value_count; ++index) {
		heap.push_heap(values[index]);
	}
	EXPECT_TRUE(hxvector_test_is_max_heap(heap));
	EXPECT_EQ(heap.size(), value_count);
	hxvector<int, 16> removed;
	hxsize_t expected_size = value_count;
	while(!heap.empty()) {
		const int root_value = heap.front();
		removed.push_back(root_value);
		heap.pop_heap();
		--expected_size;
		EXPECT_EQ(heap.size(), expected_size);
		EXPECT_TRUE(hxvector_test_is_max_heap(heap));
		for(hxsize_t index = 0; index < heap.size(); ++index) {
			EXPECT_LE(heap[index], root_value);
		}
	}
	EXPECT_TRUE(heap.empty());
	EXPECT_EQ(removed.size(), value_count);
	static const int expected[] = { 15, 12, 9, 7, 5, 5, 4, 3 };
	for(hxsize_t index = 0; index < value_count; ++index) {
		EXPECT_EQ(removed[index], expected[index]);
	}
}

TEST(hxvector_test, pop_heap_single_element_empties_vector) {
	hxvector<int, 2> heap;
	heap.push_heap(34);
	EXPECT_EQ(heap.size(), 1);
	heap.pop_heap();
	EXPECT_TRUE(heap.empty());
}

TEST(hxvector_test, pop_heap_two_elements_correct_order) {
	hxvector<int, 4> heap;
	heap.push_heap(3);
	heap.push_heap(7);
	EXPECT_EQ(heap.front(), 7);
	heap.pop_heap();
	EXPECT_EQ(heap.size(), 1);
	EXPECT_EQ(heap.front(), 3);
	EXPECT_TRUE(hxvector_test_is_max_heap(heap));
}

TEST(hxvector_test, erase_if_heap_removes_matching_values) {
	const hxsystem_allocator_scope temporary_stack_scope(hxsystem_allocator_stack_0);
	static const int values[] = { 14, 3, 7, 2, 9, 5, 8, 1, 6 };
	const hxsize_t value_count = hxsize(values);
	hxvector<int, 16> heap;
	hxsize_t expected_removed = 0;
	for(hxsize_t index = 0; index < value_count; ++index) {
		if((values[index] & 1) == 0) {
			++expected_removed;
		}
		heap.push_heap(values[index]);
	}
	EXPECT_TRUE(hxvector_test_is_max_heap(heap));
	const hxsize_t removed = heap.erase_if_heap([](int value) -> bool {
		return (value & 1) == 0;
	});
	EXPECT_EQ(removed, expected_removed);
	EXPECT_EQ(heap.size(), value_count - expected_removed);
	EXPECT_TRUE(hxvector_test_is_max_heap(heap));
	int previous = INT_MAX;
	while(!heap.empty()) {
		const int root = heap.front();
		EXPECT_EQ(root & 1, 1);
		EXPECT_LE(root, previous);
		previous = root;
		heap.pop_heap();
	}
	EXPECT_TRUE(heap.empty());
	EXPECT_EQ(heap.erase_if_heap([](int) { return false; }), 0);
}

TEST(hxvector_test, erase_if_heap_sift_up_required_single_erase) {
	static const int prebuilt[] = { 10, 9, 5, 8, 7, 3, 4, 6 };
	hxvector<int, 8> heap;
	heap.assign(prebuilt, prebuilt + hxsize(prebuilt));
	EXPECT_TRUE(hxvector_test_is_max_heap(heap));
	const hxsize_t removed = heap.erase_if_heap([](int value) -> bool {
		return value == 3;
	});
	EXPECT_EQ(removed, 1);
	EXPECT_EQ(heap.size(), 7);
	EXPECT_TRUE(hxvector_test_is_max_heap(heap));
	hxtest_gdb_break_hxvector_static();
}

TEST(hxvector_test, erase_if_heap_all_removed_count) {
	hxvector<int, 4> heap;
	heap.push_heap(1);
	heap.push_heap(3);
	heap.push_heap(5);
	const hxsize_t removed = heap.erase_if_heap([](int) { return true; });
	EXPECT_EQ(removed, 3);
	EXPECT_TRUE(heap.empty());
}

TEST(hxvector_test, erase_if_heap_none_removed_count) {
	hxvector<int, 4> heap;
	heap.push_heap(2);
	heap.push_heap(4);
	const hxsize_t removed = heap.erase_if_heap([](int) { return false; });
	EXPECT_EQ(removed, 0);
	EXPECT_EQ(heap.size(), 2);
	EXPECT_TRUE(hxvector_test_is_max_heap(heap));
}

TEST(hxvector_test, make_heap_builds_max_heap) {
	static const int values[] = { 12, 3, 17, 8, 5, 14, 6 };
	hxvector<int, 7> heap;
	heap.assign(values, values + hxsize(values));
	heap.make_heap();
	EXPECT_TRUE(hxvector_test_is_max_heap(heap));
	EXPECT_EQ(heap.front(), 17);
}

TEST(hxvector_test, make_heap_two_elements_reorders) {
	static const int values[] = { 2, 5 };
	hxvector<int, 2> heap;
	heap.assign(values, values + 2);
	heap.make_heap();
	EXPECT_EQ(heap.front(), 5);
	EXPECT_TRUE(hxvector_test_is_max_heap(heap));
}

TEST(hxvector_test, insertion_sort_orders_elements) {
	static const int unsorted[] = { 9, 2, 7, 4, 4, 1 };
	hxvector<int, 6> values(unsorted);
	values.insertion_sort();
	static const int expected[] = { 1, 2, 4, 4, 7, 9 };
	for(hxsize_t index = 0; index < hxsize(expected); ++index) {
		EXPECT_EQ(values[index], expected[index]);
	}
}

TEST(hxvector_test, insertion_sort_two_element_boundary) {
	static const int unsorted[] = { 5, 2 };
	hxvector<int, 2> v(unsorted);
	v.insertion_sort();
	EXPECT_EQ(v[0], 2);
	EXPECT_EQ(v[1], 5);
}

TEST(hxvector_test, insertion_sort_single_element) {
	hxvector<int, 1> v;
	v.push_back(7);
	v.insertion_sort();
	EXPECT_EQ(v[0], 7);
}

TEST(hxvector_test, sort_orders_elements) {
	static const int unsorted[] = { 13, -5, 7, 0, 13, 2 };
	hxvector<int, 6> values(unsorted);
	values.sort();
	static const int expected[] = { -5, 0, 2, 7, 13, 13 };
	for(hxsize_t index = 0; index < hxsize(expected); ++index) {
		EXPECT_EQ(values[index], expected[index]);
	}
}

TEST_F(hxvector_test_f, emplace_back) {
	const hxsystem_allocator_scope temporary_stack_scope(hxsystem_allocator_stack_0);
	{
		hxvector<hxtest_object> objs;
		objs.reserve(3);
		const hxtest_object& default_inserted = objs.emplace_back();
		EXPECT_EQ(objs.data(), &default_inserted);
		EXPECT_EQ(default_inserted.value, 0);
		EXPECT_FALSE(default_inserted.moved_from);
		hxtest_object original(34);
		const hxtest_object& move_inserted = objs.emplace_back(hxmove(original));
		EXPECT_EQ(objs.data() + 1, &move_inserted);
		EXPECT_FALSE(move_inserted.moved_from);
		EXPECT_TRUE(original.moved_from);
		const hxtest_object& value_inserted = objs.emplace_back(77);
		EXPECT_EQ(objs.data() + 2, &value_inserted);
		EXPECT_EQ(value_inserted.value, 77);
		EXPECT_FALSE(value_inserted.moved_from);
		EXPECT_EQ(objs.size(), 3);
		EXPECT_EQ(objs.back().value, 77);
	}
	EXPECT_TRUE(check_stats(4, 4, 0, 1, 0, 0, 0, 0));
}

TEST(hxvector_test, emplace_back_returns_correct_address) {
	hxvector<int, 3> v;
	const int& r0 = v.emplace_back(10);
	EXPECT_EQ(&r0, v.data());
	const int& r1 = v.emplace_back(20);
	EXPECT_EQ(&r1, v.data() + 1);
	const int& r2 = v.emplace_back(30);
	EXPECT_EQ(&r2, v.data() + 2);
	EXPECT_EQ(v.size(), 3);
}

TEST(hxvector_test, for_each_invokes_callables) {
	const hxsystem_allocator_scope temporary_stack_scope(hxsystem_allocator_stack_0);
	hxvector<int> objs { 91, 92, 93, 94, 95 };
	objs.for_each([](int& x) { x -= 90; });
	hxvector<int>& objs_ref = objs;
	EXPECT_EQ(objs_ref.size(), 5);
	EXPECT_EQ(objs_ref[0], 1);
	EXPECT_EQ(objs_ref[1], 2);
	EXPECT_EQ(objs_ref[2], 3);
	EXPECT_EQ(objs_ref[3], 4);
	EXPECT_EQ(objs_ref[4], 5);
	struct hxvector_test_x_t { int n; hxvector_test_x_t() : n(0) { }; void operator()(int&) { ++n; } } x;
	objs.for_each(x);
	EXPECT_EQ(x.n, 5);
	objs.clear();
	struct hxvector_test_y_t { void operator()(int&) const { hxassertmsg(0, "internal error"); } } y;
	objs.for_each(y);
}

TEST(hxvector_test, for_each_visits_last_element) {
	hxvector<int, 3> v;
	v.push_back(1);
	v.push_back(2);
	v.push_back(3);
	v.for_each([](int& x) { x *= 10; });
	EXPECT_EQ(v[0], 10);
	EXPECT_EQ(v[1], 20);
	EXPECT_EQ(v[2], 30);
}

TEST(hxvector_test, for_each_visits_first_element) {
	hxvector<int, 3> v;
	v.push_back(5);
	v.push_back(6);
	v.push_back(7);
	int count = 0;
	v.for_each([&](int& x) { x = count++; });
	EXPECT_EQ(v[0], 0);
}

TEST(hxvector_test, generate_n_appends_callable_results) {
	const hxsystem_allocator_scope temporary_stack_scope(hxsystem_allocator_stack_0);
	hxvector<int> values;
	values.reserve(6);
	values.push_back(34);
	int next_value = 5;
	int call_count = 0;
	values.generate_n(3, [&]() -> int {
		++call_count;
		return next_value++;
	});
	EXPECT_EQ(call_count, 3);
	EXPECT_EQ(values.size(), 4);
	EXPECT_EQ(values[0], 34);
	EXPECT_EQ(values[1], 5);
	EXPECT_EQ(values[2], 6);
	EXPECT_EQ(values[3], 7);
	values.generate_n(0, []() -> int {
		// GCOVR_EXCL_START
		hxassertmsg(0, "generate_n invoked for zero size");
		return 0;
		// GCOVR_EXCL_STOP
	});
}

TEST(hxvector_test, generate_n_exact_invocation_count) {
	hxvector<int, 4> v;
	int count = 0;
	v.generate_n(3, [&]() -> int { return ++count; });
	EXPECT_EQ(count, 3);
	EXPECT_EQ(v.size(), 3);
}

TEST(hxvector_test, generate_n_one_invocation) {
	hxvector<int, 2> v;
	int count = 0;
	v.generate_n(1, [&]() -> int { ++count; return 99; });
	EXPECT_EQ(count, 1);
	EXPECT_EQ(v.size(), 1);
	EXPECT_EQ(v[0], 99);
}

TEST(hxvector_test, all_of_any_of) {
	const hxsystem_allocator_scope temporary_stack_scope(hxsystem_allocator_stack_0);
	hxvector<int> objs { 91, 92, 93, 94, 95 };
	EXPECT_TRUE(objs.all_of([](const int& x) { return x > 0; }));
	EXPECT_FALSE(objs.all_of([](const int& x) { return x < 95; }));
	int all_calls = 0;
	EXPECT_FALSE(objs.all_of([&](const int& value) -> bool {
		++all_calls;
		return value < 93;
	}));
	EXPECT_EQ(all_calls, 3);
	int any_calls = 0;
	EXPECT_TRUE(objs.any_of([&](const int& value) -> bool {
		++any_calls;
		return value == 94;
	}));
	EXPECT_EQ(any_calls, 4);
	int miss_calls = 0;
	EXPECT_FALSE(objs.any_of([&](const int& value) -> bool {
		++miss_calls;
		return value == -1;
	}));
	EXPECT_EQ(miss_calls, 5);
	objs.clear();
	// GCOVR_EXCL_START
	auto empty_predicate = [](const int&) -> bool {
		hxassertmsg(0, "internal error");
		return false;
	};
	// GCOVR_EXCL_STOP
	EXPECT_TRUE(objs.all_of(empty_predicate));
	EXPECT_FALSE(objs.any_of(empty_predicate));
}

TEST(hxvector_test, all_of_stops_at_first_false) {
	hxvector<int, 3> v;
	v.push_back(1);
	v.push_back(0);
	v.push_back(1);
	int calls = 0;
	EXPECT_FALSE(v.all_of([&](const int& x) -> bool { ++calls; return x != 0; }));
	EXPECT_EQ(calls, 2);
}

TEST(hxvector_test, const_all_of_any_of_stop_at_boundary) {
	hxvector<int, 3> v;
	v.push_back(1);
	v.push_back(0);
	v.push_back(1);
	const hxvector<int, 3>& cv = v;
	int all_calls = 0;
	EXPECT_FALSE(cv.all_of([&](const int& x) -> bool { ++all_calls; return x != 0; }));
	EXPECT_EQ(all_calls, 2);
	EXPECT_TRUE(cv.all_of([](const int& x) { return x >= 0; }));
	int any_calls = 0;
	EXPECT_TRUE(cv.any_of([&](const int& x) -> bool { ++any_calls; return x == 0; }));
	EXPECT_EQ(any_calls, 2);
}

TEST(hxvector_test, all_of_single_true_element) {
	hxvector<int, 1> v;
	v.push_back(5);
	EXPECT_TRUE(v.all_of([](const int& x) { return x > 0; }));
}

TEST(hxvector_test, any_of_stops_at_first_true) {
	hxvector<int, 3> v;
	v.push_back(0);
	v.push_back(1);
	v.push_back(0);
	int calls = 0;
	EXPECT_TRUE(v.any_of([&](const int& x) -> bool { ++calls; return x != 0; }));
	EXPECT_EQ(calls, 2);
}

TEST(hxvector_test, any_of_single_false_element) {
	hxvector<int, 1> v;
	v.push_back(0);
	EXPECT_FALSE(v.any_of([](const int& x) { return x > 0; }));
}

TEST(hxvector_test, binary_search) {
	static const int sorted_values[] = { 1, 3, 5, 7, 9 };
	hxvector<int, 5> values(sorted_values);
	const hxvector<int, 5>& const_values = values;
	const int* const_missing = const_values.binary_search(4);
	EXPECT_EQ(const_missing, values.end());
	const int* mutable_found = values.binary_search(7);
	EXPECT_EQ(mutable_found, values.begin() + 3);
	EXPECT_EQ(values.binary_search(2), values.end());
}

TEST(hxvector_test, binary_search_first_and_last_element) {
	static const int sorted[] = { 2, 4, 6, 8, 10 };
	hxvector<int, 5> v(sorted);
	EXPECT_EQ(v.binary_search(2), v.begin());
	EXPECT_EQ(v.binary_search(10), v.begin() + 4);
	EXPECT_EQ(v.binary_search(1), v.end());
	EXPECT_EQ(v.binary_search(11), v.end());
}

TEST(hxvector_test, binary_search_single_element) {
	hxvector<int, 1> v;
	v.push_back(7);
	EXPECT_EQ(v.binary_search(7), v.begin());
	EXPECT_EQ(v.binary_search(6), v.end());
	EXPECT_EQ(v.binary_search(8), v.end());
}

TEST(hxvector_test, find_returns_first_match) {
	static const int values_source[] = { 2, 4, 4, 8, 16 };
	hxvector<int, 5> values(values_source);
	const hxvector<int, 5>& const_values = values;
	const int* const_pos = const_values.find(4);
	EXPECT_EQ(const_pos, values.begin() + 1);
	const int* const_missing = const_values.find(32);
	EXPECT_EQ(const_missing, values.end());
	int* mutable_pos = values.find(8);
	EXPECT_EQ(mutable_pos, values.begin() + 3);
	*mutable_pos = 64;
	EXPECT_EQ(values[3], 64);
	*mutable_pos = 8;
	const int* const_predicate = const_values.find_if([](int value) {
		return value >= 8;
	});
	EXPECT_EQ(const_predicate, values.begin() + 3);
	const int* const_predicate_missing = const_values.find_if([](int value) {
		return value < 0;
	});
	EXPECT_EQ(const_predicate_missing, values.end());
	const int* mutable_predicate = values.find_if([](int& value) {
		return (value & 1) == 0 && value > 4;
	});
	EXPECT_EQ(mutable_predicate, values.begin() + 3);
	const int* mutable_predicate_missing = values.find_if([](int& value) {
		return value < 0;
	});
	EXPECT_EQ(mutable_predicate_missing, values.end());
#if HX_CPLUSPLUS >= 202002L
	if(const int* t = values.find_if([](int& value) { return value == 8; }); t == values.end()) {
		// GCOVR_EXCL_START
		ADD_FAILURE();
		// GCOVR_EXCL_STOP
	}
#endif
}

TEST(hxvector_test, find_at_index_zero) {
	hxvector<int, 3> v;
	v.push_back(34);
	v.push_back(43);
	v.push_back(44);
	EXPECT_EQ(v.find(34), v.begin());
}

TEST(hxvector_test, find_at_last_index) {
	hxvector<int, 3> v;
	v.push_back(1);
	v.push_back(2);
	v.push_back(99);
	EXPECT_EQ(v.find(99), v.begin() + 2);
}

TEST(hxvector_test, find_if_returns_first_match_at_boundary) {
	hxvector<int, 4> v;
	v.push_back(10);
	v.push_back(10);
	v.push_back(20);
	v.push_back(30);
	const int* p = v.find_if([](const int& x) { return x == 10; });
	EXPECT_EQ(p, v.begin());
}

TEST(hxvector_test, erase_if_unordered) {
	const hxsystem_allocator_scope temporary_stack_scope(hxsystem_allocator_stack_0);
	hxvector<int, 5> objs { 1, 2, 3, 4, 5 };
	int remove_calls = 0;
	auto remove_even = [&](int& value) -> bool {
		++remove_calls;
		return (value & 1) == 0;
	};
	EXPECT_EQ(objs.erase_if_unordered(remove_even), 2);
	EXPECT_EQ(remove_calls, 5);
	static const int expected[] = { 1, 5, 3 };
	for(hxsize_t i = 0; i < 3; ++i) {
		EXPECT_EQ(objs[i], expected[i]);
	}
	EXPECT_EQ(objs.erase_if_unordered([](int x) { return x == 1; }), 1);
	EXPECT_EQ(objs.size(), 2);
	objs.clear();
	// GCOVR_EXCL_START
	auto empty_predicate = [](int&) -> bool {
		hxassertmsg(0, "internal error");
		return false;
	};
	// GCOVR_EXCL_STOP
	EXPECT_EQ(objs.erase_if_unordered(empty_predicate), 0);
}

TEST(hxvector_test, erase_if_unordered_removes_first_element) {
	hxvector<int, 4> v;
	v.push_back(7);
	v.push_back(2);
	v.push_back(3);
	const hxsize_t removed = v.erase_if_unordered([](int& x) { return x == 7; });
	EXPECT_EQ(removed, 1);
	EXPECT_EQ(v.size(), 2);
	EXPECT_EQ(v.find(7), v.end());
}

TEST(hxvector_test, erase_if_unordered_removes_last_element) {
	hxvector<int, 4> v;
	v.push_back(1);
	v.push_back(2);
	v.push_back(99);
	const hxsize_t removed = v.erase_if_unordered([](int& x) { return x == 99; });
	EXPECT_EQ(removed, 1);
	EXPECT_EQ(v.size(), 2);
	EXPECT_EQ(v.find(99), v.end());
}

TEST(hxvector_test, erase_if_unordered_swap_replaces_erased_slot) {
	hxvector<int, 4> v;
	v.push_back(1);
	v.push_back(2);
	v.push_back(3);
	v.push_back(4);
	v.erase_if_unordered([](int& x) { return x == 2; });
	EXPECT_EQ(v.size(), 3);
	EXPECT_EQ(v.find(2), v.end());
	EXPECT_NE(v.find(1), v.end());
	EXPECT_NE(v.find(3), v.end());
	EXPECT_NE(v.find(4), v.end());
}

TEST(hxvector_test, erase_if_unordered_scans_all_elements) {
	hxvector<int, 5> v;
	v.push_back(10);
	v.push_back(20);
	v.push_back(30);
	v.push_back(40);
	v.push_back(50);
	int calls = 0;
	v.erase_if_unordered([&](int&) -> bool { ++calls; return false; });
	EXPECT_EQ(calls, 5);
}

TEST_F(hxvector_test_f, resizing) {
	const hxsystem_allocator_scope temporary_stack_scope(hxsystem_allocator_stack_0);
	{
		static const int32_t nums[5] = { 51, 52, 53, 54, 55 };
		hxvector<hxtest_object> objs(12);
		objs.reserve(10);
		objs = nums;
		objs.resize(3, hxtest_object());
		EXPECT_EQ(objs.size(), 3);
		EXPECT_EQ(objs[0].value, 51);
		EXPECT_EQ(objs[2].value, 53);
		for(hxsize_t i = 0; i < objs.size(); ++i) {
			EXPECT_FALSE(objs[i].moved_from);
		}
		objs.resize(4);
		EXPECT_EQ(objs.size(), 4);
		EXPECT_EQ(objs[0].value, 51);
		EXPECT_EQ(objs[2].value, 53);
		EXPECT_EQ(objs[3].value, 0);
		EXPECT_EQ(objs.capacity(), 12);
		for(hxsize_t i = 0; i < objs.size(); ++i) {
			EXPECT_FALSE(objs[i].moved_from);
		}
		objs.resize(10);
		EXPECT_EQ(objs.size(), 10);
		EXPECT_EQ(objs[9].value, 0);
		EXPECT_FALSE(objs[9].moved_from);
		EXPECT_FALSE(objs.empty());
		objs.clear();
		EXPECT_EQ(objs.size(), 0);
		EXPECT_TRUE(objs.empty());
		EXPECT_EQ(objs.capacity(), 12);
	}
	EXPECT_TRUE(check_stats(25, 25, 0, 0, 0, 0, 0, 0));
}

TEST_F(hxvector_test_f, resize_shrink_boundary) {
	const hxsystem_allocator_scope temporary_stack_scope(hxsystem_allocator_stack_0);
	{
		hxvector<hxtest_object> objs;
		objs.reserve(4);
		objs.emplace_back(1);
		objs.emplace_back(2);
		objs.emplace_back(3);
		objs.resize(2, hxtest_object());
		EXPECT_EQ(objs.size(), 2);
		EXPECT_EQ(objs[1].value, 2);
	}
	EXPECT_TRUE(check_stats(4, 4, 0, 0, 0, 0, 0, 0));
}

TEST_F(hxvector_test_f, resize_grow_boundary) {
	const hxsystem_allocator_scope temporary_stack_scope(hxsystem_allocator_stack_0);
	{
		hxvector<hxtest_object> objs;
		objs.reserve(3);
		objs.emplace_back(10);
		objs.resize(2);
		EXPECT_EQ(objs.size(), 2);
		EXPECT_FALSE(objs[1].moved_from);
	}
	EXPECT_TRUE(check_stats(2, 2, 0, 0, 0, 0, 0, 0));
}

TEST_F(hxvector_test_f, assignment) {
	const hxsystem_allocator_scope temporary_stack_scope(hxsystem_allocator_stack_0);
	{
		hxvector<hxtest_object> objs;
		objs.reserve(1);
		hxtest_object to;
		to.value = 67;
		objs.push_back(to);
		hxvector<hxtest_object> objs2;
		objs2 = objs;
		hxvector<hxtest_object, 1> objs3;
		objs3 = objs;
		hxvector<hxtest_object> objs4(objs);
		hxvector<hxtest_object, 1> objs5(objs);
		EXPECT_EQ(objs2.size(), 1);
		EXPECT_EQ(objs3.size(), 1);
		EXPECT_EQ(objs4.size(), 1);
		EXPECT_EQ(objs5.size(), 1);
		EXPECT_EQ(objs2[0].value, 67);
		EXPECT_EQ(objs3[0].value, 67);
		EXPECT_EQ(objs4[0].value, 67);
		EXPECT_EQ(objs5[0].value, 67);
	}
	EXPECT_TRUE(check_stats(6, 6, 5, 0, 0, 0, 0, 0));
}

TEST_F(hxvector_test_f, assign_copies_all_elements_including_last) {
	const hxsystem_allocator_scope temporary_stack_scope(hxsystem_allocator_stack_0);
	{
		hxvector<hxtest_object> dst;
		dst.reserve(3);
		static const int32_t src[] = { 10, 20, 30 };
		dst.assign(src, src + 3);
		EXPECT_EQ(dst.size(), 3);
		EXPECT_EQ(dst[2].value, 30);
	}
	EXPECT_TRUE(check_stats(3, 3, 0, 0, 0, 0, 0, 0));
}

#if HX_CPLUSPLUS >= 202002L
TEST_F(hxvector_test_f, assign_range_from_rvalue) {
	const hxsystem_allocator_scope temporary_stack_scope(hxsystem_allocator_stack_0);
	hxtest_object source_elements[] = {
		hxtest_object(5),
		hxtest_object(9),
		hxtest_object(13)
	};
	const hxsize_t source_count = hxsize(source_elements);
	hxvector<hxtest_object> elements;
	elements.reserve(source_count);
	elements.assign_range(hxtest_pointer_range(
		source_elements, source_elements + source_count));
	EXPECT_EQ(elements.size(), source_count);
	EXPECT_EQ(elements[0].value, 5);
	EXPECT_EQ(elements[1].value, 9);
	EXPECT_EQ(elements[2].value, 13);
	for(hxsize_t i = 0; i < source_count; ++i) {
		EXPECT_FALSE(elements[i].moved_from);
		EXPECT_TRUE(source_elements[i].moved_from);
	}
	EXPECT_TRUE(check_stats(6, 0, 0, 3, 0, 0, 0, 0));
}

TEST_F(hxvector_test_f, assign_range_from_const) {
	const hxsystem_allocator_scope temporary_stack_scope(hxsystem_allocator_stack_0);
	const int32_t assigned_element_ints[] = { 4, 7, 11, 18 };
	const hxvector<hxtest_object> assigned_elements = assigned_element_ints;
	const hxsize_t assigned_count = hxsize(assigned_element_ints);
	hxvector<hxtest_object> elements;
	elements.reserve(assigned_count + 1);
	elements.push_back(hxtest_object(91));
	hxtest_pointer_range<const hxtest_object> range(
		assigned_elements.begin(), assigned_elements.end());
	elements.assign_range(range);
	EXPECT_EQ(elements.size(), assigned_count);
	EXPECT_EQ(elements[0], assigned_elements[0]);
	EXPECT_EQ(elements[1], assigned_elements[1]);
	EXPECT_EQ(elements[2], assigned_elements[2]);
	EXPECT_EQ(elements[3], assigned_elements[3]);
	for(hxsize_t i = 0; i < assigned_count; ++i) {
		EXPECT_FALSE(elements[i].moved_from);
		EXPECT_FALSE(assigned_elements[i].moved_from);
	}
	EXPECT_TRUE(check_stats(10, 2, 4, 1, 0, 0, 4, 0));
}

TEST_F(hxvector_test_f, assign_range_from_mutable_range) {
	const hxsystem_allocator_scope temporary_stack_scope(hxsystem_allocator_stack_0);
	hxtest_object source_elements[] = {
		hxtest_object(2),
		hxtest_object(3),
		hxtest_object(5)
	};
	const hxsize_t source_count = hxsize(source_elements);
	hxvector<hxtest_object> elements;
	elements.reserve(source_count);
	hxtest_pointer_range<hxtest_object> range(
		source_elements, source_elements + source_count);
	elements.assign_range(range);
	EXPECT_EQ(elements.size(), source_count);
	EXPECT_EQ(elements[0].value, 2);
	EXPECT_EQ(elements[1].value, 3);
	EXPECT_EQ(elements[2].value, 5);
	for(hxsize_t i = 0; i < source_count; ++i) {
		EXPECT_FALSE(elements[i].moved_from);
		EXPECT_FALSE(source_elements[i].moved_from);
	}
	EXPECT_TRUE(check_stats(6, 0, 3, 0, 0, 0, 0, 0));
}
#endif

TEST_F(hxvector_test_f, push_back_move_tracker) {
	const hxsystem_allocator_scope temporary_stack_scope(hxsystem_allocator_stack_0);
	hxtest_object source(34);
	hxvector<hxtest_object> elements;
	elements.reserve(3);
	elements.push_back(hxmove(source));
	EXPECT_EQ(elements.size(), 1);
	EXPECT_EQ(elements[0].value, 34);
	EXPECT_FALSE(elements[0].moved_from);
	EXPECT_TRUE(source.moved_from);
	hxtest_object x(84);
	elements.push_back(x);
	EXPECT_EQ(elements.size(), 2);
	EXPECT_EQ(elements[1].value, 84);
	EXPECT_FALSE(elements[0].moved_from);
	EXPECT_FALSE(elements[1].moved_from);
	EXPECT_FALSE(x.moved_from);
	EXPECT_TRUE(check_stats(4, 0, 1, 1, 0, 0, 0, 0));
}

TEST_F(hxvector_test_f, plus_equals_move_tracker_element) {
	const hxsystem_allocator_scope temporary_stack_scope(hxsystem_allocator_stack_0);
	hxtest_object source(5);
	hxvector<hxtest_object> elements;
	elements.reserve(3);
	elements += hxmove(source);
	EXPECT_EQ(elements.size(), 1);
	EXPECT_EQ(elements[0].value, 5);
	EXPECT_FALSE(elements[0].moved_from);
	EXPECT_TRUE(source.moved_from);
	const hxtest_object x(11);
	elements += x;
	EXPECT_EQ(elements.size(), 2);
	EXPECT_EQ(elements[1].value, 11);
	EXPECT_FALSE(elements[0].moved_from);
	EXPECT_FALSE(elements[1].moved_from);
	EXPECT_FALSE(x.moved_from);
	EXPECT_TRUE(check_stats(4, 0, 1, 1, 0, 0, 0, 0));
}

TEST_F(hxvector_test_f, plus_equals_move_tracker_array) {
	const hxsystem_allocator_scope temporary_stack_scope(hxsystem_allocator_stack_0);
	hxtest_object initial(1);
	static const int32_t appended_values[] = { 3, 5, 7 };
	hxvector<hxtest_object> move_target;
	move_target.reserve(5);
	move_target.push_back(initial);
	hxvector<hxtest_object> copy_target;
	copy_target.reserve(5);
	copy_target.push_back(initial);
	EXPECT_FALSE(initial.moved_from);
	hxvector<hxtest_object> move_source;
	move_source = appended_values;
	hxvector<hxtest_object> copy_source;
	copy_source = appended_values;
	move_target += hxmove(move_source);
	EXPECT_EQ(move_target.size(), 4);
	EXPECT_EQ(move_target[0].value, 1);
	EXPECT_EQ(move_target[1].value, 3);
	EXPECT_EQ(move_target[2].value, 5);
	EXPECT_EQ(move_target[3].value, 7);
	for(hxsize_t i = 0; i < move_target.size(); ++i) {
		EXPECT_FALSE(move_target[i].moved_from);
	}
	EXPECT_EQ(move_source.size(), 3);
	for(hxsize_t i = 0; i < move_source.size(); ++i) {
		EXPECT_TRUE(move_source[i].moved_from);
	}
	copy_target += copy_source;
	EXPECT_EQ(copy_target.size(), 4);
	EXPECT_EQ(copy_target[0].value, 1);
	EXPECT_EQ(copy_target[1].value, 3);
	EXPECT_EQ(copy_target[2].value, 5);
	EXPECT_EQ(copy_target[3].value, 7);
	for(hxsize_t i = 0; i < copy_target.size(); ++i) {
		EXPECT_FALSE(copy_target[i].moved_from);
	}
	EXPECT_EQ(copy_source.size(), 3);
	for(hxsize_t i = 0; i < copy_source.size(); ++i) {
		EXPECT_FALSE(copy_source[i].moved_from);
	}
	EXPECT_TRUE(check_stats(15, 0, 5, 3, 0, 0, 0, 0));
}

TEST_F(hxvector_test_f, plus_equals_array_copies_last_element) {
	const hxsystem_allocator_scope temporary_stack_scope(hxsystem_allocator_stack_0);
	{
		hxvector<hxtest_object> dst;
		dst.reserve(4);
		dst.emplace_back(0);
		hxvector<hxtest_object> src;
		src.reserve(3);
		src.emplace_back(1);
		src.emplace_back(2);
		src.emplace_back(3);
		dst += src;
		EXPECT_EQ(dst.size(), 4);
		EXPECT_EQ(dst[3].value, 3);
	}
	EXPECT_TRUE(check_stats(7, 7, 3, 0, 0, 0, 0, 0));
}

TEST_F(hxvector_test_f, insert_move_tracker_move_and_copy) {
	const hxsystem_allocator_scope temporary_stack_scope(hxsystem_allocator_stack_0);
	hxvector<hxtest_object> elements;
	elements.reserve(4);
	hxtest_object initial(10);
	elements.push_back(initial);
	EXPECT_FALSE(initial.moved_from);
	hxtest_object move_source(20);
	elements.insert(elements.begin(), hxmove(move_source));
	EXPECT_EQ(elements.size(), 2);
	EXPECT_EQ(elements[0].value, 20);
	EXPECT_EQ(elements[1].value, 10);
	EXPECT_TRUE(move_source.moved_from);
	EXPECT_FALSE(elements[0].moved_from);
	EXPECT_FALSE(elements[1].moved_from);
	hxtest_object copy_source(30);
	elements.insert(2, copy_source);
	EXPECT_EQ(elements.size(), 3);
	EXPECT_EQ(elements[0].value, 20);
	EXPECT_EQ(elements[1].value, 10);
	EXPECT_EQ(elements[2].value, 30);
	EXPECT_FALSE(copy_source.moved_from);
	for(hxsize_t i = 0; i < elements.size(); ++i) {
		EXPECT_FALSE(elements[i].moved_from);
	}
	EXPECT_TRUE(check_stats(6, 0, 2, 1, 0, 1, 0, 0));
}

TEST_F(hxvector_test_f, insert_at_end_is_push_back) {
	const hxsystem_allocator_scope temporary_stack_scope(hxsystem_allocator_stack_0);
	{
		hxvector<hxtest_object> v;
		v.reserve(3);
		v.emplace_back(1);
		v.emplace_back(2);
		v.insert(v.end(), hxtest_object(3));
		EXPECT_EQ(v.size(), 3);
		EXPECT_EQ(v[2].value, 3);
		EXPECT_FALSE(v[0].moved_from);
		EXPECT_FALSE(v[1].moved_from);
	}
	EXPECT_TRUE(check_stats(4, 4, 0, 1, 0, 0, 0, 0));
}

TEST_F(hxvector_test_f, insert_at_begin_shifts_all_elements) {
	const hxsystem_allocator_scope temporary_stack_scope(hxsystem_allocator_stack_0);
	{
		hxvector<hxtest_object> v;
		v.reserve(4);
		v.emplace_back(2);
		v.emplace_back(3);
		v.emplace_back(4);
		v.insert(v.begin(), hxtest_object(1));
		EXPECT_EQ(v.size(), 4);
		EXPECT_EQ(v[0].value, 1);
		EXPECT_EQ(v[1].value, 2);
		EXPECT_EQ(v[2].value, 3);
		EXPECT_EQ(v[3].value, 4);
	}
	EXPECT_TRUE(check_stats(5, 5, 0, 1, 0, 3, 0, 0));
}

TEST_F(hxvector_test_f, insert_at_middle_preserves_neighbors) {
	const hxsystem_allocator_scope temporary_stack_scope(hxsystem_allocator_stack_0);
	{
		hxvector<hxtest_object> v;
		v.reserve(4);
		v.emplace_back(1);
		v.emplace_back(3);
		v.emplace_back(4);
		v.insert(1, hxtest_object(2));
		EXPECT_EQ(v.size(), 4);
		EXPECT_EQ(v[0].value, 1);
		EXPECT_EQ(v[1].value, 2);
		EXPECT_EQ(v[2].value, 3);
		EXPECT_EQ(v[3].value, 4);
	}
	EXPECT_TRUE(check_stats(5, 5, 0, 1, 0, 2, 0, 0));
}

TEST_F(hxvector_test_f, emplace_back_move_tracker_forwarding) {
	const hxsystem_allocator_scope temporary_stack_scope(hxsystem_allocator_stack_0);
	hxvector<hxtest_object> elements;
	elements.reserve(3);
	hxtest_object copy_source(40);
	hxtest_object move_source(50);
	const hxtest_object& copy_inserted = elements.emplace_back(copy_source);
	EXPECT_EQ(&copy_inserted, &elements[0]);
	EXPECT_EQ(elements[0].value, 40);
	EXPECT_FALSE(elements[0].moved_from);
	EXPECT_FALSE(copy_source.moved_from);
	const hxtest_object& move_inserted = elements.emplace_back(hxmove(move_source));
	EXPECT_EQ(&move_inserted, &elements[1]);
	EXPECT_EQ(elements[1].value, 50);
	EXPECT_FALSE(elements[1].moved_from);
	EXPECT_TRUE(move_source.moved_from);
	EXPECT_EQ(elements.size(), 2);
	EXPECT_TRUE(check_stats(4, 0, 1, 1, 0, 0, 0, 0));
}

TEST(hxvector_test, cbegin_cend) {
	hxvector<int, 4> values;
	values.push_back(1);
	values.push_back(3);
	values.push_back(5);
	const hxvector<int, 4>& const_values = values;
	const int expected[] = { 1, 3, 5 };
	const hxsize_t expected_count = hxsize(expected);
	hxsize_t index = 0;
	for (hxvector<int, 4>::const_iterator it = const_values.cbegin(); it != const_values.cend(); ++it) {
		ASSERT_LT(index, expected_count);
		EXPECT_EQ(*it, expected[hxmin<hxsize_t>(index, 2)]);
		++index;
	}
	EXPECT_EQ(index, const_values.size());
	EXPECT_EQ(const_values.cbegin(), const_values.begin());
	EXPECT_EQ(const_values.cend(), const_values.end());
	EXPECT_EQ(const_values.cbegin() + const_values.size(), const_values.cend());
}

TEST(hxvector_test, cend_points_past_last_element) {
	hxvector<int, 2> v;
	v.push_back(10);
	v.push_back(20);
	EXPECT_EQ(v.cend(), v.begin() + 2);
	EXPECT_EQ(*(v.cend() - 1), 20);
}

#if HX_CPLUSPLUS >= 202002L
TEST_F(hxvector_test_f, plus_equals) {
	const hxsystem_allocator_scope temporary_stack_scope(hxsystem_allocator_stack_0);
	{
		hxvector<hxtest_object> objs;
		objs.reserve(10);
		objs += hxvector<hxtest_object>{ 1, 7, 11 };
		const hxvector<hxtest_object> objs2 { 10, 70, 110 };
		objs += objs2;
		const hxvector<hxtest_object> objs3 { 1, 7, 11, 10, 70, 110 };
		EXPECT_TRUE(hxkey_equal(objs, objs3));
		EXPECT_FALSE(hxkey_less(objs, objs3));
		const hxtest_object t(440);
		objs += t;
		EXPECT_FALSE(hxkey_equal(objs, objs3));
		EXPECT_TRUE(hxkey_less(objs3, objs));
		objs.resize(5);
		objs += hxtest_object(220);
		EXPECT_FALSE(hxkey_equal(objs, objs3));
		EXPECT_TRUE(hxkey_less(objs3, objs));
	}
	EXPECT_TRUE(check_stats(22, 22, 4, 4, 0, 0, 30, 1));
}

TEST_F(hxvector_test_f, erase) {
	const hxsystem_allocator_scope temporary_stack_scope(hxsystem_allocator_stack_0);
	{
		hxvector<hxtest_object> objs { 1, 2, 3, 4, 5 };
		objs.erase(1);
		objs.erase(objs.begin() + 2);
		static const int expected_values[] = { 1, 3, 5 };
		const hxvector<hxtest_object> expected(expected_values);
		EXPECT_TRUE(hxkey_equal(objs, expected));
		objs.erase(objs.begin());
		objs.erase(objs.end() - 1);
		EXPECT_TRUE(hxkey_equal(objs[0], 3));
		EXPECT_EQ(objs.size(), 1);
	}
	EXPECT_TRUE(check_stats(8, 8, 0, 0, 0, 6, 3, 0));
}

TEST_F(hxvector_test_f, insert) {
	const hxsystem_allocator_scope temporary_stack_scope(hxsystem_allocator_stack_0);
	{
		hxvector<hxtest_object> objs; objs.reserve(5);
		objs.push_back(hxtest_object(3));
		objs.insert(objs.begin(), hxtest_object(1));
		objs.insert(2, hxtest_object(5));
		const hxvector<hxtest_object> expected { 1, 3, 5 };
		EXPECT_TRUE(hxkey_equal(objs, expected));
		for(hxsize_t i = 0; i < objs.size(); ++i) {
			EXPECT_FALSE(objs[i].moved_from);
		}
		objs.insert(1, hxtest_object(2));
		objs.insert(3, hxtest_object(4));
		const hxvector<hxtest_object> final_expected { 1, 2, 3, 4, 5 };
		EXPECT_TRUE(hxkey_equal(objs, final_expected));
		for(hxsize_t i = 0; i < objs.size(); ++i) {
			EXPECT_FALSE(objs[i].moved_from);
		}
	}
	EXPECT_TRUE(check_stats(18, 18, 0, 5, 0, 4, 8, 0));
}
#endif

TEST_F(hxvector_test_f, erase_last_element_by_pointer) {
	const hxsystem_allocator_scope temporary_stack_scope(hxsystem_allocator_stack_0);
	{
		hxvector<hxtest_object> v;
		v.reserve(3);
		v.emplace_back(1);
		v.emplace_back(2);
		v.emplace_back(3);
		v.erase(v.end() - 1);
		EXPECT_EQ(v.size(), 2);
		EXPECT_EQ(v[1].value, 2);
	}
	EXPECT_TRUE(check_stats(3, 3, 0, 0, 0, 0, 0, 0));
}

TEST_F(hxvector_test_f, erase_first_element_shifts_remainder) {
	const hxsystem_allocator_scope temporary_stack_scope(hxsystem_allocator_stack_0);
	{
		hxvector<hxtest_object> v;
		v.reserve(4);
		v.emplace_back(10);
		v.emplace_back(20);
		v.emplace_back(30);
		v.erase(v.begin());
		EXPECT_EQ(v.size(), 2);
		EXPECT_EQ(v[0].value, 20);
		EXPECT_EQ(v[1].value, 30);
	}
	EXPECT_TRUE(check_stats(3, 3, 0, 0, 0, 2, 0, 0));
}

TEST_F(hxvector_test_f, erase_unordered_non_last_moves_end) {
	const hxsystem_allocator_scope temporary_stack_scope(hxsystem_allocator_stack_0);
	{
		hxvector<hxtest_object> v;
		v.reserve(3);
		v.emplace_back(1);
		v.emplace_back(2);
		v.emplace_back(3);
		v.erase_unordered(v.begin());
		EXPECT_EQ(v.size(), 2);
		EXPECT_EQ(v[0].value, 3);
		EXPECT_EQ(v[1].value, 2);
	}
	EXPECT_TRUE(check_stats(3, 3, 0, 0, 0, 1, 0, 0));
}

TEST_F(hxvector_test_f, erase_unordered_last_element_no_swap) {
	const hxsystem_allocator_scope temporary_stack_scope(hxsystem_allocator_stack_0);
	{
		hxvector<hxtest_object> v;
		v.reserve(2);
		v.emplace_back(5);
		v.emplace_back(9);
		v.erase_unordered(v.end() - 1);
		EXPECT_EQ(v.size(), 1);
		EXPECT_EQ(v[0].value, 5);
	}
	EXPECT_TRUE(check_stats(2, 2, 0, 0, 0, 0, 0, 0));
}

TEST(hxvector_test, less_equal_vectors_not_less) {
	static const int vals[] = { 1, 2, 3 };
	const hxvector<int, 3> a(vals);
	const hxvector<int, 3> b(vals);
	EXPECT_FALSE(a.less(b));
	EXPECT_FALSE(b.less(a));
}

TEST(hxvector_test, less_shorter_prefix_is_less) {
	static const int short_vals[] = { 1, 2 };
	static const int long_vals[] = { 1, 2, 3 };
	const hxvector<int, 2> a(short_vals);
	const hxvector<int, 3> b(long_vals);
	EXPECT_TRUE(a.less(b));
	EXPECT_FALSE(b.less(a));
}

TEST(hxvector_test, equal_different_sizes_not_equal) {
	hxvector<int, 3> a;
	a.push_back(1);
	a.push_back(2);
	hxvector<int, 3> b;
	b.push_back(1);
	EXPECT_FALSE(a.equal(b));
}

TEST(hxvector_test, equal_differs_at_last_element) {
	static const int vals_a[] = { 1, 2, 3 };
	static const int vals_b[] = { 1, 2, 4 };
	const hxvector<int, 3> a(vals_a);
	const hxvector<int, 3> b(vals_b);
	EXPECT_FALSE(a.equal(b));
}

TEST(hxvector_test, equal_same_content_returns_true) {
	static const int vals[] = { 5, 10, 15 };
	const hxvector<int, 3> a(vals);
	const hxvector<int, 3> b(vals);
	EXPECT_TRUE(a.equal(b));
}

TEST(hxvector_test, size_after_push_back) {
	hxvector<int, 4> v;
	EXPECT_EQ(v.size(), 0);
	v.push_back(1);
	EXPECT_EQ(v.size(), 1);
	v.push_back(2);
	EXPECT_EQ(v.size(), 2);
	v.push_back(3);
	EXPECT_EQ(v.size(), 3);
}

TEST(hxvector_test, size_after_pop_back) {
	hxvector<int, 3> v;
	v.push_back(10);
	v.push_back(20);
	v.push_back(30);
	v.pop_back();
	EXPECT_EQ(v.size(), 2);
	v.pop_back();
	EXPECT_EQ(v.size(), 1);
}

TEST(hxvector_test, back_returns_last_element) {
	hxvector<int, 3> v;
	v.push_back(100);
	EXPECT_EQ(v.back(), 100);
	v.push_back(200);
	EXPECT_EQ(v.back(), 200);
	EXPECT_EQ(v.front(), 100);
}

TEST(hxvector_test, front_returns_first_element) {
	hxvector<int, 3> v;
	v.push_back(34);
	EXPECT_EQ(v.front(), 34);
	v.push_back(43);
	EXPECT_EQ(v.front(), 34);
}

TEST(hxvector_test, memcpy_clones_contents) {
	hxvector<unsigned char,5> source;
	source.resize(5);
	unsigned char index = 0u;
	for(auto& it : source) { it = ++index; }
	hxvector<unsigned char,5> destination;
	destination.memcpy(source);
	EXPECT_EQ(destination.size(), source.size());
	while(index-- != 0u) {
		EXPECT_EQ(destination[index], source[index]);
	}
}

TEST(hxvector_test, memcpy_copies_exact_count) {
	hxvector<unsigned char, 5> src;
	src.resize(5);
	for(hxsize_t i = 0; i < 5; ++i) { src[i] = static_cast<unsigned char>(i + 1); }
	hxvector<unsigned char, 5> dst;
	dst.memcpy(src);
	EXPECT_EQ(dst.size(), 5);
	EXPECT_EQ(dst[0], 1);
	EXPECT_EQ(dst[4], 5);
}

TEST(hxvector_test, memset_sets_bytes) {
	hxvector<unsigned char,6> bytes;
	bytes.resize(6, 0u);
	bytes.memset(0xca);
	for(hxsize_t index = 0; index < bytes.size(); ++index) {
		EXPECT_EQ(static_cast<int>(bytes[index]), 0xca);
	}
}

TEST(hxvector_test, c_strings) {
	hxvector<char, HX_MAX_LINE> z("prefix array 1");
	while(z[0] != 'a') {
		z.erase(static_cast<hxsize_t>(0));
	}
	EXPECT_STREQ(z.data(), "array 1");
	z = "array 2";
	EXPECT_STREQ(z.data(), "array 2");
}

TEST(hxvector_test, initializer_list_brace_support) {
	const hxsystem_allocator_scope temporary_stack_scope(hxsystem_allocator_stack_0);
	hxvector<int, 2> x = { 2, 7 };
	EXPECT_EQ(x[1], 7);
	hxvector<int> y { 12, 17 };
	EXPECT_EQ(y[1], 17);
}

TEST(hxvector_test, initializer_list_first_element) {
	hxvector<int, 3> v = { 5, 6, 7 };
	EXPECT_EQ(v[0], 5);
	EXPECT_EQ(v.size(), 3);
}

TEST(hxvector_test, swaps) {
	const hxsystem_allocator_scope allocator_scope(hxsystem_allocator_stack_0);
	hxvector<int> x(hxvector<int>({ 2, 7 }));
	hxvector<int> y = hxmove(x);
	hxvector<int> z;
	hxswap(y, z);
	EXPECT_TRUE(x.empty());
	EXPECT_TRUE(y.empty());
	EXPECT_EQ(z[0], 2);
	EXPECT_EQ(z[1], 7);
}

TEST(hxvector_test, move_assign_swaps_contents) {
	const hxsystem_allocator_scope allocator_scope(hxsystem_allocator_stack_0);
	hxvector<int> destination({ 1, 2, 3 });
	hxvector<int> source({ 7, 8 });
	destination = hxmove(source);
	EXPECT_EQ(destination.size(), 2);
	EXPECT_EQ(destination[0], 7);
	EXPECT_EQ(destination[1], 8);
	EXPECT_EQ(source.size(), 3);
	EXPECT_EQ(source[0], 1);
	EXPECT_EQ(source[2], 3);
}

TEST(hxvector_test, move_assign_into_empty) {
	const hxsystem_allocator_scope allocator_scope(hxsystem_allocator_stack_0);
	hxvector<int> destination;
	hxvector<int> source({ 5, 6, 7 });
	destination = hxmove(source);
	EXPECT_EQ(destination.size(), 3);
	EXPECT_EQ(destination[0], 5);
	EXPECT_EQ(destination[2], 7);
	EXPECT_TRUE(source.empty());
}

TEST(hxvector_test, subscript_first_and_last_index) {
	hxvector<int, 4> v;
	v.push_back(11);
	v.push_back(22);
	v.push_back(33);
	EXPECT_EQ(v[0], 11);
	EXPECT_EQ(v[2], 33);
	const hxvector<int, 4>& cv = v;
	EXPECT_EQ(cv[0], 11);
	EXPECT_EQ(cv[2], 33);
}
