// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

namespace {

struct hxtest_list_node_t : hxlist_node {
	explicit hxtest_list_node_t(int v) : value(v) { }
	int value;
};

// A node type that tracks destructor calls to verify deleter behavior.
int hxs_list_test_destructor_count = 0;

struct hxtest_list_counted_node_t : hxlist_node {
	explicit hxtest_list_counted_node_t(int v) : value(v) { }
	~hxtest_list_counted_node_t(void) { ++hxs_list_test_destructor_count; }
	int value;
};

// A custom deleter that tracks how many times it is called.
int hxs_test_custom_deleter_count = 0;

struct hxtest_list_custom_deleter_t {
	void operator()(hxtest_list_counted_node_t* ptr) const {
		++hxs_test_custom_deleter_count;
		hxdelete(ptr);
	}
	operator bool(void) const { return true; }
};

} // namespace


// Newly constructed list is empty, size is 0, begin equals end.
TEST(hxlist_test, empty_on_construction) {
	hxlist<hxtest_list_node_t, hxdo_not_delete> list;
	EXPECT_TRUE(list.empty());
	EXPECT_EQ(list.size(), 0u);
	EXPECT_EQ(list.begin(), list.end());
}

// push_back preserves insertion order and size over three nodes.
TEST(hxlist_test, push_back_and_iterate) {
	hxlist<hxtest_list_node_t, hxdo_not_delete> list;
	hxtest_list_node_t a(1), b(2), c(3);
	const hxlist<hxtest_list_node_t, hxdo_not_delete>::iterator ia = list.push_back(&a);
	const hxlist<hxtest_list_node_t, hxdo_not_delete>::iterator ib = list.push_back(&b);
	const hxlist<hxtest_list_node_t, hxdo_not_delete>::iterator ic = list.push_back(&c);
	hxtest_gdb_break_hxlist();
	EXPECT_EQ(list.size(), 3u);
	EXPECT_FALSE(list.empty());
	EXPECT_EQ(&*ia, &a);
	EXPECT_EQ(&*ib, &b);
	EXPECT_EQ(&*ic, &c);
	EXPECT_EQ(ia, list.begin());
	int expected = 1;
	for(const hxtest_list_node_t& n : list) {
		EXPECT_EQ(n.value, expected++);
	}
	EXPECT_EQ(expected, 4);
	list.release_all();
}

// push_back two nodes: front is first pushed, back is second pushed.
TEST(hxlist_test, push_back_two_nodes_order) {
	hxtest_list_node_t a(10), b(20);
	hxlist<hxtest_list_node_t, hxdo_not_delete> list;
	const hxlist<hxtest_list_node_t, hxdo_not_delete>::iterator ia = list.push_back(&a);
	const hxlist<hxtest_list_node_t, hxdo_not_delete>::iterator ib = list.push_back(&b);
	EXPECT_EQ(&list.front(), &a);
	EXPECT_EQ(&list.back(), &b);
	EXPECT_EQ(&*ia, &a);
	EXPECT_EQ(&*ib, &b);
	list.release_all();
}

// push_front reverses insertion order so iteration is 1,2,3.
TEST(hxlist_test, push_front_and_iterate) {
	hxlist<hxtest_list_node_t, hxdo_not_delete> list;
	hxtest_list_node_t a(1), b(2), c(3);
	const hxlist<hxtest_list_node_t, hxdo_not_delete>::iterator ic = list.push_front(&c);
	const hxlist<hxtest_list_node_t, hxdo_not_delete>::iterator ib = list.push_front(&b);
	const hxlist<hxtest_list_node_t, hxdo_not_delete>::iterator ia = list.push_front(&a);
	EXPECT_EQ(list.size(), 3u);
	EXPECT_EQ(&*ia, &a);
	EXPECT_EQ(&*ib, &b);
	EXPECT_EQ(&*ic, &c);
	EXPECT_EQ(ia, list.begin());
	int expected = 1;
	for(const hxtest_list_node_t& n : list) {
		EXPECT_EQ(n.value, expected++);
	}
	EXPECT_EQ(expected, 4);
	list.release_all();
}

