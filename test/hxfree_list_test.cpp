// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

#include <hx/hxfree_list.hpp>
#include <hx/hxhash_table.hpp>
#include <hx/hxhash_table_nodes.hpp>
#include <hx/hxlist.hpp>
#include "./hxtest_util.hpp"

HX_NS_USE

#if !defined _MSC_VER && !defined __wasm__
static_assert(sizeof(size_t) != 4 || (
		sizeof(hxfree_list<hxtest_object>) == 16u),
	"hxfree_list with dynamic capacity must pack its data pointer, capacity,"
	" free head pointer and size with no padding");
static_assert(sizeof(size_t) != 8 || (
		sizeof(hxfree_list<hxtest_object>) == 32u),
	"hxfree_list with dynamic capacity must pack its data pointer, capacity,"
	" free head pointer and size with no padding");
#endif

namespace {
using hxfree_list_test_f = hxtest_object_fixture;

class hxtest_free_list_node : public hxlist_node {
public:
	explicit hxtest_free_list_node(int32_t v) : obj(v) { }
	hxtest_object obj;
};

using hxtest_node_pool_t = hxfree_list<hxtest_free_list_node, 4>;
using hxtest_node_list_t = hxlist<hxtest_free_list_node, hxtest_node_pool_t::deleter_t>;

class hxtest_hash_node : public hxhash_table_set_node<int32_t> {
public:
	explicit hxtest_hash_node(int32_t k) : hxhash_table_set_node<int32_t>(k), obj(k) { }
	hxtest_object obj;
};

using hxtest_hash_pool_t = hxfree_list<hxtest_hash_node, 8>;
using hxtest_hash_table_t = hxhash_table<hxtest_hash_node, hxtest_hash_pool_t::deleter_t, false, 2>;

} // namespace

TEST_F(hxfree_list_test_f, construct) {
	const hxfree_list<hxtest_object, 4> fixed_pool;
	EXPECT_EQ(fixed_pool.size(), hxsize_t{4});
	EXPECT_EQ(fixed_pool.capacity(), hxsize_t{4});
	EXPECT_EQ(fixed_pool.max_size(), hxsize_t{4});
	EXPECT_FALSE(fixed_pool.empty());
	const hxfree_list<hxtest_object> dynamic_pool;
	EXPECT_EQ(dynamic_pool.size(), hxsize_t{0});
	EXPECT_EQ(dynamic_pool.capacity(), hxsize_t{0});
	EXPECT_TRUE(dynamic_pool.empty());
	EXPECT_TRUE(check_no_stats());
}

TEST_F(hxfree_list_test_f, allocate) {
	hxfree_list<hxtest_object, 2> pool;
	hxfree_list<hxtest_object, 2>::ptr_t a = pool.allocate(7);
	EXPECT_TRUE((bool)a);
	EXPECT_NE(a.get(), static_cast<hxtest_object*>(hxnull));
	EXPECT_EQ(a->value(), (int32_t)7);
	EXPECT_EQ(pool.size(), hxsize_t{1});
	hxfree_list<hxtest_object, 2>::ptr_t b = pool.allocate(34);
	EXPECT_EQ(b->value(), (int32_t)34);
	EXPECT_EQ(pool.size(), hxsize_t{0});
	EXPECT_TRUE(check_stats(2, 0, 0, 2, 0, 0, 0, 0, 0, 0));
}

TEST_F(hxfree_list_test_f, try_allocate) {
	hxfree_list<hxtest_object, 1> pool;
	EXPECT_EQ(pool.size(), hxsize_t{1});
	hxfree_list<hxtest_object, 1>::ptr_t a = pool.try_allocate(5);
	EXPECT_TRUE((bool)a);
	EXPECT_EQ(a->value(), (int32_t)5);
	EXPECT_EQ(pool.size(), hxsize_t{0});
	const hxfree_list<hxtest_object, 1>::ptr_t b = pool.try_allocate(6);
	EXPECT_FALSE((bool)b);
	EXPECT_EQ(pool.size(), hxsize_t{0});
	EXPECT_TRUE(check_stats(1, 0, 0, 1, 0, 0, 0, 0, 0, 0));
}

