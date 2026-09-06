// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

#include <hx/hxconsole.hpp>

#include "./hxtest_util.hpp"

// -- hxtest_object_fixture ----------------------------------------------------

namespace hxtest_util {

#if !defined _MSC_VER && !defined __wasm__
static_assert(sizeof(hxtest_object) == 8u, "hxtest_object must be 8 bytes");
#endif

hxtest_object_fixture* hxs_object_current = hxnull;

// Test scripts must opt in to operator call counting. E.g. the coverage test
// uses compiler flags that cause stats checking to fail.
static bool hxs_check_stats = false;
hxconsole_variable_named(hxs_check_stats, check_stats);

hxtest_object_fixture::hxtest_object_fixture(void) :
		m_constructed(0),
		m_destructed(0),
		m_copy_construct(0),
		m_move_construct(0),
		m_copy_assign(0),
		m_move_assign(0),
		m_default_construct(0),
		m_equal_to(0),
		m_less_than(0),
		m_value_construct(0),
		m_check_stats_called(false),
		m_next_ticket(100u) {
	hxassertmsg(hxs_object_current == hxnull, "sys_err");
	hxs_object_current = this;
}

hxtest_object_fixture::~hxtest_object_fixture(void) {
	hxassert_always(m_check_stats_called, "check_stats_not_called");
	hxassert_always(m_constructed == m_destructed, "lifecycle_error");
	hxs_object_current = hxnull;
}

bool hxtest_object_fixture::check_stats(int constructed, int destructed,
		int default_construct, int value_construct,
		int copy_construct, int move_construct,
		int copy_assign, int move_assign,
		int equal_to, int less_than) {
	m_check_stats_called = true;
	bool ok = true;
	if(!hxs_check_stats) {
		return ok;
	}
	// GCOVR_EXCL_START
	if(m_constructed != constructed) {
		hxlog_warning("constructed: expected %d found %d", constructed, m_constructed);
		ok = false;
	}
	if(m_destructed != destructed) {
		hxlog_warning("destructed: expected %d found %d", destructed, m_destructed);
		ok = false;
	}
	if(m_copy_construct != copy_construct) {
		hxlog_warning("copy_construct: expected %d found %d", copy_construct, m_copy_construct);
		ok = false;
	}
	if(m_move_construct != move_construct) {
		hxlog_warning("move_construct: expected %d found %d", move_construct, m_move_construct);
		ok = false;
	}
	if(m_copy_assign != copy_assign) {
		hxlog_warning("copy_assign: expected %d found %d", copy_assign, m_copy_assign);
		ok = false;
	}
	if(m_move_assign != move_assign) {
		hxlog_warning("move_assign: expected %d found %d", move_assign, m_move_assign);
		ok = false;
	}
	if(m_equal_to != equal_to) {
		hxlog_warning("equal_to: expected %d found %d", equal_to, m_equal_to);
		ok = false;
	}
	if(m_less_than != less_than) {
		hxlog_warning("less_than: expected %d found %d", less_than, m_less_than);
		ok = false;
	}
	if(m_default_construct != default_construct) {
		hxlog_warning("default_construct: expected %d found %d",
			default_construct, m_default_construct);
		ok = false;
	}
	if(m_value_construct != value_construct) {
		hxlog_warning("value_construct: expected %d found %d",
			value_construct, m_value_construct);
		ok = false;
	}
	return ok;
	// GCOVR_EXCL_STOP
}

bool hxtest_object_fixture::check_no_stats(void) {
	return check_stats(0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
}

hxtest_object_fixture& hxtest_object_fixture::get(void) {
	hxassert_always(hxs_object_current != hxnull, "no_active_fixture");
	return *hxs_object_current;
}

uint16_t hxtest_object_fixture::next_ticket(void) {
	return m_next_ticket++;
}

// -- hxtest_object --------------------------------------------------------------

hxtest_object::hxtest_object(void) {
	hxtest_object_fixture& fixture = hxtest_object_fixture::get();
	++fixture.m_constructed;
	++fixture.m_default_construct;
	this->m_value = 0;
	this->m_ticket = fixture.next_ticket();
	this->m_state = hxtest_object_state::valid;
}

hxtest_object::hxtest_object(const hxtest_object& x) {
	hxassert_always(x.m_state == hxtest_object_state::valid, "copy_from_bad");
	hxtest_object_fixture& fixture = hxtest_object_fixture::get();
	++fixture.m_constructed;
	++fixture.m_copy_construct;
	this->m_value = x.m_value;
	this->m_ticket = x.m_ticket;
	this->m_state = hxtest_object_state::valid;
}

hxtest_object::hxtest_object(int32_t v) {
	hxtest_object_fixture& fixture = hxtest_object_fixture::get();
	++fixture.m_constructed;
	++fixture.m_value_construct;
	this->m_value = v;
	this->m_ticket = fixture.next_ticket();
	this->m_state = hxtest_object_state::valid;
}

hxtest_object::hxtest_object(int32_t first, int32_t second) {
	hxtest_object_fixture& fixture = hxtest_object_fixture::get();
	++fixture.m_constructed;
	++fixture.m_value_construct;
	this->m_value = first + second;
	this->m_ticket = fixture.next_ticket();
	this->m_state = hxtest_object_state::valid;
}

hxtest_object::hxtest_object(hxtest_object&& x) noexcept {
	hxassert_always(x.m_state == hxtest_object_state::valid, "move_from_bad");
	hxtest_object_fixture& fixture = hxtest_object_fixture::get();
	++fixture.m_constructed;
	++fixture.m_move_construct;
	this->m_value = x.m_value;
	this->m_ticket = x.m_ticket;
	this->m_state = hxtest_object_state::valid;
	x.m_value = x.m_ticket = 0xbad;
	x.m_state = hxtest_object_state::moved;
}

hxtest_object::~hxtest_object(void) {
	hxassert_always(this->m_state == hxtest_object_state::valid
		|| this->m_state == hxtest_object_state::moved, "double_destroy");
	++hxtest_object_fixture::get().m_destructed;
	this->m_value = this->m_ticket = 0xbad;
	this->m_state = hxtest_object_state::destructed;
}

void hxtest_object::operator=(const hxtest_object& x) {
	hxassertmsg(this != &x, "self_assign");
	hxassert_always(x.m_state == hxtest_object_state::valid, "copy_from_bad");
	hxassert_always(this->m_state == hxtest_object_state::valid
		|| this->m_state == hxtest_object_state::moved, "copy_to_dead");
	++hxtest_object_fixture::get().m_copy_assign;
	this->m_value = x.m_value;
	this->m_ticket = x.m_ticket;
	this->m_state = hxtest_object_state::valid;
}

hxtest_object& hxtest_object::operator=(hxtest_object&& x) noexcept {
	hxassertmsg(this != &x, "self_assign");
	hxassert_always(x.m_state == hxtest_object_state::valid, "move_from_bad");
	hxassert_always(this->m_state == hxtest_object_state::valid
		|| this->m_state == hxtest_object_state::moved, "move_to_dead");
	++hxtest_object_fixture::get().m_move_assign;
	this->m_value = x.m_value;
	this->m_ticket = x.m_ticket;
	this->m_state = hxtest_object_state::valid;
	x.m_value = x.m_ticket = 0xbad;
	x.m_state = hxtest_object_state::moved;
	return *this;
}

bool hxtest_object::operator==(int32_t x) const {
	hxassert_always(this->m_state == hxtest_object_state::valid, "bad_compare");
	return this->value() == x;
}

bool hxtest_object::operator==(const hxtest_object& x) const {
	hxassert_always(this->m_state == hxtest_object_state::valid, "bad_compare");
	hxassert_always(x.m_state == hxtest_object_state::valid, "bad_compare");
	++hxtest_object_fixture::get().m_equal_to;
	return this->value() == x.value();
}

bool hxtest_object::operator<(const hxtest_object& x) const {
	hxassert_always(this->m_state == hxtest_object_state::valid, "bad_compare");
	hxassert_always(x.m_state == hxtest_object_state::valid, "bad_compare");
	++hxtest_object_fixture::get().m_less_than;
	return this->value() < x.value();
}

hxtest_object_state hxtest_object::state(void) const {
	hxassert_always(this->m_state == hxtest_object_state::valid
		|| this->m_state == hxtest_object_state::moved
		|| this->m_state == hxtest_object_state::destructed, "corrupt_state");
	return m_state;
}

uint16_t hxtest_object::ticket(void) const {
	hxassert_always(this->m_state == hxtest_object_state::valid, "bad_ticket");
	return m_ticket;
}

int32_t& hxtest_object::value(void) {
	hxassert_always(this->m_state == hxtest_object_state::valid, "bad_value");
	return m_value;
}
const int32_t& hxtest_object::value(void) const {
	hxassert_always(this->m_state == hxtest_object_state::valid, "bad_value");
	return m_value;
}

bool hxtest_value_less(const hxtest_object& a, const hxtest_object& b) {
	return a < b;
}

bool hxtest_value_greater(const hxtest_object& a, const hxtest_object& b) {
	return b < a;
}

bool hxtest_value_equal(const hxtest_object& a, const hxtest_object& b) {
	return a == b ;
}

// -- hxtest_bidirectional_iterator_api_t ---------------------------------------------

hxtest_bidirectional_iterator_api_t& hxtest_bidirectional_iterator_api_t::operator--(void) {
	--m_pointer; return *this;
}

hxtest_bidirectional_iterator_api_t hxtest_bidirectional_iterator_api_t::operator--(int) {
	hxtest_bidirectional_iterator_api_t it(*this); --m_pointer; return it;
}

// -- hxtest_rand_iterator_api_t -------------------------------------------------------

hxtest_rand_iterator_api_t& hxtest_rand_iterator_api_t::operator--(void) {
	--m_pointer; return *this;
}

hxtest_rand_iterator_api_t hxtest_rand_iterator_api_t::operator--(int) {
	hxtest_rand_iterator_api_t it(*this); --m_pointer; return it;
}

hxtest_rand_iterator_api_t hxtest_rand_iterator_api_t::operator+(ptrdiff_t offset) const {
	return hxtest_rand_iterator_api_t(m_pointer + offset);
}

hxtest_rand_iterator_api_t hxtest_rand_iterator_api_t::operator-(ptrdiff_t offset) const {
	return hxtest_rand_iterator_api_t(m_pointer - offset);
}

// -- hxtest_skip_asserts ------------------------------------------------------

int hxtest_skip_asserts::hxs_remaining = 0;

hxtest_skip_asserts::hxtest_skip_asserts(int count) {
	hxs_remaining = count;
	hxset_assert_handler(handler);
}

hxtest_skip_asserts::~hxtest_skip_asserts(void) {
	hxset_assert_handler(hxnull);
	hxs_remaining = 0;
}

bool hxtest_skip_asserts::handler(void) {
	if(hxs_remaining > 0) {
		--hxs_remaining;
		return true;
	}
	// GCOVR_EXCL_START
	return false;
	// GCOVR_EXCL_STOP
}

} // namespace hxtest_util

#if defined HX_USE_NAMESPACE
namespace HX_USE_NAMESPACE {
#endif
hxhash_t hxkey_hash_t<hxtest_util::hxtest_object>::operator()(const hxtest_util::hxtest_object& x) const {
	return hxkey_hash(x.value());
}
#if defined HX_USE_NAMESPACE
}
#endif