// push_front two nodes: most-recently pushed is front, first pushed is back.
TEST(hxlist_test, push_front_two_nodes_order) {
	hxtest_list_node_t a(10), b(20);
	hxlist<hxtest_list_node_t, hxdo_not_delete> list;
	const hxlist<hxtest_list_node_t, hxdo_not_delete>::iterator ia = list.push_front(&a);
	const hxlist<hxtest_list_node_t, hxdo_not_delete>::iterator ib = list.push_front(&b);
	EXPECT_EQ(&list.front(), &b);
	EXPECT_EQ(&list.back(), &a);
	EXPECT_EQ(&*ia, &a);
	EXPECT_EQ(&*ib, &b);
	list.release_all();
}

// pop_front and pop_back remove from the correct ends, leaving the middle intact,
// and return null once the list is empty.
TEST(hxlist_test, pop_front_and_pop_back) {
	hxlist<hxtest_list_node_t, hxdo_not_delete> list;
	hxtest_list_node_t a(1), b(2), c(3);
	list.push_back(&a);
	list.push_back(&b);
	list.push_back(&c);
	EXPECT_EQ(list.pop_front()->value, 1);
	EXPECT_EQ(list.size(), 2u);
	EXPECT_EQ(list.front().value, 2);
	EXPECT_EQ(list.pop_back()->value, 3);
	EXPECT_EQ(list.size(), 1u);
	EXPECT_EQ(list.pop_front()->value, 2);
	EXPECT_TRUE(list.empty());
}

// insert the front places new node first.
TEST(hxlist_test, insert_front) {
	hxtest_list_node_t a(2), b(3), c(1);
	hxlist<hxtest_list_node_t, hxdo_not_delete> list;
	list.push_back(&a);
	list.push_back(&b);
	const hxlist<hxtest_list_node_t, hxdo_not_delete>::iterator ic = list.insert(list.begin(), &c);
	EXPECT_EQ(list.size(), 3u);
	EXPECT_EQ(&list.front(), &c);
	EXPECT_EQ(&*ic, &c);
	EXPECT_EQ(ic, list.begin());
	int expected = 1;
	for(const hxtest_list_node_t& n : list) {
		EXPECT_EQ(n.value, expected++);
	}
	list.release_all();
}

// insert a middle node places the new node between its neighbors.
TEST(hxlist_test, insert_middle) {
	hxlist<hxtest_list_node_t, hxdo_not_delete> list;
	hxtest_list_node_t a(1), b(3), mid(2);
	list.push_back(&a);
	list.push_back(&b);
	hxlist<hxtest_list_node_t, hxdo_not_delete>::iterator it = list.begin();
	++it;
	const hxlist<hxtest_list_node_t, hxdo_not_delete>::iterator imid = list.insert(it, &mid);
	EXPECT_EQ(list.size(), 3u);
	EXPECT_EQ(&*imid, &mid);
	// imid advanced past a, so decrementing reaches a and incrementing reaches b.
	hxlist<hxtest_list_node_t, hxdo_not_delete>::iterator prev = imid;
	--prev;
	EXPECT_EQ(&*prev, &a);
	hxlist<hxtest_list_node_t, hxdo_not_delete>::iterator next = imid;
	++next;
	EXPECT_EQ(&*next, &b);
	int expected = 1;
	for(const hxtest_list_node_t& n : list) {
		EXPECT_EQ(n.value, expected++);
	}
	EXPECT_EQ(expected, 4);
	list.release_all();
}

// insert_after the back places new node at the tail.
TEST(hxlist_test, insert_after_back) {
	hxtest_list_node_t a(1), b(2), c(3);
	hxlist<hxtest_list_node_t, hxdo_not_delete> list;
	list.push_back(&a);
	list.push_back(&b);
	hxlist<hxtest_list_node_t, hxdo_not_delete>::iterator it = list.end();
	--it;
	const hxlist<hxtest_list_node_t, hxdo_not_delete>::iterator ic = list.insert_after(it, &c);
	EXPECT_EQ(list.size(), 3u);
	EXPECT_EQ(&list.back(), &c);
	EXPECT_EQ(&*ic, &c);
	hxlist<hxtest_list_node_t, hxdo_not_delete>::iterator next = ic;
	++next;
	EXPECT_EQ(next, list.end());
	int expected = 1;
	for(const hxtest_list_node_t& n : list) {
		EXPECT_EQ(n.value, expected++);
	}
	list.release_all();
}