TEST_F(hxfree_list_test_f, release_raw_pointer) {
	hxfree_list<hxtest_object, 2> pool;
	hxfree_list<hxtest_object, 2>::ptr_t p = pool.allocate(5);
	hxtest_object* const raw = p.release();
	EXPECT_EQ(pool.size(), hxsize_t{1});
	EXPECT_TRUE(check_stats(1, 0, 0, 1, 0, 0, 0, 0, 0, 0));
	pool.release(raw);
	EXPECT_EQ(pool.size(), hxsize_t{2});
	EXPECT_TRUE(check_stats(1, 1, 0, 1, 0, 0, 0, 0, 0, 0));
}

TEST_F(hxfree_list_test_f, release_hxptr) {
	hxfree_list<hxtest_object, 1> pool;
	hxfree_list<hxtest_object, 1>::ptr_t p = pool.allocate(8);
	EXPECT_TRUE(check_stats(1, 0, 0, 1, 0, 0, 0, 0, 0, 0));
	pool.release(hxmove(p));
	EXPECT_TRUE(check_stats(1, 1, 0, 1, 0, 0, 0, 0, 0, 0));
	EXPECT_EQ(pool.size(), hxsize_t{1});
	EXPECT_TRUE(check_stats(1, 1, 0, 1, 0, 0, 0, 0, 0, 0));
}

TEST_F(hxfree_list_test_f, deleter_ignores_null) {
	hxfree_list<hxtest_object, 1> pool;
	hxfree_list<hxtest_object, 1>::deleter_t deleter = pool.deleter();
	deleter(static_cast<hxtest_object*>(hxnull));
	EXPECT_TRUE(check_no_stats());
	EXPECT_EQ(pool.size(), hxsize_t{1});
}

TEST_F(hxfree_list_test_f, deleter_releases_on_ptr_destruction) {
	hxfree_list<hxtest_object, 1> pool;
	{
		const hxfree_list<hxtest_object, 1>::ptr_t p = pool.allocate(11);
		EXPECT_EQ(pool.size(), hxsize_t{0});
		EXPECT_TRUE(check_stats(1, 0, 0, 1, 0, 0, 0, 0, 0, 0));
	}
	EXPECT_TRUE(check_stats(1, 1, 0, 1, 0, 0, 0, 0, 0, 0));
	EXPECT_EQ(pool.size(), hxsize_t{1});
	EXPECT_TRUE(check_stats(1, 1, 0, 1, 0, 0, 0, 0, 0, 0));
}

TEST_F(hxfree_list_test_f, is_allocator_raw_pointer) {
	hxfree_list<hxtest_object, 1> pool;
	const hxtest_object external(0);
	EXPECT_FALSE(pool.is_allocator(static_cast<hxtest_object*>(hxnull)));
	EXPECT_FALSE(pool.is_allocator(&external));
	const hxfree_list<hxtest_object, 1>::ptr_t p = pool.allocate(9);
	EXPECT_TRUE(pool.is_allocator(p.get()));
	EXPECT_TRUE(check_stats(2, 0, 0, 2, 0, 0, 0, 0, 0, 0));
}

TEST_F(hxfree_list_test_f, is_allocator_hxptr) {
	hxfree_list<hxtest_object, 2> pool_a;
	const hxfree_list<hxtest_object, 1> pool_b;
	const hxfree_list<hxtest_object, 2>::ptr_t p = pool_a.allocate(3);
	EXPECT_TRUE(pool_a.is_allocator(p));
	EXPECT_FALSE(pool_b.is_allocator(p));
	EXPECT_TRUE(check_stats(1, 0, 0, 1, 0, 0, 0, 0, 0, 0));
}

