// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

// This file is used for hxarray, hxvector, hxflat_set and hxdeque to show they
// have a shared subset of APIs. Initializer lists must be in sorted order so
// that the same literals work for hxflat_set. hxdeque requires a power-of-two
// capacity, so every populated container is exactly full at a capacity of 4.
// All shared tests operate on containers equivalent to:
//   hxarray<hxtest_object, 4> a{ 31, 32, 33, 34 };
//   hxarray<hxtest_object> b{ };
//
// Each including translation unit renames `hxarray` and `hxarray_shared_test_f`
// to the class it tests and its own test suite name, then defines these macros.
//
//   HXSHARED_ARRAY_STATS_1 to HXSHARED_ARRAY_STATS_24 - The check_stats
//     arguments for each test, in the order the tests appear below.
//
// It also predefines a HX_ARRAY_TEST_NO_ macro for every category of test its
// class does not support.
//
//   HX_ARRAY_TEST_NO_INDEX       - operator[] returning a reference.
//   HX_ARRAY_TEST_NO_DATA        - data().
//   HX_ARRAY_TEST_NO_MUTATE      - non-const begin()/end() and mutable
//                                  elements.
//   HX_ARRAY_TEST_NO_ADD_RANGE   - add_range().
//   HX_ARRAY_TEST_NO_CLEAR       - clear().
//   HX_ARRAY_TEST_NO_BACK        - front(), back(), push_back(),
//                                  emplace_back() and pop_back().
//   HX_ARRAY_TEST_NO_ALGORITHM   - all_of(), any_of(), find_if() and
//                                  for_each().
//   HX_ARRAY_TEST_NO_SORT        - sort(), insertion_sort() and search().
//   HX_ARRAY_TEST_NO_HASH        - hash().
//   HX_ARRAY_TEST_NO_MEMSET      - memcpy() and memset().
//   HX_ARRAY_TEST_NO_ERASE       - erase() taking an iterator.
//   HX_ARRAY_TEST_NO_COPY_ASSIGN - copy construction and operator=.
//   HX_ARRAY_TEST_NO_MONADIC     - and_then(), or_else() and value_or().

#define HXSHARED_ARRAY_CAPACITY 4

using hxarray_shared_test_f = hxtest_object_fixture;

TEST_F(hxarray_shared_test_f, construction_and_size) {
	const hxarray<hxtest_object, HXSHARED_ARRAY_CAPACITY> a{
		hxtest_object(31), hxtest_object(32), hxtest_object(33), hxtest_object(34) };
	const hxarray<hxtest_object> b{ };
	EXPECT_EQ(a.size(), 4);
	EXPECT_EQ(a.capacity(), HXSHARED_ARRAY_CAPACITY);
	EXPECT_EQ(a.max_size(), HXSHARED_ARRAY_CAPACITY);
	EXPECT_FALSE(a.empty());
	EXPECT_TRUE(a.full());
	EXPECT_EQ(b.size(), 0);
	EXPECT_TRUE(b.empty());
	EXPECT_TRUE(check_stats(HXSHARED_ARRAY_STATS_1));
}

TEST_F(hxarray_shared_test_f, gdb_print_empty_and_populated) {
	const hxarray<hxtest_object, HXSHARED_ARRAY_CAPACITY> a{
		hxtest_object(31), hxtest_object(32), hxtest_object(33), hxtest_object(34) };
	const hxarray<hxtest_object> b{ };
	hxtest_gdb_break_hxarray_shared();
	EXPECT_EQ(a.size(), 4);
	EXPECT_TRUE(b.empty());
	EXPECT_TRUE(check_stats(HXSHARED_ARRAY_STATS_2));
}

TEST_F(hxarray_shared_test_f, begin_end_const_iteration) {
	const hxarray<hxtest_object, HXSHARED_ARRAY_CAPACITY> a{
		hxtest_object(31), hxtest_object(32), hxtest_object(33), hxtest_object(34) };
	int32_t expected = 31;
	hxsize_t count = 0;
	for(hxarray<hxtest_object, HXSHARED_ARRAY_CAPACITY>::const_iterator it = a.begin();
			it != a.end(); ++it) {
		EXPECT_EQ((*it).value(), expected++);
		++count;
	}
	EXPECT_EQ(count, 4);
	EXPECT_EQ(expected, 35);
	EXPECT_EQ(a.end() - a.begin(), 4);
	EXPECT_TRUE(check_stats(HXSHARED_ARRAY_STATS_3));
}

