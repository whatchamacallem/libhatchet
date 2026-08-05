// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

#include "./hxtest_util.hpp"

// -- hxtest_object_fixture ----------------------------------------------------

hxtest_object_fixture* hxs_object_current = hxnull;

hxtest_object_fixture::hxtest_object_fixture(void) :
		m_constructed(0),
		m_destructed(0),
		m_copy_construct(0),
		m_move_construct(0),
		m_copy_assign(0),
		m_move_assign(0),
		m_equal_to(0),
		m_less_than(0),
		m_check_stats_called(false) {
	hxassert(hxs_object_current == hxnull);
	hxs_object_current = this;
}

hxtest_object_fixture::~hxtest_object_fixture(void) {
	hxassert_always(m_check_stats_called, "check_stats_not_called");
	hxassert_always(m_constructed == m_destructed, "lifecycle_error");
	hxs_object_current = hxnull;
}

bool hxtest_object_fixture::check_stats(int constructed, int destructed,
		int copy_construct, int move_construct,
		int copy_assign, int move_assign,
		int equal_to, int less_than) {
	m_check_stats_called = true;
	bool ok = true;
	// GCOVR_EXCL_START
	if(m_constructed > constructed) {
		hxlog_warning("constructed: expected %d found %d", constructed, m_constructed);
		ok = false;
	}
	if(m_destructed > destructed) {
		hxlog_warning("destructed: expected %d found %d", destructed, m_destructed);
		ok = false;
	}
	if(m_copy_construct > copy_construct) {
		hxlog_warning("copy_construct: expected %d found %d", copy_construct, m_copy_construct);
		ok = false;
	}
	if(m_move_construct > move_construct) {
		hxlog_warning("move_construct: expected %d found %d", move_construct, m_move_construct);
		ok = false;
	}
	if(m_copy_assign > copy_assign) {
		hxlog_warning("copy_assign: expected %d found %d", copy_assign, m_copy_assign);
		ok = false;
	}
	if(m_move_assign > move_assign) {
		hxlog_warning("move_assign: expected %d found %d", move_assign, m_move_assign);
		ok = false;
	}
	if(m_equal_to > equal_to) {
		hxlog_warning("equal_to: expected %d found %d", equal_to, m_equal_to);
		ok = false;
	}
	if(m_less_than > less_than) {
		hxlog_warning("less_than: expected %d found %d", less_than, m_less_than);
		ok = false;
	}
	// GCOVR_EXCL_STOP
	return ok;
}

hxtest_object_fixture& hxtest_object_fixture::get(void) {
	hxassertmsg(hxs_object_current != hxnull, "no active hxtest_object_fixture");
	return *hxs_object_current;
}

// -- hxtest_object --------------------------------------------------------------

hxtest_object::hxtest_object(void) {
	++hxtest_object_fixture::get().m_constructed;
	this->value = 0;
	this->moved_from = 0;
}

hxtest_object::hxtest_object(const hxtest_object& x) {
	hxtest_object_fixture& fixture = hxtest_object_fixture::get();
	++fixture.m_constructed;
	++fixture.m_copy_construct;
	this->value = x.value;
	this->moved_from = 0;
}

hxtest_object::hxtest_object(int32_t v) {
	++hxtest_object_fixture::get().m_constructed;
	this->value = v;
	this->moved_from = 0;
}

hxtest_object::hxtest_object(hxtest_object&& x) noexcept {
	hxtest_object_fixture& fixture = hxtest_object_fixture::get();
	++fixture.m_constructed;
	++fixture.m_move_construct;
	this->value = x.value;
	this->moved_from = 0;
	x.value = 0xbad;
	x.moved_from = 1;
}

hxtest_object::~hxtest_object(void) {
	++hxtest_object_fixture::get().m_destructed;
	this->value = 0xbad;
	this->moved_from = 1;
}

void hxtest_object::operator=(const hxtest_object& x) {
	hxassert(this != &x);
	++hxtest_object_fixture::get().m_copy_assign;
	this->value = x.value;
	this->moved_from = 0;
}

hxtest_object& hxtest_object::operator=(hxtest_object&& x) noexcept {
	hxassert(this != &x);
	++hxtest_object_fixture::get().m_move_assign;
	value = x.value;
	this->moved_from = 0;
	x.value = 0xbad;
	x.moved_from = 1;
	return *this;
}

bool hxtest_object::operator==(const hxtest_object& x) const {
	++hxtest_object_fixture::get().m_equal_to;
	return this->value == x.value;
}

bool hxtest_object::operator<(const hxtest_object& x) const {
	++hxtest_object_fixture::get().m_less_than;
	return this->value < x.value;
}

bool hxtest_value_less(const hxtest_object& lhs, const hxtest_object& rhs) {
	return lhs.value < rhs.value;
}

bool hxtest_value_greater(const hxtest_object& lhs, const hxtest_object& rhs) {
	return lhs.value > rhs.value;
}

bool hxtest_value_equal(const hxtest_object& lhs, const hxtest_object& rhs) {
	return lhs.value == rhs.value;
}

// -- hxtest_bidirectional_iter_api_t ---------------------------------------------

hxtest_bidirectional_iter_api_t& hxtest_bidirectional_iter_api_t::operator--(void) {
	--m_pointer; return *this;
}

hxtest_bidirectional_iter_api_t hxtest_bidirectional_iter_api_t::operator--(int) {
	hxtest_bidirectional_iter_api_t it(*this); --m_pointer; return it;
}

// -- hxtest_rand_iter_api_t -------------------------------------------------------

hxtest_rand_iter_api_t& hxtest_rand_iter_api_t::operator--(void) {
	--m_pointer; return *this;
}

hxtest_rand_iter_api_t hxtest_rand_iter_api_t::operator--(int) {
	hxtest_rand_iter_api_t it(*this); --m_pointer; return it;
}

hxtest_rand_iter_api_t hxtest_rand_iter_api_t::operator+(ptrdiff_t offset) const {
	return hxtest_rand_iter_api_t(m_pointer + offset);
}

hxtest_rand_iter_api_t hxtest_rand_iter_api_t::operator-(ptrdiff_t offset) const {
	return hxtest_rand_iter_api_t(m_pointer - offset);
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