// insert_after a middle node places the new node between its neighbors.
TEST(hxlist_test, insert_after_middle) {
	hxtest_list_node_t a(1), b(3), c(2);
	hxlist<hxtest_list_node_t, hxdo_not_delete> list;
	list.push_back(&a);
	list.push_back(&b);
	const hxlist<hxtest_list_node_t, hxdo_not_delete>::iterator it = list.begin();
	const hxlist<hxtest_list_node_t, hxdo_not_delete>::iterator ic = list.insert_after(it, &c);
	EXPECT_EQ(list.size(), 3u);
	EXPECT_EQ(&*ic, &c);
	hxlist<hxtest_list_node_t, hxdo_not_delete>::iterator prev = ic;
	--prev;
	EXPECT_EQ(&*prev, &a);
	hxlist<hxtest_list_node_t, hxdo_not_delete>::iterator next = ic;
	++next;
	EXPECT_EQ(&*next, &b);
	int expected = 1;
	for(const hxtest_list_node_t& n : list) {
		EXPECT_EQ(n.value, expected++);
	}
	list.release_all();
}

// extract the only node leaves list empty and returns an hxptr owning that node.
TEST(hxlist_test, extract_single_node) {
	hxtest_list_node_t a(1);
	hxlist<hxtest_list_node_t, hxdo_not_delete> list;
	list.push_back(&a);
	EXPECT_EQ(list.extract(list.begin()).get(), &a);
	EXPECT_TRUE(list.empty());
}

// extract front, back, and middle nodes verify neighbor relinking in all cases.
TEST(hxlist_test, extract_and_erase) {
	hxlist<hxtest_list_node_t, hxdo_not_delete> list;
	hxtest_list_node_t a(1), b(2), c(3);
	list.push_back(&a);
	list.push_back(&b);
	list.push_back(&c);
	hxlist<hxtest_list_node_t, hxdo_not_delete>::iterator mid = list.begin();
	++mid;
	list.extract(mid);
	EXPECT_EQ(list.size(), 2u);
	EXPECT_EQ(list.front().value, 1);
	EXPECT_EQ(list.back().value, 3);
	list.extract(list.begin());
	EXPECT_EQ(list.size(), 1u);
	EXPECT_EQ(list.front().value, 3);
	list.extract(list.begin());
	EXPECT_TRUE(list.empty());
}

// erase with hxdefault_delete calls destructor on the erased node and on clear.
TEST(hxlist_test, erase_with_default_deleter_calls_destructor) {
	const hxsystem_allocator_scope scope(hxsystem_allocator_temporary_stack);
	hxs_list_test_destructor_count = 0;
	hxlist<hxtest_list_counted_node_t> list;
	for(int i = 1; i <= 3; ++i) {
		hxptr<hxtest_list_counted_node_t> p(hxnew<hxtest_list_counted_node_t>(i));
		list.push_back(p.release());
	}
	list.erase(list.begin());
	EXPECT_EQ(hxs_list_test_destructor_count, 1);
	EXPECT_EQ(list.size(), 2u);
	EXPECT_EQ(list.front().value, 2);
	list.clear();
	EXPECT_EQ(hxs_list_test_destructor_count, 3);
}

// erase with hxdo_not_delete override does not call destructor.
TEST(hxlist_test, erase_with_do_not_delete_override) {
	hxs_list_test_destructor_count = 0;
	hxtest_list_counted_node_t a(1);
	hxlist<hxtest_list_counted_node_t, hxdo_not_delete> list;
	list.push_back(&a);
	list.erase(list.begin());
	EXPECT_EQ(hxs_list_test_destructor_count, 0);
	EXPECT_TRUE(list.empty());
}

