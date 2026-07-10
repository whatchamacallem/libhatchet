// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

#include <hx/hxflat_map.hpp>
#include <hx/hxmemory_manager.h>
#include "./hxtest_util.hpp"

HX_NS_USE

hxattr_noinline static void hxtest_gdb_break_hxflat_map_static(void) { }
hxattr_noinline static void hxtest_gdb_break_hxflat_map_dynamic(void) { }

#if !defined _MSC_VER && !defined __wasm__
static_assert(sizeof(size_t) != 4 || (
		sizeof(hxflat_map<int32_t, int32_t, hxkey_less_t<int32_t>, false, 4>) == 36u
		&& sizeof(hxflat_map<int32_t, int32_t, hxkey_less_t<int32_t>, false>) == 20u),
	"hxflat_map must pack fixed storage as a hxsize_t size plus two capacity *"
	" sizeof(T) key and value arrays and dynamic storage as a hxsize_t size"
	" plus two hxsize_t/T* allocator pairs with no padding");
static_assert(sizeof(size_t) != 8 || (
		sizeof(hxflat_map<int32_t, int32_t, hxkey_less_t<int32_t>, false, 4>) == 40u
		&& sizeof(hxflat_map<int32_t, int32_t, hxkey_less_t<int32_t>, false>) == 40u),
	"hxflat_map must pack fixed storage as a hxsize_t size plus two capacity *"
	" sizeof(T) key and value arrays and dynamic storage as a hxsize_t size"
	" plus two hxsize_t/T* allocator pairs with no padding");
#endif

namespace {
class hxflat_map_test_f : public hxtest_object_fixture { };
} // namespace

TEST_F(hxflat_map_test_f, gdb_break) {
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 4> ms;
	const hxtest_object v1(10), v2(20), v3(30);
	ms.insert(1, v1);
	ms.insert(2, v2);
	ms.insert(3, v3);
	hxtest_gdb_break_hxflat_map_static();
	EXPECT_EQ(ms.size(), 3);
	const hxsystem_allocator_scope temp_scope(hxsystem_allocator_stack_0);
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false> md;
	md.reserve(8);
	md.insert(1, v1);
	md.insert(2, v2);
	md.insert(3, v3);
	hxtest_gdb_break_hxflat_map_dynamic();
	EXPECT_EQ(md.size(), 3);
}

TEST_F(hxflat_map_test_f, construct) {
	const hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 1> ms;
	EXPECT_TRUE(ms.empty());
	EXPECT_EQ(ms.size(), 0);
	EXPECT_EQ(ms.capacity(), 1);
	EXPECT_EQ(ms.max_size(), 1);
	EXPECT_FALSE(ms.full());
	const hxsystem_allocator_scope temp_scope(hxsystem_allocator_stack_0);
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false> md;
	EXPECT_TRUE(md.empty());
	EXPECT_EQ(md.capacity(), 0);
	EXPECT_EQ(md.max_size(), 0);
	md.reserve(8);
	EXPECT_EQ(md.capacity(), 8);
	EXPECT_EQ(md.max_size(), 8);
}

TEST_F(hxflat_map_test_f, reserve_static_exact) {
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 1> m;
	m.reserve(1);
	EXPECT_EQ(m.capacity(), 1);
}

TEST_F(hxflat_map_test_f, insert_unique_basic) {
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 2> m;
	const hxtest_object v1(10), v2(20);
	const hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 2>::iterator it1 = m.insert(1, v1);
	EXPECT_EQ(it1.key(), 1);
	EXPECT_EQ(it1.value().id, 10);
	EXPECT_EQ(m.size(), 1);
	const hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 2>::iterator it2 = m.insert(1, v2);
	EXPECT_EQ(it2.key(), 1);
	EXPECT_EQ(it2.value().id, 10);
	EXPECT_EQ(m.size(), 1);
}