TEST_F(hxarray_shared_test_f, begin_end_const_iteration_empty) {
	const hxarray<hxtest_object> b{ };
	EXPECT_EQ(b.begin(), b.end());
	EXPECT_EQ(b.end() - b.begin(), 0);
	EXPECT_TRUE(check_stats(HXSHARED_ARRAY_STATS_4));
}

TEST_F(hxarray_shared_test_f, cbegin_cend) {
	const hxarray<hxtest_object, HXSHARED_ARRAY_CAPACITY> a{
		hxtest_object(31), hxtest_object(32), hxtest_object(33), hxtest_object(34) };
	EXPECT_TRUE(a.cbegin() == a.begin());
	EXPECT_TRUE(a.cend() == a.end());
	EXPECT_EQ((*a.cbegin()).value(), 31);
	EXPECT_EQ((*(a.cend() - 1)).value(), 34);
	int32_t expected = 31;
	hxsize_t count = 0;
	for(hxarray<hxtest_object, HXSHARED_ARRAY_CAPACITY>::const_iterator it = a.cbegin();
			it != a.cend(); ++it) {
		EXPECT_EQ((*it).value(), expected++);
		++count;
	}
	EXPECT_EQ(count, 4);
	EXPECT_TRUE(check_stats(HXSHARED_ARRAY_STATS_5));
}

TEST_F(hxarray_shared_test_f, find_first_middle_last_and_miss) {
	const hxarray<hxtest_object, HXSHARED_ARRAY_CAPACITY> a{
		hxtest_object(31), hxtest_object(32), hxtest_object(33), hxtest_object(34) };
	EXPECT_EQ(a.find(hxtest_object(31)), a.begin());
	EXPECT_EQ(a.find(hxtest_object(32)), a.begin() + 1);
	EXPECT_EQ(a.find(hxtest_object(33)), a.begin() + 2);
	EXPECT_EQ(a.find(hxtest_object(34)), a.begin() + 3);
	EXPECT_EQ(a.find(hxtest_object(30)), a.end());
	EXPECT_EQ(a.find(hxtest_object(35)), a.end());
	EXPECT_TRUE(check_stats(HXSHARED_ARRAY_STATS_6));
}

TEST_F(hxarray_shared_test_f, find_on_empty) {
	const hxarray<hxtest_object> b{ };
	EXPECT_EQ(b.find(hxtest_object(31)), b.end());
	EXPECT_TRUE(check_stats(HXSHARED_ARRAY_STATS_7));
}

TEST_F(hxarray_shared_test_f, operator_equal) {
	const hxarray<hxtest_object> a{
		hxtest_object(31), hxtest_object(32), hxtest_object(33), hxtest_object(34) };
	const hxarray<hxtest_object> a2{
		hxtest_object(31), hxtest_object(32), hxtest_object(33), hxtest_object(34) };
	const hxarray<hxtest_object> c{
		hxtest_object(31), hxtest_object(32), hxtest_object(33), hxtest_object(35) };
	const hxarray<hxtest_object> b{ };
	const hxarray<hxtest_object> b2{ };
	EXPECT_TRUE(a == a2);
	EXPECT_FALSE(a == c);
	EXPECT_TRUE(b == b2);
	EXPECT_FALSE(a == b);
	EXPECT_FALSE(b == a);
	EXPECT_TRUE(check_stats(HXSHARED_ARRAY_STATS_8));
}

