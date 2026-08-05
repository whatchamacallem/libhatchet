// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

#include <hx/hxptr.hpp>
#include "./hxtest_util.hpp"

HX_NS_USE

#if !defined _MSC_VER && !defined __wasm__
static_assert(sizeof(size_t) != 4 || (
		sizeof(hxptr<int>) == 4u),
	"hxptr with the default deleter must be exactly pointer sized due to"
	" empty base optimization");
static_assert(sizeof(size_t) != 8 || (
		sizeof(hxptr<int>) == 8u),
	"hxptr with the default deleter must be exactly pointer sized due to"
	" empty base optimization");
#endif

namespace {
using hxptr_test_f = hxtest_object_fixture;

int hxs_ptr_stateful_delete_count = 0;
int hxs_ptr_stateful_delete_tag = 0;
struct hxtest_ptr_stateful_deleter {
	explicit hxtest_ptr_stateful_deleter(int tag) : m_tag(tag) { }
	void operator()(hxtest_object* ptr) const {
		++hxs_ptr_stateful_delete_count;
		hxs_ptr_stateful_delete_tag = m_tag;
		hxdelete(ptr);
	}
	operator bool(void) const = delete;
	int m_tag;
};
} // namespace

TEST_F(hxptr_test_f, hxptr_construction_and_destruction) {
	{
		const hxptr<hxtest_object> null_ptr;
		EXPECT_EQ(null_ptr.get(), static_cast<hxtest_object*>(hxnull));
		EXPECT_FALSE((bool)null_ptr);
	}
	EXPECT_TRUE(check_stats(0, 0, 0, 0, 0, 0, 0, 0));
	{
		const hxptr<hxtest_object> owned(hxnew<hxtest_object>(34));
		EXPECT_TRUE((bool)owned);
		EXPECT_EQ(owned->value, 34);
	}
	EXPECT_TRUE(check_stats(1, 1, 0, 0, 0, 0, 0, 0));
	{
		const hxptr<hxtest_object, hxtest_ptr_stateful_deleter> with_deleter(
			hxnew<hxtest_object>(1), hxtest_ptr_stateful_deleter(99));
		EXPECT_EQ(hxs_ptr_stateful_delete_count, 0);
	}
	EXPECT_EQ(hxs_ptr_stateful_delete_count, 1);
	EXPECT_EQ(hxs_ptr_stateful_delete_tag, 99);
	EXPECT_TRUE(check_stats(2, 2, 0, 0, 0, 0, 0, 0));
}

TEST_F(hxptr_test_f, hxptr_move_construction) {
	hxptr<hxtest_object> a(hxnew<hxtest_object>(3));
	const hxtest_object* const raw = a.get();
	const hxptr<hxtest_object> b(hxmove(a));
	EXPECT_EQ(a.get(), static_cast<hxtest_object*>(hxnull));
	EXPECT_FALSE((bool)a);
	EXPECT_EQ(b.get(), raw);
	EXPECT_TRUE((bool)b);
	EXPECT_TRUE(check_stats(1, 0, 0, 0, 0, 0, 0, 0));
}

TEST_F(hxptr_test_f, hxptr_operator_assign_move) {
	hxptr<hxtest_object> a(hxnew<hxtest_object>(10));
	hxptr<hxtest_object> b(hxnew<hxtest_object>(20));
	const hxtest_object* const raw_a = a.get();
	b = hxmove(a);
	EXPECT_TRUE(check_stats(2, 1, 0, 0, 0, 0, 0, 0));
	EXPECT_EQ(b.get(), raw_a);
	EXPECT_EQ(a.get(), static_cast<hxtest_object*>(hxnull));
	hxptr<hxtest_object> c;
	b = hxmove(c);
	EXPECT_TRUE(check_stats(2, 2, 0, 0, 0, 0, 0, 0));
	EXPECT_EQ(b.get(), static_cast<hxtest_object*>(hxnull));
	hxptr<hxtest_object> d(hxnew<hxtest_object>(30));
	const hxtest_object* const raw_d = d.get();
	b = hxmove(d);
	EXPECT_TRUE(check_stats(3, 2, 0, 0, 0, 0, 0, 0));
	EXPECT_EQ(b.get(), raw_d);
	EXPECT_EQ(d.get(), static_cast<hxtest_object*>(hxnull));
	EXPECT_TRUE(check_stats(3, 2, 0, 0, 0, 0, 0, 0));
}

