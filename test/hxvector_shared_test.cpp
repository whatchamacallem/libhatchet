// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

#include <hx/hxrange.hpp>
#include <hx/hxtest.hpp>
#include <hx/hxvector.hpp>
#if HX_CPLUSPLUS >= 202302L
#include <hx/hxexpected.hpp>
#endif // HX_CPLUSPLUS >= 202302L

#include "./hxtest_util.hpp"

HX_NS_USE

// Show API is a shared subset with hxarray, hxflat_set and hxdeque.
hxattr_noinline static void hxtest_gdb_break_hxvector_shared(void) { }
#define hxtest_gdb_break_hxarray_shared hxtest_gdb_break_hxvector_shared
#define hxarray hxvector
#define hxarray_shared_test_f hxvector_shared_test_f

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
#define HXSHARED_ARRAY_STATS_13 8, 4, 0, 4, 4, 0, 0, 0, 0, 0, 0
#define HXSHARED_ARRAY_STATS_14 11, 7, 0, 7, 4, 0, 0, 0, 9, 0, 0
#define HXSHARED_ARRAY_STATS_15 9, 0, 0, 4, 4, 1, 0, 0, 0, 0, 0
#define HXSHARED_ARRAY_STATS_16 9, 9, 0, 5, 4, 0, 0, 0, 0, 0, 0
#define HXSHARED_ARRAY_STATS_17 6, 6, 0, 4, 0, 2, 0, 0, 0, 0, 0
#define HXSHARED_ARRAY_STATS_18 8, 4, 0, 4, 4, 0, 0, 0, 0, 0, 0
#define HXSHARED_ARRAY_STATS_19 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
#define HXSHARED_ARRAY_STATS_20 20, 16, 0, 12, 4, 4, 0, 11, 0, 35, 0
#define HXSHARED_ARRAY_STATS_21 24, 12, 0, 12, 12, 0, 0, 0, 0, 0, 0
#define HXSHARED_ARRAY_STATS_22 10, 10, 0, 6, 4, 0, 0, 2, 5, 0, 0
#define HXSHARED_ARRAY_STATS_23 16, 4, 0, 4, 12, 0, 0, 0, 8, 0, 0
#define HXSHARED_ARRAY_STATS_24 12, 8, 0, 6, 6, 0, 0, 0, 0, 0, 0

#include "./hxshared_array_test.inl"
