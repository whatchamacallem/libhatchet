// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

#include <hx/hxarray.hpp>
#include <hx/hxtest.hpp>

#if HX_USE_LIBCXX
#include <utility>
#endif

HX_NS_USE

hxattr_noinline static void hxtest_gdb_break_hxarray_static(void) { }
hxattr_noinline static void hxtest_gdb_break_hxarray_dynamic(void) { }

namespace {

class hxarray_test_f* hxs_test_current = hxnull;

class hxarray_test_f :
	public testing::Test
{
public:
	class hxtest_object {
	public:
		hxtest_object(void) {
			++hxs_test_current->m_constructed;
			id = hxs_test_current->m_next_id--;
			moved_from = false;
		}

		hxtest_object(const hxtest_object& x) {
			++hxs_test_current->m_constructed;
			id = x.id;
			moved_from = false;
		}
		explicit hxtest_object(int32_t x) {
			EXPECT_GE(x, 0); // User supplied IDs are positive.
			++hxs_test_current->m_constructed;
			id = x;
			moved_from = false;
		}
		hxtest_object(hxtest_object&& x) noexcept {
			++hxs_test_current->m_constructed;
			id = x.id;
			moved_from = false;
			x.id = 0xefef; // Poison value;
			x.moved_from = true;
		}
		~hxtest_object(void) {
			++hxs_test_current->m_destructed;
			id = 0xefef; // Poison value;
			moved_from = true;
		}

		void operator=(const hxtest_object& x) {
			hxassert(this != &x);
			id = x.id;
			moved_from = false;
		}
		hxtest_object& operator=(hxtest_object&& x) noexcept {
			hxassert(this != &x);
			id = x.id;
			moved_from = false;
			x.id = 0xefef;
			x.moved_from = true;
			return *this;
		}
		bool operator==(int32_t x) const { return id == x; }
		bool operator==(const hxtest_object& x) const { return id == x.id; }
		bool operator<(const hxtest_object& x) const { return id < x.id; }

		bool moved_from;
		int32_t id;
	};

	hxarray_test_f(void) {
		hxassert(hxs_test_current == hxnull);
		m_constructed = 0;
		m_destructed = 0;
		m_next_id = -1;
		hxs_test_current = this;
	}
	~hxarray_test_f(void) {
		hxassert_always(m_constructed == m_destructed, "hxarray_test_f Test object lifecycle error.");
		hxs_test_current = 0;
	}

	bool check_totals(size_t total) const {
		return m_constructed == total && m_destructed == total;
	}

	size_t m_constructed;
	size_t m_destructed;
	int32_t m_next_id;
};

template<typename T>
class hxarray_test_pointer_range {
public:
	hxarray_test_pointer_range(T* b, T* e)
		: begin_ptr(b), end_ptr(e) { }

	T* begin() { return begin_ptr; }
	T* end() { return end_ptr; }

	const T& operator[](size_t index) const { return begin_ptr[index]; }
	T& operator[](size_t index) { return begin_ptr[index]; }

private:
	T* begin_ptr;
	T* end_ptr;
};

} // namespace {

TEST_F(hxarray_test_f, hxarray_default_constructs_all_elements) {
	{
		hxarray<hxtest_object, 3u> a;
		EXPECT_EQ(a.size(), 3u);
		EXPECT_EQ(a.max_size(), 3u);
		EXPECT_EQ(a.size_bytes(), 3u * sizeof(hxtest_object));
		for(size_t i = 0u; i < a.size(); ++i) {
			EXPECT_FALSE(a[i].moved_from);
		}
	}
	EXPECT_TRUE(check_totals(3));
}

TEST_F(hxarray_test_f, hxarray_fill_constructor) {
	{
		const hxtest_object fill(99);
		hxarray<hxtest_object, 4u> a(fill);
		EXPECT_EQ(a.size(), 4u);
		for(size_t i = 0u; i < a.size(); ++i) {
			EXPECT_EQ(a[i].id, 99);
			EXPECT_FALSE(a[i].moved_from);
		}
	}
	EXPECT_TRUE(check_totals(5));
}

