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

hxattr_noinline static void hxtest_gdb_break_hxhandle_table(void) { }

using hxhandle_table_f = hxtest_object_fixture;

#if HX_CPLUSPLUS >= 202302L
TEST_F(hxhandle_table_f, value_or_emplaces_fallback) {
	hxhandle_table<hxtest_object, hxdefault_delete, 2> t;
	const hxhandle_t handle = t.insert(hxnew<hxtest_object>(10));
	EXPECT_EQ(t.value_or(handle, 14, 17).value(), 10);
	EXPECT_EQ(t.value_or(handle + static_cast<hxhandle_t>(4), 14, 17).value(), 31);
	EXPECT_TRUE(check_stats(3, 2, 0, 2, 1, 0, 0, 0, 0, 0));
}

TEST_F(hxhandle_table_f, or_else_returns_value_or_calls_callable) {
	hxhandle_table<hxtest_object, hxdefault_delete, 2> t;
	hxtest_object* const fallback = hxnew<hxtest_object>(31);
	const hxhandle_t handle = t.insert(hxnew<hxtest_object>(32));
	hxtest_object* const hit = t.or_else(handle, [fallback]{ return fallback; });
	EXPECT_EQ(hit->value(), 32);
	hxtest_object* const miss = t.or_else(
		hxnull_handle, [fallback]{ return fallback; });
	EXPECT_EQ(miss, fallback);
	hxdelete(fallback);
	EXPECT_TRUE(check_stats(2, 1, 0, 2, 0, 0, 0, 0, 0, 0));
}
#endif // HX_CPLUSPLUS >= 202302L

TEST_F(hxhandle_table_f, hxhandle_table_construction) {
	const hxhandle_table<hxtest_object, hxdo_not_delete, 2> fixed;
	EXPECT_EQ(fixed.capacity(), hxsize_t{3});
	EXPECT_EQ(fixed.max_size(), hxsize_t{3});
	EXPECT_EQ(fixed.size(), hxsize_t{0});
	EXPECT_TRUE(fixed.empty());
	EXPECT_FALSE(fixed.full());
	const hxhandle_table<hxtest_object, hxdo_not_delete> dynamic;
	EXPECT_EQ(dynamic.capacity(), hxsize_t{0});
	EXPECT_EQ(dynamic.max_size(), hxsize_t{0});
	EXPECT_EQ(dynamic.size(), hxsize_t{0});
	EXPECT_TRUE(dynamic.empty());
	EXPECT_TRUE(dynamic.full());
	EXPECT_TRUE(check_no_stats());
}

TEST_F(hxhandle_table_f, hxhandle_table_full) {
	hxhandle_table<hxtest_object, hxdefault_delete, 2> t;
	EXPECT_FALSE(t.full());
	t.insert(hxnew<hxtest_object>(0));
	EXPECT_FALSE(t.full());
	t.insert(hxnew<hxtest_object>(1));
	EXPECT_FALSE(t.full());
	const hxhandle_t h2 = t.insert(hxnew<hxtest_object>(2));
	EXPECT_TRUE(t.full());
	EXPECT_TRUE(t.reset(h2));
	EXPECT_FALSE(t.full());
	EXPECT_TRUE(check_stats(3, 1, 0, 3, 0, 0, 0, 0, 0, 0));
}

TEST_F(hxhandle_table_f, hxhandle_table_insert_raw_pointer) {
	hxhandle_table<hxtest_object, hxdefault_delete, 2> a;
	const hxhandle_t h0 = a.insert(hxnew<hxtest_object>(0));
	EXPECT_NE(h0, static_cast<hxhandle_t>(0));
	EXPECT_EQ(a.size(), hxsize_t{1});
	EXPECT_FALSE(a.empty());
	const hxhandle_t h1 = a.insert(hxnew<hxtest_object>(1));
	const hxhandle_t h2 = a.insert(hxnew<hxtest_object>(2));
	EXPECT_EQ(a.size(), hxsize_t{3});
	EXPECT_NE(h0, h1); EXPECT_NE(h0, h2); EXPECT_NE(h1, h2);
	EXPECT_EQ(a.value(h0)->value(), (int32_t)0);
	EXPECT_EQ(a.value(h1)->value(), (int32_t)1);
	EXPECT_EQ(a.value(h2)->value(), (int32_t)2);
	EXPECT_TRUE(a.reset(h1));
	const hxhandle_table<hxtest_object, hxdefault_delete> b;
	hxtest_gdb_break_hxhandle_table();
	EXPECT_TRUE(check_stats(3, 1, 0, 3, 0, 0, 0, 0, 0, 0));
}

