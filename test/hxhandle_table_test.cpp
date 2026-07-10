// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

#include <hx/hxhandle_table.hpp>
#include <hx/hxmemory_manager.h>
#include "./hxtest_util.hpp"

HX_NS_USE

#if !defined _MSC_VER && !defined __wasm__
static_assert(sizeof(size_t) != 4 || (
		sizeof(hxhandle_table<hxtest_object, hxdefault_delete>) == 20u),
	"hxhandle_table with dynamic capacity must pack its size, free head"
	" pointer, cached bits, allocator capacity and data pointer with no"
	" padding beyond natural alignment");
static_assert(sizeof(size_t) != 8 || (
		sizeof(hxhandle_table<hxtest_object, hxdefault_delete>) == 40u),
	"hxhandle_table with dynamic capacity must pack its size, free head"
	" pointer, cached bits, allocator capacity and data pointer with no"
	" padding beyond natural alignment");

static_assert(sizeof(size_t) != 4 || (
		sizeof(hxhandle_table<hxtest_object, hxdefault_delete, 2>) == 56u),
	"hxhandle_table with fixed capacity must pack its size, free head"
	" pointer and inline slot array with no padding beyond natural"
	" alignment");
static_assert(sizeof(size_t) != 8 || (
		sizeof(hxhandle_table<hxtest_object, hxdefault_delete, 2>) == 80u),
	"hxhandle_table with fixed capacity must pack its size, free head"
	" pointer and inline slot array with no padding beyond natural"
	" alignment");
#endif

namespace {

int hxs_handle_table_deleted = 0;

struct hxtest_handle_counting_deleter {
	void operator()(hxtest_object* ptr) const {
		if(ptr != hxnull) {
			++hxs_handle_table_deleted;
			hxdelete(ptr);
		}
	}
	operator bool(void) const { return true; }
};

struct hxtest_handle_disabled_deleter {
	// GCOVR_EXCL_START
	void operator()(hxtest_object*) const { }
	// GCOVR_EXCL_STOP
	operator bool(void) const { return false; }
};

} // namespace

TEST_F(hxtest_object_fixture, hxhandle_table_construction) {
	const hxhandle_table<hxtest_object, hxdo_not_delete, 2> fixed;
	EXPECT_EQ(fixed.capacity(), (hxsize_t)4);
	EXPECT_EQ(fixed.size(), (hxsize_t)0);
	EXPECT_TRUE(fixed.empty());
	const hxhandle_table<hxtest_object, hxdo_not_delete> dynamic;
	EXPECT_EQ(dynamic.capacity(), (hxsize_t)0);
	EXPECT_EQ(dynamic.size(), (hxsize_t)0);
	EXPECT_TRUE(dynamic.empty());
}

TEST_F(hxtest_object_fixture, hxhandle_table_insert_raw_pointer) {
	const hxsystem_allocator_scope scope(hxsystem_allocator_stack_0);
	hxhandle_table<hxtest_object, hxdefault_delete, 2> t;
	const hxhandle_t h0 = t.insert(hxnew<hxtest_object>(0));
	EXPECT_NE(h0, static_cast<hxhandle_t>(0));
	EXPECT_EQ(t.size(), (hxsize_t)1);
	EXPECT_FALSE(t.empty());
	const hxhandle_t h1 = t.insert(hxnew<hxtest_object>(1));
	const hxhandle_t h2 = t.insert(hxnew<hxtest_object>(2));
	const hxhandle_t h3 = t.insert(hxnew<hxtest_object>(3));
	EXPECT_EQ(t.size(), (hxsize_t)4);
	EXPECT_NE(h0, h1); EXPECT_NE(h0, h2); EXPECT_NE(h0, h3);
	EXPECT_NE(h1, h2); EXPECT_NE(h1, h3); EXPECT_NE(h2, h3);
	EXPECT_EQ(t.get(h0)->id, (int32_t)0);
	EXPECT_EQ(t.get(h1)->id, (int32_t)1);
	EXPECT_EQ(t.get(h2)->id, (int32_t)2);
	EXPECT_EQ(t.get(h3)->id, (int32_t)3);
}