// erase with a custom deleter override is called exactly once per node.
TEST(hxlist_test, erase_with_custom_deleter_override) {
	const hxsystem_allocator_scope scope(hxsystem_allocator_temporary_stack);
	hxs_test_custom_deleter_count = 0;
	hxptr<hxtest_list_counted_node_t> n(hxnew<hxtest_list_counted_node_t>(42));
	hxlist<hxtest_list_counted_node_t, hxdo_not_delete> list;
	list.push_back(n.release());
	list.erase(list.begin(), hxtest_list_custom_deleter_t());
	EXPECT_EQ(hxs_test_custom_deleter_count, 1);
	EXPECT_TRUE(list.empty());
}

// clear on an empty list is a no-op (m_size != 0 branch not taken).
TEST(hxlist_test, clear_empty_list) {
	hxlist<hxtest_list_node_t, hxdo_not_delete> list;
	list.clear();
	EXPECT_TRUE(list.empty());
	EXPECT_EQ(list.size(), 0u);
}

// clear with hxdo_not_delete override unlinks nodes without calling destructor.
TEST(hxlist_test, clear_with_do_not_delete_override) {
	hxs_list_test_destructor_count = 0;
	hxtest_list_counted_node_t a(1), b(2);
	hxlist<hxtest_list_counted_node_t, hxdo_not_delete> list;
	list.push_back(&a);
	list.push_back(&b);
	list.clear(hxdo_not_delete());
	EXPECT_EQ(hxs_list_test_destructor_count, 0);
	EXPECT_TRUE(list.empty());
	EXPECT_EQ(list.size(), 0u);
}

// clear with custom deleter override is called for each node.
TEST(hxlist_test, clear_with_custom_deleter_override) {
	const hxsystem_allocator_scope scope(hxsystem_allocator_temporary_stack);
	hxs_test_custom_deleter_count = 0;
	hxlist<hxtest_list_counted_node_t, hxdo_not_delete> list;
	for(int i = 1; i <= 2; ++i) {
		hxptr<hxtest_list_counted_node_t> p(hxnew<hxtest_list_counted_node_t>(i));
		list.push_back(p.release());
	}
	list.clear(hxtest_list_custom_deleter_t());
	EXPECT_EQ(hxs_test_custom_deleter_count, 2);
	EXPECT_TRUE(list.empty());
}

// release_all resets size and sentinel linkage without invoking deleter.
TEST(hxlist_test, release_all_resets_state) {
	hxtest_list_node_t a(1), b(2);
	hxlist<hxtest_list_node_t, hxdo_not_delete> list;
	list.push_back(&a);
	list.push_back(&b);
	list.release_all();
	EXPECT_TRUE(list.empty());
	EXPECT_EQ(list.size(), 0u);
	EXPECT_EQ(list.begin(), list.end());
}

// The list can be reused after release_all.
TEST(hxlist_test, reuse_after_release_all) {
	hxtest_list_node_t a(1), b(2), c(3);
	hxlist<hxtest_list_node_t, hxdo_not_delete> list;
	list.push_back(&a);
	list.push_back(&b);
	list.release_all();
	list.push_back(&c);
	EXPECT_EQ(list.size(), 1u);
	EXPECT_EQ(list.front().value, 3);
	list.release_all();
}

// Destructor calls clear() which invokes deleter on remaining nodes.
TEST(hxlist_test, destructor_calls_clear) {
	const hxsystem_allocator_scope scope(hxsystem_allocator_temporary_stack);
	hxs_list_test_destructor_count = 0;
	{
		hxlist<hxtest_list_counted_node_t> list;
		for(int i = 1; i <= 2; ++i) {
			hxptr<hxtest_list_counted_node_t> p(hxnew<hxtest_list_counted_node_t>(i));
			list.push_back(p.release());
		}
	}
	EXPECT_EQ(hxs_list_test_destructor_count, 2);
}

