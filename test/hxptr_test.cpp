// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

#include <hx/hxptr.hpp>
#include <hx/hxtest.hpp>

HX_NS_USE

namespace {

int hxs_ptr_test_destructor_count = 0;

struct hxtest_ptr_counted_t {
	explicit hxtest_ptr_counted_t(int v) : value(v) { }
	~hxtest_ptr_counted_t(void) { ++hxs_ptr_test_destructor_count; }
	int value;
};
int hxs_test_custom_deleter_count = 0;
struct hxtest_ptr_custom_deleter_t {
	void operator()(hxtest_ptr_counted_t* ptr) const {
		++hxs_test_custom_deleter_count;
		hxdelete(ptr);
	}
	operator bool(void) const = delete;
};
} // namespace

TEST(hxptr_test, default_construction_is_null) {
	const hxptr<int> p;
	EXPECT_EQ(p.get(), static_cast<int*>(hxnull));
	EXPECT_FALSE((bool)p);
	EXPECT_TRUE(p == hxnullptr);
	EXPECT_FALSE(p != hxnullptr);
}

TEST(hxptr_test, construct_from_pointer) {
	const hxsystem_allocator_scope scope(hxsystem_allocator_stack_0);
	const hxptr<int> p(hxnew<int>(34));
	EXPECT_TRUE((bool)p);
	EXPECT_NE(p.get(), static_cast<int*>(hxnull));
	EXPECT_EQ(*p, 34);
	EXPECT_FALSE(p == hxnullptr);
	EXPECT_TRUE(p != hxnullptr);
}

TEST(hxptr_test, destructor_calls_deleter) {
	const hxsystem_allocator_scope scope(hxsystem_allocator_stack_0);
	hxs_ptr_test_destructor_count = 0;
	{
		const hxptr<hxtest_ptr_counted_t> p(hxnew<hxtest_ptr_counted_t>(1));
	}
	EXPECT_EQ(hxs_ptr_test_destructor_count, 1);
}

TEST(hxptr_test, destructor_null_no_delete) {
	hxs_ptr_test_destructor_count = 0;
	{
		const hxptr<hxtest_ptr_counted_t> p;
	}
	EXPECT_EQ(hxs_ptr_test_destructor_count, 0);
}

TEST(hxptr_test, deref_and_arrow) {
	const hxsystem_allocator_scope scope(hxsystem_allocator_stack_0);
	hxptr<hxtest_ptr_counted_t> p = hxmake_ptr<hxtest_ptr_counted_t>(7);
	EXPECT_EQ((*p).value, 7);
	EXPECT_EQ(p->value, 7);
}

TEST(hxptr_test, move_construction_transfers_ownership) {
	const hxsystem_allocator_scope scope(hxsystem_allocator_stack_0);
	hxs_ptr_test_destructor_count = 0;
	hxptr<hxtest_ptr_counted_t> a(hxnew<hxtest_ptr_counted_t>(3));
	const hxtest_ptr_counted_t* const raw = a.get();
	const hxptr<hxtest_ptr_counted_t> b(hxmove(a));
	EXPECT_EQ(a.get(), (hxtest_ptr_counted_t*)hxnull);
	EXPECT_FALSE((bool)a);
	EXPECT_EQ(b.get(), raw);
	EXPECT_TRUE((bool)b);
	EXPECT_EQ(hxs_ptr_test_destructor_count, 0);
}

TEST(hxptr_test, move_assignment_transfers_ownership) {
	const hxsystem_allocator_scope scope(hxsystem_allocator_stack_0);
	hxs_ptr_test_destructor_count = 0;
	hxptr<hxtest_ptr_counted_t> a = hxmake_ptr<hxtest_ptr_counted_t>(10);
	hxptr<hxtest_ptr_counted_t> b = hxmake_ptr<hxtest_ptr_counted_t>(20);
	const hxtest_ptr_counted_t* const raw_a = a.get();
	b = hxmove(a);
	EXPECT_EQ(hxs_ptr_test_destructor_count, 1);
	EXPECT_EQ(b.get(), raw_a);
	EXPECT_EQ(a.get(), (hxtest_ptr_counted_t*)hxnull);
}

TEST(hxptr_test, move_assign_from_null_deletes_owned) {
	const hxsystem_allocator_scope scope(hxsystem_allocator_stack_0);
	hxs_ptr_test_destructor_count = 0;
	hxptr<hxtest_ptr_counted_t> a = hxmake_ptr<hxtest_ptr_counted_t>(5);
	hxptr<hxtest_ptr_counted_t> b;
	a = hxmove(b);
	EXPECT_EQ(hxs_ptr_test_destructor_count, 1);
	EXPECT_EQ(a.get(), (hxtest_ptr_counted_t*)hxnull);
}

