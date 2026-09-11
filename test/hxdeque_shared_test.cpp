// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

#include <hx/hxdeque.hpp>
#include <hx/hxrange.hpp>
#include <hx/hxtest.hpp>

#include "./hxtest_util.hpp"

HX_NS_USE

// Show API is a shared subset with hxarray, hxvector and hxflat_set.
hxattr_noinline static void hxtest_gdb_break_hxdeque_shared(void) { }
#define hxtest_gdb_break_hxarray_shared hxtest_gdb_break_hxdeque_shared
#define hxarray hxdeque
#define hxarray_shared_test_f hxdeque_shared_test_f

#define HXSHARED_ARRAY_EQUAL(a_, b_) ((a_) == (b_))
#define HXSHARED_ARRAY_LESS(a_, b_) ((a_) < (b_))

// Elements are addressed through a ring buffer instead of contiguous storage.
#define HX_ARRAY_TEST_NO_DATA
#define HX_ARRAY_TEST_NO_ALGORITHM
#define HX_ARRAY_TEST_NO_SORT
#define HX_ARRAY_TEST_NO_HASH
#define HX_ARRAY_TEST_NO_MEMSET
#define HX_ARRAY_TEST_NO_ERASE
#define HX_ARRAY_TEST_NO_COPY_ASSIGN
#define HX_ARRAY_TEST_NO_MONADIC

#define HXSHARED_ARRAY_STATS_1 8, 4, 0, 4, 4, 0, 0, 0, 0, 0, 0
#define HXSHARED_ARRAY_STATS_2 8, 4, 0, 4, 4, 0, 0, 0, 0, 0, 0
#define HXSHARED_ARRAY_STATS_3 8, 4, 0, 4, 4, 0, 0, 0, 0, 0, 0
#define HXSHARED_ARRAY_STATS_4 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
#define HXSHARED_ARRAY_STATS_5 8, 4, 0, 4, 4, 0, 0, 0, 0, 0, 0
#define HXSHARED_ARRAY_STATS_6 14, 10, 0, 10, 4, 0, 0, 0, 18, 0, 0
#define HXSHARED_ARRAY_STATS_7 1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0
#define HXSHARED_ARRAY_STATS_8 24, 12, 0, 12, 12, 0, 0, 0, 8, 0, 0
#define HXSHARED_ARRAY_STATS_9 16, 8, 0, 8, 8, 0, 0, 0, 12, 2, 0
#define HXSHARED_ARRAY_STATS_10 10, 6, 0, 6, 4, 0, 0, 0, 5, 0, 0
#define HXSHARED_ARRAY_STATS_11 8, 4, 0, 4, 4, 0, 0, 0, 2, 0, 0
#define HXSHARED_ARRAY_STATS_12 8, 4, 0, 4, 4, 0, 0, 0, 0, 0, 0
#define HXSHARED_ARRAY_STATS_14 11, 7, 0, 7, 4, 0, 0, 0, 9, 0, 0
#define HXSHARED_ARRAY_STATS_15 9, 0, 0, 4, 4, 1, 0, 0, 0, 0, 0
#define HXSHARED_ARRAY_STATS_16 9, 9, 0, 5, 4, 0, 0, 0, 0, 0, 0
#define HXSHARED_ARRAY_STATS_17 6, 6, 0, 4, 0, 2, 0, 0, 0, 0, 0
#define HXSHARED_ARRAY_STATS_19 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0

#include "./hxshared_array_test.inl"