TEST_F(hxfree_list_test_f, reserve) {
	hxfree_list<hxtest_object> pool;
	pool.reserve(2);
	EXPECT_EQ(pool.size(), hxsize_t{2});
	EXPECT_EQ(pool.capacity(), hxsize_t{2});
	EXPECT_EQ(pool.max_size(), hxsize_t{2});
	hxfree_list<hxtest_object>::ptr_t a = pool.allocate(10);
	hxfree_list<hxtest_object>::ptr_t b = pool.allocate(20);
	EXPECT_EQ(a->value(), (int32_t)10);
	EXPECT_EQ(b->value(), (int32_t)20);
	EXPECT_EQ(pool.size(), hxsize_t{0});
	pool.release(hxmove(a));
	EXPECT_EQ(pool.size(), hxsize_t{1});
	EXPECT_TRUE(check_stats(2, 1, 0, 2, 0, 0, 0, 0, 0, 0));
}

TEST_F(hxfree_list_test_f, move_construct) {
	hxfree_list<hxtest_object> source;
	source.reserve(2);
	hxfree_list<hxtest_object>::ptr_t p = source.allocate(5);
	hxtest_object* const raw = p.release();
	source.release(raw);
	const hxfree_list<hxtest_object> dest(hxmove(source));
	EXPECT_EQ(dest.size(), hxsize_t{2});
	EXPECT_EQ(dest.capacity(), hxsize_t{2});
	EXPECT_TRUE(check_stats(1, 1, 0, 1, 0, 0, 0, 0, 0, 0));
}

TEST_F(hxfree_list_test_f, size_across_allocate_and_release) {
	hxfree_list<hxtest_object, 2> pool;
	EXPECT_EQ(pool.size(), hxsize_t{2});
	hxfree_list<hxtest_object, 2>::ptr_t a = pool.allocate(1);
	EXPECT_EQ(pool.size(), hxsize_t{1});
	hxfree_list<hxtest_object, 2>::ptr_t b = pool.allocate(2);
	EXPECT_EQ(pool.size(), hxsize_t{0});
	pool.release(hxmove(a));
	EXPECT_EQ(pool.size(), hxsize_t{1});
	pool.release(hxmove(b));
	EXPECT_EQ(pool.size(), hxsize_t{2});
	EXPECT_TRUE(check_stats(2, 2, 0, 2, 0, 0, 0, 0, 0, 0));
}

TEST_F(hxfree_list_test_f, reallocate_slot_after_release) {
	hxfree_list<hxtest_object, 1> pool;
	hxtest_object* addr_first = hxnull;
	{
		const hxfree_list<hxtest_object, 1>::ptr_t p = pool.allocate(1);
		addr_first = p.get();
	}
	hxfree_list<hxtest_object, 1>::ptr_t q = pool.allocate(2);
	EXPECT_EQ(q.get(), addr_first);
	EXPECT_EQ(q->value(), (int32_t)2);
	EXPECT_TRUE(check_stats(2, 1, 0, 2, 0, 0, 0, 0, 0, 0));
}

TEST_F(hxfree_list_test_f, deleter_with_hxdo_not_delete) {
	hxfree_list<hxtest_object, 1> pool;
	hxfree_list<hxtest_object, 1>::ptr_t owned = pool.allocate(7);
	hxtest_object* const raw = owned.release();
	{
		const hxptr<hxtest_object, hxdo_not_delete> unowned(raw, hxdo_not_delete());
		EXPECT_TRUE(pool.is_allocator(unowned));
		EXPECT_EQ(pool.size(), hxsize_t{0});
	}
	EXPECT_TRUE(check_stats(1, 0, 0, 1, 0, 0, 0, 0, 0, 0));
	EXPECT_EQ(pool.size(), hxsize_t{0});
	pool.release(raw);
	EXPECT_TRUE(check_stats(1, 1, 0, 1, 0, 0, 0, 0, 0, 0));
	EXPECT_EQ(pool.size(), hxsize_t{1});
	EXPECT_TRUE(check_stats(1, 1, 0, 1, 0, 0, 0, 0, 0, 0));
}

