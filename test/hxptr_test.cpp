// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

#include <hx/hxptr.hpp>
#include <hx/hxtest.hpp>

namespace {

// Tracks destructor calls to verify deleter behavior.
int s_hxptr_test_destructor_count = 0;

struct hxtest_ptr_counted_t {
	explicit hxtest_ptr_counted_t(int v) : value(v) { }
	~hxtest_ptr_counted_t(void) { ++s_hxptr_test_destructor_count; }
	int value;
};

// A custom deleter that tracks how many times it is called.
int s_hxtest_custom_deleter_count = 0;

struct hxtest_ptr_custom_deleter_t {
	void operator()(hxtest_ptr_counted_t* ptr) const {
		++s_hxtest_custom_deleter_count;
		hxdelete(ptr);
	}
	operator bool(void) const { return true; }
};

} // namespace


// Default construction yields a null, falsy pointer.
TEST(hxptr_test, default_construction_is_null) {
	const hxptr<int> p;
	EXPECT_EQ(p.get(), (int*)hxnull);
	EXPECT_FALSE((bool)p);
	EXPECT_TRUE(p == hxnullptr);
	EXPECT_FALSE(p != hxnullptr);
}

// Construction from a raw pointer takes ownership and is truthy.
TEST(hxptr_test, construct_from_pointer) {
	const hxsystem_allocator_scope scope(hxsystem_allocator_temporary_stack);
	const hxptr<int> p(hxnew<int>(42));
	EXPECT_TRUE((bool)p);
	EXPECT_NE(p.get(), (int*)hxnull);
	EXPECT_EQ(*p, 42);
	EXPECT_FALSE(p == hxnullptr);
	EXPECT_TRUE(p != hxnullptr);
}

// Destructor calls deleter exactly once for a non-null pointer.
TEST(hxptr_test, destructor_calls_deleter) {
	const hxsystem_allocator_scope scope(hxsystem_allocator_temporary_stack);
	s_hxptr_test_destructor_count = 0;
	{
		const hxptr<hxtest_ptr_counted_t> p(hxnew<hxtest_ptr_counted_t>(1));
	}
	EXPECT_EQ(s_hxptr_test_destructor_count, 1);
}

// Destructor does not call deleter for a null pointer.
TEST(hxptr_test, destructor_null_no_delete) {
	s_hxptr_test_destructor_count = 0;
	{
		const hxptr<hxtest_ptr_counted_t> p;
	}
	EXPECT_EQ(s_hxptr_test_destructor_count, 0);
}

// operator* and operator-> both reach the owned object.
TEST(hxptr_test, deref_and_arrow) {
	const hxsystem_allocator_scope scope(hxsystem_allocator_temporary_stack);
	hxptr<hxtest_ptr_counted_t> p = hxmake_ptr<hxtest_ptr_counted_t>(7);
	EXPECT_EQ((*p).value, 7);
	EXPECT_EQ(p->value, 7);
}

// Move construction transfers ownership; source becomes null without deleting.
TEST(hxptr_test, move_construction_transfers_ownership) {
	const hxsystem_allocator_scope scope(hxsystem_allocator_temporary_stack);
	s_hxptr_test_destructor_count = 0;
	hxptr<hxtest_ptr_counted_t> a(hxnew<hxtest_ptr_counted_t>(3));
	hxtest_ptr_counted_t* raw = a.get();
	const hxptr<hxtest_ptr_counted_t> b(hxmove(a));
	// NOLINT: Intentionally verifying moved-from state.
	EXPECT_EQ(a.get(), (hxtest_ptr_counted_t*)hxnull); // NOLINT
	EXPECT_FALSE((bool)a); // NOLINT
	EXPECT_EQ(b.get(), raw);
	EXPECT_TRUE((bool)b);
	EXPECT_EQ(s_hxptr_test_destructor_count, 0);
}

// Move assignment deletes the old object and transfers ownership; source is null.
TEST(hxptr_test, move_assignment_transfers_ownership) {
	const hxsystem_allocator_scope scope(hxsystem_allocator_temporary_stack);
	s_hxptr_test_destructor_count = 0;
	hxptr<hxtest_ptr_counted_t> a = hxmake_ptr<hxtest_ptr_counted_t>(10);
	hxptr<hxtest_ptr_counted_t> b = hxmake_ptr<hxtest_ptr_counted_t>(20);
	hxtest_ptr_counted_t* raw_a = a.get();
	b = hxmove(a);
	EXPECT_EQ(s_hxptr_test_destructor_count, 1);
	EXPECT_EQ(b.get(), raw_a);
	EXPECT_EQ(a.get(), (hxtest_ptr_counted_t*)hxnull); // NOLINT(clang-analyzer-cplusplus.Move)
}

