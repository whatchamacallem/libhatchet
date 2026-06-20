// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

#include <hx/hxconstexpr_list.hpp>
#include <hx/hxtest.hpp>

HX_NS_USE

// Test consteval context
#if HX_CPLUSPLUS >= 202302L
namespace {

consteval bool hxtest_hxconst_list_consteval_integration(void) {
	struct hxtest_node_t : hxconst_list_node {
		constexpr explicit hxtest_node_t(int v) : value(v) { }
		int value;
	};
	using list_t = hxconstexpr_list<hxtest_node_t, hxconsteval_delete>;
	list_t list;

	// push_back 1..5 -> list: [1, 2, 3, 4, 5].
	const list_t::iterator i1 = list.push_back(::new hxtest_node_t(1));
	list.push_back(::new hxtest_node_t(2));
	list.push_back(::new hxtest_node_t(3));
	list.push_back(::new hxtest_node_t(4));
	const list_t::iterator i5 = list.push_back(::new hxtest_node_t(5));
	if(list.size() != 5u || list.front().value != 1 || list.back().value != 5) {
		return false;
	}
	if(i1 != list.begin() || i1->value != 1) { return false; }
	if(i5->value != 5) { return false; }
	{
		list_t::iterator it = i5;
		++it;
		if(it != list.end()) { return false; }
	}

	// Forward iteration via mutable iterator with post-increment.
	{
		list_t::iterator it = list.begin();
		for(int exp = 1; exp <= 5; ++exp) {
			if((it++)->value != exp) { return false; }
		}
		if(it != list.end()) { return false; }
	}

	// pop_front removes 1, pop_back removes 5 -> list: [2, 3, 4].
	{
		hxptr<hxtest_node_t, hxconsteval_delete> f = list.pop_front();
		hxptr<hxtest_node_t, hxconsteval_delete> b = list.pop_back();
		if(f->value != 1 || b->value != 5 || list.size() != 3u) {
			return false;
		}
	}
	if(list.front().value != 2 || list.back().value != 4) { return false; }

	// insert_after node(2) inserts node(20) -> list: [2, 20, 3, 4].
	// insert before node(3) inserts node(30) -> list: [2, 20, 30, 3, 4].
	hxtest_node_t* n20 = ::new hxtest_node_t(20);
	const list_t::iterator i20 = list.insert_after(list.begin(), n20);
	if(i20->value != 20) { return false; }
	// n20 is now the second node; find the node with value 3.
	{
		list_t::iterator it = list.begin();
		++it;
		++it;
		// it points to 30's future position; currently points to 3.
		if(it->value != 3) { return false; }
		const list_t::iterator i30 = list.insert(it, ::new hxtest_node_t(30));
		if(i30->value != 30) { return false; }
		list_t::iterator i30next = i30;
		++i30next;
		if(i30next->value != 3) { return false; }
	}
	// list: [2, 20, 30, 3, 4] size 5.
	if(list.size() != 5u) { return false; }
	{
		const int expected[] = { 2, 20, 30, 3, 4 };
		size_t idx = 0u;
		for(const hxtest_node_t& n : list) {
			if(n.value != expected[idx++]) { return false; }
		}
		if(idx != 5u) { return false; }
	}

	// extract node(20) without deletion and re-insert it at back
	// -> list: [2, 30, 3, 4, 20].
	{
		list_t::iterator n20_it = list.begin();
		++n20_it;
		hxptr<hxtest_node_t, hxconsteval_delete> extracted = list.extract(n20_it);
		if(extracted.release() != n20 || list.size() != 4u) { return false; }
		if(list.front().value != 2 || list.back().value != 4) { return false; }
		list.push_back(n20);
	}
	// list: [2, 30, 3, 4, 20] size 5.
	if(list.back().value != 20 || list.size() != 5u) { return false; }

	// erase front node(2) with explicit hxconsteval_delete override
	// -> list: [30, 3, 4, 20].
	list.erase(list.begin(), hxconsteval_delete());
	if(list.size() != 4u || list.front().value != 30) { return false; }

	// splice a donor list [100, 200] before current front(30)
	// -> list: [100, 200, 30, 3, 4, 20].
	{
		list_t donor;
		donor.push_back(::new hxtest_node_t(100));
		donor.push_back(::new hxtest_node_t(200));
		list.splice(list.begin(), donor);
		if(!donor.empty() || list.size() != 6u) { return false; }
	}
	if(list.front().value != 100 || list.back().value != 20) { return false; }

	// remove_if values > 50 removes 100 and 200 -> list: [30, 3, 4, 20].
	{
		const size_t removed = list.remove_if([](hxtest_node_t& n) {
			return n.value > 50;
		});
		if(removed != 2u || list.size() != 4u) { return false; }
	}
	if(list.front().value != 30 || list.back().value != 20) { return false; }

	// reverse -> list: [20, 4, 3, 30].
	list.reverse();
	if(list.size() != 4u) { return false; }
	{
		const int expected[] = { 20, 4, 3, 30 };
		size_t idx = 0u;
		// Iterate using pre-increment and verify == / != on iterators.
		list_t::const_iterator it = list.cbegin();
		const list_t::const_iterator begin = list.cbegin();
		if(!(it == begin) || it != begin) { return false; }
		for(; it != list.cend(); ++it) {
			if(it->value != expected[idx++]) { return false; }
		}
		if(idx != 4u) { return false; }
	}

	list.clear();

	return true;
}

static_assert(hxtest_hxconst_list_consteval_integration(),
	"hxconstexpr_list consteval: integration test must pass");

} // namespace {
#endif // HX_CPLUSPLUS >= 202302L

hxattr_noinline static void hxtest_gdb_break_hxconst_list(void) { }

// Use the exact same tests as hxlist with by renaming hxlist to hxconstexpr_list.
// This ensures identical APIs and eliminates code duplication.
#define hxtest_gdb_break_hxlist hxtest_gdb_break_hxconst_list

#define hxlist hxconstexpr_list
#define hxlist_node hxconst_list_node

#define hxlist_test hxconst_list_test
#define hxlist_node_test hxconst_list_node_test

#define hxtest_list_counted_node_t hxtest_const_list_counted_node_t
#define hxtest_list_node_t hxtest_const_list_node_t

#include "hxshared_list_test.inl"