TEST_F(hxhandle_table_f, hxhandle_table_insert_null_pointer_returns_null_handle) {
	hxhandle_table<hxtest_object, hxdefault_delete, 2> t;
	const hxhandle_t h = t.insert(static_cast<hxtest_object*>(hxnull));
	EXPECT_EQ(h, hxnull_handle);
	EXPECT_EQ(t.size(), hxsize_t{0});
	EXPECT_TRUE(check_no_stats());
}

TEST_F(hxhandle_table_f, hxhandle_table_insert_hxptr_takes_ownership) {
	hxhandle_table<hxtest_object, hxtest_handle_counting_deleter, 2> t;
	hxptr<hxtest_object, hxtest_handle_counting_deleter> p(hxnew<hxtest_object>(88));
	hxtest_object* const raw = p.get();
	const hxhandle_t h = t.insert(hxmove(p));
	EXPECT_EQ(p.get(), static_cast<hxtest_object*>(hxnull));
	EXPECT_EQ(t.value(h), raw);
	EXPECT_EQ(t.size(), hxsize_t{1});
	EXPECT_TRUE(check_stats(1, 0, 0, 1, 0, 0, 0, 0, 0, 0));
}

TEST_F(hxhandle_table_f, hxhandle_table_value) {
	hxhandle_table<hxtest_object, hxdefault_delete, 2> t;
	EXPECT_EQ(t.value(static_cast<hxhandle_t>(0)), static_cast<hxtest_object*>(hxnull));
	const hxhandle_t h = t.insert(hxnew<hxtest_object>(7));
	hxtest_object* const v = t.value(h);
	ASSERT_NE(v, static_cast<hxtest_object*>(hxnull));
	EXPECT_EQ(v->value(), (int32_t)7);
	const hxhandle_table<hxtest_object, hxdefault_delete, 2>& ct = t;
	const hxtest_object* const cv = ct.value(h);
	ASSERT_NE(cv, static_cast<const hxtest_object*>(hxnull));
	EXPECT_EQ(cv->value(), (int32_t)7);
	EXPECT_EQ(ct.value(h ^ static_cast<hxhandle_t>(4)), static_cast<const hxtest_object*>(hxnull));
	EXPECT_TRUE(t.reset(h));
	const hxhandle_t fresh = t.insert(hxnew<hxtest_object>(9));
	EXPECT_NE(h, fresh);
	EXPECT_EQ(t.value(h), static_cast<hxtest_object*>(hxnull));
	EXPECT_TRUE(check_stats(2, 1, 0, 2, 0, 0, 0, 0, 0, 0));
}

TEST_F(hxhandle_table_f, hxhandle_table_reset) {
	hxs_handle_table_deleted = 0;
	hxhandle_table<hxtest_object, hxtest_handle_counting_deleter, 2> t;
	EXPECT_FALSE(t.reset(static_cast<hxhandle_t>(0)));
	const hxhandle_t h = t.insert(hxnew<hxtest_object>(9));
	EXPECT_TRUE(t.reset(h));
	EXPECT_EQ(hxs_handle_table_deleted, 1);
	EXPECT_EQ(t.size(), hxsize_t{0});
	EXPECT_EQ(t.value(h), static_cast<hxtest_object*>(hxnull));
	EXPECT_FALSE(t.reset(h));
	EXPECT_EQ(hxs_handle_table_deleted, 1);
	EXPECT_TRUE(check_stats(1, 1, 0, 1, 0, 0, 0, 0, 0, 0));
}

