// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

#include <hx/hxlist.hpp>
#include <hx/hxtest.hpp>

HX_NS_USE

hxattr_noinline static void hxtest_gdb_break_hxlist(void) { }

// Use the exact same tests as hxconstexpr_list. This ensures identical APIs and
// eliminates code duplication.

#include "hxshared_list_test.inl"
