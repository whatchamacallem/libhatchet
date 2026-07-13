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
	hxtest_hash_node* hash_next(void) const {
		return static_cast<hxtest_hash_node*>(hxhash_table_set_node<int32_t>::hash_next());
	}
	hxtest_object obj;
};

using hxtest_hash_pool_t = hxfree_list<hxtest_hash_node, 8>;
using hxtest_hash_table_t = hxhash_table<hxtest_hash_node, hxtest_hash_pool_t::deleter_t, false, 2>;

} // namespace

TEST_F(hxfree_list_test_f, hxfree_list_construct) {
	const hxfree_list<hxtest_object, 4> fixed_pool;
	EXPECT_EQ(fixed_pool.size(), (hxsize_t)4);
	EXPECT_EQ(fixed_pool.capacity(), (hxsize_t)4);
	EXPECT_EQ(fixed_pool.max_size(), (hxsize_t)4);
	EXPECT_FALSE(fixed_pool.empty());
	const hxsystem_allocator_scope scope(hxsystem_allocator_stack_0);
	const hxfree_list<hxtest_object> dynamic_pool;
	EXPECT_EQ(dynamic_pool.size(), (hxsize_t)0);
	EXPECT_EQ(dynamic_pool.capacity(), (hxsize_t)0);
	EXPECT_TRUE(dynamic_pool.empty());
}

TEST_F(hxfree_list_test_f, hxfree_list_allocate) {
	hxfree_list<hxtest_object, 2> pool;
	auto a = pool.allocate(7);
	EXPECT_TRUE((bool)a);
	EXPECT_NE(a.get(), static_cast<hxtest_object*>(hxnull));
	EXPECT_EQ(a->id, (int32_t)7);
	EXPECT_EQ(pool.size(), (hxsize_t)1);
	auto b = pool.allocate(34);
	EXPECT_EQ(b->id, (int32_t)34);
	EXPECT_EQ(pool.size(), (hxsize_t)0);
}

TEST_F(hxfree_list_test_f, hxfree_list_try_allocate) {
	hxfree_list<hxtest_object, 1> pool;
	EXPECT_EQ(pool.size(), (hxsize_t)1);
	auto a = pool.try_allocate(5);
	EXPECT_TRUE((bool)a);
	EXPECT_EQ(a->id, (int32_t)5);
	EXPECT_EQ(pool.size(), (hxsize_t)0);
	auto b = pool.try_allocate(6);
	EXPECT_FALSE((bool)b);
	EXPECT_EQ(pool.size(), (hxsize_t)0);
}

TEST_F(hxfree_list_test_f, hxfree_list_release_raw_pointer) {
	hxfree_list<hxtest_object, 2> pool;
	auto p = pool.allocate(5);
	hxtest_object* const raw = p.release();
	EXPECT_EQ(pool.size(), (hxsize_t)1);
	EXPECT_EQ(this->m_destructed, (hxsize_t)0);
	pool.release(raw);
	EXPECT_EQ(pool.size(), (hxsize_t)2);
	EXPECT_EQ(this->m_destructed, (hxsize_t)1);
}

TEST_F(hxfree_list_test_f, hxfree_list_release_hxptr) {
	hxfree_list<hxtest_object, 1> pool;
	auto p = pool.allocate(8);
	EXPECT_EQ(this->m_destructed, (hxsize_t)0);
	pool.release(hxmove(p));
	EXPECT_EQ(this->m_destructed, (hxsize_t)1);
	EXPECT_EQ(pool.size(), (hxsize_t)1);
}

TEST_F(hxfree_list_test_f, hxfree_list_deleter_ignores_null) {
	hxfree_list<hxtest_object, 1> pool;
	hxfree_list<hxtest_object, 1>::deleter_t deleter = pool.deleter();
	deleter(static_cast<hxtest_object*>(hxnull));
	EXPECT_EQ(this->m_destructed, (hxsize_t)0);
	EXPECT_EQ(pool.size(), (hxsize_t)1);
}

TEST_F(hxfree_list_test_f, hxfree_list_deleter_releases_on_ptr_destruction) {
	hxfree_list<hxtest_object, 1> pool;
	{
		auto p = pool.allocate(11);
		EXPECT_EQ(pool.size(), (hxsize_t)0);
		EXPECT_EQ(this->m_destructed, (hxsize_t)0);
	}
	EXPECT_EQ(this->m_destructed, (hxsize_t)1);
	EXPECT_EQ(pool.size(), (hxsize_t)1);
}

TEST_F(hxfree_list_test_f, hxfree_list_is_allocator_raw_pointer) {
	hxfree_list<hxtest_object, 1> pool;
	const hxtest_object external(0);
	EXPECT_FALSE(pool.is_allocator(static_cast<hxtest_object*>(hxnull)));
	EXPECT_FALSE(pool.is_allocator(&external));
	auto p = pool.allocate(9);
	EXPECT_TRUE(pool.is_allocator(p.get()));
}

