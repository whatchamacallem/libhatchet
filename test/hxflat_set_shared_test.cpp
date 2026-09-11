// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

#include <hx/hxflat_set.hpp>
#if HX_CPLUSPLUS >= 202302L
#include <hx/hxexpected.hpp>
#endif // HX_CPLUSPLUS >= 202302L
#include <hx/hxrange.hpp>
#include <hx/hxtest.hpp>

#include "./hxtest_util.hpp"

HX_NS_USE

// Show API is a shared subset with hxarray, hxvector and hxdeque.
hxattr_noinline static void hxtest_gdb_break_hxflat_set_shared(void) { }
#define hxtest_gdb_break_hxarray_shared hxtest_gdb_break_hxflat_set_shared
#define hxarray hxflat_set
#define hxarray_shared_test_f hxflat_set_shared_test_f

#define HXSHARED_ARRAY_EQUAL(a_, b_) ((a_) == (b_))
#define HXSHARED_ARRAY_LESS(a_, b_) ((a_) < (b_))
#define HXSHARED_ARRAY_ERASE(a_, it_) ((a_).erase(it_))

// Keys are const and always sorted.
#define HX_ARRAY_TEST_NO_INDEX
#define HX_ARRAY_TEST_NO_MUTATE
#define HX_ARRAY_TEST_NO_BACK
#define HX_ARRAY_TEST_NO_ALGORITHM
#define HX_ARRAY_TEST_NO_SORT
#define HX_ARRAY_TEST_NO_HASH
#define HX_ARRAY_TEST_NO_MEMSET

#define HXSHARED_ARRAY_STATS_1 8, 4, 0, 4, 4, 0, 0, 0, 0, 0, 4
#define HXSHARED_ARRAY_STATS_2 8, 4, 0, 4, 4, 0, 0, 0, 0, 0, 4
#define HXSHARED_ARRAY_STATS_3 8, 4, 0, 4, 4, 0, 0, 0, 0, 0, 4
#define HXSHARED_ARRAY_STATS_4 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
#define HXSHARED_ARRAY_STATS_5 8, 4, 0, 4, 4, 0, 0, 0, 0, 0, 4
#define HXSHARED_ARRAY_STATS_6 14, 10, 0, 10, 4, 0, 0, 0, 0, 0, 19
#define HXSHARED_ARRAY_STATS_7 1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0
#define HXSHARED_ARRAY_STATS_8 24, 12, 0, 12, 12, 0, 0, 0, 8, 0, 12
#define HXSHARED_ARRAY_STATS_9 16, 8, 0, 8, 8, 0, 0, 0, 12, 2, 8
#define HXSHARED_ARRAY_STATS_10 10, 6, 0, 6, 4, 0, 0, 0, 0, 0, 9
#define HXSHARED_ARRAY_STATS_11 8, 4, 0, 4, 4, 0, 0, 0, 2, 0, 4
#define HXSHARED_ARRAY_STATS_13 8, 4, 0, 4, 4, 0, 0, 0, 0, 0, 4
#define HXSHARED_ARRAY_STATS_15 9, 0, 0, 4, 5, 0, 0, 0, 0, 0, 4
#define HXSHARED_ARRAY_STATS_16 9, 9, 0, 5, 4, 0, 0, 0, 0, 0, 4
#define HXSHARED_ARRAY_STATS_19 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
#define HXSHARED_ARRAY_STATS_22 10, 10, 0, 6, 4, 0, 0, 2, 0, 0, 8
#define HXSHARED_ARRAY_STATS_23 16, 4, 0, 4, 12, 0, 0, 0, 8, 0, 4
#define HXSHARED_ARRAY_STATS_24 12, 8, 0, 6, 6, 0, 0, 0, 0, 0, 4

#include "./hxshared_array_test.inl"
