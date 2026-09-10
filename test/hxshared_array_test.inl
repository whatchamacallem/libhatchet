// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

// This file is used for hxarray, hxvector, hxflat_set and hxdeque to show they
// have a shared subset of APIs. Initializer lists must be in sorted order so
// that the same literals work for hxflat_set. All shared tests operate on
// containers equivalent to:
//   hxarray<hxtest_object> a{ 31, 32, 33 };
//   hxarray<hxtest_object> b{ };
//
// hxdeque requires a fixed power-of-two capacity, so HXSHARED_ARRAY_MAKE3 and
// HXSHARED_ARRAY_MAKE0 construct by size instead of naming a literal type, and
// HXSHARED_ARRAY_CAPACITY reports what capacity()/max_size() must read back
// for a populated container of 3 elements (3 for a dynamic-capacity container,
// or the fixed capacity otherwise).

#ifndef HXSHARED_ARRAY_MAKE3
#define HXSHARED_ARRAY_MAKE3(name_, v1_, v2_, v3_) \
	hxarray<hxtest_object> name_{ hxtest_object(v1_), hxtest_object(v2_), hxtest_object(v3_) }
#endif
#ifndef HXSHARED_ARRAY_MAKE0
#define HXSHARED_ARRAY_MAKE0(name_) \
	hxarray<hxtest_object> name_{ }
#endif
#ifndef HXSHARED_ARRAY_CAPACITY
#define HXSHARED_ARRAY_CAPACITY 3
#endif

using hxarray_test_f = hxtest_object_fixture;

TEST_F(hxarray_test_f, construction_and_size) {
	const HXSHARED_ARRAY_MAKE3(a, 31, 32, 33);
	const HXSHARED_ARRAY_MAKE0(b);
	EXPECT_EQ(a.size(), 3);
	EXPECT_EQ(a.capacity(), HXSHARED_ARRAY_CAPACITY);
	EXPECT_EQ(a.max_size(), HXSHARED_ARRAY_CAPACITY);
	EXPECT_FALSE(a.empty());
	EXPECT_TRUE(a.full() == (HXSHARED_ARRAY_CAPACITY == 3));
	EXPECT_EQ(b.size(), 0);
	EXPECT_TRUE(b.empty());
	EXPECT_TRUE(check_stats(6, 3, 0, 3, 3, 0, 0, 0, HXSHARED_ARRAY_CMP(0, 0, 0, 0, 2)));
}

TEST_F(hxarray_test_f, gdb_print_empty_and_populated) {
	const HXSHARED_ARRAY_MAKE3(a, 31, 32, 33);
	const HXSHARED_ARRAY_MAKE0(b);
	hxtest_gdb_break_hxarray_shared();
	EXPECT_EQ(a.size(), 3);
	EXPECT_TRUE(b.empty());
	EXPECT_TRUE(check_stats(6, 3, 0, 3, 3, 0, 0, 0, HXSHARED_ARRAY_CMP(0, 0, 0, 0, 2)));
}

TEST_F(hxarray_test_f, begin_end_const_iteration) {
	const HXSHARED_ARRAY_MAKE3(a, 31, 32, 33);
	int32_t expected = 31;
	hxsize_t count = 0;
	for(hxarray<hxtest_object>::const_iterator it = a.begin(); it != a.end(); ++it) {
		EXPECT_EQ(it->value(), expected++);
		++count;
	}
	EXPECT_EQ(count, 3);
	EXPECT_EQ(expected, 34);
	EXPECT_TRUE(check_stats(6, 3, 0, 3, 3, 0, 0, 0, HXSHARED_ARRAY_CMP(0, 0, 0, 0, 2)));
}

TEST_F(hxarray_test_f, begin_end_const_iteration_empty) {
	const HXSHARED_ARRAY_MAKE0(b);
	EXPECT_EQ(b.begin(), b.end());
	EXPECT_TRUE(check_no_stats());
}

TEST_F(hxarray_test_f, cbegin_cend) {
	const HXSHARED_ARRAY_MAKE3(a, 31, 32, 33);
	EXPECT_TRUE(a.cbegin() == a.begin());
	EXPECT_TRUE(a.cend() == a.end());
	EXPECT_EQ(a.cbegin()->value(), 31);
	int32_t expected = 31;
	hxsize_t count = 0;
	for(hxarray<hxtest_object>::const_iterator it = a.cbegin(); it != a.cend(); ++it) {
		EXPECT_EQ(it->value(), expected++);
		++count;
	}
	EXPECT_EQ(count, 3);
	EXPECT_TRUE(check_stats(6, 3, 0, 3, 3, 0, 0, 0, HXSHARED_ARRAY_CMP(0, 0, 0, 0, 2)));
}