TEST_F(hxfree_list_test_f, hxfree_list_is_allocator_hxptr) {
	hxfree_list<hxtest_object, 2> pool_a;
	const hxfree_list<hxtest_object, 1> pool_b;
	auto p = pool_a.allocate(3);
	EXPECT_TRUE(pool_a.is_allocator(p));
	EXPECT_FALSE(pool_b.is_allocator(p));
}

TEST_F(hxfree_list_test_f, hxfree_list_reserve) {
	const hxsystem_allocator_scope scope(hxsystem_allocator_stack_0);
	hxfree_list<hxtest_object> pool;
	pool.reserve(2);
	EXPECT_EQ(pool.size(), (hxsize_t)2);
	EXPECT_EQ(pool.capacity(), (hxsize_t)2);
	EXPECT_EQ(pool.max_size(), (hxsize_t)2);
	auto a = pool.allocate(10);
	auto b = pool.allocate(20);
	EXPECT_EQ(a->id, (int32_t)10);
	EXPECT_EQ(b->id, (int32_t)20);
	EXPECT_EQ(pool.size(), (hxsize_t)0);
	pool.release(hxmove(a));
	EXPECT_EQ(pool.size(), (hxsize_t)1);
}

TEST_F(hxfree_list_test_f, hxfree_list_move_construct) {
	const hxsystem_allocator_scope scope(hxsystem_allocator_stack_0);
	hxfree_list<hxtest_object> source;
	source.reserve(2);
	auto p = source.allocate(5);
	hxtest_object* const raw = p.release();
	source.release(raw);
	const hxfree_list<hxtest_object> dest(hxmove(source));
	EXPECT_EQ(dest.size(), (hxsize_t)2);
	EXPECT_EQ(dest.capacity(), (hxsize_t)2);
}

TEST_F(hxfree_list_test_f, hxfree_list_size_across_allocate_and_release) {
	hxfree_list<hxtest_object, 2> pool;
	EXPECT_EQ(pool.size(), (hxsize_t)2);
	auto a = pool.allocate(1);
	EXPECT_EQ(pool.size(), (hxsize_t)1);
	auto b = pool.allocate(2);
	EXPECT_EQ(pool.size(), (hxsize_t)0);
	pool.release(hxmove(a));
	EXPECT_EQ(pool.size(), (hxsize_t)1);
	pool.release(hxmove(b));
	EXPECT_EQ(pool.size(), (hxsize_t)2);
}

TEST_F(hxfree_list_test_f, hxfree_list_reallocate_slot_after_release) {
	hxfree_list<hxtest_object, 1> pool;
	hxtest_object* addr_first = hxnull;
	{
		auto p = pool.allocate(1);
		addr_first = p.get();
	}
	auto q = pool.allocate(2);
	EXPECT_EQ(q.get(), addr_first);
	EXPECT_EQ(q->id, (int32_t)2);
}

TEST_F(hxfree_list_test_f, hxfree_list_deleter_with_hxdo_not_delete) {
	hxfree_list<hxtest_object, 1> pool;
	auto owned = pool.allocate(7);
	hxtest_object* const raw = owned.release();
	{
		const hxptr<hxtest_object, hxdo_not_delete> unowned(raw, hxdo_not_delete());
		EXPECT_TRUE(pool.is_allocator(unowned));
		EXPECT_EQ(pool.size(), (hxsize_t)0);
	}
	EXPECT_EQ(this->m_destructed, (hxsize_t)0);
	EXPECT_EQ(pool.size(), (hxsize_t)0);
	pool.release(raw);
	EXPECT_EQ(this->m_destructed, (hxsize_t)1);
	EXPECT_EQ(pool.size(), (hxsize_t)1);
}

TEST_F(hxfree_list_test_f, hxlist_with_free_list_deleter_push_and_clear) {
	hxtest_node_pool_t pool;
	hxtest_node_list_t list(pool.deleter());
	list.push_back(pool.allocate(10));
	list.push_back(pool.allocate(20));
	list.push_back(pool.allocate(30));
	EXPECT_EQ(list.size(), (hxsize_t)3);
	EXPECT_EQ(pool.size(), (hxsize_t)1);
	list.clear();
	EXPECT_EQ(this->m_destructed, (hxsize_t)3);
	EXPECT_EQ(pool.size(), (hxsize_t)4);
	EXPECT_TRUE(list.empty());
}

TEST_F(hxfree_list_test_f, hxlist_with_free_list_deleter_erase) {
	hxtest_node_pool_t pool;
	hxtest_node_list_t list(pool.deleter());
	list.push_back(pool.allocate(1));
	list.push_back(pool.allocate(2));
	list.erase(list.begin());
	EXPECT_EQ(this->m_destructed, (hxsize_t)1);
	EXPECT_EQ(pool.size(), (hxsize_t)3);
	EXPECT_EQ(list.size(), (hxsize_t)1);
	EXPECT_EQ(list.front().obj.id, (int32_t)2);
	list.erase(list.begin());
	EXPECT_EQ(this->m_destructed, (hxsize_t)2);
	EXPECT_EQ(pool.size(), (hxsize_t)4);
	EXPECT_TRUE(list.empty());
}

