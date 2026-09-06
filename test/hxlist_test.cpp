// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

#include <hx/hxlist.hpp>
#include <hx/hxtest.hpp>
#include <hx/hxarray.hpp>
#include <hx/hxvector.hpp>

#include "./hxtest_util.hpp"

HX_NS_USE

#if !defined _MSC_VER && !defined __wasm__
static_assert(sizeof(size_t) != 4 || (
		sizeof(hxlist_node) == 4u),
	"hxlist_node must pack its XOR-linked pointer with no padding");
static_assert(sizeof(size_t) != 8 || (
		sizeof(hxlist_node) == 8u),
	"hxlist_node must pack its XOR-linked pointer with no padding");

static_assert(sizeof(size_t) != 4 || (
		sizeof(hxlist<hxlist_node, hxdo_not_delete>) == 12u),
	"hxlist must pack its size, sentinel and tail with no padding");
static_assert(sizeof(size_t) != 8 || (
		sizeof(hxlist<hxlist_node, hxdo_not_delete>) == 24u),
	"hxlist must pack its size, sentinel and tail with no padding");
#endif

hxattr_noinline static void hxtest_gdb_break_hxlist(void) { }
hxattr_noinline static void hxtest_gdb_break_hxlist_empty(void) { }

#include "./hxshared_list_test.inl"