TEST_F(hxflat_map_test_f, insert_unique_sorted_order) {
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 3> m;
	const hxtest_object v3(3), v1(1), v2(2);
	m.insert(30, v3);
	m.insert(10, v1);
	m.insert(20, v2);
	EXPECT_EQ(m.size(), 3);
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 3>::const_iterator it = m.begin();
	EXPECT_EQ(it.key(), 10); ++it;
	EXPECT_EQ(it.key(), 20); ++it;
	EXPECT_EQ(it.key(), 30);
}

TEST_F(hxflat_map_test_f, insert_unique_move) {
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 3> m;
	hxtest_object v(34);
	const hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 3>::iterator it = m.insert(5, hxmove(v));
	EXPECT_EQ(it.key(), 5);
	EXPECT_EQ(it.value().id, 34);
	EXPECT_TRUE(v.moved_from);
	const hxtest_object v1(10);
	m.insert(1, v1);
	hxtest_object v2(20);
	const hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 3>::iterator it2 = m.insert(1, hxmove(v2));
	EXPECT_EQ(it2.value().id, 10);
	EXPECT_EQ(m.size(), 2);
	hxtest_object v3(15);
	const hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 3>::iterator it3 = m.insert(3, hxmove(v3));
	EXPECT_EQ(it3.key(), 3);
	EXPECT_EQ(it3.value().id, 15);
	EXPECT_EQ(m.size(), 3);
}

TEST_F(hxflat_map_test_f, insert_multi) {
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, true, 2> m;
	const hxtest_object v1(10);
	hxtest_object v2(20);
	m.insert(1, v1);
	m.insert(1, hxmove(v2));
	EXPECT_EQ(m.size(), 2);
	EXPECT_EQ(m.count(1), 2);
	EXPECT_TRUE(v2.moved_from);
}

TEST_F(hxflat_map_test_f, insert_shifts_elements) {
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 3> m;
	const hxtest_object va(1), vb(2), vc(3);
	m.insert(10, va);
	m.insert(30, vb);
	m.insert(20, vc);
	EXPECT_EQ(m.size(), 3);
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 3>::const_iterator it = m.begin();
	EXPECT_EQ(it.key(), 10); ++it;
	EXPECT_EQ(it.key(), 20); ++it;
	EXPECT_EQ(it.key(), 30);
}

TEST_F(hxflat_map_test_f, insert_at_front_shifts_single_element) {
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 2> m;
	const hxtest_object va(1), vb(2);
	m.insert(20, va);
	m.insert(10, vb);
	EXPECT_EQ(m.size(), 2);
	EXPECT_EQ(m.begin().key(), 10);
	EXPECT_EQ(m.begin().value().id, 2);
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 2>::const_iterator it = m.begin();
	++it;
	EXPECT_EQ(it.key(), 20);
	EXPECT_EQ(it.value().id, 1);
}

TEST_F(hxflat_map_test_f, insert_dynamic) {
	const hxsystem_allocator_scope temp_scope(hxsystem_allocator_stack_0);
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false> m;
	m.reserve(1);
	const hxtest_object v(7);
	m.insert(3, v);
	EXPECT_EQ(m.size(), 1);
	EXPECT_EQ(m.find(3)->id, 7);
}

TEST_F(hxflat_map_test_f, clear) {
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 2> m;
	const hxtest_object v1(1), v2(2);
	m.insert(1, v1);
	m.insert(2, v2);
	EXPECT_EQ(m.size(), 2);
	m.clear();
	EXPECT_EQ(m.size(), 0);
	EXPECT_TRUE(m.empty());
	m.clear();
	EXPECT_EQ(m.size(), 0);
}

