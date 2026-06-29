// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

#include <hx/hxconstexpr_list.hpp>
#include <hx/hxtest.hpp>

HX_NS_USE

#if HX_CPLUSPLUS >= 202302L
namespace {

// GCOVR_EXCL_START
consteval bool hxtest_hxconstexpr_list_consteval_integration(void) {
	struct hxtest_node_t : hxconstexpr_list_node {
		constexpr explicit hxtest_node_t(int v) : value(v) { }
		int value;
	};
	using list_t = hxconstexpr_list<hxtest_node_t, hxconsteval_delete>;
	list_t list;
	const list_t::iterator i1 = list.push_back(::new hxtest_node_t(1));
	list.push_back(::new hxtest_node_t(2));
	list.push_back(::new hxtest_node_t(3));
	list.push_back(::new hxtest_node_t(4));
	const list_t::iterator i5 = list.push_back(::new hxtest_node_t(5));
	if(list.size() != 5 || list.front().value != 1 || list.back().value != 5) {
		return false;
	}
	if(i1 != list.begin() || i1->value != 1) { return false; }
	if(i5->value != 5) { return false; }
	{
		list_t::iterator it = i5;
		++it;
		if(it != list.end()) { return false; }
	}
	{
		list_t::iterator it = list.begin();
		for(int exp = 1; exp <= 5; ++exp) {
			if((it++)->value != exp) { return false; }
		}
		if(it != list.end()) { return false; }
	}
	{
		hxptr<hxtest_node_t, hxconsteval_delete> f = list.pop_front();
		hxptr<hxtest_node_t, hxconsteval_delete> b = list.pop_back();
		if(f->value != 1 || b->value != 5 || list.size() != 3) {
			return false;
		}
	}
	if(list.front().value != 2 || list.back().value != 4) { return false; }
	hxtest_node_t* n20 = ::new hxtest_node_t(20);
	const list_t::iterator i20 = list.insert_after(list.begin(), n20);
	if(i20->value != 20) { return false; }
	{
		list_t::iterator it = list.begin();
		++it;
		++it;
		if(it->value != 3) { return false; }
		const list_t::iterator i30 = list.insert(it, ::new hxtest_node_t(30));
		if(i30->value != 30) { return false; }
		list_t::iterator i30next = i30;
		++i30next;
		if(i30next->value != 3) { return false; }
	}
	if(list.size() != 5) { return false; }
	{
		const int expected[] = { 2, 20, 30, 3, 4 };
		hxsize_t idx = 0;
		for(const hxtest_node_t& n : list) {
			if(n.value != expected[idx++]) { return false; }
		}
		if(idx != 5) { return false; }
	}
	{
		list_t::iterator n20_it = list.begin();
		++n20_it;
		hxptr<hxtest_node_t, hxconsteval_delete> extracted = list.extract(n20_it);
		if(extracted.release() != n20 || list.size() != 4) { return false; }
		if(list.front().value != 2 || list.back().value != 4) { return false; }
		list.push_back(n20);
	}
	if(list.back().value != 20 || list.size() != 5) { return false; }
	list.erase(list.begin(), hxconsteval_delete());
	if(list.size() != 4 || list.front().value != 30) { return false; }
	{
		list_t donor;
		donor.push_back(::new hxtest_node_t(100));
		donor.push_back(::new hxtest_node_t(200));
		list.splice(list.begin(), donor);
		if(!donor.empty() || list.size() != 6) { return false; }
	}
	if(list.front().value != 100 || list.back().value != 20) { return false; }
	{
		const hxsize_t removed = list.remove_if([](hxtest_node_t& n) {
			return n.value > 50;
		});
		if(removed != 2 || list.size() != 4) { return false; }
	}
	if(list.front().value != 30 || list.back().value != 20) { return false; }
	list.reverse();
	if(list.size() != 4) { return false; }
	{
		const int expected[] = { 20, 4, 3, 30 };
		hxsize_t idx = 0;
		list_t::const_iterator it = list.cbegin();
		const list_t::const_iterator begin = list.cbegin();
		if(!(it == begin) || it != begin) { return false; }
		for(; it != list.cend(); ++it) {
			if(it->value != expected[idx++]) { return false; }
		}
		if(idx != 4) { return false; }
	}
	list.clear();
	return true;
}
// GCOVR_EXCL_STOP

static_assert(hxtest_hxconstexpr_list_consteval_integration(),
	"hxconstexpr_list consteval: integration test must pass");
} // namespace
#endif // HX_CPLUSPLUS >= 202302L

hxattr_noinline static void hxtest_gdb_break_hxconstexpr_list(void) { }
#define hxtest_gdb_break_hxlist hxtest_gdb_break_hxconstexpr_list
#define hxlist hxconstexpr_list
#define hxlist_node hxconstexpr_list_node
#define hxlist_test hxconstexpr_list_test
#define hxlist_node_test hxconstexpr_list_node_test
#define hxtest_list_counted_node_t hxtest_const_list_counted_node_t
#define hxtest_list_node_t hxtest_const_list_node_t

#include "hxshared_list_test.inl"