TEST_F(hxtest_object_fixture, hxhandle_table_insert_hxptr_takes_ownership) {
	const hxsystem_allocator_scope scope(hxsystem_allocator_stack_0);
	hxhandle_table<hxtest_object, hxtest_handle_counting_deleter, 2> t;
	hxptr<hxtest_object, hxtest_handle_counting_deleter> p(hxnew<hxtest_object>(88));
	hxtest_object* const raw = p.get();
	const hxhandle_t h = t.insert(hxmove(p));
	EXPECT_EQ(p.get(), static_cast<hxtest_object*>(hxnull));
	EXPECT_EQ(t.get(h), raw);
	EXPECT_EQ(t.size(), (hxsize_t)1);
}

TEST_F(hxtest_object_fixture, hxhandle_table_get) {
	const hxsystem_allocator_scope scope(hxsystem_allocator_stack_0);
	hxhandle_table<hxtest_object, hxdefault_delete, 2> t;
	EXPECT_EQ(t.get(static_cast<hxhandle_t>(0)), static_cast<hxtest_object*>(hxnull));
	const hxhandle_t h = t.insert(hxnew<hxtest_object>(7));
	hxtest_object* const v = t.get(h);
	ASSERT_NE(v, static_cast<hxtest_object*>(hxnull));
	EXPECT_EQ(v->id, (int32_t)7);
	const hxhandle_table<hxtest_object, hxdefault_delete, 2>& ct = t;
	const hxtest_object* const cv = ct.get(h);
	ASSERT_NE(cv, static_cast<const hxtest_object*>(hxnull));
	EXPECT_EQ(cv->id, (int32_t)7);
	EXPECT_EQ(ct.get(h ^ static_cast<hxhandle_t>(4)), static_cast<const hxtest_object*>(hxnull));
	EXPECT_TRUE(t.erase(h));
	const hxhandle_t fresh = t.insert(hxnew<hxtest_object>(9));
	EXPECT_NE(h, fresh);
	EXPECT_EQ(t.get(h), static_cast<hxtest_object*>(hxnull));
}

TEST_F(hxtest_object_fixture, hxhandle_table_erase) {
	const hxsystem_allocator_scope scope(hxsystem_allocator_stack_0);
	hxs_handle_table_deleted = 0;
	hxhandle_table<hxtest_object, hxtest_handle_counting_deleter, 2> t;
	EXPECT_FALSE(t.erase(static_cast<hxhandle_t>(0)));
	const hxhandle_t h = t.insert(hxnew<hxtest_object>(9));
	EXPECT_TRUE(t.erase(h));
	EXPECT_EQ(hxs_handle_table_deleted, 1);
	EXPECT_EQ(t.size(), (hxsize_t)0);
	EXPECT_EQ(t.get(h), static_cast<hxtest_object*>(hxnull));
	EXPECT_FALSE(t.erase(h));
	EXPECT_EQ(hxs_handle_table_deleted, 1);
}

TEST_F(hxtest_object_fixture, hxhandle_table_erase_with_disabled_deleter_skips_delete) {
	const hxsystem_allocator_scope scope(hxsystem_allocator_stack_0);
	hxs_handle_table_deleted = 0;
	hxhandle_table<hxtest_object, hxtest_handle_disabled_deleter, 2> t;
	hxtest_object* const raw = hxnew<hxtest_object>(3);
	const hxhandle_t h = t.insert(raw);
	EXPECT_TRUE(t.erase(h));
	EXPECT_EQ(hxs_handle_table_deleted, 0);
	hxdelete(raw);
}