TEST_F(hxarray_test_f, find_first_middle_last_and_miss) {
	const HXSHARED_ARRAY_MAKE3(a, 31, 32, 33);
	EXPECT_EQ(a.find(hxtest_object(31)), a.begin());
	EXPECT_EQ(a.find(hxtest_object(32)), a.begin() + 1);
	EXPECT_EQ(a.find(hxtest_object(33)), a.begin() + 2);
	EXPECT_EQ(a.find(hxtest_object(34)), a.end());
	EXPECT_TRUE(check_stats(10, 7, 0, 7, 3, 0, 0, 0, HXSHARED_ARRAY_CMP(9, 0, 0, 0, 10)));
}

TEST_F(hxarray_test_f, find_on_empty) {
	const hxarray<hxtest_object> b{ };
	EXPECT_EQ(b.find(hxtest_object(31)), b.end());
	EXPECT_TRUE(check_stats(1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0));
}

TEST_F(hxarray_test_f, equal) {
	const hxarray<hxtest_object> a{ hxtest_object(31), hxtest_object(32), hxtest_object(33) };
	const hxarray<hxtest_object> a2{ hxtest_object(31), hxtest_object(32), hxtest_object(33) };
	const hxarray<hxtest_object> c{ hxtest_object(31), hxtest_object(32), hxtest_object(34) };
	const hxarray<hxtest_object> b{ };
	const hxarray<hxtest_object> b2{ };
	EXPECT_TRUE(HXSHARED_ARRAY_EQUAL(a, a2));
	EXPECT_FALSE(HXSHARED_ARRAY_EQUAL(a, c));
	EXPECT_TRUE(HXSHARED_ARRAY_EQUAL(b, b2));
	EXPECT_FALSE(HXSHARED_ARRAY_EQUAL(a, b));
	EXPECT_TRUE(check_stats(18, 9, 0, 9, 9, 0, 0, 0, HXSHARED_ARRAY_CMP(6, 0, 6, 0, 6)));
}

TEST_F(hxarray_test_f, less) {
	const hxarray<hxtest_object> a{ hxtest_object(31), hxtest_object(32), hxtest_object(33) };
	const hxarray<hxtest_object> c{ hxtest_object(31), hxtest_object(32), hxtest_object(34) };
	const hxarray<hxtest_object> b{ };
	EXPECT_TRUE(HXSHARED_ARRAY_LESS(a, c));
	EXPECT_FALSE(HXSHARED_ARRAY_LESS(c, a));
	EXPECT_FALSE(HXSHARED_ARRAY_LESS(a, a));
	EXPECT_TRUE(HXSHARED_ARRAY_LESS(b, a));
	EXPECT_FALSE(HXSHARED_ARRAY_LESS(a, b));
	EXPECT_TRUE(check_stats(12, 6, 0, 6, 6, 0, 0, 0, HXSHARED_ARRAY_CMP(9, 2, 9, 2, 4)));
}

TEST_F(hxarray_test_f, swap_exchanges_contents) {
	hxarray<hxtest_object> a{ hxtest_object(31), hxtest_object(32), hxtest_object(33) };
	hxarray<hxtest_object> b{ };
	a.swap(b);
	EXPECT_TRUE(a.empty());
	EXPECT_EQ(b.size(), 3);
	EXPECT_EQ(b.find(hxtest_object(31)), b.begin());
	EXPECT_EQ(b.find(hxtest_object(33)), b.begin() + 2);
	EXPECT_TRUE(check_stats(8, 5, 0, 5, 3, 0, 0, 0, HXSHARED_ARRAY_CMP(4, 0, 0, 0, 6)));
}

TEST_F(hxarray_test_f, conforms_to_rand_iterator_api) {
	const hxarray<hxtest_object> a{ hxtest_object(31), hxtest_object(32), hxtest_object(33) };
	EXPECT_TRUE(hxtest_check_rand_iterator_api<hxarray<hxtest_object>::const_iterator>(
		a.begin(), a.end()));
	EXPECT_TRUE(check_stats(6, 3, 0, 3, 3, 0, 0, 0, HXSHARED_ARRAY_CMP(2, 0, 2, 0, 2)));
}