TEST_F(hxarray_shared_test_f, operator_less) {
	const hxarray<hxtest_object> a{
		hxtest_object(31), hxtest_object(32), hxtest_object(33), hxtest_object(34) };
	const hxarray<hxtest_object> c{
		hxtest_object(31), hxtest_object(32), hxtest_object(33), hxtest_object(35) };
	const hxarray<hxtest_object> b{ };
	EXPECT_TRUE(a < c);
	EXPECT_FALSE(c < a);
	EXPECT_FALSE(a < a);
	EXPECT_TRUE(b < a);
	EXPECT_FALSE(a < b);
	EXPECT_FALSE(b < b);
	EXPECT_TRUE(check_stats(HXSHARED_ARRAY_STATS_9));
}

TEST_F(hxarray_shared_test_f, swap_exchanges_contents) {
	hxarray<hxtest_object> a{
		hxtest_object(31), hxtest_object(32), hxtest_object(33), hxtest_object(34) };
	hxarray<hxtest_object> b{ };
	a.swap(b);
	EXPECT_TRUE(a.empty());
	EXPECT_EQ(a.size(), 0);
	EXPECT_EQ(a.begin(), a.end());
	EXPECT_EQ(b.size(), 4);
	EXPECT_EQ(b.find(hxtest_object(31)), b.begin());
	EXPECT_EQ(b.find(hxtest_object(34)), b.begin() + 3);
	EXPECT_TRUE(check_stats(HXSHARED_ARRAY_STATS_10));
}

TEST_F(hxarray_shared_test_f, conforms_to_rand_iterator_api) {
	const hxarray<hxtest_object, HXSHARED_ARRAY_CAPACITY> a{
		hxtest_object(31), hxtest_object(32), hxtest_object(33), hxtest_object(34) };
	using const_iterator = hxarray<hxtest_object, HXSHARED_ARRAY_CAPACITY>::const_iterator;
	EXPECT_TRUE(hxtest_check_rand_iterator_api<const_iterator>(a.begin(), a.end()));
	EXPECT_TRUE(check_stats(HXSHARED_ARRAY_STATS_11));
}

#if !defined HX_ARRAY_TEST_NO_INDEX
TEST_F(hxarray_shared_test_f, index_operator_reads_every_position) {
	hxarray<hxtest_object, HXSHARED_ARRAY_CAPACITY> a{
		hxtest_object(31), hxtest_object(32), hxtest_object(33), hxtest_object(34) };
	const hxarray<hxtest_object, HXSHARED_ARRAY_CAPACITY>& const_a = a;
	EXPECT_EQ(const_a[0].value(), 31);
	EXPECT_EQ(const_a[1].value(), 32);
	EXPECT_EQ(const_a[2].value(), 33);
	EXPECT_EQ(const_a[3].value(), 34);
	EXPECT_EQ(a[0].value(), 31);
	EXPECT_EQ(a[3].value(), 34);
	EXPECT_EQ(&a[0], &*a.begin());
	EXPECT_EQ(&a[3], &*(a.begin() + 3));
	EXPECT_TRUE(check_stats(HXSHARED_ARRAY_STATS_12));
}
#endif // !HX_ARRAY_TEST_NO_INDEX

#if !defined HX_ARRAY_TEST_NO_DATA
TEST_F(hxarray_shared_test_f, data_addresses_first_element) {
	const hxarray<hxtest_object, HXSHARED_ARRAY_CAPACITY> a{
		hxtest_object(31), hxtest_object(32), hxtest_object(33), hxtest_object(34) };
	EXPECT_EQ(a.data(), a.begin());
	EXPECT_EQ(a.data()[0].value(), 31);
	EXPECT_EQ(a.data()[3].value(), 34);
	EXPECT_EQ(a.data() + 4, a.end());
	EXPECT_TRUE(check_stats(HXSHARED_ARRAY_STATS_13));
}
#endif // !HX_ARRAY_TEST_NO_DATA