TEST_F(hxhandle_table_f, hxhandle_table_reset_with_disabled_deleter_skips_delete) {
	hxs_handle_table_deleted = 0;
	hxhandle_table<hxtest_object, hxtest_handle_disabled_deleter, 2> t;
	hxtest_object* const raw = hxnew<hxtest_object>(3);
	const hxhandle_t h = t.insert(raw);
	EXPECT_TRUE(t.reset(h));
	EXPECT_EQ(hxs_handle_table_deleted, 0);
	hxdelete(raw);
	EXPECT_TRUE(check_stats(1, 1, 0, 1, 0, 0, 0, 0, 0, 0));
}

TEST_F(hxhandle_table_f, hxhandle_table_reset_from_full_table_rebuilds_free_head) {
	hxhandle_table<hxtest_object, hxdefault_delete, 2> t;
	const hxhandle_t h0 = t.insert(hxnew<hxtest_object>(10));
	const hxhandle_t h1 = t.insert(hxnew<hxtest_object>(11));
	const hxhandle_t h2 = t.insert(hxnew<hxtest_object>(12));
	EXPECT_EQ(t.size(), hxsize_t{3});
	EXPECT_TRUE(t.reset(h1));
	EXPECT_EQ(t.size(), hxsize_t{2});
	const hxhandle_t reused = t.insert(hxnew<hxtest_object>(21));
	EXPECT_EQ(t.value(reused)->value(), (int32_t)21);
	EXPECT_NE(t.value(h0), static_cast<hxtest_object*>(hxnull));
	EXPECT_NE(t.value(h2), static_cast<hxtest_object*>(hxnull));
	EXPECT_TRUE(check_stats(4, 1, 0, 4, 0, 0, 0, 0, 0, 0));
}

TEST_F(hxhandle_table_f, hxhandle_table_reuse_after_full_cycle_across_all_slots) {
	hxhandle_table<hxtest_object, hxdefault_delete, 2> t;
	const hxhandle_t h0 = t.insert(hxnew<hxtest_object>(0));
	const hxhandle_t h1 = t.insert(hxnew<hxtest_object>(1));
	const hxhandle_t h2 = t.insert(hxnew<hxtest_object>(2));
	EXPECT_TRUE(t.reset(h0));
	EXPECT_TRUE(t.reset(h1));
	EXPECT_TRUE(t.reset(h2));
	EXPECT_EQ(t.size(), hxsize_t{0});
	const hxhandle_t r0 = t.insert(hxnew<hxtest_object>(10));
	const hxhandle_t r1 = t.insert(hxnew<hxtest_object>(11));
	const hxhandle_t r2 = t.insert(hxnew<hxtest_object>(12));
	EXPECT_EQ(t.size(), hxsize_t{3});
	EXPECT_EQ(t.value(r0)->value(), (int32_t)10);
	EXPECT_EQ(t.value(r1)->value(), (int32_t)11);
	EXPECT_EQ(t.value(r2)->value(), (int32_t)12);
	EXPECT_TRUE(check_stats(6, 3, 0, 6, 0, 0, 0, 0, 0, 0));
}

TEST_F(hxhandle_table_f, hxhandle_table_extract) {
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
	EXPECT_EQ(t.size(), hxsize_t{0});
	EXPECT_EQ(t.value(h), static_cast<hxtest_object*>(hxnull));
	EXPECT_TRUE(check_stats(1, 0, 0, 1, 0, 0, 0, 0, 0, 0));
}