TEST(hxptr_test, release_returns_raw_no_delete) {
	const hxsystem_allocator_scope scope(hxsystem_allocator_stack_0);
	hxs_ptr_test_destructor_count = 0;
	hxptr<hxtest_ptr_counted_t> p(hxnew<hxtest_ptr_counted_t>(9));
	hxtest_ptr_counted_t* raw = p.release();
	EXPECT_NE(raw, (hxtest_ptr_counted_t*)hxnull);
	EXPECT_EQ(p.get(), (hxtest_ptr_counted_t*)hxnull);
	EXPECT_EQ(hxs_ptr_test_destructor_count, 0);
	hxdelete(raw);
}

TEST(hxptr_test, reset_replaces_owned_object) {
	const hxsystem_allocator_scope scope(hxsystem_allocator_stack_0);
	hxs_ptr_test_destructor_count = 0;
	hxptr<hxtest_ptr_counted_t> p = hxmake_ptr<hxtest_ptr_counted_t>(1);
	hxtest_ptr_counted_t* second = hxnew<hxtest_ptr_counted_t>(2);
	p.reset(second);
	EXPECT_EQ(hxs_ptr_test_destructor_count, 1);
	EXPECT_EQ(p.get(), second);
	EXPECT_EQ(p->value, 2);
}

TEST(hxptr_test, reset_null_deletes_owned) {
	const hxsystem_allocator_scope scope(hxsystem_allocator_stack_0);
	hxs_ptr_test_destructor_count = 0;
	hxptr<hxtest_ptr_counted_t> p = hxmake_ptr<hxtest_ptr_counted_t>(1);
	p.reset();
	EXPECT_EQ(hxs_ptr_test_destructor_count, 1);
	EXPECT_EQ(p.get(), (hxtest_ptr_counted_t*)hxnull);
	EXPECT_FALSE((bool)p);
}

TEST(hxptr_test, reset_null_on_empty_is_noop) {
	hxs_ptr_test_destructor_count = 0;
	hxptr<hxtest_ptr_counted_t> p;
	p.reset();
	EXPECT_EQ(hxs_ptr_test_destructor_count, 0);
	EXPECT_EQ(p.get(), (hxtest_ptr_counted_t*)hxnull);
}

TEST(hxptr_test, equality_operators_compare_address) {
	const hxsystem_allocator_scope scope(hxsystem_allocator_stack_0);
	const hxptr<hxtest_ptr_counted_t> a = hxmake_ptr<hxtest_ptr_counted_t>(0);
	const hxptr<hxtest_ptr_counted_t> b;
	EXPECT_FALSE(a == b);
	EXPECT_TRUE(a != b);
	const hxptr<hxtest_ptr_counted_t> c;
	EXPECT_TRUE(b == c);
	EXPECT_FALSE(b != c);
}

TEST(hxptr_test, not_equal_nullptr_distinguishes_null_and_owned) {
	const hxsystem_allocator_scope scope(hxsystem_allocator_stack_0);
	const hxptr<hxtest_ptr_counted_t> empty;
	EXPECT_FALSE(empty != hxnullptr);
	const hxptr<hxtest_ptr_counted_t> owned = hxmake_ptr<hxtest_ptr_counted_t>(0);
	EXPECT_TRUE(owned != hxnullptr);
}

TEST(hxptr_test, custom_deleter_called_on_destruction) {
	const hxsystem_allocator_scope scope(hxsystem_allocator_stack_0);
	hxs_ptr_test_destructor_count = 0;
	hxs_test_custom_deleter_count = 0;
	{
		const hxptr<hxtest_ptr_counted_t, hxtest_ptr_custom_deleter_t> p(
			hxnew<hxtest_ptr_counted_t>(55));
	}
	EXPECT_EQ(hxs_test_custom_deleter_count, 1);
	EXPECT_EQ(hxs_ptr_test_destructor_count, 1);
}

TEST(hxptr_test, swap_exchanges_ownership) {
	const hxsystem_allocator_scope scope(hxsystem_allocator_stack_0);
	hxs_ptr_test_destructor_count = 0;
	hxptr<hxtest_ptr_counted_t> a = hxmake_ptr<hxtest_ptr_counted_t>(1);
	hxptr<hxtest_ptr_counted_t> b = hxmake_ptr<hxtest_ptr_counted_t>(2);
	const hxtest_ptr_counted_t* const raw_a = a.get();
	const hxtest_ptr_counted_t* const raw_b = b.get();
	a.swap(b);
	EXPECT_EQ(hxs_ptr_test_destructor_count, 0);
	EXPECT_EQ(a.get(), raw_b);
	EXPECT_EQ(b.get(), raw_a);
	EXPECT_EQ(a->value, 2);
	EXPECT_EQ(b->value, 1);
}

