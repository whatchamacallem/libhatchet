// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

#include <hx/hxvector.hpp>
#include <hx/hxtest.hpp>

#include "./hxtest_util.hpp"

HX_NS_USE

// Show API is a shared subset with hxarray and hxflat_set.
hxattr_noinline static void hxtest_gdb_break_hxvector_shared(void) { }
#define hxtest_gdb_break_hxarray_shared hxtest_gdb_break_hxvector_shared
#define hxarray hxvector
#define hxarray_test hxvector_test
#define hxarray_test_f hxvector_test_f

#define HXSHARED_ARRAY_EQUAL(a_, b_) ((a_).equal(b_))
#define HXSHARED_ARRAY_LESS(a_, b_) ((a_).less(b_))
#define HXSHARED_ARRAY_CMP(eq_, lt_, flat_eq_, flat_lt_, flat_tw_) eq_, lt_, 0

#include "./hxshared_array_test.inl"