// Pre-increment steps through all nodes and reaches end() exactly.
TEST(hxlist_test, iterator_pre_increment) {
	hxtest_list_node_t a(1), b(2), c(3);
	hxlist<hxtest_list_node_t, hxdo_not_delete> list;
	list.push_back(&a);
	list.push_back(&b);
	list.push_back(&c);
	hxlist<hxtest_list_node_t, hxdo_not_delete>::iterator it = list.begin();
	EXPECT_EQ(it->value, 1);
	++it;
	EXPECT_EQ(it->value, 2);
	++it;
	EXPECT_EQ(it->value, 3);
	++it;
	EXPECT_EQ(it, list.end());
	list.release_all();
}

// Post-increment returns old position and advances iterator.
TEST(hxlist_test, iterator_post_increment) {
	hxtest_list_node_t a(1), b(2);
	hxlist<hxtest_list_node_t, hxdo_not_delete> list;
	list.push_back(&a);
	list.push_back(&b);
	hxlist<hxtest_list_node_t, hxdo_not_delete>::iterator it = list.begin();
	const hxlist<hxtest_list_node_t, hxdo_not_delete>::iterator old = it++;
	EXPECT_EQ(old->value, 1);
	EXPECT_EQ(it->value, 2);
	list.release_all();
}

// Pre-decrement steps backward through all nodes and reaches begin() exactly.
TEST(hxlist_test, iterator_pre_decrement) {
	hxtest_list_node_t a(1), b(2), c(3);
	hxlist<hxtest_list_node_t, hxdo_not_delete> list;
	list.push_back(&a);
	list.push_back(&b);
	list.push_back(&c);
	hxlist<hxtest_list_node_t, hxdo_not_delete>::iterator it = list.end();
	--it;
	EXPECT_EQ(it->value, 3);
	--it;
	EXPECT_EQ(it->value, 2);
	--it;
	EXPECT_EQ(it->value, 1);
	EXPECT_EQ(it, list.begin());
	list.release_all();
}

// Post-decrement returns old position and retreats iterator.
TEST(hxlist_test, iterator_post_decrement) {
	hxtest_list_node_t a(1), b(2);
	hxlist<hxtest_list_node_t, hxdo_not_delete> list;
	list.push_back(&a);
	list.push_back(&b);
	hxlist<hxtest_list_node_t, hxdo_not_delete>::iterator it = list.end();
	const hxlist<hxtest_list_node_t, hxdo_not_delete>::iterator old = it--;
	EXPECT_EQ(old, list.end());
	EXPECT_EQ(it->value, 2);
	list.release_all();
}

// Iterator equality and inequality operators.
TEST(hxlist_test, iterator_equality) {
	hxtest_list_node_t a(1), b(2);
	hxlist<hxtest_list_node_t, hxdo_not_delete> list;
	list.push_back(&a);
	list.push_back(&b);
	const hxlist<hxtest_list_node_t, hxdo_not_delete>::iterator it1 = list.begin();
	hxlist<hxtest_list_node_t, hxdo_not_delete>::iterator it2 = list.begin();
	EXPECT_TRUE(it1 == it2);
	EXPECT_FALSE(it1 != it2);
	++it2;
	EXPECT_FALSE(it1 == it2);
	EXPECT_TRUE(it1 != it2);
	list.release_all();
}

// Arrow operator on mutable iterator returns mutable pointer.
TEST(hxlist_test, iterator_arrow_operator) {
	hxtest_list_node_t a(1);
	hxlist<hxtest_list_node_t, hxdo_not_delete> list;
	list.push_back(&a);
	const hxlist<hxtest_list_node_t, hxdo_not_delete>::iterator it = list.begin();
	it->value = 99;
	EXPECT_EQ(a.value, 99);
	list.release_all();
}

// Post-increment and post-decrement on const_iterator return the prior position.
TEST(hxlist_test, const_iterator_post_increment_and_decrement) {
	hxtest_list_node_t a(1), b(2);
	hxlist<hxtest_list_node_t, hxdo_not_delete> list;
	list.push_back(&a);
	list.push_back(&b);
	hxlist<hxtest_list_node_t, hxdo_not_delete>::const_iterator it = list.begin();
	hxlist<hxtest_list_node_t, hxdo_not_delete>::const_iterator old = it++;
	EXPECT_EQ(old->value, 1);
	EXPECT_EQ(it->value, 2);
	old = it--;
	EXPECT_EQ(old->value, 2);
	EXPECT_EQ(it->value, 1);
	list.release_all();
}