TEST_F(hxhandle_table_f, hxhandle_table_extract_from_full_table_rebuilds_free_head) {
	hxhandle_table<hxtest_object, hxtest_handle_counting_deleter, 2> t;
	t.insert(hxnew<hxtest_object>(10));
	const hxhandle_t h1 = t.insert(hxnew<hxtest_object>(11));
	t.insert(hxnew<hxtest_object>(12));
	hxptr<hxtest_object, hxtest_handle_counting_deleter> p = t.extract(h1);
	ASSERT_NE(p.get(), static_cast<hxtest_object*>(hxnull));
	EXPECT_EQ(p->value(), (int32_t)11);
	EXPECT_EQ(t.size(), hxsize_t{2});
	const hxhandle_t reused = t.insert(hxnew<hxtest_object>(31));
	EXPECT_EQ(t.value(reused)->value(), (int32_t)31);
	EXPECT_TRUE(check_stats(4, 0, 0, 4, 0, 0, 0, 0, 0, 0));
}

TEST_F(hxhandle_table_f, hxhandle_table_erase_if) {
	hxs_handle_table_deleted = 0;
	hxhandle_table<hxtest_object, hxtest_handle_counting_deleter, 2> t;
	const hxhandle_t h0 = t.insert(hxnew<hxtest_object>(0));
	const hxhandle_t h1 = t.insert(hxnew<hxtest_object>(1));
	const hxhandle_t h2 = t.insert(hxnew<hxtest_object>(2));
	EXPECT_EQ(t.erase_if([](hxtest_object& v) { return (v.value() & 1) != 0; }), hxsize_t{1});
	EXPECT_EQ(hxs_handle_table_deleted, 1);
	EXPECT_EQ(t.size(), hxsize_t{2});
	EXPECT_NE(t.value(h0), static_cast<hxtest_object*>(hxnull));
	EXPECT_EQ(t.value(h1), static_cast<hxtest_object*>(hxnull));
	EXPECT_NE(t.value(h2), static_cast<hxtest_object*>(hxnull));
	const hxhandle_t reused = t.insert(hxnew<hxtest_object>(9));
	EXPECT_EQ(t.value(reused)->value(), (int32_t)9);
	EXPECT_TRUE(check_stats(4, 1, 0, 4, 0, 0, 0, 0, 0, 0));
}

TEST_F(hxhandle_table_f, hxhandle_table_erase_if_skips_free_tail_slot) {
	hxs_handle_table_deleted = 0;
	hxhandle_table<hxtest_object, hxtest_handle_counting_deleter, 2> t;
	t.insert(hxnew<hxtest_object>(0));
	EXPECT_EQ(t.erase_if([](hxtest_object&) { return true; }), hxsize_t{1});
	EXPECT_EQ(hxs_handle_table_deleted, 1);
	EXPECT_EQ(t.size(), hxsize_t{0});
	EXPECT_TRUE(check_stats(1, 1, 0, 1, 0, 0, 0, 0, 0, 0));
}

TEST_F(hxhandle_table_f, hxhandle_table_erase_if_none_match) {
	hxs_handle_table_deleted = 0;
	hxhandle_table<hxtest_object, hxtest_handle_counting_deleter, 2> t;
	t.insert(hxnew<hxtest_object>(0));
	t.insert(hxnew<hxtest_object>(1));
	EXPECT_EQ(t.erase_if([](hxtest_object&) { return false; }), hxsize_t{0});
	EXPECT_EQ(hxs_handle_table_deleted, 0);
	EXPECT_EQ(t.size(), hxsize_t{2});
	EXPECT_TRUE(check_stats(2, 0, 0, 2, 0, 0, 0, 0, 0, 0));
}

TEST_F(hxhandle_table_f, hxhandle_table_erase_if_with_disabled_deleter_skips_delete) {
	hxs_handle_table_deleted = 0;
	hxhandle_table<hxtest_object, hxtest_handle_disabled_deleter, 2> t;
	hxtest_object* const raw = hxnew<hxtest_object>(3);
	t.insert(raw);
	EXPECT_EQ(t.erase_if([](hxtest_object&) { return true; }), hxsize_t{1});
	EXPECT_EQ(hxs_handle_table_deleted, 0);
	EXPECT_EQ(t.size(), hxsize_t{0});
	hxdelete(raw);
	EXPECT_TRUE(check_stats(1, 1, 0, 1, 0, 0, 0, 0, 0, 0));
}