TEST_F(hxflat_map_test_f, find) {
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 2> m;
	const hxtest_object v(99);
	m.insert(7, v);
	hxtest_object* p = m.find(7);
	EXPECT_TRUE(p != hxnull);
	EXPECT_EQ(p->id, 99);
	p->id = 55;
	EXPECT_EQ(m.find(7)->id, 55);
	EXPECT_EQ(m.find(99), hxnullptr);
	const hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 2>& cm = m;
	EXPECT_EQ(cm.find(7)->id, 55);
	EXPECT_EQ(cm.find(99), hxnullptr);
	const hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 2> empty;
	EXPECT_EQ(empty.find(1), hxnullptr);
}

TEST_F(hxflat_map_test_f, count) {
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, true, 3> m;
	const hxtest_object v1(1), v2(2), v3(3);
	m.insert(4, v1);
	EXPECT_EQ(m.count(4), 1);
	EXPECT_EQ(m.count(9), 0);
	m.insert(4, v2);
	m.insert(4, v3);
	EXPECT_EQ(m.count(4), 3);
}

TEST_F(hxflat_map_test_f, lower_bound) {
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 3> m;
	const hxtest_object v1(1), v2(2), v3(3);
	m.insert(10, v1);
	m.insert(30, v2);
	EXPECT_EQ(m.lower_bound(10).key(), 10);
	EXPECT_EQ(m.lower_bound(20).key(), 30);
	EXPECT_TRUE(m.lower_bound(99) == m.end());
	m.insert(20, v3);
	EXPECT_EQ(m.lower_bound(20).key(), 20);
	const hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 3>::iterator it = m.lower_bound(20);
	it.value().id = 99;
	EXPECT_EQ(m.find(20)->id, 99);
}

TEST_F(hxflat_map_test_f, upper_bound) {
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, true, 3> m;
	const hxtest_object v1(1), v2(2), v3(3);
	m.insert(10, v1);
	m.insert(20, v2);
	const hxflat_map<int, hxtest_object, hxkey_less_t<int>, true, 3>& cm = m;
	EXPECT_EQ(cm.upper_bound(10).key(), 20);
	EXPECT_EQ(m.upper_bound(15).key(), 20);
	EXPECT_TRUE(m.upper_bound(20) == m.end());
	m.insert(10, v3);
	EXPECT_EQ(m.upper_bound(10).key(), 20);
	EXPECT_TRUE(m.upper_bound(9999) == m.end());
}

TEST_F(hxflat_map_test_f, erase_key_unique) {
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 1> m;
	const hxtest_object v(1);
	m.insert(5, v);
	EXPECT_EQ(m.erase(9), 0);
	EXPECT_EQ(m.size(), 1);
	EXPECT_EQ(m.erase(99), 0);
	EXPECT_EQ(m.erase(1), 0);
	EXPECT_EQ(m.size(), 1);
	EXPECT_EQ(m.erase(5), 1);
	EXPECT_TRUE(m.empty());
}

TEST_F(hxflat_map_test_f, erase_key_multi) {
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, true, 4> m;
	const hxtest_object v1(1), v2(2), v3(3), v4(4);
	m.insert(5, v1);
	EXPECT_EQ(m.erase(9), 0);
	m.insert(5, v2);
	m.insert(10, v3);
	EXPECT_EQ(m.erase(5), 2);
	EXPECT_EQ(m.size(), 1);
	EXPECT_EQ(m.count(10), 1);
	m.insert(10, v4);
	m.insert(10, v1);
	EXPECT_EQ(m.erase(10), 3);
	EXPECT_EQ(m.size(), 0);
}

TEST_F(hxflat_map_test_f, erase_key_multi_tail_relative_to_count) {
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, true, 4> ma;
	const hxtest_object v1(1), v2(2), v3(3), v4(4);
	ma.insert(5, v1);
	ma.insert(5, v2);
	ma.insert(5, v3);
	ma.insert(10, v4);
	EXPECT_EQ(ma.erase(5), 3);
	EXPECT_EQ(ma.size(), 1);
	EXPECT_EQ(ma.find(10)->id, 4);
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, true, 4> mb;
	mb.insert(5, v1);
	mb.insert(10, v2);
	mb.insert(10, v3);
	mb.insert(10, v4);
	EXPECT_EQ(mb.erase(5), 1);
	EXPECT_EQ(mb.size(), 3);
	EXPECT_EQ(mb.count(10), 3);
}