// Move assignment from null to non-null deletes the owned object.
TEST(hxptr_test, move_assign_from_null_deletes_owned) {
	const hxsystem_allocator_scope scope(hxsystem_allocator_temporary_stack);
	s_hxptr_test_destructor_count = 0;
	hxptr<hxtest_ptr_counted_t> a = hxmake_ptr<hxtest_ptr_counted_t>(5);
	hxptr<hxtest_ptr_counted_t> b;
	a = hxmove(b);
	EXPECT_EQ(s_hxptr_test_destructor_count, 1);
	EXPECT_EQ(a.get(), (hxtest_ptr_counted_t*)hxnull);
}

// release() returns the raw pointer and leaves hxptr null without deleting.
TEST(hxptr_test, release_returns_raw_no_delete) {
	const hxsystem_allocator_scope scope(hxsystem_allocator_temporary_stack);
	s_hxptr_test_destructor_count = 0;
	hxptr<hxtest_ptr_counted_t> p(hxnew<hxtest_ptr_counted_t>(9));
	hxtest_ptr_counted_t* raw = p.release();
	EXPECT_NE(raw, (hxtest_ptr_counted_t*)hxnull);
	EXPECT_EQ(p.get(), (hxtest_ptr_counted_t*)hxnull);
	EXPECT_EQ(s_hxptr_test_destructor_count, 0);
	hxdelete(raw);
}

// reset(new) deletes the old object and takes ownership of the new one.
TEST(hxptr_test, reset_replaces_owned_object) {
	const hxsystem_allocator_scope scope(hxsystem_allocator_temporary_stack);
	s_hxptr_test_destructor_count = 0;
	hxptr<hxtest_ptr_counted_t> p = hxmake_ptr<hxtest_ptr_counted_t>(1);
	hxtest_ptr_counted_t* second = hxnew<hxtest_ptr_counted_t>(2);
	p.reset(second);
	EXPECT_EQ(s_hxptr_test_destructor_count, 1);
	EXPECT_EQ(p.get(), second);
	EXPECT_EQ(p->value, 2);
}

// reset() with no argument deletes the owned object and leaves hxptr null.
TEST(hxptr_test, reset_null_deletes_owned) {
	const hxsystem_allocator_scope scope(hxsystem_allocator_temporary_stack);
	s_hxptr_test_destructor_count = 0;
	hxptr<hxtest_ptr_counted_t> p = hxmake_ptr<hxtest_ptr_counted_t>(1);
	p.reset();
	EXPECT_EQ(s_hxptr_test_destructor_count, 1);
	EXPECT_EQ(p.get(), (hxtest_ptr_counted_t*)hxnull);
	EXPECT_FALSE((bool)p);
}

// reset() on a null hxptr is a no-op.
TEST(hxptr_test, reset_null_on_empty_is_noop) {
	s_hxptr_test_destructor_count = 0;
	hxptr<hxtest_ptr_counted_t> p;
	p.reset();
	EXPECT_EQ(s_hxptr_test_destructor_count, 0);
	EXPECT_EQ(p.get(), (hxtest_ptr_counted_t*)hxnull);
}

// operator== and operator!= compare two hxptrs by address.
TEST(hxptr_test, equality_operators_compare_address) {
	const hxsystem_allocator_scope scope(hxsystem_allocator_temporary_stack);
	const hxptr<hxtest_ptr_counted_t> a = hxmake_ptr<hxtest_ptr_counted_t>(0);
	const hxptr<hxtest_ptr_counted_t> b;
	// Non-null vs null: not equal.
	EXPECT_FALSE(a == b);
	EXPECT_TRUE(a != b);
	// Two nulls: equal.
	const hxptr<hxtest_ptr_counted_t> c;
	EXPECT_TRUE(b == c);
	EXPECT_FALSE(b != c);
}