#if !defined HX_ARRAY_TEST_NO_MUTATE
TEST_F(hxarray_shared_test_f, mutable_begin_end_writes_through) {
	hxarray<hxtest_object, HXSHARED_ARRAY_CAPACITY> a{
		hxtest_object(31), hxtest_object(32), hxtest_object(33), hxtest_object(34) };
	hxsize_t count = 0;
	for(hxarray<hxtest_object, HXSHARED_ARRAY_CAPACITY>::iterator it = a.begin();
			it != a.end(); ++it) {
		(*it).value() += 100;
		++count;
	}
	EXPECT_EQ(count, 4);
	EXPECT_EQ((*a.begin()).value(), 131);
	EXPECT_EQ((*(a.end() - 1)).value(), 134);
	EXPECT_EQ(a.find(hxtest_object(131)), a.begin());
	EXPECT_EQ(a.find(hxtest_object(134)), a.begin() + 3);
	EXPECT_EQ(a.find(hxtest_object(31)), a.end());
	EXPECT_TRUE(check_stats(HXSHARED_ARRAY_STATS_14));
}
#endif // !HX_ARRAY_TEST_NO_MUTATE

#if !defined HX_ARRAY_TEST_NO_ADD_RANGE
TEST_F(hxarray_shared_test_f, add_range_from_lvalue_and_rvalue) {
	const hxtest_object source[] = {
		hxtest_object(31), hxtest_object(32), hxtest_object(33), hxtest_object(34) };
	hxarray<hxtest_object, HXSHARED_ARRAY_CAPACITY> a;
	EXPECT_TRUE(a.empty());
	a.add_range(hxmake_range(source + 0, source + 4));
	EXPECT_EQ(a.size(), 4);
	EXPECT_TRUE(a.full());
	EXPECT_EQ((*a.begin()).value(), 31);
	EXPECT_EQ((*(a.end() - 1)).value(), 34);

	hxarray<hxtest_object, HXSHARED_ARRAY_CAPACITY> b;
	b.add_range(hxmake_range(a.begin(), a.begin() + 1));
	EXPECT_EQ(b.size(), 1);
	EXPECT_FALSE(b.full());
	EXPECT_EQ((*b.begin()).value(), 31);

	hxarray<hxtest_object, HXSHARED_ARRAY_CAPACITY> c;
	c.add_range(hxmake_range(a.begin(), a.begin()));
	EXPECT_EQ(c.size(), 0);
	EXPECT_TRUE(c.empty());
	EXPECT_TRUE(check_stats(HXSHARED_ARRAY_STATS_15));
}
#endif // !HX_ARRAY_TEST_NO_ADD_RANGE

#if !defined HX_ARRAY_TEST_NO_CLEAR
TEST_F(hxarray_shared_test_f, clear_destroys_all_and_is_idempotent) {
	hxarray<hxtest_object, HXSHARED_ARRAY_CAPACITY> a{
		hxtest_object(31), hxtest_object(32), hxtest_object(33), hxtest_object(34) };
	EXPECT_EQ(a.size(), 4);
	a.clear();
	EXPECT_EQ(a.size(), 0);
	EXPECT_TRUE(a.empty());
	EXPECT_FALSE(a.full());
	EXPECT_EQ(a.begin(), a.end());
	EXPECT_EQ(a.find(hxtest_object(31)), a.end());
	a.clear();
	EXPECT_EQ(a.size(), 0);
	EXPECT_EQ(a.capacity(), HXSHARED_ARRAY_CAPACITY);
	EXPECT_TRUE(check_stats(HXSHARED_ARRAY_STATS_16));
}
#endif // !HX_ARRAY_TEST_NO_CLEAR