// Copy/move construction produces a fresh unlinked node; copy/move assignment
// leaves the destination's linkage unchanged. All four require an unlinked source.
TEST(hxlist_node_test, copy_move_construct_and_assign) {
	const hxtest_list_node_t a(1), b(2), c(3), d(4);
	hxlist<hxtest_list_node_t, hxdo_not_delete> list;
	// copy-construct: b is unlinked, gets a fresh node linkable into a list.
	hxtest_list_node_t e(a);
	list.push_back(&e);
	// move-construct: produces a fresh unlinked node.
	hxtest_list_node_t f(hxmove(b));
	list.push_back(&f);
	EXPECT_EQ(list.size(), 2u);
	// copy-assign: destination e stays linked, source c is unchanged.
	e = c;
	EXPECT_EQ(&*list.begin(), &e);
	// move-assign: destination f stays linked.
	f = hxmove(d);
	EXPECT_EQ(&list.back(), &f);
	EXPECT_EQ(list.size(), 2u);
	list.release_all();
}

// remove_if: empty list returns 0; removes front, middle, and back nodes
// (off-by-one positions); predicate false leaves list unchanged; deleter
// is called for each removed node.
TEST(hxlist_test, remove_if) {
	const hxsystem_allocator_scope scope(hxsystem_allocator_temporary_stack);
	hxs_list_test_destructor_count = 0;
	// Empty list: loop body never entered, returns 0.
	{
		hxlist<hxtest_list_counted_node_t> list;
		EXPECT_EQ(list.remove_if([](hxtest_list_counted_node_t&) { return true; }), 0u);
	}
	// Predicate always false: no nodes removed, size unchanged.
	{
		hxlist<hxtest_list_counted_node_t> list;
		list.push_back(hxnew<hxtest_list_counted_node_t>(1));
		list.push_back(hxnew<hxtest_list_counted_node_t>(2));
		EXPECT_EQ(list.remove_if([](hxtest_list_counted_node_t&) { return false; }), 0u);
		EXPECT_EQ(list.size(), 2u);
		list.clear();
	}
	hxs_list_test_destructor_count = 0;
	// Remove front and back (off-by-one), keep middle; deleter called twice.
	{
		hxlist<hxtest_list_counted_node_t> list;
		list.push_back(hxnew<hxtest_list_counted_node_t>(1));
		list.push_back(hxnew<hxtest_list_counted_node_t>(2));
		list.push_back(hxnew<hxtest_list_counted_node_t>(3));
		const size_t count = list.remove_if([](hxtest_list_counted_node_t& n) {
			return n.value != 2;
		});
		EXPECT_EQ(count, 2u);
		EXPECT_EQ(hxs_list_test_destructor_count, 2);
		EXPECT_EQ(list.size(), 1u);
		EXPECT_EQ(list.front().value, 2);
		list.clear();
	}
	hxs_list_test_destructor_count = 0;
	// Remove middle, keep front and back; correct neighbors relinked.
	{
		hxlist<hxtest_list_counted_node_t> list;
		list.push_back(hxnew<hxtest_list_counted_node_t>(1));
		list.push_back(hxnew<hxtest_list_counted_node_t>(2));
		list.push_back(hxnew<hxtest_list_counted_node_t>(3));
		const size_t count = list.remove_if([](hxtest_list_counted_node_t& n) {
			return n.value == 2;
		});
		EXPECT_EQ(count, 1u);
		EXPECT_EQ(hxs_list_test_destructor_count, 1);
		EXPECT_EQ(list.front().value, 1);
		EXPECT_EQ(list.back().value, 3);
		list.clear();
	}
}