TEST_F(hxptr_test_f, hxptr_operator_deref_and_arrow) {
	const hxptr<hxtest_object> p(hxnew<hxtest_object>(7));
	EXPECT_EQ((*p).value, 7);
	EXPECT_EQ(p->value, 7);
	EXPECT_TRUE(check_stats(1, 0, 0, 0, 0, 0, 0, 0));
}

TEST_F(hxptr_test_f, hxptr_operator_bool) {
	const hxptr<hxtest_object> empty;
	EXPECT_FALSE((bool)empty);
	const hxptr<hxtest_object> owned(hxnew<hxtest_object>(1));
	EXPECT_TRUE((bool)owned);
	EXPECT_TRUE(check_stats(1, 0, 0, 0, 0, 0, 0, 0));
}

TEST_F(hxptr_test_f, hxptr_operator_equal_hxptr) {
	const hxptr<hxtest_object> a(hxnew<hxtest_object>(0));
	const hxptr<hxtest_object> b;
	const hxptr<hxtest_object> c;
	EXPECT_FALSE(a == b);
	EXPECT_TRUE(a != b);
	EXPECT_TRUE(b == c);
	EXPECT_FALSE(b != c);
	const hxptr<hxtest_object> d(hxnew<hxtest_object>(5));
	const hxptr<hxtest_object> e(hxnew<hxtest_object>(5));
	EXPECT_FALSE(d == e);
	EXPECT_TRUE(d != e);
	EXPECT_TRUE(check_stats(3, 0, 0, 0, 0, 0, 0, 0));
}

TEST_F(hxptr_test_f, hxptr_operator_equal_nullptr) {
	const hxptr<hxtest_object> empty;
	EXPECT_TRUE(empty == hxnullptr);
	EXPECT_FALSE(empty != hxnullptr);
	const hxptr<hxtest_object> owned(hxnew<hxtest_object>(0));
	EXPECT_FALSE(owned == hxnullptr);
	EXPECT_TRUE(owned != hxnullptr);
	EXPECT_TRUE(check_stats(1, 0, 0, 0, 0, 0, 0, 0));
}

TEST_F(hxptr_test_f, hxptr_and_then) {
	const hxptr<hxtest_object> engaged(hxnew<hxtest_object>(3));
	const hxptr<hxtest_object> mapped = engaged.and_then([](hxtest_object& v) {
		return hxptr<hxtest_object>(hxnew<hxtest_object>(v.value + 1));
	});
	EXPECT_TRUE((bool)mapped);
	EXPECT_EQ(mapped->value, 4);
	EXPECT_FALSE((bool)engaged.and_then([](hxtest_object&) { return hxptr<hxtest_object>(); }));
	const hxptr<hxtest_object> empty;
	bool called = false;
	const hxptr<hxtest_object> from_null = empty.and_then([&called](hxtest_object& v) {
		// GCOVR_EXCL_START
		called = true; return hxptr<hxtest_object>(hxnew<hxtest_object>(v.value));
		// GCOVR_EXCL_STOP
	});
	EXPECT_FALSE((bool)from_null);
	EXPECT_FALSE(called);
	EXPECT_TRUE(check_stats(2, 0, 0, 0, 0, 0, 0, 0));
}

TEST_F(hxptr_test_f, hxptr_get) {
	const hxptr<hxtest_object> empty;
	EXPECT_EQ(empty.get(), static_cast<hxtest_object*>(hxnull));
	hxtest_object* const raw = hxnew<hxtest_object>(9);
	const hxptr<hxtest_object> owned(raw);
	EXPECT_EQ(owned.get(), raw);
	EXPECT_TRUE(check_stats(1, 0, 0, 0, 0, 0, 0, 0));
}

TEST_F(hxptr_test_f, hxptr_get_deleter) {
	hxptr<hxtest_object, hxtest_ptr_stateful_deleter> p(
		hxnew<hxtest_object>(1), hxtest_ptr_stateful_deleter(5));
	EXPECT_EQ(p.deleter().m_tag, 5);
	p.deleter().m_tag = 6;
	const hxptr<hxtest_object, hxtest_ptr_stateful_deleter>& const_ref = p;
	EXPECT_EQ(const_ref.deleter().m_tag, 6);
	EXPECT_TRUE(check_stats(1, 0, 0, 0, 0, 0, 0, 0));
}