#if !defined HX_ARRAY_TEST_NO_BACK
TEST_F(hxarray_shared_test_f, push_back_front_back_and_pop_back) {
	hxarray<hxtest_object, HXSHARED_ARRAY_CAPACITY> a;
	a.push_back(hxtest_object(31));
	EXPECT_EQ(a.size(), 1);
	EXPECT_EQ(a.front().value(), 31);
	EXPECT_EQ(a.back().value(), 31);
	EXPECT_EQ(&a.front(), &a.back());
	a.push_back(hxtest_object(32));
	a.emplace_back(33);
	a.emplace_back(34);
	EXPECT_EQ(a.size(), 4);
	EXPECT_TRUE(a.full());
	EXPECT_EQ(a.front().value(), 31);
	EXPECT_EQ(a.back().value(), 34);

	const hxarray<hxtest_object, HXSHARED_ARRAY_CAPACITY>& const_a = a;
	EXPECT_EQ(const_a.front().value(), 31);
	EXPECT_EQ(const_a.back().value(), 34);

	a.pop_back();
	EXPECT_EQ(a.size(), 3);
	EXPECT_FALSE(a.full());
	EXPECT_EQ(a.back().value(), 33);
	a.pop_back();
	a.pop_back();
	EXPECT_EQ(a.size(), 1);
	EXPECT_EQ(a.front().value(), 31);
	EXPECT_EQ(a.back().value(), 31);
	a.pop_back();
	EXPECT_TRUE(a.empty());
	EXPECT_EQ(a.begin(), a.end());
	EXPECT_TRUE(check_stats(HXSHARED_ARRAY_STATS_17));
}
#endif // !HX_ARRAY_TEST_NO_BACK

#if !defined HX_ARRAY_TEST_NO_ALGORITHM
TEST_F(hxarray_shared_test_f, all_of_any_of_find_if_and_for_each) {
	hxarray<hxtest_object, HXSHARED_ARRAY_CAPACITY> a{
		hxtest_object(31), hxtest_object(32), hxtest_object(33), hxtest_object(34) };
	const hxarray<hxtest_object, HXSHARED_ARRAY_CAPACITY>& const_a = a;

	EXPECT_TRUE(const_a.all_of([](const hxtest_object& x) { return x.value() >= 31; }));
	EXPECT_FALSE(const_a.all_of([](const hxtest_object& x) { return x.value() >= 32; }));
	EXPECT_FALSE(const_a.all_of([](const hxtest_object& x) { return x.value() <= 33; }));
	EXPECT_TRUE(a.all_of([](hxtest_object& x) { return x.value() <= 34; }));

	EXPECT_TRUE(const_a.any_of([](const hxtest_object& x) { return x.value() == 31; }));
	EXPECT_TRUE(const_a.any_of([](const hxtest_object& x) { return x.value() == 34; }));
	EXPECT_FALSE(const_a.any_of([](const hxtest_object& x) { return x.value() == 30; }));
	EXPECT_FALSE(const_a.any_of([](const hxtest_object& x) { return x.value() == 35; }));
	EXPECT_TRUE(a.any_of([](hxtest_object& x) { return x.value() == 32; }));

	EXPECT_EQ(const_a.find_if([](const hxtest_object& x) { return x.value() == 31; }),
		const_a.begin());
	EXPECT_EQ(const_a.find_if([](const hxtest_object& x) { return x.value() == 34; }),
		const_a.begin() + 3);
	EXPECT_EQ(const_a.find_if([](const hxtest_object& x) { return x.value() > 31; }),
		const_a.begin() + 1);
	EXPECT_EQ(const_a.find_if([](const hxtest_object& x) { return x.value() == 35; }),
		const_a.end());
	EXPECT_EQ(a.find_if([](hxtest_object& x) { return x.value() == 33; }), a.begin() + 2);

	int32_t total = 0;
	const_a.for_each([&total](const hxtest_object& x) { total += x.value(); });
	EXPECT_EQ(total, 130);
	a.for_each([](hxtest_object& x) { x.value() += 1; });
	EXPECT_EQ((*a.begin()).value(), 32);
	EXPECT_EQ((*(a.end() - 1)).value(), 35);
	EXPECT_TRUE(check_stats(HXSHARED_ARRAY_STATS_18));
}
#endif // !HX_ARRAY_TEST_NO_ALGORITHM

