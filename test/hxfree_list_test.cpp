// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

#include <hx/hxfree_list.hpp>
#include <hx/hxhash_table.hpp>
#include <hx/hxhash_table_nodes.hpp>
#include <hx/hxlist.hpp>
#include "./hxtest_util.hpp"

HX_NS_USE

namespace {
class hxfree_list_test_f : public hxtest_object_fixture { };

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

TEST_F(hxfree_list_test_f, static_capacity_initial_size_equals_capacity) {
	const hxfree_list<hxtest_object, 4> pool;
	EXPECT_EQ(pool.size(), (hxsize_t)4);
}

TEST_F(hxfree_list_test_f, allocate_returns_nonnull) {
	hxfree_list<hxtest_object, 1> pool;
	auto p = pool.allocate(7);
	EXPECT_TRUE((bool)p);
	EXPECT_NE(p.get(), static_cast<hxtest_object*>(hxnull));
}

TEST_F(hxfree_list_test_f, allocate_constructs_value) {
	hxfree_list<hxtest_object, 1> pool;
	auto p = pool.allocate(34);
	EXPECT_EQ(p->id, (int32_t)34);
}

TEST_F(hxfree_list_test_f, release_raw_increments_size_by_one) {
	hxfree_list<hxtest_object, 2> pool;
	auto p = pool.allocate(5);
	hxtest_object* raw = p.release();
	EXPECT_EQ(pool.size(), (hxsize_t)1);
	pool.release(raw);
	EXPECT_EQ(pool.size(), (hxsize_t)2);
}

TEST_F(hxfree_list_test_f, release_raw_calls_destructor) {
	hxfree_list<hxtest_object, 1> pool;
	auto p = pool.allocate(3);
	hxtest_object* raw = p.release();
	EXPECT_EQ(this->m_destructed, (hxsize_t)0);
	pool.release(raw);
	EXPECT_EQ(this->m_destructed, (hxsize_t)1);
}

TEST_F(hxfree_list_test_f, release_hxptr_calls_destructor) {
	hxfree_list<hxtest_object, 1> pool;
	auto p = pool.allocate(8);
	EXPECT_EQ(this->m_destructed, (hxsize_t)0);
	pool.release(hxmove(p));
	EXPECT_EQ(this->m_destructed, (hxsize_t)1);
}

TEST_F(hxfree_list_test_f, deleter_releases_on_ptr_destruction) {
	hxfree_list<hxtest_object, 1> pool;
	{
		auto p = pool.allocate(11);
		EXPECT_EQ(pool.size(), (hxsize_t)0);
		EXPECT_EQ(this->m_destructed, (hxsize_t)0);
	}
	EXPECT_EQ(this->m_destructed, (hxsize_t)1);
	EXPECT_EQ(pool.size(), (hxsize_t)1);
}

TEST_F(hxfree_list_test_f, try_allocate_returns_nonnull_when_available) {
	hxfree_list<hxtest_object, 1> pool;
	auto p = pool.try_allocate(5);
	EXPECT_TRUE((bool)p);
	EXPECT_EQ(p->id, (int32_t)5);
}

TEST_F(hxfree_list_test_f, try_allocate_returns_null_when_full) {
	hxfree_list<hxtest_object, 1> pool;
	auto a = pool.try_allocate(1);
	auto b = pool.try_allocate(2);
	EXPECT_FALSE((bool)b);
}

TEST_F(hxfree_list_test_f, try_allocate_does_not_decrement_size_when_full) {
	hxfree_list<hxtest_object, 1> pool;
	auto a = pool.allocate(1);
	EXPECT_EQ(pool.size(), (hxsize_t)0);
	auto b = pool.try_allocate(2);
	EXPECT_EQ(pool.size(), (hxsize_t)0);
}

TEST_F(hxfree_list_test_f, is_allocator_raw_true_for_owned_pointer) {
	hxfree_list<hxtest_object, 1> pool;
	auto p = pool.allocate(9);
	EXPECT_TRUE(pool.is_allocator(p.get()));
}

TEST_F(hxfree_list_test_f, is_allocator_raw_false_for_null) {
	const hxfree_list<hxtest_object, 1> pool;
	EXPECT_FALSE(pool.is_allocator(static_cast<hxtest_object*>(hxnull)));
}

TEST_F(hxfree_list_test_f, is_allocator_raw_false_for_external_pointer) {
	const hxfree_list<hxtest_object, 1> pool;
	const hxtest_object external(0);
	EXPECT_FALSE(pool.is_allocator(&external));
}

TEST_F(hxfree_list_test_f, is_allocator_hxptr_true_for_owned) {
	hxfree_list<hxtest_object, 2> pool;
	auto p = pool.allocate(3);
	EXPECT_TRUE(pool.is_allocator(p));
}

TEST_F(hxfree_list_test_f, is_allocator_different_pool_false) {
	hxfree_list<hxtest_object, 1> pool_a;
	const hxfree_list<hxtest_object, 1> pool_b;
	auto p = pool_a.allocate(1);
	EXPECT_FALSE(pool_b.is_allocator(p));
}

TEST_F(hxfree_list_test_f, dynamic_reserve_establishes_capacity) {
	const hxsystem_allocator_scope scope(hxsystem_allocator_stack_0);
	hxfree_list<hxtest_object> pool;
	pool.reserve(3);
	EXPECT_EQ(pool.size(), (hxsize_t)3);
	EXPECT_EQ(pool.capacity(), (hxsize_t)3);
	EXPECT_EQ(pool.max_size(), (hxsize_t)3);
}

TEST_F(hxfree_list_test_f, dynamic_allocate_after_reserve) {
	const hxsystem_allocator_scope scope(hxsystem_allocator_stack_0);
	hxfree_list<hxtest_object> pool;
	pool.reserve(2);
	auto a = pool.allocate(10);
	auto b = pool.allocate(20);
	EXPECT_EQ(a->id, (int32_t)10);
	EXPECT_EQ(b->id, (int32_t)20);
	EXPECT_EQ(pool.size(), (hxsize_t)0);
}

TEST_F(hxfree_list_test_f, dynamic_release_restores_slot) {
	const hxsystem_allocator_scope scope(hxsystem_allocator_stack_0);
	hxfree_list<hxtest_object> pool;
	pool.reserve(1);
	auto p = pool.allocate(7);
	EXPECT_EQ(pool.size(), (hxsize_t)0);
	pool.release(hxmove(p));
	EXPECT_EQ(pool.size(), (hxsize_t)1);
}

TEST_F(hxfree_list_test_f, dynamic_move_construct_transfers_ownership) {
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

TEST_F(hxfree_list_test_f, size_decrements_at_first_allocation) {
	hxfree_list<hxtest_object, 2> pool;
	EXPECT_EQ(pool.size(), (hxsize_t)2);
	auto p = pool.allocate(1);
	EXPECT_EQ(pool.size(), (hxsize_t)1);
}

TEST_F(hxfree_list_test_f, size_decrements_at_last_allocation) {
	hxfree_list<hxtest_object, 2> pool;
	auto a = pool.allocate(1);
	EXPECT_EQ(pool.size(), (hxsize_t)1);
	auto b = pool.allocate(2);
	EXPECT_EQ(pool.size(), (hxsize_t)0);
}

TEST_F(hxfree_list_test_f, size_increments_at_first_release) {
	hxfree_list<hxtest_object, 2> pool;
	auto a = pool.allocate(1);
	auto b = pool.allocate(2);
	EXPECT_EQ(pool.size(), (hxsize_t)0);
	pool.release(hxmove(a));
	EXPECT_EQ(pool.size(), (hxsize_t)1);
}

TEST_F(hxfree_list_test_f, size_increments_at_last_release) {
	hxfree_list<hxtest_object, 2> pool;
	auto a = pool.allocate(1);
	auto b = pool.allocate(2);
	pool.release(hxmove(a));
	EXPECT_EQ(pool.size(), (hxsize_t)1);
	pool.release(hxmove(b));
	EXPECT_EQ(pool.size(), (hxsize_t)2);
}

TEST_F(hxfree_list_test_f, try_allocate_size_decrements_at_last_slot) {
	hxfree_list<hxtest_object, 1> pool;
	EXPECT_EQ(pool.size(), (hxsize_t)1);
	auto p = pool.try_allocate(0);
	EXPECT_EQ(pool.size(), (hxsize_t)0);
}

TEST_F(hxfree_list_test_f, reallocate_slot_after_release) {
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

static int hxs_ptr_stateful_delete_count = 0;
static int hxs_ptr_stateful_delete_tag = 0;
TEST_F(hxfree_list_test_f, construct_with_deleter_instance_stores_deleter) {
	const hxsystem_allocator_scope scope(hxsystem_allocator_stack_0);

	struct hxtest_stateful_deleter {
		explicit hxtest_stateful_deleter(int tag) : m_tag(tag) { }
		void operator()(hxtest_object* p) const {
			++hxs_ptr_stateful_delete_count;
			hxs_ptr_stateful_delete_tag = m_tag;
			hxdelete(p);
		}
		operator bool(void) const = delete;
		int m_tag;
	};

	{
		const hxptr<hxtest_object, hxtest_stateful_deleter> p(
			hxnew<hxtest_object>(1), hxtest_stateful_deleter(99));
	}
	EXPECT_EQ(hxs_ptr_stateful_delete_count, 1);
	EXPECT_EQ(hxs_ptr_stateful_delete_tag, 99);
}

TEST_F(hxfree_list_test_f, hxdo_not_delete_ptr_does_not_release_slot) {
	hxfree_list<hxtest_object, 1> pool;
	hxtest_object* raw = hxnull;
	{
		auto owned = pool.allocate(3);
		raw = owned.release();
		const hxptr<hxtest_object, hxdo_not_delete> unowned(raw, hxdo_not_delete());
		EXPECT_EQ(pool.size(), (hxsize_t)0);
	}
	EXPECT_EQ(this->m_destructed, (hxsize_t)0);
	EXPECT_EQ(pool.size(), (hxsize_t)0);
	pool.release(raw);
	EXPECT_EQ(this->m_destructed, (hxsize_t)1);
	EXPECT_EQ(pool.size(), (hxsize_t)1);
}

TEST_F(hxfree_list_test_f, hxdo_not_delete_ptr_is_allocator_still_true) {
	hxfree_list<hxtest_object, 1> pool;
	auto owned = pool.allocate(7);
	hxtest_object* const raw = owned.release();
	const hxptr<hxtest_object, hxdo_not_delete> unowned(raw, hxdo_not_delete());
	EXPECT_TRUE(pool.is_allocator(unowned));
	pool.release(raw);
}

TEST_F(hxfree_list_test_f, hxlist_free_list_deleter_push_and_size) {
	hxtest_node_pool_t pool;
	hxtest_node_list_t list(pool.deleter());
	list.push_back(pool.allocate(1));
	list.push_back(pool.allocate(2));
	EXPECT_EQ(list.size(), (hxsize_t)2);
	EXPECT_EQ(pool.size(), (hxsize_t)2);
}

TEST_F(hxfree_list_test_f, hxlist_free_list_deleter_clear_returns_all_slots) {
	hxtest_node_pool_t pool;
	hxtest_node_list_t list(pool.deleter());
	list.push_back(pool.allocate(10));
	list.push_back(pool.allocate(20));
	list.push_back(pool.allocate(30));
	EXPECT_EQ(pool.size(), (hxsize_t)1);
	list.clear();
	EXPECT_EQ(this->m_destructed, (hxsize_t)3);
	EXPECT_EQ(pool.size(), (hxsize_t)4);
	EXPECT_TRUE(list.empty());
}

TEST_F(hxfree_list_test_f, hxlist_free_list_deleter_erase_returns_one_slot) {
	hxtest_node_pool_t pool;
	hxtest_node_list_t list(pool.deleter());
	list.push_back(pool.allocate(1));
	list.push_back(pool.allocate(2));
	list.erase(list.begin());
	EXPECT_EQ(this->m_destructed, (hxsize_t)1);
	EXPECT_EQ(pool.size(), (hxsize_t)3);
	EXPECT_EQ(list.size(), (hxsize_t)1);
	EXPECT_EQ(list.front().obj.id, (int32_t)2);
}

TEST_F(hxfree_list_test_f, hxlist_free_list_deleter_erase_at_last_element) {
	hxtest_node_pool_t pool;
	hxtest_node_list_t list(pool.deleter());
	list.push_back(pool.allocate(5));
	list.erase(list.begin());
	EXPECT_EQ(this->m_destructed, (hxsize_t)1);
	EXPECT_EQ(pool.size(), (hxsize_t)4);
	EXPECT_TRUE(list.empty());
}

TEST_F(hxfree_list_test_f, hxlist_free_list_deleter_pop_front_returns_slot) {
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

TEST_F(hxfree_list_test_f, hxhash_table_free_list_deleter_insert_and_find) {
	hxtest_hash_pool_t pool;
	hxtest_hash_table_t table(pool.deleter());
	table.insert(pool.allocate(10));
	table.insert(pool.allocate(20));
	EXPECT_EQ(table.size(), (hxsize_t)2);
	EXPECT_NE(table.find(10), static_cast<hxtest_hash_node*>(hxnull));
	EXPECT_NE(table.find(20), static_cast<hxtest_hash_node*>(hxnull));
	EXPECT_EQ(table.find(30), static_cast<hxtest_hash_node*>(hxnull));
}

TEST_F(hxfree_list_test_f, hxhash_table_free_list_deleter_clear_returns_all_slots) {
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

TEST_F(hxfree_list_test_f, hxhash_table_free_list_deleter_erase_returns_slot) {
	hxtest_hash_pool_t pool;
	hxtest_hash_table_t table(pool.deleter());
	table.insert(pool.allocate(10));
	table.insert(pool.allocate(20));
	const hxsize_t removed = table.erase(10);
	EXPECT_EQ(removed, (hxsize_t)1);
	EXPECT_EQ(this->m_destructed, (hxsize_t)1);
	EXPECT_EQ(pool.size(), (hxsize_t)7);
	EXPECT_EQ(table.size(), (hxsize_t)1);
	EXPECT_EQ(table.find(10), static_cast<hxtest_hash_node*>(hxnull));
}

TEST_F(hxfree_list_test_f, hxhash_table_free_list_deleter_erase_missing_key_is_zero) {
	hxtest_hash_pool_t pool;
	hxtest_hash_table_t table(pool.deleter());
	table.insert(pool.allocate(10));
	const hxsize_t removed = table.erase(99);
	EXPECT_EQ(removed, (hxsize_t)0);
	EXPECT_EQ(this->m_destructed, (hxsize_t)0);
	EXPECT_EQ(pool.size(), (hxsize_t)7);
	EXPECT_EQ(table.size(), (hxsize_t)1);
}

TEST_F(hxfree_list_test_f, hxhash_table_free_list_deleter_extract_returns_slot) {
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