TEST_F(hxflat_map_test_f, erase_iterator_only_and_pair) {
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 2> m;
	const hxtest_object v1(1), v2(2);
	m.insert(7, v1);
	const hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 2>::iterator next1 = m.erase(m.begin());
	EXPECT_TRUE(next1 == m.end());
	EXPECT_TRUE(m.empty());
	m.insert(10, v1);
	m.insert(20, v2);
	const hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 2>::iterator next2 = m.erase(m.begin());
	EXPECT_EQ(next2.key(), 20);
	EXPECT_EQ(m.size(), 1);
	const hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 2>::iterator last = m.begin();
	const hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 2>::iterator next3 = m.erase(last);
	EXPECT_TRUE(next3 == m.end());
	EXPECT_EQ(m.size(), 0);
}

TEST_F(hxflat_map_test_f, erase_iterator_first_and_middle_of_three) {
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 3> m;
	const hxtest_object v1(1), v2(2), v3(3);
	m.insert(10, v1);
	m.insert(20, v2);
	m.insert(30, v3);
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 3>::iterator mid = m.begin();
	++mid;
	const hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 3>::iterator next1 = m.erase(mid);
	EXPECT_EQ(next1.key(), 30);
	EXPECT_EQ(m.size(), 2);
	const hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 3>::iterator next2 = m.erase(m.begin());
	EXPECT_EQ(next2.key(), 30);
	EXPECT_EQ(m.size(), 1);
	EXPECT_EQ(m.find(10), hxnullptr);
	EXPECT_EQ(m.find(30)->id, 3);
}

TEST_F(hxflat_map_test_f, const_iterator_arithmetic) {
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 3> m;
	const hxtest_object v1(1), v2(2), v3(3);
	m.insert(10, v1);
	m.insert(20, v2);
	m.insert(30, v3);
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 3>::const_iterator it = m.begin();
	it += 2;
	EXPECT_EQ(it.key(), 30);
	it -= 1;
	EXPECT_EQ(it.key(), 20);
	const hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 3>::const_iterator it2 = it + 1;
	EXPECT_EQ(it2.key(), 30);
	const hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 3>::const_iterator it3 = it2 - 1;
	EXPECT_EQ(it3.key(), 20);
	EXPECT_EQ(m.begin()[1].key(), 20);
	EXPECT_EQ(m.end() - m.begin(), ptrdiff_t{3});
}

TEST_F(hxflat_map_test_f, const_iterator_pre_post_increment_decrement) {
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 2> m;
	const hxtest_object v1(1), v2(2);
	m.insert(10, v1);
	m.insert(20, v2);
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 2>::const_iterator it = m.begin();
	const hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 2>::const_iterator it2 = ++it;
	EXPECT_EQ(it.key(), 20);
	EXPECT_EQ(it2.key(), 20);
	--it;
	EXPECT_EQ(it.key(), 10);
	const hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 2>::const_iterator it3 = it++;
	EXPECT_EQ(it3.key(), 10);
	EXPECT_EQ(it.key(), 20);
	const hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 2>::const_iterator it4 = it--;
	EXPECT_EQ(it4.key(), 20);
	EXPECT_EQ(it.key(), 10);
}

