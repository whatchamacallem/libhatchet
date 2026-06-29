// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

namespace {

struct hxtest_list_node_t : hxlist_node {
	explicit hxtest_list_node_t(int v) : value(v) { }
	int value;
};
int hxs_list_test_destructor_count = 0;
struct hxtest_list_counted_node_t : hxlist_node {
	explicit hxtest_list_counted_node_t(int v) : value(v) { }
	~hxtest_list_counted_node_t(void) { ++hxs_list_test_destructor_count; }
	int value;
};
int hxs_test_custom_deleter_count = 0;
struct hxtest_list_custom_deleter_t {
	void operator()(hxtest_list_counted_node_t* ptr) const {
		++hxs_test_custom_deleter_count;
		hxdelete(ptr);
	}
	operator bool(void) const { return true; }
};
} // namespace

TEST(hxlist_test, empty_on_construction) {
	hxlist<hxtest_list_node_t, hxdo_not_delete> list;
	EXPECT_TRUE(list.empty());
	EXPECT_EQ(list.size(), 0);
	EXPECT_EQ(list.begin(), list.end());
}

TEST(hxlist_test, default_constructed_iterators) {
	hxtest_list_node_t a(1);
	hxlist<hxtest_list_node_t, hxdo_not_delete> list;
	list.push_back(&a);
	const hxlist<hxtest_list_node_t, hxdo_not_delete>::const_iterator cd1;
	const hxlist<hxtest_list_node_t, hxdo_not_delete>::const_iterator cd2;
	EXPECT_TRUE(cd1 == cd2);
	EXPECT_FALSE(cd1 == list.begin());
	hxlist<hxtest_list_node_t, hxdo_not_delete>::iterator d1;
	const hxlist<hxtest_list_node_t, hxdo_not_delete>::iterator d2;
	EXPECT_TRUE(d1 == d2);
	EXPECT_FALSE(d1 == list.begin());
	d1 = list.begin();
	EXPECT_TRUE(d1 == list.begin());
	EXPECT_EQ(d1->value, 1);
	list.release_all();
}