TEST_F(hxfree_list_test_f, hxlist_with_free_list_deleter_push_and_clear) {
	hxtest_node_pool_t pool;
	hxtest_node_list_t list(pool.deleter());
	list.push_back(pool.allocate(10));
	list.push_back(pool.allocate(20));
	list.push_back(pool.allocate(30));
	EXPECT_EQ(list.size(), hxsize_t{3});
	EXPECT_EQ(pool.size(), hxsize_t{1});
	list.clear();
	EXPECT_TRUE(check_stats(3, 3, 0, 3, 0, 0, 0, 0, 0, 0));
	EXPECT_EQ(pool.size(), hxsize_t{4});
	EXPECT_TRUE(list.empty());
	EXPECT_TRUE(check_stats(3, 3, 0, 3, 0, 0, 0, 0, 0, 0));
}

TEST_F(hxfree_list_test_f, hxlist_with_free_list_deleter_erase) {
	hxtest_node_pool_t pool;
	hxtest_node_list_t list(pool.deleter());
	list.push_back(pool.allocate(1));
	list.push_back(pool.allocate(2));
	list.erase(list.begin());
	EXPECT_TRUE(check_stats(2, 1, 0, 2, 0, 0, 0, 0, 0, 0));
	EXPECT_EQ(pool.size(), hxsize_t{3});
	EXPECT_EQ(list.size(), hxsize_t{1});
	EXPECT_EQ(list.front().obj.value(), (int32_t)2);
	list.erase(list.begin());
	EXPECT_TRUE(check_stats(2, 2, 0, 2, 0, 0, 0, 0, 0, 0));
	EXPECT_EQ(pool.size(), hxsize_t{4});
	EXPECT_TRUE(list.empty());
	EXPECT_TRUE(check_stats(2, 2, 0, 2, 0, 0, 0, 0, 0, 0));
}

TEST_F(hxfree_list_test_f, hxlist_with_free_list_deleter_pop_front) {
	hxtest_node_pool_t pool;
	hxtest_node_list_t list(pool.deleter());
	list.push_back(pool.allocate(7));
	list.push_back(pool.allocate(8));
	{
		hxptr<hxtest_free_list_node, hxtest_node_pool_t::deleter_t> p = list.pop_front();
		EXPECT_EQ(p->obj.value(), (int32_t)7);
		EXPECT_TRUE(check_stats(2, 0, 0, 2, 0, 0, 0, 0, 0, 0));
		EXPECT_EQ(pool.size(), hxsize_t{2});
	}
	EXPECT_TRUE(check_stats(2, 1, 0, 2, 0, 0, 0, 0, 0, 0));
	EXPECT_EQ(pool.size(), hxsize_t{3});
	EXPECT_TRUE(check_stats(2, 1, 0, 2, 0, 0, 0, 0, 0, 0));
}

TEST_F(hxfree_list_test_f, hxhash_table_with_free_list_deleter_insert_and_find) {
	hxtest_hash_pool_t pool;
	hxtest_hash_table_t table(pool.deleter());
	table.insert(pool.allocate(10));
	table.insert(pool.allocate(20));
	EXPECT_EQ(table.size(), hxsize_t{2});
	EXPECT_NE(table.find(10), table.end());
	EXPECT_NE(table.find(20), table.end());
	EXPECT_EQ(table.find(30), table.end());
	EXPECT_TRUE(check_stats(2, 0, 0, 2, 0, 0, 0, 0, 0, 0));
}