#if !defined HX_ARRAY_TEST_NO_SORT
TEST_F(hxarray_shared_test_f, sort_insertion_sort_and_search) {
	hxarray<hxtest_object, HXSHARED_ARRAY_CAPACITY> a{
		hxtest_object(31), hxtest_object(32), hxtest_object(33), hxtest_object(34) };
	a[0].value() = 34;
	a[1].value() = 33;
	a[2].value() = 32;
	a[3].value() = 31;
	a.sort();
	EXPECT_EQ(a[0].value(), 31);
	EXPECT_EQ(a[1].value(), 32);
	EXPECT_EQ(a[2].value(), 33);
	EXPECT_EQ(a[3].value(), 34);

	a[0].value() = 32;
	a[1].value() = 31;
	a.insertion_sort();
	EXPECT_EQ(a[0].value(), 31);
	EXPECT_EQ(a[1].value(), 32);
	EXPECT_EQ(a[3].value(), 34);

	EXPECT_EQ(a.search(hxtest_object(31)), a.begin());
	EXPECT_EQ(a.search(hxtest_object(32)), a.begin() + 1);
	EXPECT_EQ(a.search(hxtest_object(33)), a.begin() + 2);
	EXPECT_EQ(a.search(hxtest_object(34)), a.begin() + 3);
	EXPECT_EQ(a.search(hxtest_object(30)), a.end());
	EXPECT_EQ(a.search(hxtest_object(35)), a.end());

	const hxarray<hxtest_object, HXSHARED_ARRAY_CAPACITY>& const_a = a;
	EXPECT_EQ(const_a.search(hxtest_object(31)), const_a.begin());
	EXPECT_EQ(const_a.search(hxtest_object(35)), const_a.end());
	EXPECT_TRUE(check_stats(HXSHARED_ARRAY_STATS_20));
}
#endif // !HX_ARRAY_TEST_NO_SORT

#if !defined HX_ARRAY_TEST_NO_HASH
TEST_F(hxarray_shared_test_f, hash_distinguishes_contents_and_order) {
	const hxarray<hxtest_object, HXSHARED_ARRAY_CAPACITY> a{
		hxtest_object(31), hxtest_object(32), hxtest_object(33), hxtest_object(34) };
	const hxarray<hxtest_object, HXSHARED_ARRAY_CAPACITY> a2{
		hxtest_object(31), hxtest_object(32), hxtest_object(33), hxtest_object(34) };
	const hxarray<hxtest_object, HXSHARED_ARRAY_CAPACITY> c{
		hxtest_object(31), hxtest_object(32), hxtest_object(33), hxtest_object(35) };
	const hxarray<hxtest_object> b{ };
	EXPECT_EQ(a.hash(), a2.hash());
	EXPECT_NE(a.hash(), c.hash());
	EXPECT_NE(a.hash(), b.hash());
	EXPECT_EQ(b.hash(), hxarray<hxtest_object>{ }.hash());
	EXPECT_TRUE(check_stats(HXSHARED_ARRAY_STATS_21));
}
#endif // !HX_ARRAY_TEST_NO_HASH

#if !defined HX_ARRAY_TEST_NO_MEMSET
TEST_F(hxarray_shared_test_f, memcpy_and_memset_replace_contents) {
	hxarray<int32_t, HXSHARED_ARRAY_CAPACITY> a{ 31, 32, 33, 34 };
	hxarray<int32_t, HXSHARED_ARRAY_CAPACITY> b{ 0, 0, 0, 0 };
	b.memcpy(a);
	EXPECT_EQ(b[0], 31);
	EXPECT_EQ(b[1], 32);
	EXPECT_EQ(b[2], 33);
	EXPECT_EQ(b[3], 34);

	b.memset();
	EXPECT_EQ(b[0], 0);
	EXPECT_EQ(b[3], 0);
	EXPECT_EQ(a[0], 31);
	EXPECT_EQ(a[3], 34);

	b.memset(-1);
	EXPECT_EQ(b[0], -1);
	EXPECT_EQ(b[3], -1);
	EXPECT_TRUE(check_stats(HXSHARED_ARRAY_STATS_19));
}
#endif // !HX_ARRAY_TEST_NO_MEMSET