TEST_F(hxflat_map_test_f, const_iterator_equality_and_order) {
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 2> m;
	const hxtest_object v1(1), v2(2);
	m.insert(10, v1);
	m.insert(20, v2);
	const hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 2>::const_iterator a = m.begin();
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 2>::const_iterator b = m.begin();
	++b;
	EXPECT_TRUE(a == a);
	EXPECT_FALSE(a == b);
	EXPECT_TRUE(a < b);
	EXPECT_FALSE(b < a);
	EXPECT_TRUE(b > a);
	EXPECT_TRUE(a <= b);
	EXPECT_TRUE(a <= a);
	EXPECT_TRUE(b >= a);
	EXPECT_TRUE(b >= b);
	EXPECT_TRUE(a != b);
	EXPECT_FALSE(a != a);
}

TEST_F(hxflat_map_test_f, const_iterator_dereference_proxy) {
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 1> m;
	const hxtest_object v(7);
	m.insert(3, v);
	const hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 1>::const_iterator it = m.begin();
	EXPECT_EQ((*it).key(), 3);
	EXPECT_EQ((*it).value().id, 7);
}

TEST_F(hxflat_map_test_f, iterator_dereference_and_mutation) {
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 1> m;
	const hxtest_object v(7);
	m.insert(3, v);
	const hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 1>::iterator it = m.begin();
	EXPECT_EQ((*it).key(), 3);
	(*it).value().id = 99;
	EXPECT_EQ(m.find(3)->id, 99);
}

TEST_F(hxflat_map_test_f, iterator_arithmetic) {
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 3> m;
	const hxtest_object v1(1), v2(2), v3(3);
	m.insert(10, v1);
	m.insert(20, v2);
	m.insert(30, v3);
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 3>::iterator it = m.begin();
	it += 2;
	EXPECT_EQ(it.key(), 30);
	it -= 1;
	EXPECT_EQ(it.key(), 20);
	const hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 3>::iterator it2 = it + 1;
	EXPECT_EQ(it2.key(), 30);
	const hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 3>::iterator it3 = it2 - 1;
	EXPECT_EQ(it3.key(), 20);
	EXPECT_EQ(it[1].key(), 30);
	EXPECT_EQ(it2 - it3, hxsize_t{1});
}

TEST_F(hxflat_map_test_f, iterator_pre_post_increment_decrement) {
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 2> m;
	const hxtest_object v1(1), v2(2);
	m.insert(10, v1);
	m.insert(20, v2);
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 2>::iterator it = m.begin();
	const hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 2>::iterator before = it++;
	EXPECT_EQ(before.key(), 10);
	EXPECT_EQ(it.key(), 20);
	const hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 2>::iterator after = it--;
	EXPECT_EQ(after.key(), 20);
	EXPECT_EQ(it.key(), 10);
	++it;
	EXPECT_EQ(it.key(), 20);
	--it;
	EXPECT_EQ(it.key(), 10);
}

TEST_F(hxflat_map_test_f, begin_end) {
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 1> m;
	EXPECT_TRUE(m.begin() == m.end());
	EXPECT_TRUE(m.cbegin() == m.cend());
	const hxtest_object v(1);
	m.insert(5, v);
	EXPECT_FALSE(m.begin() == m.end());
	EXPECT_EQ(m.begin().key(), 5);
}

TEST_F(hxflat_map_test_f, full) {
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 2> m;
	const hxtest_object v1(1), v2(2);
	EXPECT_FALSE(m.full());
	m.insert(1, v1);
	EXPECT_FALSE(m.full());
	m.insert(2, v2);
	EXPECT_TRUE(m.full());
}

TEST_F(hxflat_map_test_f, dynamic_multimap_insert_erase) {
	const hxsystem_allocator_scope temp_scope(hxsystem_allocator_stack_0);
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, true> m;
	m.reserve(5);
	const hxtest_object va(1), vb(2), vc(3), vd(4), ve(5);
	m.insert(10, va);
	m.insert(10, vb);
	m.insert(20, vc);
	m.insert(30, vd);
	m.insert(30, ve);
	EXPECT_EQ(m.size(), 5);
	EXPECT_EQ(m.count(10), 2);
	EXPECT_EQ(m.count(20), 1);
	EXPECT_EQ(m.count(30), 2);
	EXPECT_EQ(m.erase(10), 2);
	EXPECT_EQ(m.size(), 3);
	EXPECT_EQ(m.erase(30), 2);
	EXPECT_EQ(m.size(), 1);
}