TEST_F(hxfree_list_test_f, hxhash_table_with_free_list_deleter_clear) {
	hxtest_hash_pool_t pool;
	hxtest_hash_table_t table(pool.deleter());
	table.insert(pool.allocate(1));
	table.insert(pool.allocate(2));
	table.insert(pool.allocate(3));
	EXPECT_EQ(pool.size(), hxsize_t{5});
	table.clear();
	EXPECT_TRUE(check_stats(3, 3, 0, 3, 0, 0, 0, 0, 0, 0));
	EXPECT_EQ(pool.size(), hxsize_t{8});
	EXPECT_TRUE(table.empty());
	EXPECT_TRUE(check_stats(3, 3, 0, 3, 0, 0, 0, 0, 0, 0));
}

TEST_F(hxfree_list_test_f, hxhash_table_with_free_list_deleter_erase) {
	hxtest_hash_pool_t pool;
	hxtest_hash_table_t table(pool.deleter());
	table.insert(pool.allocate(10));
	table.insert(pool.allocate(20));
	EXPECT_EQ(table.erase(99), hxsize_t{0});
	EXPECT_TRUE(check_stats(2, 0, 0, 2, 0, 0, 0, 0, 0, 0));
	const hxsize_t removed = table.erase(10);
	EXPECT_EQ(removed, hxsize_t{1});
	EXPECT_TRUE(check_stats(2, 1, 0, 2, 0, 0, 0, 0, 0, 0));
	EXPECT_EQ(pool.size(), hxsize_t{7});
	EXPECT_EQ(table.size(), hxsize_t{1});
	EXPECT_EQ(table.find(10), table.end());
	EXPECT_TRUE(check_stats(2, 1, 0, 2, 0, 0, 0, 0, 0, 0));
}

TEST_F(hxfree_list_test_f, hxhash_table_with_free_list_deleter_extract) {
	hxtest_hash_pool_t pool;
	hxtest_hash_table_t table(pool.deleter());
	table.insert(pool.allocate(10));
	table.insert(pool.allocate(20));
	{
		hxptr<hxtest_hash_node, hxtest_hash_pool_t::deleter_t> p = table.extract(10);
		EXPECT_TRUE((bool)p);
		EXPECT_EQ(p->hash_key(), (int32_t)10);
		EXPECT_TRUE(check_stats(2, 0, 0, 2, 0, 0, 0, 0, 0, 0));
		EXPECT_EQ(pool.size(), hxsize_t{6});
	}
	EXPECT_TRUE(check_stats(2, 1, 0, 2, 0, 0, 0, 0, 0, 0));
	EXPECT_EQ(pool.size(), hxsize_t{7});
	EXPECT_TRUE(check_stats(2, 1, 0, 2, 0, 0, 0, 0, 0, 0));
}

TEST_F(hxfree_list_test_f, hxhash_table_with_hxdo_not_delete_does_not_release_slots) {
	hxtest_hash_pool_t pool;
	hxtest_hash_node* raws[2] = { hxnull, hxnull };
	{
		hxhash_table<hxtest_hash_node, hxdo_not_delete, false, 2> table;
		hxtest_hash_pool_t::ptr_t a = pool.allocate(1);
		hxtest_hash_pool_t::ptr_t b = pool.allocate(2);
		raws[0] = a.get();
		raws[1] = b.get();
		table.insert(hxmove(a));
		table.insert(hxmove(b));
		EXPECT_EQ(pool.size(), hxsize_t{6});
	}
	EXPECT_TRUE(check_stats(2, 0, 0, 2, 0, 0, 0, 0, 0, 0));
	EXPECT_EQ(pool.size(), hxsize_t{6});
	pool.release(raws[0]);
	pool.release(raws[1]);
	EXPECT_TRUE(check_stats(2, 2, 0, 2, 0, 0, 0, 0, 0, 0));
	EXPECT_EQ(pool.size(), hxsize_t{8});
	EXPECT_TRUE(check_stats(2, 2, 0, 2, 0, 0, 0, 0, 0, 0));
}