TEST(hxptr_test, swap_with_null) {
	const hxsystem_allocator_scope scope(hxsystem_allocator_stack_0);
	hxs_ptr_test_destructor_count = 0;
	hxptr<hxtest_ptr_counted_t> a = hxmake_ptr<hxtest_ptr_counted_t>(7);
	hxptr<hxtest_ptr_counted_t> b;
	const hxtest_ptr_counted_t* const raw_a = a.get();
	a.swap(b);
	EXPECT_EQ(hxs_ptr_test_destructor_count, 0);
	EXPECT_EQ(a.get(), (hxtest_ptr_counted_t*)hxnull);
	EXPECT_EQ(b.get(), raw_a);
}

TEST(hxptr_test, make_ptr_constructs_and_owns) {
	const hxsystem_allocator_scope scope(hxsystem_allocator_stack_0);
	hxs_ptr_test_destructor_count = 0;
	{
		const hxptr<hxtest_ptr_counted_t> p =
			hxmake_ptr<hxtest_ptr_counted_t>(34);
		EXPECT_TRUE((bool)p);
		EXPECT_EQ(p->value, 34);
		EXPECT_EQ(hxs_ptr_test_destructor_count, 0);
	}
	EXPECT_EQ(hxs_ptr_test_destructor_count, 1);
}

TEST(hxptr_test, custom_deleter_called_on_reset) {
	const hxsystem_allocator_scope scope(hxsystem_allocator_stack_0);
	hxs_ptr_test_destructor_count = 0;
	hxs_test_custom_deleter_count = 0;
	hxptr<hxtest_ptr_counted_t, hxtest_ptr_custom_deleter_t> p(
		hxnew<hxtest_ptr_counted_t>(6));
	p.reset();
	EXPECT_EQ(hxs_test_custom_deleter_count, 1);
	EXPECT_EQ(hxs_ptr_test_destructor_count, 1);
	p.reset();
	EXPECT_EQ(hxs_test_custom_deleter_count, 1);
}

TEST(hxptr_test, destructor_exactly_one_call) {
	const hxsystem_allocator_scope scope(hxsystem_allocator_stack_0);
	hxs_ptr_test_destructor_count = 0;
	{
		const hxptr<hxtest_ptr_counted_t> p(hxnew<hxtest_ptr_counted_t>(1));
		EXPECT_EQ(hxs_ptr_test_destructor_count, 0);
	}
	EXPECT_EQ(hxs_ptr_test_destructor_count, 1);
}

TEST(hxptr_test, equality_different_address_not_equal) {
	const hxsystem_allocator_scope scope(hxsystem_allocator_stack_0);
	const hxptr<hxtest_ptr_counted_t> a = hxmake_ptr<hxtest_ptr_counted_t>(5);
	const hxptr<hxtest_ptr_counted_t> b = hxmake_ptr<hxtest_ptr_counted_t>(5);
	EXPECT_FALSE(a == b);
	EXPECT_TRUE(a != b);
}

TEST(hxptr_test, release_returns_exact_pointer) {
	const hxsystem_allocator_scope scope(hxsystem_allocator_stack_0);
	hxtest_ptr_counted_t* raw = hxnew<hxtest_ptr_counted_t>(77);
	hxptr<hxtest_ptr_counted_t> p(raw);
	hxtest_ptr_counted_t* released = p.release();
	EXPECT_EQ(released, raw);
	EXPECT_EQ(released->value, 77);
	hxdelete(released);
}

TEST(hxptr_test, reset_old_deleted_exactly_once) {
	const hxsystem_allocator_scope scope(hxsystem_allocator_stack_0);
	hxs_ptr_test_destructor_count = 0;
	hxptr<hxtest_ptr_counted_t> p = hxmake_ptr<hxtest_ptr_counted_t>(3);
	hxtest_ptr_counted_t* second = hxnew<hxtest_ptr_counted_t>(4);
	p.reset(second);
	EXPECT_EQ(hxs_ptr_test_destructor_count, 1);
	EXPECT_EQ(p->value, 4);
}

TEST(hxptr_test, move_construction_source_is_exactly_null) {
	const hxsystem_allocator_scope scope(hxsystem_allocator_stack_0);
	hxptr<hxtest_ptr_counted_t> a = hxmake_ptr<hxtest_ptr_counted_t>(9);
	const hxptr<hxtest_ptr_counted_t> b(hxmove(a));
	EXPECT_EQ(a.get(), static_cast<hxtest_ptr_counted_t*>(hxnull));
	EXPECT_TRUE((bool)b);
}

TEST(hxptr_test, swap_holds_exact_addresses) {
	const hxsystem_allocator_scope scope(hxsystem_allocator_stack_0);
	hxptr<hxtest_ptr_counted_t> a = hxmake_ptr<hxtest_ptr_counted_t>(11);
	hxptr<hxtest_ptr_counted_t> b = hxmake_ptr<hxtest_ptr_counted_t>(22);
	hxtest_ptr_counted_t* const addr_a = a.get();
	hxtest_ptr_counted_t* const addr_b = b.get();
	a.swap(b);
	EXPECT_EQ(a.get(), addr_b);
	EXPECT_EQ(b.get(), addr_a);
}