TEST_F(hxtest_object_fixture, hxhandle_table_erase_from_full_table_rebuilds_free_head) {
	const hxsystem_allocator_scope scope(hxsystem_allocator_stack_0);
	hxhandle_table<hxtest_object, hxdefault_delete, 2> t;
	const hxhandle_t h0 = t.insert(hxnew<hxtest_object>(10));
	const hxhandle_t h1 = t.insert(hxnew<hxtest_object>(11));
	const hxhandle_t h2 = t.insert(hxnew<hxtest_object>(12));
	const hxhandle_t h3 = t.insert(hxnew<hxtest_object>(13));
	EXPECT_EQ(t.size(), (hxsize_t)4);
	EXPECT_TRUE(t.erase(h1));
	EXPECT_EQ(t.size(), (hxsize_t)3);
	const hxhandle_t reused = t.insert(hxnew<hxtest_object>(21));
	EXPECT_EQ(t.get(reused)->id, (int32_t)21);
	EXPECT_NE(t.get(h0), static_cast<hxtest_object*>(hxnull));
	EXPECT_NE(t.get(h2), static_cast<hxtest_object*>(hxnull));
	EXPECT_NE(t.get(h3), static_cast<hxtest_object*>(hxnull));
}

TEST_F(hxtest_object_fixture, hxhandle_table_reuse_after_full_cycle_across_all_slots) {
	const hxsystem_allocator_scope scope(hxsystem_allocator_stack_0);
	hxhandle_table<hxtest_object, hxdefault_delete, 2> t;
	const hxhandle_t h0 = t.insert(hxnew<hxtest_object>(0));
	const hxhandle_t h1 = t.insert(hxnew<hxtest_object>(1));
	const hxhandle_t h2 = t.insert(hxnew<hxtest_object>(2));
	const hxhandle_t h3 = t.insert(hxnew<hxtest_object>(3));
	EXPECT_TRUE(t.erase(h0));
	EXPECT_TRUE(t.erase(h1));
	EXPECT_TRUE(t.erase(h2));
	EXPECT_TRUE(t.erase(h3));
	EXPECT_EQ(t.size(), (hxsize_t)0);
	const hxhandle_t r0 = t.insert(hxnew<hxtest_object>(10));
	const hxhandle_t r1 = t.insert(hxnew<hxtest_object>(11));
	const hxhandle_t r2 = t.insert(hxnew<hxtest_object>(12));
	const hxhandle_t r3 = t.insert(hxnew<hxtest_object>(13));
	EXPECT_EQ(t.size(), (hxsize_t)4);
	EXPECT_EQ(t.get(r0)->id, (int32_t)10);
	EXPECT_EQ(t.get(r1)->id, (int32_t)11);
	EXPECT_EQ(t.get(r2)->id, (int32_t)12);
	EXPECT_EQ(t.get(r3)->id, (int32_t)13);
}

TEST_F(hxtest_object_fixture, hxhandle_table_extract) {
	const hxsystem_allocator_scope scope(hxsystem_allocator_stack_0);
	hxs_handle_table_deleted = 0;
	hxhandle_table<hxtest_object, hxtest_handle_counting_deleter, 2> t;
	const hxptr<hxtest_object, hxtest_handle_counting_deleter> missing =
		t.extract(static_cast<hxhandle_t>(0));
	EXPECT_EQ(missing.get(), static_cast<hxtest_object*>(hxnull));
	hxtest_object* const raw = hxnew<hxtest_object>(55);
	const hxhandle_t h = t.insert(raw);
	const hxptr<hxtest_object, hxtest_handle_counting_deleter> p = t.extract(h);
	EXPECT_EQ(p.get(), raw);
	EXPECT_EQ(hxs_handle_table_deleted, 0);
	EXPECT_EQ(t.size(), (hxsize_t)0);
	EXPECT_EQ(t.get(h), static_cast<hxtest_object*>(hxnull));
}

