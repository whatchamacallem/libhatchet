// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

// This file is used for hxlist and hxconstexpr_list to show they have identical
// APIs.

namespace {

struct hxtest_list_node_t : hxlist_node {
	explicit hxtest_list_node_t(int v) : value(v) { }
	int value;
};
struct hxtest_list_object_node_t : hxlist_node {
	explicit hxtest_list_object_node_t(const hxtest_object& v) : value(v) { }
	bool operator==(const hxtest_list_object_node_t& x) const { return value == x.value; }
	bool operator<(const hxtest_list_object_node_t& x) const { return value < x.value; }
	hxtest_object value;
};
struct hxtest_list_pair_node_t : hxlist_node {
	explicit hxtest_list_pair_node_t(int f, int s) : first(f), second(s) { }
	int first;
	int second;
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

using hxlist_test_f = hxtest_object_fixture;

TEST(hxlist_test, gdb_print_empty_and_multi_field_node) {
	const hxlist<hxtest_list_pair_node_t, hxdo_not_delete> a;
	hxlist<hxtest_list_pair_node_t, hxdo_not_delete> b;
	hxtest_list_pair_node_t n1(31, 32);
	b.push_back(&n1);
	hxtest_gdb_break_hxlist_empty();
	EXPECT_TRUE(a.empty());
	EXPECT_EQ(b.size(), 1);
	b.release_all();
}

TEST(hxlist_test, construction) {
	hxlist<hxtest_list_node_t, hxdo_not_delete> list;
	EXPECT_TRUE(list.empty());
	EXPECT_EQ(list.size(), 0);
	EXPECT_EQ(list.begin(), list.end());
	hxtest_list_node_t a(1);
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

TEST_F(hxlist_test_f, push_back) {
	hxlist<hxtest_list_object_node_t, hxdo_not_delete> list;
	hxtest_list_object_node_t a(hxtest_object(1)), b(hxtest_object(2)), c(hxtest_object(3));
	const hxlist<hxtest_list_object_node_t, hxdo_not_delete>::iterator ia = list.push_back(&a);
	const hxlist<hxtest_list_object_node_t, hxdo_not_delete>::iterator ib = list.push_back(&b);
	const hxlist<hxtest_list_object_node_t, hxdo_not_delete>::iterator ic = list.push_back(&c);
	hxtest_gdb_break_hxlist();
	EXPECT_EQ(list.size(), 3);
	EXPECT_FALSE(list.empty());
	EXPECT_EQ(&*ia, &a);
	EXPECT_EQ(&*ib, &b);
	EXPECT_EQ(&*ic, &c);
	EXPECT_EQ(ia, list.begin());
	EXPECT_EQ(&list.front(), &a);
	EXPECT_EQ(&list.back(), &c);
	int expected = 1;
	for(const hxtest_list_object_node_t& n : list) {
		EXPECT_EQ(n.value.value(), expected++);
	}
	EXPECT_EQ(expected, 4);
	list.release_all();
	EXPECT_TRUE(check_stats(6, 3, 0, 3, 3, 0, 0, 0, 0, 0));
}

TEST_F(hxlist_test_f, operator_equal) {
	hxlist<hxtest_list_object_node_t, hxdo_not_delete> a;
	hxlist<hxtest_list_object_node_t, hxdo_not_delete> b;
	hxtest_list_object_node_t a1(hxtest_object(31)), a2(hxtest_object(32));
	hxtest_list_object_node_t b1(hxtest_object(31)), b2(hxtest_object(32));
	EXPECT_TRUE(a == b);
	a.push_back(&a1);
	EXPECT_FALSE(a == b);
	b.push_back(&b1);
	EXPECT_TRUE(a == b);
	a.push_back(&a2);
	b.push_back(&b2);
	EXPECT_TRUE(a == b);
	a.release_all();
	b.release_all();
	EXPECT_TRUE(check_stats(8, 4, 0, 4, 4, 0, 0, 0, 3, 0));
}

TEST_F(hxlist_test_f, operator_equal_detects_shared_prefix_length_mismatch) {
	hxlist<hxtest_list_object_node_t, hxdo_not_delete> a;
	hxlist<hxtest_list_object_node_t, hxdo_not_delete> b;
	hxtest_list_object_node_t a1(hxtest_object(31)), a2(hxtest_object(32));
	hxtest_list_object_node_t b1(hxtest_object(31));
	a.push_back(&a1);
	a.push_back(&a2);
	b.push_back(&b1);
	EXPECT_FALSE(a == b);
	EXPECT_FALSE(b == a);
	a.release_all();
	b.release_all();
	EXPECT_TRUE(check_stats(6, 3, 0, 3, 3, 0, 0, 0, 2, 0));
}

TEST_F(hxlist_test_f, operator_less) {
	hxlist<hxtest_list_object_node_t, hxdo_not_delete> a;
	hxlist<hxtest_list_object_node_t, hxdo_not_delete> b;
	EXPECT_FALSE(a < b);
	hxtest_list_object_node_t b1(hxtest_object(31));
	b.push_back(&b1);
	EXPECT_TRUE(a < b);
	EXPECT_FALSE(b < a);
	hxtest_list_object_node_t a1(hxtest_object(31));
	a.push_back(&a1);
	EXPECT_FALSE(a < b);
	EXPECT_FALSE(b < a);
	hxtest_list_object_node_t b2(hxtest_object(32));
	b.push_back(&b2);
	EXPECT_TRUE(a < b);
	EXPECT_FALSE(b < a);
	a.release_all();
	b.release_all();
	EXPECT_TRUE(check_stats(6, 3, 0, 3, 3, 0, 0, 0, 4, 0));
}

TEST_F(hxlist_test_f, operator_less_smaller_element_is_less) {
	hxlist<hxtest_list_object_node_t, hxdo_not_delete> a;
	hxlist<hxtest_list_object_node_t, hxdo_not_delete> b;
	hxtest_list_object_node_t a1(hxtest_object(31));
	hxtest_list_object_node_t b1(hxtest_object(32));
	a.push_back(&a1);
	b.push_back(&b1);
	EXPECT_TRUE(a < b);
	EXPECT_FALSE(b < a);
	a.release_all();
	b.release_all();
	EXPECT_TRUE(check_stats(4, 2, 0, 2, 2, 0, 0, 0, 2, 2));
}

TEST(hxlist_test, front_and_back_const) {
	hxarray<hxtest_list_node_t, 2> nodes{ hxtest_list_node_t(10), hxtest_list_node_t(20) };
	hxlist<hxtest_list_node_t, hxdo_not_delete> list;
	list.add_range(hxmove(nodes));
	const hxlist<hxtest_list_node_t, hxdo_not_delete>& clist = list;
	EXPECT_EQ(&clist.front(), &nodes[0]);
	EXPECT_EQ(&clist.back(), &nodes[1]);
	EXPECT_EQ(clist.front().value, 10);
	EXPECT_EQ(clist.back().value, 20);
	list.release_all();
}

TEST(hxlist_test, begin_end_const_iteration) {
	hxarray<hxtest_list_node_t, 3> nodes{
		hxtest_list_node_t(1), hxtest_list_node_t(2), hxtest_list_node_t(3) };
	hxlist<hxtest_list_node_t, hxdo_not_delete> list;
	list.add_range(hxmove(nodes));
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

TEST(hxlist_test, cbegin_cend) {
	hxarray<hxtest_list_node_t, 3> nodes{
		hxtest_list_node_t(1), hxtest_list_node_t(2), hxtest_list_node_t(3) };
	hxlist<hxtest_list_node_t, hxdo_not_delete> list;
	list.add_range(hxmove(nodes));
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
	list.release_all();
}

TEST(hxlist_test, push_front) {
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
	EXPECT_EQ(&list.front(), &a);
	EXPECT_EQ(&list.back(), &c);
	int expected = 1;
	for(const hxtest_list_node_t& n : list) {
		EXPECT_EQ(n.value, expected++);
	}
	EXPECT_EQ(expected, 4);
	list.release_all();
}

TEST(hxlist_test, pop_front_and_pop_back) {
	hxlist<hxtest_list_node_t, hxdo_not_delete> list;
	hxarray<hxtest_list_node_t, 3> nodes{
		hxtest_list_node_t(1), hxtest_list_node_t(2), hxtest_list_node_t(3) };
	list.add_range(hxmove(nodes));
	EXPECT_EQ(list.pop_front()->value, 1);
	EXPECT_EQ(list.size(), 2);
	EXPECT_EQ(list.front().value, 2);
	EXPECT_EQ(list.pop_back()->value, 3);
	EXPECT_EQ(list.size(), 1);
	EXPECT_EQ(list.pop_front()->value, 2);
	EXPECT_TRUE(list.empty());
}

TEST(hxlist_test, insert) {
	hxtest_list_node_t a(2), b(3), c(1), mid(0), tail(4);
	hxlist<hxtest_list_node_t, hxdo_not_delete> list;
	list.push_back(&a);
	list.push_back(&b);
	const hxlist<hxtest_list_node_t, hxdo_not_delete>::iterator ic = list.insert(list.begin(), &c);
	EXPECT_EQ(&list.front(), &c);
	EXPECT_EQ(&*ic, &c);
	EXPECT_EQ(ic, list.begin());
	hxlist<hxtest_list_node_t, hxdo_not_delete>::iterator it = list.begin();
	++it;
	++it;
	const hxlist<hxtest_list_node_t, hxdo_not_delete>::iterator imid = list.insert(it, &mid);
	EXPECT_EQ(list.size(), 4);
	hxlist<hxtest_list_node_t, hxdo_not_delete>::iterator prev = imid;
	--prev;
	EXPECT_EQ(&*prev, &a);
	hxlist<hxtest_list_node_t, hxdo_not_delete>::iterator next = imid;
	++next;
	EXPECT_EQ(&*next, &b);
	const hxlist<hxtest_list_node_t, hxdo_not_delete>::iterator itail = list.insert(list.end(), &tail);
	EXPECT_EQ(list.size(), 5);
	EXPECT_EQ(&list.back(), &tail);
	EXPECT_EQ(&*itail, &tail);
	hxlist<hxtest_list_node_t, hxdo_not_delete>::iterator after_tail = itail;
	++after_tail;
	EXPECT_EQ(after_tail, list.end());
	list.release_all();
}

TEST(hxlist_test, insert_after) {
	hxtest_list_node_t a(1), b(2), c(3), mid(9);
	hxlist<hxtest_list_node_t, hxdo_not_delete> list;
	list.push_back(&a);
	list.push_back(&b);
	hxlist<hxtest_list_node_t, hxdo_not_delete>::iterator it = list.end();
	--it;
	const hxlist<hxtest_list_node_t, hxdo_not_delete>::iterator ic = list.insert_after(it, &c);
	EXPECT_EQ(list.size(), 3);
	EXPECT_EQ(&list.back(), &c);
	hxlist<hxtest_list_node_t, hxdo_not_delete>::iterator next = ic;
	++next;
	EXPECT_EQ(next, list.end());
	const hxlist<hxtest_list_node_t, hxdo_not_delete>::iterator imid =
		list.insert_after(list.begin(), &mid);
	EXPECT_EQ(list.size(), 4);
	hxlist<hxtest_list_node_t, hxdo_not_delete>::iterator prev = imid;
	--prev;
	EXPECT_EQ(&*prev, &a);
	list.release_all();
}

TEST(hxlist_test, insert_and_insert_after_hxptr_rvalue_overloads) {
	hxs_list_test_destructor_count = 0;
	hxlist<hxtest_list_counted_node_t> list;
	list.push_back(hxptr<hxtest_list_counted_node_t>(hxnew<hxtest_list_counted_node_t>(2)));
	list.push_front(hxptr<hxtest_list_counted_node_t>(hxnew<hxtest_list_counted_node_t>(1)));
	list.insert(--list.end(),
		hxptr<hxtest_list_counted_node_t>(hxnew<hxtest_list_counted_node_t>(3)));
	list.insert_after(list.begin(),
		hxptr<hxtest_list_counted_node_t>(hxnew<hxtest_list_counted_node_t>(4)));
	const hxarray<int, 4> expected{ 1, 4, 3, 2 };
	hxsize_t idx = 0;
	for(const hxtest_list_counted_node_t& n : list) {
		EXPECT_EQ(n.value, expected[idx++]);
	}
	EXPECT_EQ(idx, 4);
	EXPECT_EQ(list.size(), 4);
	list.clear();
	EXPECT_EQ(hxs_list_test_destructor_count, 4);
}

TEST(hxlist_test, extract) {
	hxtest_list_node_t a(1), b(2), c(3);
	hxlist<hxtest_list_node_t, hxdo_not_delete> list;
	list.push_back(&a);
	EXPECT_EQ(list.extract(list.begin()).get(), &a);
	EXPECT_TRUE(list.empty());
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
	EXPECT_EQ(&list.back(), &c);
	EXPECT_EQ(&list.front(), &c);
	list.extract(list.begin());
	EXPECT_TRUE(list.empty());
}

TEST(hxlist_test, erase) {
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

TEST(hxlist_test, erase_with_deleter_override) {
	hxs_list_test_destructor_count = 0;
	hxtest_list_counted_node_t a(1);
	hxlist<hxtest_list_counted_node_t, hxdo_not_delete> list;
	list.push_back(&a);
	list.erase(list.begin());
	EXPECT_EQ(hxs_list_test_destructor_count, 0);
	EXPECT_TRUE(list.empty());
	hxs_test_custom_deleter_count = 0;
	hxptr<hxtest_list_counted_node_t> n(hxnew<hxtest_list_counted_node_t>(34));
	hxlist<hxtest_list_counted_node_t, hxdo_not_delete> list2;
	list2.push_back(n.release());
	list2.erase(list2.begin(), hxtest_list_custom_deleter_t());
	EXPECT_EQ(hxs_test_custom_deleter_count, 1);
	EXPECT_TRUE(list2.empty());
}

TEST(hxlist_test, clear) {
	hxlist<hxtest_list_node_t, hxdo_not_delete> list;
	list.clear();
	EXPECT_TRUE(list.empty());
	EXPECT_EQ(list.size(), 0);
	hxs_list_test_destructor_count = 0;
	hxtest_list_counted_node_t a(1), b(2);
	hxlist<hxtest_list_counted_node_t, hxdo_not_delete> list2;
	list2.push_back(&a);
	list2.push_back(&b);
	list2.clear(hxdo_not_delete());
	EXPECT_EQ(hxs_list_test_destructor_count, 0);
	EXPECT_TRUE(list2.empty());
}

TEST(hxlist_test, clear_with_custom_deleter_override) {
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

TEST(hxlist_test, release_all) {
	hxtest_list_node_t a(1), b(2), c(3);
	hxlist<hxtest_list_node_t, hxdo_not_delete> list;
	list.push_back(&a);
	list.push_back(&b);
	list.release_all();
	EXPECT_TRUE(list.empty());
	EXPECT_EQ(list.size(), 0);
	EXPECT_EQ(list.begin(), list.end());
	list.push_back(&c);
	EXPECT_EQ(list.size(), 1);
	EXPECT_EQ(list.front().value, 3);
	list.release_all();
}

TEST(hxlist_test, destructor_calls_clear) {
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

TEST(hxlist_test, iterator_increment) {
	hxarray<hxtest_list_node_t, 3> nodes{
		hxtest_list_node_t(1), hxtest_list_node_t(2), hxtest_list_node_t(3) };
	hxlist<hxtest_list_node_t, hxdo_not_delete> list;
	list.add_range(hxmove(nodes));
	hxlist<hxtest_list_node_t, hxdo_not_delete>::iterator it = list.begin();
	EXPECT_EQ(it->value, 1);
	++it;
	EXPECT_EQ(it->value, 2);
	const hxlist<hxtest_list_node_t, hxdo_not_delete>::iterator old = it++;
	EXPECT_EQ(old->value, 2);
	EXPECT_EQ(it->value, 3);
	++it;
	EXPECT_EQ(it, list.end());
	list.release_all();
}

TEST(hxlist_test, iterator_decrement) {
	hxarray<hxtest_list_node_t, 3> nodes{
		hxtest_list_node_t(1), hxtest_list_node_t(2), hxtest_list_node_t(3) };
	hxlist<hxtest_list_node_t, hxdo_not_delete> list;
	list.add_range(hxmove(nodes));
	hxlist<hxtest_list_node_t, hxdo_not_delete>::iterator it = list.end();
	const hxlist<hxtest_list_node_t, hxdo_not_delete>::iterator old = it--;
	EXPECT_EQ(old, list.end());
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

TEST(hxlist_test, iterator_equality) {
	hxarray<hxtest_list_node_t, 2> nodes{ hxtest_list_node_t(1), hxtest_list_node_t(2) };
	hxlist<hxtest_list_node_t, hxdo_not_delete> list;
	list.add_range(hxmove(nodes));
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
	hxarray<hxtest_list_node_t, 2> nodes{ hxtest_list_node_t(1), hxtest_list_node_t(2) };
	hxlist<hxtest_list_node_t, hxdo_not_delete> list;
	list.add_range(hxmove(nodes));
	hxlist<hxtest_list_node_t, hxdo_not_delete>::const_iterator it = list.begin();
	hxlist<hxtest_list_node_t, hxdo_not_delete>::const_iterator old = it++;
	EXPECT_EQ(old->value, 1);
	EXPECT_EQ(it->value, 2);
	old = it--;
	EXPECT_EQ(old->value, 2);
	EXPECT_EQ(it->value, 1);
	list.release_all();
}

TEST(hxlist_test, conforms_to_bidirectional_iterator_api) {
	hxarray<hxtest_list_node_t, 3> nodes{
		hxtest_list_node_t(1), hxtest_list_node_t(2), hxtest_list_node_t(3) };
	hxlist<hxtest_list_node_t, hxdo_not_delete> list;
	list.add_range(hxmove(nodes));
	EXPECT_TRUE(hxtest_check_bidirectional_iterator_api(list.begin(), list.end()));
	const hxlist<hxtest_list_node_t, hxdo_not_delete>& clist = list;
	EXPECT_TRUE(hxtest_check_bidirectional_iterator_api(clist.begin(), clist.end()));
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
	hxlist<hxtest_list_counted_node_t> list;
	EXPECT_EQ(list.remove_if([](hxtest_list_counted_node_t&) { return true; }), 0);
	list.push_back(hxnew<hxtest_list_counted_node_t>(1));
	list.push_back(hxnew<hxtest_list_counted_node_t>(2));
	EXPECT_EQ(list.remove_if([](hxtest_list_counted_node_t&) { return false; }), 0);
	EXPECT_EQ(list.size(), 2);
	hxs_list_test_destructor_count = 0;
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

TEST(hxlist_test, remove_if_with_do_not_delete_override) {
	hxs_list_test_destructor_count = 0;
	hxtest_list_counted_node_t a(1), b(2), c(3);
	hxlist<hxtest_list_counted_node_t, hxdo_not_delete> list;
	list.push_back(&a);
	list.push_back(&b);
	list.push_back(&c);
	const hxsize_t count = list.remove_if([](hxtest_list_counted_node_t& n) {
		return n.value != 2;
	}, hxdo_not_delete());
	EXPECT_EQ(count, 2);
	EXPECT_EQ(hxs_list_test_destructor_count, 0);
	EXPECT_EQ(list.size(), 1);
	EXPECT_EQ(list.front().value, 2);
}

TEST(hxlist_test, remove_if_with_custom_deleter_override) {
	hxs_test_custom_deleter_count = 0;
	hxlist<hxtest_list_counted_node_t, hxdo_not_delete> list;
	list.push_back(hxnew<hxtest_list_counted_node_t>(4));
	const hxsize_t count = list.remove_if([](hxtest_list_counted_node_t& n) {
		return n.value == 4;
	}, hxtest_list_custom_deleter_t());
	EXPECT_EQ(count, 1);
	EXPECT_EQ(hxs_test_custom_deleter_count, 1);
	EXPECT_TRUE(list.empty());
}

TEST(hxlist_test, find_if) {
	hxarray<hxtest_list_node_t, 3> nodes{
		hxtest_list_node_t(1), hxtest_list_node_t(2), hxtest_list_node_t(3) };
	hxlist<hxtest_list_node_t, hxdo_not_delete> list;
	EXPECT_EQ(list.find_if([](const hxtest_list_node_t&) { return true; }), list.end());
	list.add_range(hxmove(nodes));
	EXPECT_EQ(&*list.find_if([](const hxtest_list_node_t& n) { return n.value == 1; }), &nodes[0]);
	EXPECT_EQ(&*list.find_if([](const hxtest_list_node_t& n) { return n.value == 3; }), &nodes[2]);
	EXPECT_EQ(list.find_if([](const hxtest_list_node_t& n) { return n.value == 4; }), list.end());
	const hxlist<hxtest_list_node_t, hxdo_not_delete>& clist = list;
	EXPECT_EQ(&*clist.find_if([](const hxtest_list_node_t& n) { return n.value == 2; }), &nodes[1]);
	EXPECT_EQ(clist.find_if([](const hxtest_list_node_t& n) { return n.value == 5; }), clist.end());
	list.release_all();
}

TEST(hxlist_test, for_each_mutable) {
	hxarray<hxtest_list_node_t, 3> nodes{
		hxtest_list_node_t(1), hxtest_list_node_t(2), hxtest_list_node_t(3) };
	hxlist<hxtest_list_node_t, hxdo_not_delete> list;
	int count = 0;
	list.for_each([&](hxtest_list_node_t&) { ++count; });
	EXPECT_EQ(count, 0);
	list.add_range(hxmove(nodes));
	int expected = 1;
	count = 0;
	list.for_each([&](hxtest_list_node_t& n) {
		EXPECT_EQ(n.value, expected++);
		n.value += 10;
		++count;
	});
	EXPECT_EQ(count, 3);
	EXPECT_EQ(expected, 4);
	EXPECT_EQ(nodes[0].value, 11);
	list.release_all();
}

TEST(hxlist_test, for_each_const) {
	hxarray<hxtest_list_node_t, 3> nodes{
		hxtest_list_node_t(11), hxtest_list_node_t(12), hxtest_list_node_t(13) };
	hxlist<hxtest_list_node_t, hxdo_not_delete> list;
	list.add_range(hxmove(nodes));
	const hxlist<hxtest_list_node_t, hxdo_not_delete>& clist = list;
	int seen = 0;
	int count = 0;
	clist.for_each([&](const hxtest_list_node_t& n) { seen += n.value; ++count; });
	EXPECT_EQ(count, 3);
	EXPECT_EQ(seen, 11 + 12 + 13);
	list.release_all();
}

TEST(hxlist_test, reverse_empty_and_small) {
	hxlist<hxtest_list_node_t, hxdo_not_delete> empty;
	empty.reverse();
	EXPECT_TRUE(empty.empty());
	hxtest_list_node_t a(1), b(2);
	hxlist<hxtest_list_node_t, hxdo_not_delete> list;
	list.push_back(&a);
	list.reverse();
	EXPECT_EQ(list.front().value, 1);
	EXPECT_EQ(list.back().value, 1);
	list.push_front(&b);
	list.reverse();
	EXPECT_EQ(list.front().value, 1);
	EXPECT_EQ(list.back().value, 2);
	list.release_all();
}

TEST(hxlist_test, reverse_three_nodes) {
	hxarray<hxtest_list_node_t, 3> nodes{
		hxtest_list_node_t(1), hxtest_list_node_t(2), hxtest_list_node_t(3) };
	hxlist<hxtest_list_node_t, hxdo_not_delete> list;
	list.add_range(hxmove(nodes));
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

TEST(hxlist_test, splice_at_end) {
	hxtest_list_node_t a(1), b(2), c(3), d(4);
	hxlist<hxtest_list_node_t, hxdo_not_delete> dst;
	hxlist<hxtest_list_node_t, hxdo_not_delete> src;
	dst.push_back(&a);
	dst.splice(dst.end(), src);
	EXPECT_EQ(dst.size(), 1);
	EXPECT_TRUE(src.empty());
	dst.push_back(&b);
	src.push_back(&c);
	src.push_back(&d);
	dst.splice(dst.end(), src);
	EXPECT_EQ(dst.size(), 4);
	EXPECT_TRUE(src.empty());
	EXPECT_EQ(dst.back().value, 4);
	EXPECT_EQ(dst.front().value, 1);
	dst.release_all();
}

TEST(hxlist_test, splice_before_iterator) {
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
	EXPECT_EQ(&dst.back(), &b);
	int expected = 1;
	for(const hxtest_list_node_t& n : dst) {
		EXPECT_EQ(n.value, expected++);
	}
	EXPECT_EQ(expected, 5);
	dst.release_all();
}

TEST(hxlist_test, mixed_push_front_insert_and_push_back) {
	hxtest_list_node_t a(1), b(4), c(2), d(3);
	hxlist<hxtest_list_node_t, hxdo_not_delete> list;
	list.push_back(&a);
	list.push_back(&b);
	list.push_front(&c);
	hxlist<hxtest_list_node_t, hxdo_not_delete>::iterator it = list.end();
	--it;
	list.insert(it, &d);
	const hxarray<int, 4> expected{ 2, 1, 3, 4 };
	int idx = 0;
	for(const hxtest_list_node_t& n : list) {
		EXPECT_EQ(n.value, expected[idx++]);
	}
	EXPECT_EQ(idx, 4);
	list.release_all();
}

TEST(hxlist_test, add_range_empty) {
	hxlist<hxtest_list_node_t, hxdo_not_delete> list;
	hxvector<hxtest_list_node_t, 1> nodes;
	list.add_range(hxmove(nodes));
	EXPECT_TRUE(list.empty());
	EXPECT_EQ(list.size(), 0);
}

TEST(hxlist_test, add_range_appends_to_non_empty_list_in_order) {
	hxtest_list_node_t a(31);
	hxlist<hxtest_list_node_t, hxdo_not_delete> list;
	list.push_back(&a);
	hxarray<hxtest_list_node_t, 3> nodes{
		hxtest_list_node_t(32), hxtest_list_node_t(33), hxtest_list_node_t(34) };
	list.add_range(hxmove(nodes));
	EXPECT_EQ(list.size(), 4);
	const hxarray<int, 4> expected{ 31, 32, 33, 34 };
	int idx = 0;
	for(const hxtest_list_node_t& n : list) {
		EXPECT_EQ(n.value, expected[idx++]);
	}
	EXPECT_EQ(idx, 4);
	EXPECT_EQ(&list.front(), &a);
	EXPECT_EQ(&list.back(), &nodes[2]);
	list.release_all();
}