TEST_F(hxhandle_table_f, hxhandle_table_clear) {
	hxs_handle_table_deleted = 0;
	hxhandle_table<hxtest_object, hxtest_handle_counting_deleter, 2> t;
	t.clear();
	EXPECT_EQ(hxs_handle_table_deleted, 0);
	EXPECT_EQ(t.size(), hxsize_t{0});
	t.insert(hxnew<hxtest_object>(1));
	t.insert(hxnew<hxtest_object>(2));
	t.clear();
	EXPECT_EQ(hxs_handle_table_deleted, 2);
	EXPECT_EQ(t.size(), hxsize_t{0});
	EXPECT_TRUE(t.empty());
	EXPECT_TRUE(check_stats(2, 2, 0, 2, 0, 0, 0, 0, 0, 0));
}

TEST_F(hxhandle_table_f, hxhandle_table_clear_with_disabled_deleter_skips_delete) {
	hxs_handle_table_deleted = 0;
	hxhandle_table<hxtest_object, hxtest_handle_disabled_deleter, 2> t;
	hxtest_object* const raw0 = hxnew<hxtest_object>(1);
	hxtest_object* const raw1 = hxnew<hxtest_object>(2);
	t.insert(raw0);
	t.insert(raw1);
	t.clear();
	EXPECT_EQ(hxs_handle_table_deleted, 0);
	EXPECT_EQ(t.size(), hxsize_t{0});
	hxdelete(raw0);
	hxdelete(raw1);
	EXPECT_TRUE(check_stats(2, 2, 0, 2, 0, 0, 0, 0, 0, 0));
}

TEST_F(hxhandle_table_f, hxhandle_table_destructor_deletes_owned_values) {
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
	EXPECT_TRUE(check_stats(4, 4, 0, 4, 0, 0, 0, 0, 0, 0));
}

TEST_F(hxhandle_table_f, hxhandle_table_release_all) {
	hxs_handle_table_deleted = 0;
	hxhandle_table<hxtest_object, hxtest_handle_disabled_deleter, 2> t;
	hxtest_object* const raw0 = hxnew<hxtest_object>(1);
	hxtest_object* const raw1 = hxnew<hxtest_object>(2);
	const hxhandle_t h0 = t.insert(raw0);
	t.insert(raw1);
	t.release_all();
	EXPECT_EQ(hxs_handle_table_deleted, 0);
	EXPECT_EQ(t.size(), hxsize_t{0});
	EXPECT_EQ(t.value(h0), static_cast<hxtest_object*>(hxnull));
	hxdelete(raw0);
	hxdelete(raw1);
	hxhandle_table<hxtest_object, hxdo_not_delete> dynamic;
	dynamic.release_all();
	EXPECT_EQ(dynamic.size(), hxsize_t{0});
	EXPECT_EQ(dynamic.capacity(), hxsize_t{0});
	EXPECT_TRUE(check_stats(2, 2, 0, 2, 0, 0, 0, 0, 0, 0));
}

TEST_F(hxhandle_table_f, hxhandle_table_set_size_bits_allocates) {
	hxhandle_table<hxtest_object, hxtest_handle_counting_deleter> t;
	t.set_size_bits(3);
	EXPECT_EQ(t.capacity(), hxsize_t{7});
	const hxhandle_t h = t.insert(hxnew<hxtest_object>(77));
	hxtest_object* const v = t.value(h);
	ASSERT_NE(v, static_cast<hxtest_object*>(hxnull));
	EXPECT_EQ(v->value(), (int32_t)77);
	EXPECT_EQ(t.size(), hxsize_t{1});
	EXPECT_TRUE(t.reset(h));
	EXPECT_TRUE(check_stats(1, 1, 0, 1, 0, 0, 0, 0, 0, 0));
}