TEST_F(hxtest_object_fixture, hxhandle_table_extract_from_full_table_rebuilds_free_head) {
	const hxsystem_allocator_scope scope(hxsystem_allocator_stack_0);
	hxhandle_table<hxtest_object, hxtest_handle_counting_deleter, 2> t;
	t.insert(hxnew<hxtest_object>(10));
	const hxhandle_t h1 = t.insert(hxnew<hxtest_object>(11));
	t.insert(hxnew<hxtest_object>(12));
	t.insert(hxnew<hxtest_object>(13));
	hxptr<hxtest_object, hxtest_handle_counting_deleter> p = t.extract(h1);
	ASSERT_NE(p.get(), static_cast<hxtest_object*>(hxnull));
	EXPECT_EQ(p->id, (int32_t)11);
	EXPECT_EQ(t.size(), (hxsize_t)3);
	const hxhandle_t reused = t.insert(hxnew<hxtest_object>(31));
	EXPECT_EQ(t.get(reused)->id, (int32_t)31);
}

TEST_F(hxtest_object_fixture, hxhandle_table_replace_missing_handle_returns_null) {
	const hxsystem_allocator_scope scope(hxsystem_allocator_stack_0);
	hxhandle_table<hxtest_object, hxdefault_delete, 2> t;
	hxtest_object* const replacement = hxnew<hxtest_object>(1);
	const hxptr<hxtest_object, hxdefault_delete> old =
		t.replace(static_cast<hxhandle_t>(0), replacement);
	EXPECT_EQ(old.get(), static_cast<hxtest_object*>(hxnull));
	hxdelete(replacement);
}

TEST_F(hxtest_object_fixture, hxhandle_table_replace_stores_new_pointer_and_returns_old) {
	const hxsystem_allocator_scope scope(hxsystem_allocator_stack_0);
	hxhandle_table<hxtest_object, hxdefault_delete, 2> t;
	hxtest_object* const original = hxnew<hxtest_object>(5);
	const hxhandle_t h = t.insert(original);
	hxtest_object* const replacement = hxnew<hxtest_object>(6);
	const hxptr<hxtest_object, hxdefault_delete> old = t.replace(h, replacement);
	EXPECT_EQ(old.get(), original);
	EXPECT_EQ(t.get(h), replacement);
	EXPECT_EQ(t.size(), (hxsize_t)1);
}

TEST_F(hxtest_object_fixture, hxhandle_table_replace_wrong_generation_returns_null) {
	const hxsystem_allocator_scope scope(hxsystem_allocator_stack_0);
	hxhandle_table<hxtest_object, hxdefault_delete, 2> t;
	const hxhandle_t h = t.insert(hxnew<hxtest_object>(7));
	EXPECT_TRUE(t.erase(h));
	hxtest_object* const replacement = hxnew<hxtest_object>(8);
	const hxptr<hxtest_object, hxdefault_delete> old = t.replace(h, replacement);
	EXPECT_EQ(old.get(), static_cast<hxtest_object*>(hxnull));
	hxdelete(replacement);
}

TEST_F(hxtest_object_fixture, hxhandle_table_replace_does_not_change_size) {
	const hxsystem_allocator_scope scope(hxsystem_allocator_stack_0);
	hxhandle_table<hxtest_object, hxdefault_delete, 2> t;
	const hxhandle_t h0 = t.insert(hxnew<hxtest_object>(1));
	const hxhandle_t h1 = t.insert(hxnew<hxtest_object>(2));
	EXPECT_EQ(t.size(), (hxsize_t)2);
	const hxptr<hxtest_object, hxdefault_delete> old = t.replace(h0, hxnew<hxtest_object>(3));
	EXPECT_EQ(t.size(), (hxsize_t)2);
	EXPECT_NE(t.get(h1), static_cast<hxtest_object*>(hxnull));
}

TEST_F(hxtest_object_fixture, hxhandle_table_clear) {
	const hxsystem_allocator_scope scope(hxsystem_allocator_stack_0);
	hxs_handle_table_deleted = 0;
	hxhandle_table<hxtest_object, hxtest_handle_counting_deleter, 2> t;
	t.clear();
	EXPECT_EQ(hxs_handle_table_deleted, 0);
	EXPECT_EQ(t.size(), (hxsize_t)0);
	t.insert(hxnew<hxtest_object>(1));
	t.insert(hxnew<hxtest_object>(2));
	t.clear();
	EXPECT_EQ(hxs_handle_table_deleted, 2);
	EXPECT_EQ(t.size(), (hxsize_t)0);
	EXPECT_TRUE(t.empty());
}