#if !defined HX_ARRAY_TEST_NO_ERASE
TEST_F(hxarray_shared_test_f, erase_iterator_from_front_middle_and_back) {
	hxarray<hxtest_object, HXSHARED_ARRAY_CAPACITY> a{
		hxtest_object(31), hxtest_object(32), hxtest_object(33), hxtest_object(34) };
	a.erase(a.begin() + 3);
	EXPECT_EQ(a.size(), 3);
	EXPECT_EQ(a.find(hxtest_object(34)), a.end());
	EXPECT_EQ((*(a.end() - 1)).value(), 33);

	a.erase(a.begin() + 1);
	EXPECT_EQ(a.size(), 2);
	EXPECT_EQ((*a.begin()).value(), 31);
	EXPECT_EQ((*(a.begin() + 1)).value(), 33);
	EXPECT_EQ(a.find(hxtest_object(32)), a.end());

	a.erase(a.begin());
	EXPECT_EQ(a.size(), 1);
	EXPECT_EQ((*a.begin()).value(), 33);

	a.erase(a.begin());
	EXPECT_TRUE(a.empty());
	EXPECT_EQ(a.begin(), a.end());
	EXPECT_TRUE(check_stats(HXSHARED_ARRAY_STATS_22));
}
#endif // !HX_ARRAY_TEST_NO_ERASE

#if !defined HX_ARRAY_TEST_NO_COPY_ASSIGN
TEST_F(hxarray_shared_test_f, copy_construct_and_copy_assign) {
	const hxarray<hxtest_object, HXSHARED_ARRAY_CAPACITY> a{
		hxtest_object(31), hxtest_object(32), hxtest_object(33), hxtest_object(34) };
	const hxarray<hxtest_object, HXSHARED_ARRAY_CAPACITY> copy(a); // NOLINT(performance-unnecessary-copy-initialization)
	EXPECT_EQ(copy.size(), 4);
	EXPECT_TRUE(copy == a);
	EXPECT_EQ((*copy.begin()).value(), 31);
	EXPECT_EQ((*(copy.end() - 1)).value(), 34);
	EXPECT_NE(&*copy.begin(), &*a.begin());

	hxarray<hxtest_object, HXSHARED_ARRAY_CAPACITY> assigned;
	assigned = a;
	EXPECT_EQ(assigned.size(), 4);
	EXPECT_TRUE(assigned == a);
	EXPECT_EQ((*assigned.begin()).value(), 31);
	EXPECT_EQ((*(assigned.end() - 1)).value(), 34);
	EXPECT_TRUE(check_stats(HXSHARED_ARRAY_STATS_23));
}
#endif // !HX_ARRAY_TEST_NO_COPY_ASSIGN

#if !defined HX_ARRAY_TEST_NO_MONADIC && HX_CPLUSPLUS >= 202302L
TEST_F(hxarray_shared_test_f, and_then_or_else_and_value_or_on_iterators) {
	const hxarray<hxtest_object, HXSHARED_ARRAY_CAPACITY> a{
		hxtest_object(31), hxtest_object(32), hxtest_object(33), hxtest_object(34) };
	const auto and_then = [](const hxtest_object& x) {
		return hxexpected<int32_t>(false, x.value() + 1);
	};
	static_assert(hxis_same<decltype(a.and_then(a.begin(), and_then)), hxexpected<int32_t>>());
	EXPECT_EQ(a.and_then(a.begin(), and_then).value(), 32);
	EXPECT_EQ(a.and_then(a.begin() + 3, and_then).value(), 35);
	EXPECT_FALSE(static_cast<bool>(a.and_then(a.end(), and_then)));

	const auto or_else = [&a]() { return a.begin() + 1; };
	EXPECT_EQ(a.or_else(a.begin(), or_else), a.begin());
	EXPECT_EQ(a.or_else(a.begin() + 3, or_else), a.begin() + 3);
	EXPECT_EQ(a.or_else(a.end(), or_else), a.begin() + 1);

	EXPECT_EQ(a.value_or(a.begin(), 20).value(), 31);
	EXPECT_EQ(a.value_or(a.begin() + 3, 20).value(), 34);
	EXPECT_EQ(a.value_or(a.end(), 20).value(), 20);
	EXPECT_EQ(a.value_or(a.end(), 20, 23).value(), 43);
	EXPECT_TRUE(check_stats(HXSHARED_ARRAY_STATS_24));
}
#endif // !HX_ARRAY_TEST_NO_MONADIC && HX_CPLUSPLUS >= 202302L