TEST_F(hxfree_list_test_f, hxlist_with_free_list_deleter_pop_front) {
	hxtest_node_pool_t pool;
	hxtest_node_list_t list(pool.deleter());
	list.push_back(pool.allocate(7));
	list.push_back(pool.allocate(8));
	{
		auto p = list.pop_front();
		EXPECT_EQ(p->obj.id, (int32_t)7);
		EXPECT_EQ(this->m_destructed, (hxsize_t)0);
		EXPECT_EQ(pool.size(), (hxsize_t)2);
	}
	EXPECT_EQ(this->m_destructed, (hxsize_t)1);
	EXPECT_EQ(pool.size(), (hxsize_t)3);
}

TEST_F(hxfree_list_test_f, hxhash_table_with_free_list_deleter_insert_and_find) {
	hxtest_hash_pool_t pool;
	hxtest_hash_table_t table(pool.deleter());
	table.insert(pool.allocate(10));
	table.insert(pool.allocate(20));
	EXPECT_EQ(table.size(), (hxsize_t)2);
	EXPECT_NE(table.find(10), static_cast<hxtest_hash_node*>(hxnull));
	EXPECT_NE(table.find(20), static_cast<hxtest_hash_node*>(hxnull));
	EXPECT_EQ(table.find(30), static_cast<hxtest_hash_node*>(hxnull));
}

TEST_F(hxfree_list_test_f, hxhash_table_with_free_list_deleter_clear) {
	hxtest_hash_pool_t pool;
	hxtest_hash_table_t table(pool.deleter());
	table.insert(pool.allocate(1));
	table.insert(pool.allocate(2));
	table.insert(pool.allocate(3));
	EXPECT_EQ(pool.size(), (hxsize_t)5);
	table.clear();
	EXPECT_EQ(this->m_destructed, (hxsize_t)3);
	EXPECT_EQ(pool.size(), (hxsize_t)8);
	EXPECT_TRUE(table.empty());
}

TEST_F(hxfree_list_test_f, hxhash_table_with_free_list_deleter_erase) {
	hxtest_hash_pool_t pool;
	hxtest_hash_table_t table(pool.deleter());
	table.insert(pool.allocate(10));
	table.insert(pool.allocate(20));
	EXPECT_EQ(table.erase(99), (hxsize_t)0);
	EXPECT_EQ(this->m_destructed, (hxsize_t)0);
	const hxsize_t removed = table.erase(10);
	EXPECT_EQ(removed, (hxsize_t)1);
	EXPECT_EQ(this->m_destructed, (hxsize_t)1);
	EXPECT_EQ(pool.size(), (hxsize_t)7);
	EXPECT_EQ(table.size(), (hxsize_t)1);
	EXPECT_EQ(table.find(10), static_cast<hxtest_hash_node*>(hxnull));
}

TEST_F(hxfree_list_test_f, hxhash_table_with_free_list_deleter_extract) {
	hxtest_hash_pool_t pool;
	hxtest_hash_table_t table(pool.deleter());
	table.insert(pool.allocate(10));
	table.insert(pool.allocate(20));
	{
		auto p = table.extract(10);
		EXPECT_TRUE((bool)p);
		EXPECT_EQ(p->hash_key(), (int32_t)10);
		EXPECT_EQ(this->m_destructed, (hxsize_t)0);
		EXPECT_EQ(pool.size(), (hxsize_t)6);
	}
	EXPECT_EQ(this->m_destructed, (hxsize_t)1);
	EXPECT_EQ(pool.size(), (hxsize_t)7);
}

TEST_F(hxfree_list_test_f, hxhash_table_with_hxdo_not_delete_does_not_release_slots) {
	hxtest_hash_pool_t pool;
	hxtest_hash_node* raws[2] = { hxnull, hxnull };
	{
		hxhash_table<hxtest_hash_node, hxdo_not_delete, false, 2> table;
		auto a = pool.allocate(1);
		auto b = pool.allocate(2);
		raws[0] = a.get();
		raws[1] = b.get();
		table.insert(hxmove(a));
		table.insert(hxmove(b));
		EXPECT_EQ(pool.size(), (hxsize_t)6);
	}
	EXPECT_EQ(this->m_destructed, (hxsize_t)0);
	EXPECT_EQ(pool.size(), (hxsize_t)6);
	pool.release(raws[0]);
	pool.release(raws[1]);
	EXPECT_EQ(this->m_destructed, (hxsize_t)2);
	EXPECT_EQ(pool.size(), (hxsize_t)8);
}