TEST_F(hxflat_map_test_f, destructor_destroys_elements) {
	hxsize_t destructed_before = 0;
	{
		hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 2> m;
		{
			const hxtest_object v1(10), v2(20);
			m.insert(1, v1);
			m.insert(2, v2);
		}
		destructed_before = m_destructed;
	}
	EXPECT_EQ(m_destructed, destructed_before + 2);
}

TEST_F(hxflat_map_test_f, get) {
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 3> m;
	const hxtest_object v1(10), v2(20), v3(30);
	m.insert(1, v1);
	m.insert(2, v2);
	m.insert(3, v3);
	EXPECT_EQ(m.get(0).key(), 1);
	EXPECT_EQ(m.get(1).key(), 2);
	EXPECT_EQ(m.get(1).value().id, 20);
	EXPECT_TRUE(m.get(3) == m.end());
	EXPECT_TRUE(m.get(4) == m.end());
	m.get(0).value().id = 99;
	EXPECT_EQ(m.find(1)->id, 99);
	const hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 3>& cm = m;
	EXPECT_EQ(cm.get(1).key(), 2);
	EXPECT_TRUE(cm.get(3) == cm.end());
}

TEST_F(hxflat_map_test_f, default_constructed_iterator_assignable) {
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 1> m;
	const hxtest_object v(7);
	m.insert(3, v);
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 1>::const_iterator cit;
	cit = m.begin();
	EXPECT_EQ(cit.key(), 3);
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 1>::iterator it;
	it = m.begin();
	EXPECT_EQ(it.key(), 3);
	it.value().id = 99;
	EXPECT_EQ(m.find(3)->id, 99);
}

TEST_F(hxflat_map_test_f, equal) {
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 3> a;
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 4> b;
	const hxtest_object v1(10), v2(20), v3(10), v4(20);
	EXPECT_TRUE(a.equal(b));
	a.insert(1, v1);
	a.insert(2, v2);
	b.insert(1, v3);
	EXPECT_FALSE(a.equal(b));
	b.insert(2, v4);
	EXPECT_TRUE(a.equal(b));
}

TEST_F(hxflat_map_test_f, equal_mismatched_keys_or_values) {
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 2> a;
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 2> b;
	const hxtest_object v1(10), v2(99);
	a.insert(1, v1);
	b.insert(1, v2);
	EXPECT_FALSE(a.equal(b));
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 2> c;
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 2> d;
	const hxtest_object v3(10), v4(10);
	c.insert(1, v3);
	d.insert(2, v4);
	EXPECT_FALSE(c.equal(d));
}

TEST_F(hxflat_map_test_f, less) {
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 3> a;
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 3> b;
	EXPECT_FALSE(a.less(b));
	const hxtest_object v(1);
	b.insert(1, v);
	EXPECT_TRUE(a.less(b));
	EXPECT_FALSE(b.less(a));
	a.insert(1, v);
	EXPECT_FALSE(a.less(b));
	EXPECT_FALSE(b.less(a));
	b.insert(2, v);
	EXPECT_TRUE(a.less(b));
	EXPECT_FALSE(b.less(a));
}

TEST_F(hxflat_map_test_f, less_smaller_key_or_value_is_less) {
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 2> a;
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 2> b;
	const hxtest_object v1(10), v2(10);
	a.insert(1, v1);
	b.insert(2, v2);
	EXPECT_TRUE(a.less(b));
	EXPECT_FALSE(b.less(a));
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 2> c;
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 2> d;
	const hxtest_object v3(5), v4(10);
	c.insert(1, v3);
	d.insert(1, v4);
	EXPECT_TRUE(c.less(d));
	EXPECT_FALSE(d.less(c));
}