// Custom deleter is invoked on destruction.
TEST(hxptr_test, custom_deleter_called_on_destruction) {
	const hxsystem_allocator_scope scope(hxsystem_allocator_temporary_stack);
	s_hxptr_test_destructor_count = 0;
	s_hxtest_custom_deleter_count = 0;
	{
		const hxptr<hxtest_ptr_counted_t, hxtest_ptr_custom_deleter_t> p(
			hxnew<hxtest_ptr_counted_t>(55));
	}
	EXPECT_EQ(s_hxtest_custom_deleter_count, 1);
	EXPECT_EQ(s_hxptr_test_destructor_count, 1);
}

// operator[] indexes relative to the owned pointer.
TEST(hxptr_test, index_operator_reads_elements) {
	const hxsystem_allocator_scope scope(hxsystem_allocator_temporary_stack);
	int* arr = static_cast<int*>(hxmalloc_ext(3 * sizeof(int),
		hxsystem_allocator_temporary_stack, HX_ALIGNMENT));
	arr[0] = 10;
	arr[1] = 20;
	arr[2] = 30;
	const hxptr<int> p(arr);
	EXPECT_EQ(p[0], 10);
	EXPECT_EQ(p[1], 20);
	EXPECT_EQ(p[2], 30);
	// operator[] at index 0 matches operator*.
	EXPECT_EQ(p[0], *p);
}

// swap exchanges ownership without deleting either object.
TEST(hxptr_test, swap_exchanges_ownership) {
	const hxsystem_allocator_scope scope(hxsystem_allocator_temporary_stack);
	s_hxptr_test_destructor_count = 0;
	hxptr<hxtest_ptr_counted_t> a = hxmake_ptr<hxtest_ptr_counted_t>(1);
	hxptr<hxtest_ptr_counted_t> b = hxmake_ptr<hxtest_ptr_counted_t>(2);
	hxtest_ptr_counted_t* raw_a = a.get();
	hxtest_ptr_counted_t* raw_b = b.get();
	a.swap(b);
	EXPECT_EQ(s_hxptr_test_destructor_count, 0);
	EXPECT_EQ(a.get(), raw_b);
	EXPECT_EQ(b.get(), raw_a);
	EXPECT_EQ(a->value, 2);
	EXPECT_EQ(b->value, 1);
}

// swap with a null hxptr transfers ownership to the null side.
TEST(hxptr_test, swap_with_null) {
	const hxsystem_allocator_scope scope(hxsystem_allocator_temporary_stack);
	s_hxptr_test_destructor_count = 0;
	hxptr<hxtest_ptr_counted_t> a = hxmake_ptr<hxtest_ptr_counted_t>(7);
	hxptr<hxtest_ptr_counted_t> b;
	hxtest_ptr_counted_t* raw_a = a.get();
	a.swap(b);
	EXPECT_EQ(s_hxptr_test_destructor_count, 0);
	EXPECT_EQ(a.get(), (hxtest_ptr_counted_t*)hxnull);
	EXPECT_EQ(b.get(), raw_a);
}

// hxmake_ptr constructs the object and returns a non-null owning hxptr.
TEST(hxptr_test, make_ptr_constructs_and_owns) {
	const hxsystem_allocator_scope scope(hxsystem_allocator_temporary_stack);
	s_hxptr_test_destructor_count = 0;
	{
		const hxptr<hxtest_ptr_counted_t> p =
			hxmake_ptr<hxtest_ptr_counted_t>(42);
		EXPECT_TRUE((bool)p);
		EXPECT_EQ(p->value, 42);
		EXPECT_EQ(s_hxptr_test_destructor_count, 0);
	}
	EXPECT_EQ(s_hxptr_test_destructor_count, 1);
}

// Custom deleter is invoked on reset; second reset on null is a no-op.
TEST(hxptr_test, custom_deleter_called_on_reset) {
	const hxsystem_allocator_scope scope(hxsystem_allocator_temporary_stack);
	s_hxptr_test_destructor_count = 0;
	s_hxtest_custom_deleter_count = 0;
	hxptr<hxtest_ptr_counted_t, hxtest_ptr_custom_deleter_t> p(
		hxnew<hxtest_ptr_counted_t>(6));
	p.reset();
	EXPECT_EQ(s_hxtest_custom_deleter_count, 1);
	EXPECT_EQ(s_hxptr_test_destructor_count, 1);
	p.reset();
	EXPECT_EQ(s_hxtest_custom_deleter_count, 1);
}