TEST_F(hxarray_test_f, hxarray_copy_constructor) {
	{
		hxarray<hxtest_object, 3u> src;
		// 3 default constructs, 3 temporaries constructed and destroyed.
		src[0] = hxtest_object(10);
		src[1] = hxtest_object(20);
		src[2] = hxtest_object(30);

		hxarray<hxtest_object, 3u> dst(src);
		EXPECT_EQ(dst[0].id, 10);
		EXPECT_EQ(dst[1].id, 20);
		EXPECT_EQ(dst[2].id, 30);
		for(size_t i = 0u; i < dst.size(); ++i) {
			EXPECT_FALSE(dst[i].moved_from);
			EXPECT_FALSE(src[i].moved_from);
		}
	}
	// 3 default + 3 temporaries + 3 copy = 9 constructed, 3 temporaries + 3 dst + 3 src = 9 destructed.
	EXPECT_TRUE(check_totals(9));
}

TEST_F(hxarray_test_f, hxarray_copy_assignment) {
	{
		hxarray<hxtest_object, 3u> src;
		// 3 default constructs, 3 temporaries constructed and destroyed.
		src[0] = hxtest_object(11);
		src[1] = hxtest_object(22);
		src[2] = hxtest_object(33);

		// 3 default constructs, then copy-assigned (no new constructs).
		hxarray<hxtest_object, 3u> dst;
		dst = src;
		EXPECT_EQ(dst[0].id, 11);
		EXPECT_EQ(dst[1].id, 22);
		EXPECT_EQ(dst[2].id, 33);
		for(size_t i = 0u; i < dst.size(); ++i) {
			EXPECT_FALSE(dst[i].moved_from);
		}
	}
	// 3 + 3 temporaries + 3 = 9 constructed; 3 temporaries + 3 dst + 3 src = 9 destructed.
	EXPECT_TRUE(check_totals(9));
}

TEST(hxarray_test, hxarray_c_array_constructor) {
	static const int values[4] = { 7, 14, 21, 28 };
	hxarray<int, 4u> a(values);
	EXPECT_EQ(a[0], 7);
	EXPECT_EQ(a[1], 14);
	EXPECT_EQ(a[2], 21);
	EXPECT_EQ(a[3], 28);
	hxtest_gdb_break_hxarray_static();
}

TEST(hxarray_test, hxarray_c_array_assignment) {
	static const int values[3] = { 5, 10, 15 };
	hxarray<int, 3u> a;
	a = values;
	EXPECT_EQ(a[0], 5);
	EXPECT_EQ(a[1], 10);
	EXPECT_EQ(a[2], 15);
}

#if HX_CPLUSPLUS >= 202002L
TEST(hxarray_test, hxarray_initializer_list_constructor) {
	hxarray<int, 4u> a { 3, 6, 9, 12 };
	EXPECT_EQ(a[0], 3);
	EXPECT_EQ(a[1], 6);
	EXPECT_EQ(a[2], 9);
	EXPECT_EQ(a[3], 12);
}
#endif

TEST(hxarray_test, hxarray_iteration) {
	static const int values[5] = { 1, 2, 3, 4, 5 };
	hxarray<int, 5u> a(values);
	const hxarray<int, 5u>& ca = a;

	size_t index = 0u;
	for(hxarray<int, 5u>::iterator it = a.begin(); it != a.end(); ++it) {
		EXPECT_EQ(*it, values[index++]);
	}
	EXPECT_EQ(index, 5u);

	index = 0u;
	for(hxarray<int, 5u>::const_iterator it = ca.cbegin(); it != ca.cend(); ++it) {
		EXPECT_EQ(*it, values[index++]);
	}
	EXPECT_EQ(index, 5u);

	EXPECT_EQ(ca.cbegin(), ca.begin());
	EXPECT_EQ(ca.cend(), ca.end());
}

TEST(hxarray_test, hxarray_get) {
	static const int values[3] = { 10, 20, 30 };
	hxarray<int, 3u> a(values);
	const hxarray<int, 3u>& ca = a;

	EXPECT_EQ(a.get(0), a.begin());
	EXPECT_EQ(a.get(1), a.begin() + 1);
	EXPECT_EQ(a.get(3), hxnullptr);

	EXPECT_EQ(ca.get(0), ca.begin());
	EXPECT_EQ(ca.get(2), ca.begin() + 2);
	EXPECT_EQ(ca.get(3), hxnullptr);
}