TEST_F(hxflat_map_test_f, subscript) {
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 3> m;
	const hxtest_object v1(10), v2(20), v3(30);
	m.insert(1, v1);
	m.insert(2, v2);
	m.insert(3, v3);
	const hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 3>& cm = m;
	EXPECT_EQ(cm[0].key(), 1);
	EXPECT_EQ(cm[0].value().id, 10);
	EXPECT_EQ(cm[2].key(), 3);
	m[0].value().id = 99;
	EXPECT_EQ(m.find(1)->id, 99);
	EXPECT_EQ(m[m.size() - 1].key(), 3);
}

TEST_F(hxflat_map_test_f, copy_assign) {
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 3> a;
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 4> b;
	const hxtest_object v1(10), v2(20), vb(99);
	b.insert(7, vb);
	a.insert(1, v1);
	a.insert(2, v2);
	b = a;
	EXPECT_EQ(b.size(), 2);
	EXPECT_EQ(b.find(1)->id, 10);
	EXPECT_EQ(b.find(2)->id, 20);
	EXPECT_EQ(b.find(7), hxnullptr);
	const hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 3> empty;
	b = empty;
	EXPECT_TRUE(b.empty());
}

TEST_F(hxflat_map_test_f, move_assign_transfers_elements) {
	const hxsystem_allocator_scope temp_scope(hxsystem_allocator_stack_0);
	{
		hxflat_map<int, hxtest_object> a;
		a.reserve(3);
		hxflat_map<int, hxtest_object> b;
		b.reserve(1);
		const hxtest_object v1(10), v2(20), v3(30);
		a.insert(1, v1);
		a.insert(2, v2);
		a.insert(3, v3);
		b = hxmove(a);
		EXPECT_EQ(b.size(), 3);
		EXPECT_EQ(b.find(1)->id, 10);
		EXPECT_EQ(b.find(2)->id, 20);
		EXPECT_EQ(b.find(3)->id, 30);
		EXPECT_EQ(a.size(), 0);
		EXPECT_EQ(a.capacity(), 1);
	}
	EXPECT_EQ(m_constructed, m_destructed);
}

TEST_F(hxflat_map_test_f, move_constructor_transfers_elements) {
	const hxsystem_allocator_scope temp_scope(hxsystem_allocator_stack_0);
	{
		hxflat_map<int, hxtest_object> src;
		src.reserve(3);
		const hxtest_object v1(10), v2(20), v3(30);
		src.insert(1, v1);
		src.insert(2, v2);
		src.insert(3, v3);
		hxflat_map<int, hxtest_object> dst(hxmove(src));
		EXPECT_EQ(dst.size(), 3);
		EXPECT_EQ(dst.find(1)->id, 10);
		EXPECT_EQ(dst.find(2)->id, 20);
		EXPECT_EQ(dst.find(3)->id, 30);
		EXPECT_EQ(src.size(), 0);
		EXPECT_EQ(src.capacity(), 0);
	}
	EXPECT_EQ(m_constructed, m_destructed);
}

TEST_F(hxflat_map_test_f, copy_constructor) {
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 3> src;
	const hxtest_object v1(10), v2(20), v3(30);
	src.insert(1, v1);
	src.insert(2, v2);
	src.insert(3, v3);
	const hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 3> dst(src);
	EXPECT_EQ(dst.size(), 3);
	EXPECT_EQ(dst.find(1)->id, 10);
	EXPECT_EQ(dst.find(2)->id, 20);
	EXPECT_EQ(dst.find(3)->id, 30);
	EXPECT_EQ(src.size(), 3);
	src.find(1)->id = 99;
	EXPECT_EQ(dst.find(1)->id, 10);
}

TEST_F(hxflat_map_test_f, copy_constructor_empty) {
	const hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 4> src;
	const hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 4> dst(src); // NOLINT(performance-unnecessary-copy-initialization)
	EXPECT_TRUE(dst.empty());
	EXPECT_EQ(dst.size(), 0);
}