TEST(hxlist_test, push_back_and_iterate) {
	hxlist<hxtest_list_node_t, hxdo_not_delete> list;
	hxtest_list_node_t a(1), b(2), c(3);
	const hxlist<hxtest_list_node_t, hxdo_not_delete>::iterator ia = list.push_back(&a);
	const hxlist<hxtest_list_node_t, hxdo_not_delete>::iterator ib = list.push_back(&b);
	const hxlist<hxtest_list_node_t, hxdo_not_delete>::iterator ic = list.push_back(&c);
	hxtest_gdb_break_hxlist();
	EXPECT_EQ(list.size(), 3);
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

TEST(hxlist_test, const_front_and_back) {
	hxtest_list_node_t a(10), b(20);
	hxlist<hxtest_list_node_t, hxdo_not_delete> list;
	list.push_back(&a);
	list.push_back(&b);
	const hxlist<hxtest_list_node_t, hxdo_not_delete>& clist = list;
	EXPECT_EQ(&clist.front(), &a);
	EXPECT_EQ(&clist.back(), &b);
	EXPECT_EQ(clist.front().value, 10);
	EXPECT_EQ(clist.back().value, 20);
	list.release_all();
}

TEST(hxlist_test, const_begin_end_iteration) {
	hxtest_list_node_t a(1), b(2), c(3);
	hxlist<hxtest_list_node_t, hxdo_not_delete> list;
	list.push_back(&a);
	list.push_back(&b);
	list.push_back(&c);
	const hxlist<hxtest_list_node_t, hxdo_not_delete>& clist = list;
	int expected = 1;
	hxsize_t count = 0;
	for(hxlist<hxtest_list_node_t, hxdo_not_delete>::const_iterator it = clist.begin();
			it != clist.end(); ++it) {
		EXPECT_EQ(it->value, expected++);
		++count;
	}
	EXPECT_EQ(count, 3);
	EXPECT_EQ(expected, 4);
	list.release_all();
}

TEST(hxlist_test, cbegin_cend_iteration) {
	hxtest_list_node_t a(1), b(2), c(3);
	hxlist<hxtest_list_node_t, hxdo_not_delete> list;
	list.push_back(&a);
	list.push_back(&b);
	list.push_back(&c);
	const hxlist<hxtest_list_node_t, hxdo_not_delete>& clist = list;
	EXPECT_EQ(clist.cbegin()->value, 1);
	EXPECT_TRUE(clist.cbegin() != clist.cend());
	EXPECT_TRUE(clist.cend() == clist.end());
	EXPECT_TRUE(clist.cbegin() == clist.begin());
	int expected = 1;
	hxsize_t count = 0;
	for(hxlist<hxtest_list_node_t, hxdo_not_delete>::const_iterator it = clist.cbegin();
			it != clist.cend(); ++it) {
		EXPECT_EQ(it->value, expected++);
		++count;
	}
	EXPECT_EQ(count, 3);
	EXPECT_EQ(expected, 4);
	list.release_all();
}

TEST(hxlist_test, push_front_and_iterate) {
	hxlist<hxtest_list_node_t, hxdo_not_delete> list;
	hxtest_list_node_t a(1), b(2), c(3);
	const hxlist<hxtest_list_node_t, hxdo_not_delete>::iterator ic = list.push_front(&c);
	const hxlist<hxtest_list_node_t, hxdo_not_delete>::iterator ib = list.push_front(&b);
	const hxlist<hxtest_list_node_t, hxdo_not_delete>::iterator ia = list.push_front(&a);
	EXPECT_EQ(list.size(), 3);
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

TEST(hxlist_test, pop_front_and_pop_back) {
	hxlist<hxtest_list_node_t, hxdo_not_delete> list;
	hxtest_list_node_t a(1), b(2), c(3);
	list.push_back(&a);
	list.push_back(&b);
	list.push_back(&c);
	EXPECT_EQ(list.pop_front()->value, 1);
	EXPECT_EQ(list.size(), 2);
	EXPECT_EQ(list.front().value, 2);
	EXPECT_EQ(list.pop_back()->value, 3);
	EXPECT_EQ(list.size(), 1);
	EXPECT_EQ(list.pop_front()->value, 2);
	EXPECT_TRUE(list.empty());
}

TEST(hxlist_test, insert_front) {
	hxtest_list_node_t a(2), b(3), c(1);
	hxlist<hxtest_list_node_t, hxdo_not_delete> list;
	list.push_back(&a);
	list.push_back(&b);
	const hxlist<hxtest_list_node_t, hxdo_not_delete>::iterator ic = list.insert(list.begin(), &c);
	EXPECT_EQ(list.size(), 3);
	EXPECT_EQ(&list.front(), &c);
	EXPECT_EQ(&*ic, &c);
	EXPECT_EQ(ic, list.begin());
	int expected = 1;
	for(const hxtest_list_node_t& n : list) {
		EXPECT_EQ(n.value, expected++);
	}
	list.release_all();
}

TEST(hxlist_test, insert_middle) {
	hxlist<hxtest_list_node_t, hxdo_not_delete> list;
	hxtest_list_node_t a(1), b(3), mid(2);
	list.push_back(&a);
	list.push_back(&b);
	hxlist<hxtest_list_node_t, hxdo_not_delete>::iterator it = list.begin();
	++it;
	const hxlist<hxtest_list_node_t, hxdo_not_delete>::iterator imid = list.insert(it, &mid);
	EXPECT_EQ(list.size(), 3);
	EXPECT_EQ(&*imid, &mid);
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

TEST(hxlist_test, insert_after_back) {
	hxtest_list_node_t a(1), b(2), c(3);
	hxlist<hxtest_list_node_t, hxdo_not_delete> list;
	list.push_back(&a);
	list.push_back(&b);
	hxlist<hxtest_list_node_t, hxdo_not_delete>::iterator it = list.end();
	--it;
	const hxlist<hxtest_list_node_t, hxdo_not_delete>::iterator ic = list.insert_after(it, &c);
	EXPECT_EQ(list.size(), 3);
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

TEST(hxlist_test, insert_after_middle) {
	hxtest_list_node_t a(1), b(3), c(2);
	hxlist<hxtest_list_node_t, hxdo_not_delete> list;
	list.push_back(&a);
	list.push_back(&b);
	const hxlist<hxtest_list_node_t, hxdo_not_delete>::iterator it = list.begin();
	const hxlist<hxtest_list_node_t, hxdo_not_delete>::iterator ic = list.insert_after(it, &c);
	EXPECT_EQ(list.size(), 3);
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

TEST(hxlist_test, extract_single_node) {
	hxtest_list_node_t a(1);
	hxlist<hxtest_list_node_t, hxdo_not_delete> list;
	list.push_back(&a);
	EXPECT_EQ(list.extract(list.begin()).get(), &a);
	EXPECT_TRUE(list.empty());
}

TEST(hxlist_test, extract_and_erase) {
	hxlist<hxtest_list_node_t, hxdo_not_delete> list;
	hxtest_list_node_t a(1), b(2), c(3);
	list.push_back(&a);
	list.push_back(&b);
	list.push_back(&c);
	hxlist<hxtest_list_node_t, hxdo_not_delete>::iterator mid = list.begin();
	++mid;
	list.extract(mid);
	EXPECT_EQ(list.size(), 2);
	EXPECT_EQ(list.front().value, 1);
	EXPECT_EQ(list.back().value, 3);
	list.extract(list.begin());
	EXPECT_EQ(list.size(), 1);
	EXPECT_EQ(list.front().value, 3);
	list.extract(list.begin());
	EXPECT_TRUE(list.empty());
}

TEST(hxlist_test, erase_with_default_deleter_calls_destructor) {
	const hxsystem_allocator_scope scope(hxsystem_allocator_stack_0);
	hxs_list_test_destructor_count = 0;
	hxlist<hxtest_list_counted_node_t> list;
	for(int i = 1; i <= 3; ++i) {
		hxptr<hxtest_list_counted_node_t> p(hxnew<hxtest_list_counted_node_t>(i));
		list.push_back(p.release());
	}
	list.erase(list.begin());
	EXPECT_EQ(hxs_list_test_destructor_count, 1);
	EXPECT_EQ(list.size(), 2);
	EXPECT_EQ(list.front().value, 2);
	list.clear();
	EXPECT_EQ(hxs_list_test_destructor_count, 3);
}

TEST(hxlist_test, insert_overloads_taking_hxptr_rvalue) {
	const hxsystem_allocator_scope scope(hxsystem_allocator_stack_0);
	hxs_list_test_destructor_count = 0;
	hxlist<hxtest_list_counted_node_t> list;
	list.push_back(hxptr<hxtest_list_counted_node_t>(hxnew<hxtest_list_counted_node_t>(2)));
	list.push_front(hxptr<hxtest_list_counted_node_t>(hxnew<hxtest_list_counted_node_t>(1)));
	list.insert(--list.end(),
		hxptr<hxtest_list_counted_node_t>(hxnew<hxtest_list_counted_node_t>(3)));
	list.insert_after(list.begin(),
		hxptr<hxtest_list_counted_node_t>(hxnew<hxtest_list_counted_node_t>(4)));
	const int expected[] = { 1, 4, 3, 2 };
	hxsize_t idx = 0;
	for(const hxtest_list_counted_node_t& n : list) {
		EXPECT_EQ(n.value, expected[idx++]);
	}
	EXPECT_EQ(idx, 4);
	EXPECT_EQ(list.size(), 4);
	list.clear();
	EXPECT_EQ(hxs_list_test_destructor_count, 4);
}

TEST(hxlist_test, erase_with_do_not_delete_override) {
	hxs_list_test_destructor_count = 0;
	hxtest_list_counted_node_t a(1);
	hxlist<hxtest_list_counted_node_t, hxdo_not_delete> list;
	list.push_back(&a);
	list.erase(list.begin());
	EXPECT_EQ(hxs_list_test_destructor_count, 0);
	EXPECT_TRUE(list.empty());
}

TEST(hxlist_test, erase_with_custom_deleter_override) {
	const hxsystem_allocator_scope scope(hxsystem_allocator_stack_0);
	hxs_test_custom_deleter_count = 0;
	hxptr<hxtest_list_counted_node_t> n(hxnew<hxtest_list_counted_node_t>(34));
	hxlist<hxtest_list_counted_node_t, hxdo_not_delete> list;
	list.push_back(n.release());
	list.erase(list.begin(), hxtest_list_custom_deleter_t());
	EXPECT_EQ(hxs_test_custom_deleter_count, 1);
	EXPECT_TRUE(list.empty());
}

TEST(hxlist_test, clear_empty_list) {
	hxlist<hxtest_list_node_t, hxdo_not_delete> list;
	list.clear();
	EXPECT_TRUE(list.empty());
	EXPECT_EQ(list.size(), 0);
}

TEST(hxlist_test, clear_with_do_not_delete_override) {
	hxs_list_test_destructor_count = 0;
	hxtest_list_counted_node_t a(1), b(2);
	hxlist<hxtest_list_counted_node_t, hxdo_not_delete> list;
	list.push_back(&a);
	list.push_back(&b);
	list.clear(hxdo_not_delete());
	EXPECT_EQ(hxs_list_test_destructor_count, 0);
	EXPECT_TRUE(list.empty());
	EXPECT_EQ(list.size(), 0);
}

TEST(hxlist_test, clear_with_custom_deleter_override) {
	const hxsystem_allocator_scope scope(hxsystem_allocator_stack_0);
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

TEST(hxlist_test, release_all_resets_state) {
	hxtest_list_node_t a(1), b(2);
	hxlist<hxtest_list_node_t, hxdo_not_delete> list;
	list.push_back(&a);
	list.push_back(&b);
	list.release_all();
	EXPECT_TRUE(list.empty());
	EXPECT_EQ(list.size(), 0);
	EXPECT_EQ(list.begin(), list.end());
}

TEST(hxlist_test, reuse_after_release_all) {
	hxtest_list_node_t a(1), b(2), c(3);
	hxlist<hxtest_list_node_t, hxdo_not_delete> list;
	list.push_back(&a);
	list.push_back(&b);
	list.release_all();
	list.push_back(&c);
	EXPECT_EQ(list.size(), 1);
	EXPECT_EQ(list.front().value, 3);
	list.release_all();
}

TEST(hxlist_test, destructor_calls_clear) {
	const hxsystem_allocator_scope scope(hxsystem_allocator_stack_0);
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

TEST(hxlist_test, iterator_arrow_operator) {
	hxtest_list_node_t a(1);
	hxlist<hxtest_list_node_t, hxdo_not_delete> list;
	list.push_back(&a);
	const hxlist<hxtest_list_node_t, hxdo_not_delete>::iterator it = list.begin();
	it->value = 99;
	EXPECT_EQ(a.value, 99);
	list.release_all();
}

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

TEST(hxlist_node_test, copy_move_construct_and_assign) {
	const hxtest_list_node_t a(1), b(2), c(3), d(4);
	hxlist<hxtest_list_node_t, hxdo_not_delete> list;
	hxtest_list_node_t e(a);
	list.push_back(&e);
	hxtest_list_node_t f(hxmove(b));
	list.push_back(&f);
	EXPECT_EQ(list.size(), 2);
	e = c;
	EXPECT_EQ(&*list.begin(), &e);
	f = hxmove(d);
	EXPECT_EQ(&list.back(), &f);
	EXPECT_EQ(list.size(), 2);
	list.release_all();
}

TEST(hxlist_test, remove_if) {
	const hxsystem_allocator_scope scope(hxsystem_allocator_stack_0);
	hxs_list_test_destructor_count = 0;
	{
		hxlist<hxtest_list_counted_node_t> list;
		EXPECT_EQ(list.remove_if([](hxtest_list_counted_node_t&) { return true; }), 0);
	}
	{
		hxlist<hxtest_list_counted_node_t> list;
		list.push_back(hxnew<hxtest_list_counted_node_t>(1));
		list.push_back(hxnew<hxtest_list_counted_node_t>(2));
		EXPECT_EQ(list.remove_if([](hxtest_list_counted_node_t&) { return false; }), 0);
		EXPECT_EQ(list.size(), 2);
		list.clear();
	}
	hxs_list_test_destructor_count = 0;
	{
		hxlist<hxtest_list_counted_node_t> list;
		list.push_back(hxnew<hxtest_list_counted_node_t>(1));
		list.push_back(hxnew<hxtest_list_counted_node_t>(2));
		list.push_back(hxnew<hxtest_list_counted_node_t>(3));
		const hxsize_t count = list.remove_if([](hxtest_list_counted_node_t& n) {
			return n.value != 2;
		});
		EXPECT_EQ(count, 2);
		EXPECT_EQ(hxs_list_test_destructor_count, 2);
		EXPECT_EQ(list.size(), 1);
		EXPECT_EQ(list.front().value, 2);
		list.clear();
	}
	hxs_list_test_destructor_count = 0;
	{
		hxlist<hxtest_list_counted_node_t> list;
		list.push_back(hxnew<hxtest_list_counted_node_t>(1));
		list.push_back(hxnew<hxtest_list_counted_node_t>(2));
		list.push_back(hxnew<hxtest_list_counted_node_t>(3));
		const hxsize_t count = list.remove_if([](hxtest_list_counted_node_t& n) {
			return n.value == 2;
		});
		EXPECT_EQ(count, 1);
		EXPECT_EQ(hxs_list_test_destructor_count, 1);
		EXPECT_EQ(list.front().value, 1);
		EXPECT_EQ(list.back().value, 3);
		list.clear();
	}
}

TEST(hxlist_test, reverse) {
	{
		hxlist<hxtest_list_node_t, hxdo_not_delete> list;
		list.reverse();
		EXPECT_TRUE(list.empty());
	}
	{
		hxtest_list_node_t a(1);
		hxlist<hxtest_list_node_t, hxdo_not_delete> list;
		list.push_back(&a);
		list.reverse();
		EXPECT_EQ(list.front().value, 1);
		EXPECT_EQ(list.back().value, 1);
		list.release_all();
	}
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
	{
		hxtest_list_node_t a(1), b(2), c(3);
		hxlist<hxtest_list_node_t, hxdo_not_delete> list;
		list.push_back(&a);
		list.push_back(&b);
		list.push_back(&c);
		list.reverse();
		EXPECT_EQ(list.size(), 3);
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

TEST(hxlist_test, splice) {
	{
		hxtest_list_node_t a(1);
		hxlist<hxtest_list_node_t, hxdo_not_delete> dst;
		hxlist<hxtest_list_node_t, hxdo_not_delete> src;
		dst.push_back(&a);
		dst.splice(dst.end(), src);
		EXPECT_EQ(dst.size(), 1);
		EXPECT_TRUE(src.empty());
		dst.release_all();
	}
	{
		hxtest_list_node_t a(1), b(2), c(3), d(4);
		hxlist<hxtest_list_node_t, hxdo_not_delete> dst;
		hxlist<hxtest_list_node_t, hxdo_not_delete> src;
		dst.push_back(&a);
		dst.push_back(&b);
		src.push_back(&c);
		src.push_back(&d);
		dst.splice(dst.end(), src);
		EXPECT_EQ(dst.size(), 4);
		EXPECT_TRUE(src.empty());
		int expected = 1;
		for(const hxtest_list_node_t& n : dst) {
			EXPECT_EQ(n.value, expected++);
		}
		EXPECT_EQ(expected, 5);
		dst.release_all();
	}
	{
		hxtest_list_node_t a(3), b(4), c(1), d(2);
		hxlist<hxtest_list_node_t, hxdo_not_delete> dst;
		hxlist<hxtest_list_node_t, hxdo_not_delete> src;
		dst.push_back(&a);
		dst.push_back(&b);
		src.push_back(&c);
		src.push_back(&d);
		dst.splice(dst.begin(), src);
		EXPECT_EQ(dst.size(), 4);
		EXPECT_TRUE(src.empty());
		int expected = 1;
		for(const hxtest_list_node_t& n : dst) {
			EXPECT_EQ(n.value, expected++);
		}
		EXPECT_EQ(expected, 5);
		dst.release_all();
	}
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
		EXPECT_EQ(dst.size(), 4);
		EXPECT_TRUE(src.empty());
		int expected = 1;
		for(const hxtest_list_node_t& n : dst) {
			EXPECT_EQ(n.value, expected++);
		}
		EXPECT_EQ(expected, 5);
		dst.release_all();
	}
}

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

TEST(hxlist_test, decrement_end_and_begin) {
	hxtest_list_node_t a(1), b(2), c(3);
	hxlist<hxtest_list_node_t, hxdo_not_delete> list;
	list.push_back(&a);
	list.push_back(&b);
	list.push_back(&c);
	hxlist<hxtest_list_node_t, hxdo_not_delete>::iterator it = list.end();
	--it;
	EXPECT_EQ(&*it, &c);
	it = list.begin();
	--it;
	EXPECT_EQ(it, list.end());
	list.release_all();
}

TEST(hxlist_test, post_decrement_from_end) {
	hxtest_list_node_t a(1), b(2);
	hxlist<hxtest_list_node_t, hxdo_not_delete> list;
	list.push_back(&a);
	list.push_back(&b);
	hxlist<hxtest_list_node_t, hxdo_not_delete>::iterator it = list.end();
	const hxlist<hxtest_list_node_t, hxdo_not_delete>::iterator old = it--;
	EXPECT_EQ(old, list.end());
	EXPECT_EQ(&*it, &b);
	it = list.begin();
	const hxlist<hxtest_list_node_t, hxdo_not_delete>::iterator old2 = it--;
	EXPECT_EQ(old2, list.begin());
	EXPECT_EQ(it, list.end());
	list.release_all();
}

TEST(hxlist_test, extract_last_node_updates_tail) {
	hxtest_list_node_t a(1), b(2);
	hxlist<hxtest_list_node_t, hxdo_not_delete> list;
	list.push_back(&a);
	list.push_back(&b);
	hxlist<hxtest_list_node_t, hxdo_not_delete>::iterator it = list.end();
	--it;
	list.extract(it);
	EXPECT_EQ(list.size(), 1);
	EXPECT_EQ(&list.back(), &a);
	EXPECT_EQ(list.back().value, 1);
	EXPECT_EQ(&list.front(), &a);
	list.release_all();
}

TEST(hxlist_test, splice_at_end_updates_tail) {
	hxtest_list_node_t a(1), b(2), c(3);
	hxlist<hxtest_list_node_t, hxdo_not_delete> dst;
	hxlist<hxtest_list_node_t, hxdo_not_delete> src;
	dst.push_back(&a);
	src.push_back(&b);
	src.push_back(&c);
	dst.splice(dst.end(), src);
	EXPECT_EQ(dst.size(), 3);
	EXPECT_EQ(dst.back().value, 3);
	EXPECT_EQ(dst.front().value, 1);
	dst.release_all();
}

TEST(hxlist_test, mixed_push_front_and_push_back) {
	hxtest_list_node_t a(1), b(4), c(2), d(3);
	hxlist<hxtest_list_node_t, hxdo_not_delete> list;
	list.push_back(&a);
	list.push_back(&b);
	list.push_front(&c);
	{
		hxlist<hxtest_list_node_t, hxdo_not_delete>::iterator it = list.end();
		--it;
		list.insert(it, &d);
	}
	const int expected[] = { 2, 1, 3, 4 };
	int idx = 0;
	for(const hxtest_list_node_t& n : list) {
		EXPECT_EQ(n.value, expected[idx++]);
	}
	EXPECT_EQ(idx, 4);
	list.release_all();
}