TEST(hxarray_test, hxarray_find) {
	static const int values[5] = { 2, 4, 4, 8, 16 };
	hxarray<int, 5u> a(values);
	const hxarray<int, 5u>& ca = a;

	EXPECT_EQ(ca.find(4), a.begin() + 1);
	EXPECT_EQ(ca.find(32), a.end());
	EXPECT_EQ(a.find(8), a.begin() + 3);

	const int* fp = ca.find_if([](int value) { return value >= 8; });
	EXPECT_EQ(fp, a.begin() + 3);

	const int* miss = ca.find_if([](int value) { return value < 0; });
	EXPECT_EQ(miss, a.end());

	int* mp = a.find_if([](int& value) { return (value & 1) == 0 && value > 4; });
	EXPECT_EQ(mp, a.begin() + 3);
}

TEST(hxarray_test, hxarray_all_of_any_of) {
	static const int values[5] = { 91, 92, 93, 94, 95 };
	hxarray<int, 5u> a(values);

	EXPECT_TRUE(a.all_of([](const int& x) { return x > 0; }));
	EXPECT_FALSE(a.all_of([](const int& x) { return x < 95; }));

	int all_calls = 0;
	EXPECT_FALSE(a.all_of([&](const int& value) -> bool {
		++all_calls;
		return value < 93;
	}));
	EXPECT_EQ(all_calls, 3);

	int any_calls = 0;
	EXPECT_TRUE(a.any_of([&](const int& value) -> bool {
		++any_calls;
		return value == 94;
	}));
	EXPECT_EQ(any_calls, 4);

	EXPECT_FALSE(a.any_of([](const int& value) { return value < 0; }));
}

TEST(hxarray_test, hxarray_for_each) {
	static const int values[4] = { 10, 20, 30, 40 };
	hxarray<int, 4u> a(values);

	a.for_each([](int& x) { x += 1; });

	EXPECT_EQ(a[0], 11);
	EXPECT_EQ(a[1], 21);
	EXPECT_EQ(a[2], 31);
	EXPECT_EQ(a[3], 41);

	int count = 0;
	const hxarray<int, 4u>& ca = a;
	ca.for_each([&](const int&) { ++count; });
	EXPECT_EQ(count, 4);
}

TEST(hxarray_test, hxarray_assign_from_iterators) {
	static const int src[4] = { 3, 1, 4, 1 };
	hxarray<int, 4u> a;
	a.assign(src + 0, src + 4);
	EXPECT_EQ(a[0], 3);
	EXPECT_EQ(a[1], 1);
	EXPECT_EQ(a[2], 4);
	EXPECT_EQ(a[3], 1);
}

#if HX_CPLUSPLUS >= 202002L
TEST_F(hxarray_test_f, hxarray_assign_range_from_rvalue) {
	hxtest_object source_elements[] = {
		hxtest_object(5),
		hxtest_object(9),
		hxtest_object(13)
	};

	hxarray<hxtest_object, 3u> elements;
	elements.assign_range(hxarray_test_pointer_range(
		source_elements, source_elements + 3));

	EXPECT_EQ(elements[0].id, 5);
	EXPECT_EQ(elements[1].id, 9);
	EXPECT_EQ(elements[2].id, 13);
	for(size_t i = 0u; i < 3u; ++i) {
		EXPECT_FALSE(elements[i].moved_from);
		EXPECT_TRUE(source_elements[i].moved_from);
	}
}

TEST_F(hxarray_test_f, hxarray_assign_range_from_const) {
	static const int32_t src_ints[4] = { 4, 7, 11, 18 };
	const hxarray<hxtest_object, 4u> src(src_ints);

	hxarray<hxtest_object, 4u> elements;
	hxarray_test_pointer_range<const hxtest_object> range(src.begin(), src.end());
	elements.assign_range(range);

	for(size_t i = 0u; i < 4u; ++i) {
		EXPECT_EQ(elements[i].id, src[i].id);
		EXPECT_FALSE(elements[i].moved_from);
		EXPECT_FALSE(src[i].moved_from);
	}
}
#endif

TEST(hxarray_test, hxarray_binary_search) {
	static const int sorted[5] = { 1, 3, 5, 7, 9 };
	hxarray<int, 5u> a(sorted);
	const hxarray<int, 5u>& ca = a;

	const int* found = ca.binary_search(7);
	EXPECT_EQ(found, a.begin() + 3);

	const int* missing = ca.binary_search(4);
	EXPECT_EQ(missing, a.end());

	int* mfound = a.binary_search(5);
	EXPECT_EQ(mfound, a.begin() + 2);
}