TEST_F(hxtest_object_fixture, hxhandle_table_clear_with_disabled_deleter_skips_delete) {
	const hxsystem_allocator_scope scope(hxsystem_allocator_stack_0);
	hxs_handle_table_deleted = 0;
	hxhandle_table<hxtest_object, hxtest_handle_disabled_deleter, 2> t;
	hxtest_object* const raw0 = hxnew<hxtest_object>(1);
	hxtest_object* const raw1 = hxnew<hxtest_object>(2);
	t.insert(raw0);
	t.insert(raw1);
	t.clear();
	EXPECT_EQ(hxs_handle_table_deleted, 0);
	EXPECT_EQ(t.size(), (hxsize_t)0);
	hxdelete(raw0);
	hxdelete(raw1);
}

TEST_F(hxtest_object_fixture, hxhandle_table_destructor_deletes_owned_values) {
	const hxsystem_allocator_scope scope(hxsystem_allocator_stack_0);
	hxs_handle_table_deleted = 0;
	{
		hxhandle_table<hxtest_object, hxtest_handle_counting_deleter, 2> t;
		t.insert(hxnew<hxtest_object>(1));
		t.insert(hxnew<hxtest_object>(2));
	}
	EXPECT_EQ(hxs_handle_table_deleted, 2);
	hxs_handle_table_deleted = 0;
	hxtest_object* const raw0 = hxnew<hxtest_object>(3);
	hxtest_object* const raw1 = hxnew<hxtest_object>(4);
	{
		hxhandle_table<hxtest_object, hxtest_handle_disabled_deleter, 2> t;
		t.insert(raw0);
		t.insert(raw1);
	}
	EXPECT_EQ(hxs_handle_table_deleted, 0);
	hxdelete(raw0);
	hxdelete(raw1);
}

TEST_F(hxtest_object_fixture, hxhandle_table_release_all) {
	const hxsystem_allocator_scope scope(hxsystem_allocator_stack_0);
	hxs_handle_table_deleted = 0;
	hxhandle_table<hxtest_object, hxtest_handle_disabled_deleter, 2> t;
	hxtest_object* const raw0 = hxnew<hxtest_object>(1);
	hxtest_object* const raw1 = hxnew<hxtest_object>(2);
	const hxhandle_t h0 = t.insert(raw0);
	t.insert(raw1);
	t.release_all();
	EXPECT_EQ(hxs_handle_table_deleted, 0);
	EXPECT_EQ(t.size(), (hxsize_t)0);
	EXPECT_EQ(t.get(h0), static_cast<hxtest_object*>(hxnull));
	hxdelete(raw0);
	hxdelete(raw1);
	hxhandle_table<hxtest_object, hxdo_not_delete> dynamic;
	dynamic.release_all();
	EXPECT_EQ(dynamic.size(), (hxsize_t)0);
	EXPECT_EQ(dynamic.capacity(), (hxsize_t)0);
}

TEST_F(hxtest_object_fixture, hxhandle_table_set_table_size_bits_allocates) {
	const hxsystem_allocator_scope scope(hxsystem_allocator_stack_0);
	hxhandle_table<hxtest_object, hxtest_handle_counting_deleter> t;
	t.set_table_size_bits(3);
	EXPECT_EQ(t.capacity(), (hxsize_t)8);
	const hxhandle_t h = t.insert(hxnew<hxtest_object>(77));
	hxtest_object* const v = t.get(h);
	ASSERT_NE(v, static_cast<hxtest_object*>(hxnull));
	EXPECT_EQ(v->id, (int32_t)77);
	EXPECT_EQ(t.size(), (hxsize_t)1);
	EXPECT_TRUE(t.erase(h));
}