// reverse: empty list (sentinel-only loop), one node, two nodes (off-by-one
// swap), three nodes (full reversal verified by forward and backward traversal).
TEST(hxlist_test, reverse) {
	// Empty: do-while visits only the sentinel and terminates immediately.
	{
		hxlist<hxtest_list_node_t, hxdo_not_delete> list;
		list.reverse();
		EXPECT_TRUE(list.empty());
	}
	// One node: front and back unchanged after reverse.
	{
		hxtest_list_node_t a(1);
		hxlist<hxtest_list_node_t, hxdo_not_delete> list;
		list.push_back(&a);
		list.reverse();
		EXPECT_EQ(list.front().value, 1);
		EXPECT_EQ(list.back().value, 1);
		list.release_all();
	}
	// Two nodes: front and back swap (off-by-one boundary).
	{
		hxtest_list_node_t a(1), b(2);
		hxlist<hxtest_list_node_t, hxdo_not_delete> list;
		list.push_back(&a);
		list.push_back(&b);
		list.reverse();
		EXPECT_EQ(list.front().value, 2);
		EXPECT_EQ(list.back().value, 1);
		list.release_all();
	}
	// Three nodes: forward order is 3,2,1; decrement from end also correct.
	{
		hxtest_list_node_t a(1), b(2), c(3);
		hxlist<hxtest_list_node_t, hxdo_not_delete> list;
		list.push_back(&a);
		list.push_back(&b);
		list.push_back(&c);
		list.reverse();
		EXPECT_EQ(list.size(), 3u);
		hxlist<hxtest_list_node_t, hxdo_not_delete>::iterator it = list.begin();
		EXPECT_EQ((it++)->value, 3);
		EXPECT_EQ((it++)->value, 2);
		EXPECT_EQ((it++)->value, 1);
		EXPECT_EQ(it, list.end());
		--it;
		EXPECT_EQ(it->value, 1);
		list.release_all();
	}
}

// splice: empty source is a no-op; splice at end(), begin(), and a middle
// iterator (off-by-one positions); source is always left empty.
TEST(hxlist_test, splice) {
	// Empty source: no change to destination.
	{
		hxtest_list_node_t a(1);
		hxlist<hxtest_list_node_t, hxdo_not_delete> dst;
		hxlist<hxtest_list_node_t, hxdo_not_delete> src;
		dst.push_back(&a);
		dst.splice(dst.end(), src);
		EXPECT_EQ(dst.size(), 1u);
		EXPECT_TRUE(src.empty());
		dst.release_all();
	}
	// Splice at end(): source nodes appended after all existing nodes.
	{
		hxtest_list_node_t a(1), b(2), c(3), d(4);
		hxlist<hxtest_list_node_t, hxdo_not_delete> dst;
		hxlist<hxtest_list_node_t, hxdo_not_delete> src;
		dst.push_back(&a);
		dst.push_back(&b);
		src.push_back(&c);
		src.push_back(&d);
		dst.splice(dst.end(), src);
		EXPECT_EQ(dst.size(), 4u);
		EXPECT_TRUE(src.empty());
		int expected = 1;
		for(const hxtest_list_node_t& n : dst) {
			EXPECT_EQ(n.value, expected++);
		}
		EXPECT_EQ(expected, 5);
		dst.release_all();
	}
	// Splice at begin(): source nodes prepended before all existing nodes.
	{
		hxtest_list_node_t a(3), b(4), c(1), d(2);
		hxlist<hxtest_list_node_t, hxdo_not_delete> dst;
		hxlist<hxtest_list_node_t, hxdo_not_delete> src;
		dst.push_back(&a);
		dst.push_back(&b);
		src.push_back(&c);
		src.push_back(&d);
		dst.splice(dst.begin(), src);
		EXPECT_EQ(dst.size(), 4u);
		EXPECT_TRUE(src.empty());
		int expected = 1;
		for(const hxtest_list_node_t& n : dst) {
			EXPECT_EQ(n.value, expected++);
		}
		EXPECT_EQ(expected, 5);
		dst.release_all();
	}
	// Splice before last node (middle iterator): source inserted between neighbors.
	{
		hxtest_list_node_t a(1), b(4), c(2), d(3);
		hxlist<hxtest_list_node_t, hxdo_not_delete> dst;
		hxlist<hxtest_list_node_t, hxdo_not_delete> src;
		dst.push_back(&a);
		dst.push_back(&b);
		src.push_back(&c);
		src.push_back(&d);
		hxlist<hxtest_list_node_t, hxdo_not_delete>::iterator it = dst.end();
		--it;
		dst.splice(it, src);
		EXPECT_EQ(dst.size(), 4u);
		EXPECT_TRUE(src.empty());
		int expected = 1;
		for(const hxtest_list_node_t& n : dst) {
			EXPECT_EQ(n.value, expected++);
		}
		EXPECT_EQ(expected, 5);
		dst.release_all();
	}
}