TEST(hxarray_test, hxarray_sort) {
	static const int unsorted[6] = { 13, -5, 7, 0, 13, 2 };
	hxarray<int, 6u> a(unsorted);
	a.sort();

	static const int expected[6] = { -5, 0, 2, 7, 13, 13 };
	for(size_t i = 0u; i < 6u; ++i) {
		EXPECT_EQ(a[i], expected[i]);
	}
}

TEST(hxarray_test, hxarray_insertion_sort) {
	static const int unsorted[6] = { 9, 2, 7, 4, 4, 1 };
	hxarray<int, 6u> a(unsorted);
	a.insertion_sort();

	static const int expected[6] = { 1, 2, 4, 4, 7, 9 };
	for(size_t i = 0u; i < 6u; ++i) {
		EXPECT_EQ(a[i], expected[i]);
	}
}

TEST(hxarray_test, hxarray_memcpy_and_memset) {
	static const unsigned char src[4] = { 10, 20, 30, 40 };
	const hxarray<unsigned char, 4u> a(src);
	hxarray<unsigned char, 4u> b;
	b.memcpy(a);

	for(size_t i = 0u; i < 4u; ++i) {
		EXPECT_EQ(b[i], src[i]);
	}

	b.memset(0xff);
	for(size_t i = 0u; i < 4u; ++i) {
		EXPECT_EQ(static_cast<int>(b[i]), 0xff);
	}
}

#if HX_CPLUSPLUS >= 202002L
TEST(hxarray_test, hxarray_equal_and_less) {
	static const int v1[3] = { 1, 2, 3 };
	static const int v2[3] = { 1, 2, 4 };
	const hxarray<int, 3u> a(v1);
	const hxarray<int, 3u> b(v1);
	const hxarray<int, 3u> c(v2);

	EXPECT_TRUE(a.equal(b));
	EXPECT_FALSE(a.equal(c));

	EXPECT_FALSE(a.less(b));
	EXPECT_TRUE(a.less(c));
	EXPECT_FALSE(c.less(a));

	EXPECT_TRUE(hxkey_equal(a, b));
	EXPECT_FALSE(hxkey_equal(a, c));
	EXPECT_FALSE(hxkey_less(a, b));
	EXPECT_TRUE(hxkey_less(a, c));
}
#endif

TEST_F(hxarray_test_f, hxarray_set_size_dynamic) {
	const hxsystem_allocator_scope temporary_stack_scope(hxsystem_allocator_temporary_stack);
	{
		hxarray<hxtest_object, hxallocator_dynamic_capacity> a;
		EXPECT_EQ(a.size(), 0u);
		EXPECT_EQ(a.capacity(), 0u);

		a.set_size(3u);
		EXPECT_EQ(a.size(), 3u);
		EXPECT_EQ(a.capacity(), 3u);
		for(size_t i = 0u; i < a.size(); ++i) {
			EXPECT_FALSE(a[i].moved_from);
		}

		// set_size with same value is a no-op.
		a.set_size(3u);
		EXPECT_EQ(a.size(), 3u);
		hxtest_gdb_break_hxarray_dynamic();
	}
	EXPECT_TRUE(check_totals(3));
}

TEST(hxarray_test, hxarray_set_size_static_noop) {
	hxarray<int, 4u> a;
	// set_size with the fixed capacity is a no-op on static storage.
	a.set_size(4u);
	EXPECT_EQ(a.size(), 4u);
}

TEST_F(hxarray_test_f, hxarray_dynamic_copy_constructor) {
	const hxsystem_allocator_scope temporary_stack_scope(hxsystem_allocator_temporary_stack);
	{
		hxarray<hxtest_object, hxallocator_dynamic_capacity> src;
		src.set_size(2u);
		src[0] = hxtest_object(7);
		src[1] = hxtest_object(8);

		hxarray<hxtest_object, hxallocator_dynamic_capacity> dst(src);
		EXPECT_EQ(dst.size(), 2u);
		EXPECT_EQ(dst[0].id, 7);
		EXPECT_EQ(dst[1].id, 8);
		for(size_t i = 0u; i < 2u; ++i) {
			EXPECT_FALSE(dst[i].moved_from);
			EXPECT_FALSE(src[i].moved_from);
		}
	}
}