TEST_F(hxptr_test_f, hxptr_or_else) {
	hxptr<hxtest_object> engaged(hxnew<hxtest_object>(8));
	const hxtest_object* const raw = engaged.get();
	bool called_engaged = false;
	const hxptr<hxtest_object> from_engaged = hxmove(engaged).or_else([&called_engaged]() {
		// GCOVR_EXCL_START
		called_engaged = true; return hxptr<hxtest_object>();
		// GCOVR_EXCL_STOP
	});
	EXPECT_FALSE(called_engaged);
	EXPECT_EQ(from_engaged.get(), raw);
	EXPECT_EQ(engaged.get(), static_cast<hxtest_object*>(hxnull));

	hxptr<hxtest_object> empty;
	bool called_empty = false;
	const hxptr<hxtest_object> from_empty = hxmove(empty).or_else([&called_empty]() {
		called_empty = true; return hxptr<hxtest_object>(hxnew<hxtest_object>(42));
	});
	EXPECT_TRUE(called_empty);
	EXPECT_TRUE((bool)from_empty);
	EXPECT_EQ(from_empty->value, 42);
	EXPECT_TRUE(check_stats(2, 0, 0, 0, 0, 0, 0, 0));
}

TEST_F(hxptr_test_f, hxptr_release) {
	hxtest_object* const raw = hxnew<hxtest_object>(77);
	hxptr<hxtest_object> p(raw);
	hxtest_object* const released = p.release();
	EXPECT_EQ(released, raw);
	EXPECT_EQ(released->value, 77);
	EXPECT_EQ(p.get(), static_cast<hxtest_object*>(hxnull));
	EXPECT_TRUE(check_stats(1, 0, 0, 0, 0, 0, 0, 0));
	hxdelete(released);
	EXPECT_TRUE(check_stats(1, 1, 0, 0, 0, 0, 0, 0));
}

TEST_F(hxptr_test_f, hxptr_reset) {
	hxptr<hxtest_object> p(hxnew<hxtest_object>(1));
	hxtest_object* const second = hxnew<hxtest_object>(2);
	p.reset(second);
	EXPECT_TRUE(check_stats(2, 1, 0, 0, 0, 0, 0, 0));
	EXPECT_EQ(p.get(), second);
	EXPECT_EQ(p->value, 2);
	p.reset();
	EXPECT_TRUE(check_stats(2, 2, 0, 0, 0, 0, 0, 0));
	EXPECT_EQ(p.get(), static_cast<hxtest_object*>(hxnull));
	p.reset();
	EXPECT_TRUE(check_stats(2, 2, 0, 0, 0, 0, 0, 0));
}

TEST_F(hxptr_test_f, hxptr_swap) {
	hxptr<hxtest_object> a(hxnew<hxtest_object>(1));
	hxptr<hxtest_object> b(hxnew<hxtest_object>(2));
	const hxtest_object* const raw_a = a.get();
	const hxtest_object* const raw_b = b.get();
	a.swap(b);
	EXPECT_TRUE(check_stats(2, 0, 0, 0, 0, 0, 0, 0));
	EXPECT_EQ(a.get(), raw_b);
	EXPECT_EQ(b.get(), raw_a);
	EXPECT_EQ(a->value, 2);
	EXPECT_EQ(b->value, 1);

	hxptr<hxtest_object> c(hxnew<hxtest_object>(7));
	hxptr<hxtest_object> d;
	const hxtest_object* const raw_c = c.get();
	c.swap(d);
	EXPECT_EQ(c.get(), static_cast<hxtest_object*>(hxnull));
	EXPECT_EQ(d.get(), raw_c);
	EXPECT_TRUE(check_stats(3, 0, 0, 0, 0, 0, 0, 0));
}

TEST_F(hxptr_test_f, hxptr_value_or) {
	const hxptr<hxtest_object> engaged(hxnew<hxtest_object>(3));
	EXPECT_EQ(engaged.value_or(hxtest_object(99)).value, 3);
	const hxptr<hxtest_object> empty;
	EXPECT_EQ(empty.value_or(hxtest_object(7)).value, 7);
	EXPECT_TRUE(check_stats(5, 4, 1, 1, 0, 0, 0, 0));
}

TEST_F(hxptr_test_f, hxmake_ptr) {
	const hxptr<hxtest_object> p = hxmake_ptr<hxtest_object>(34);
	EXPECT_TRUE((bool)p);
	EXPECT_EQ(p->value, 34);
	EXPECT_TRUE(check_stats(1, 0, 0, 0, 0, 0, 0, 0));
}