// Backward iteration from --end() to --begin() visits every node in reverse
// and --begin() wraps to end().
TEST(hxlist_test, reverse_iteration_end_to_begin) {
	hxtest_list_node_t a(1), b(2), c(3);
	hxlist<hxtest_list_node_t, hxdo_not_delete> list;
	list.push_back(&a);
	list.push_back(&b);
	list.push_back(&c);
	hxlist<hxtest_list_node_t, hxdo_not_delete>::iterator it = list.end();
	--it;
	EXPECT_EQ(it->value, 3);
	--it;
	EXPECT_EQ(it->value, 2);
	--it;
	EXPECT_EQ(it->value, 1);
	EXPECT_EQ(it, list.begin());
	--it;
	EXPECT_EQ(it, list.end());
	list.release_all();
}

// Decrementing end() reaches the last node; decrementing begin() reaches end().
TEST(hxlist_test, decrement_end_and_begin) {
	hxtest_list_node_t a(1), b(2), c(3);
	hxlist<hxtest_list_node_t, hxdo_not_delete> list;
	list.push_back(&a);
	list.push_back(&b);
	list.push_back(&c);
	// --end() reaches the last node.
	hxlist<hxtest_list_node_t, hxdo_not_delete>::iterator it = list.end();
	--it;
	EXPECT_EQ(&*it, &c);
	// --begin() reaches end() (the sentinel, one before front).
	it = list.begin();
	--it;
	EXPECT_EQ(it, list.end());
	list.release_all();
}

// Post-decrement from end() returns end() and lands on the last node.
TEST(hxlist_test, post_decrement_from_end) {
	hxtest_list_node_t a(1), b(2);
	hxlist<hxtest_list_node_t, hxdo_not_delete> list;
	list.push_back(&a);
	list.push_back(&b);
	hxlist<hxtest_list_node_t, hxdo_not_delete>::iterator it = list.end();
	const hxlist<hxtest_list_node_t, hxdo_not_delete>::iterator old = it--;
	EXPECT_EQ(old, list.end());
	EXPECT_EQ(&*it, &b);
	// Post-decrement from begin() returns begin() and lands on end().
	it = list.begin();
	const hxlist<hxtest_list_node_t, hxdo_not_delete>::iterator old2 = it--;
	EXPECT_EQ(old2, list.begin());
	EXPECT_EQ(it, list.end());
	list.release_all();
}

// Interleaved push_front and push_back produce correct order.
TEST(hxlist_test, mixed_push_front_and_push_back) {
	hxtest_list_node_t a(1), b(4), c(2), d(3);
	hxlist<hxtest_list_node_t, hxdo_not_delete> list;
	list.push_back(&a);
	list.push_back(&b);
	list.push_front(&c); // front: c(2), a(1), b(4)
	// insert d(3) before b(4): iterator to b is at end() - 1.
	{
		hxlist<hxtest_list_node_t, hxdo_not_delete>::iterator it = list.end();
		--it;
		list.insert(it, &d); // front: c(2), a(1), d(3), b(4)
	}
	// Expected order by value: 2, 1, 3, 4.
	const int expected[] = { 2, 1, 3, 4 };
	int idx = 0;
	for(const hxtest_list_node_t& n : list) {
		EXPECT_EQ(n.value, expected[idx++]);
	}
	EXPECT_EQ(idx, 4);
	list.release_all();
}