TEST_F(hxarray_test_f, hxarray_dynamic_c_array_constructor) {
	const hxsystem_allocator_scope temporary_stack_scope(hxsystem_allocator_temporary_stack);
	{
		static const int32_t values[3] = { 2, 4, 6 };
		hxarray<hxtest_object, hxallocator_dynamic_capacity> a(values);
		EXPECT_EQ(a.size(), 3u);
		EXPECT_EQ(a[0].id, 2);
		EXPECT_EQ(a[1].id, 4);
		EXPECT_EQ(a[2].id, 6);
	}
}

#if HX_CPLUSPLUS >= 202002L
TEST_F(hxarray_test_f, hxarray_dynamic_initializer_list_constructor) {
	const hxsystem_allocator_scope temporary_stack_scope(hxsystem_allocator_temporary_stack);
	{
		hxarray<int, hxallocator_dynamic_capacity> a { 10, 20, 30 };
		EXPECT_EQ(a.size(), 3u);
		EXPECT_EQ(a[0], 10);
		EXPECT_EQ(a[1], 20);
		EXPECT_EQ(a[2], 30);
	}
}
#endif

TEST_F(hxarray_test_f, hxarray_dynamic_assign_from_iterators) {
	const hxsystem_allocator_scope temporary_stack_scope(hxsystem_allocator_temporary_stack);
	{
		hxtest_object src[3] = { hxtest_object(3), hxtest_object(6), hxtest_object(9) };
		hxarray<hxtest_object, hxallocator_dynamic_capacity> a;
		a.assign(src + 0, src + 3);
		EXPECT_EQ(a.size(), 3u);
		EXPECT_EQ(a[0].id, 3);
		EXPECT_EQ(a[1].id, 6);
		EXPECT_EQ(a[2].id, 9);
	}
}

#if HX_CPLUSPLUS >= 202002L
TEST_F(hxarray_test_f, hxarray_dynamic_assign_range_rvalue) {
	const hxsystem_allocator_scope temporary_stack_scope(hxsystem_allocator_temporary_stack);
	{
		hxtest_object source_elements[] = {
			hxtest_object(1),
			hxtest_object(2)
		};

		hxarray<hxtest_object, hxallocator_dynamic_capacity> a;
		a.assign_range(hxarray_test_pointer_range(
			source_elements, source_elements + 2));
		EXPECT_EQ(a.size(), 2u);
		EXPECT_EQ(a[0].id, 1);
		EXPECT_EQ(a[1].id, 2);
		for(size_t i = 0u; i < 2u; ++i) {
			EXPECT_FALSE(a[i].moved_from);
			EXPECT_TRUE(source_elements[i].moved_from);
		}
	}
}
#endif

TEST(hxarray_test, hxarray_equal_mismatched_capacity) {
	// Tests the early-return false branch when capacities differ.
	const hxsystem_allocator_scope temporary_stack_scope(hxsystem_allocator_temporary_stack);
	{
		hxarray<int, hxallocator_dynamic_capacity> a;
		hxarray<int, hxallocator_dynamic_capacity> b;
		a.set_size(2u);
		b.set_size(3u);
		a[0] = 1; a[1] = 2;
		b[0] = 1; b[1] = 2; b[2] = 3;
		EXPECT_FALSE(a.equal(b));
	}
}

TEST(hxarray_test, hxarray_less_different_sizes) {
	// Tests the `return n < nx` branch when all shared elements are equal.
	const hxsystem_allocator_scope temporary_stack_scope(hxsystem_allocator_temporary_stack);
	{
		hxarray<int, hxallocator_dynamic_capacity> shorter;
		hxarray<int, hxallocator_dynamic_capacity> longer;
		shorter.set_size(2u);
		longer.set_size(3u);
		shorter[0] = 1; shorter[1] = 2;
		longer[0] = 1; longer[1] = 2; longer[2] = 3;

		EXPECT_TRUE(shorter.less(longer));
		EXPECT_FALSE(longer.less(shorter));
		EXPECT_FALSE(shorter.less(shorter));
	}
}