TEST_F(hxflat_map_test_f, copy_constructor_lifecycle) {
	{
		hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 3> src;
		const hxtest_object v1(10), v2(20);
		src.insert(1, v1);
		src.insert(2, v2);
		{
			const hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 3> dst(src); // NOLINT(performance-unnecessary-copy-initialization)
			EXPECT_EQ(dst.size(), 2);
		}
	}
	EXPECT_EQ(m_constructed, m_destructed);
}

TEST_F(hxflat_map_test_f, implements_rand_iter_api) {
	hxflat_map<int, int, hxkey_less_t<int>, false, 4> m;
	m.insert(1, 10);
	m.insert(2, 20);
	m.insert(3, 30);
	const hxflat_map<int, int, hxkey_less_t<int>, false, 4>& cm = m;
	EXPECT_TRUE(hxtest_check_rand_iter_api(m.begin(), m.end()));
	EXPECT_TRUE(hxtest_check_rand_iter_api(cm.begin(), cm.end()));
}

#if HX_CPLUSPLUS >= 202002L
TEST_F(hxflat_map_test_f, hxkey_equal) {
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 2> a;
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 4> b;
	const hxtest_object v1(10), v2(20), v3(10);
	a.insert(1, v1);
	b.insert(1, v3);
	EXPECT_TRUE(hxkey_equal(a, b));
	b.insert(2, v2);
	a.insert(2, v2);
	EXPECT_TRUE(hxkey_equal(a, b));
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 2> c;
	c.insert(1, v2);
	EXPECT_FALSE(hxkey_equal(a, c));
}

TEST_F(hxflat_map_test_f, hxkey_less) {
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 2> a;
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 4> b;
	const hxtest_object v1(10), v2(10);
	a.insert(1, v1);
	b.insert(2, v2);
	EXPECT_TRUE(hxkey_less(a, b));
	EXPECT_FALSE(hxkey_less(b, a));
	hxflat_map<int, hxtest_object, hxkey_less_t<int>, false, 2> c;
	c.insert(2, v2);
	EXPECT_FALSE(hxkey_less(b, c));
	EXPECT_FALSE(hxkey_less(c, b));
}

TEST_F(hxflat_map_test_f, hxswap_exchanges_contents) {
	const hxsystem_allocator_scope temp_scope(hxsystem_allocator_stack_0);
	{
		hxflat_map<int, hxtest_object> a;
		a.reserve(2);
		hxflat_map<int, hxtest_object> b;
		b.reserve(2);
		const hxtest_object v1(10), v2(20);
		a.insert(1, v1);
		b.insert(2, v2);
		hxswap(a, b);
		EXPECT_EQ(a.size(), 1);
		EXPECT_EQ(a.find(2)->id, 20);
		EXPECT_EQ(b.size(), 1);
		EXPECT_EQ(b.find(1)->id, 10);
	}
	EXPECT_EQ(m_constructed, m_destructed);
}

TEST_F(hxflat_map_test_f, hxswap_empty_and_nonempty) {
	const hxsystem_allocator_scope temp_scope(hxsystem_allocator_stack_0);
	{
		hxflat_map<int, hxtest_object> a;
		a.reserve(2);
		hxflat_map<int, hxtest_object> b;
		b.reserve(1);
		const hxtest_object v1(10), v2(20);
		a.insert(1, v1);
		a.insert(2, v2);
		hxswap(a, b);
		EXPECT_TRUE(a.empty());
		EXPECT_EQ(b.size(), 2);
		EXPECT_EQ(b.find(1)->id, 10);
		EXPECT_EQ(b.find(2)->id, 20);
	}
	EXPECT_EQ(m_constructed, m_destructed);
}
#endif // HX_CPLUSPLUS >= 202002L
