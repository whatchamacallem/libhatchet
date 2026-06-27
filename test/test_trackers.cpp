// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

#include "test_trackers.hpp"

// -- hxtest_object_fixture ----------------------------------------------------

hxtest_object_fixture* hxs_object_current = hxnull;

hxtest_object_fixture::hxtest_object_fixture(void) :
	m_constructed(0),
	m_destructed(0),
	m_next_id(-1) {
	hxassert(hxs_object_current == hxnull);
	hxs_object_current = this;
}

hxtest_object_fixture::~hxtest_object_fixture(void) {
	hxassert_always(m_constructed == m_destructed, "lifecycle_error");
	hxs_object_current = hxnull;
}

hxtest_object_fixture& hxtest_object_fixture::get(void) {
	hxassertmsg(hxs_object_current != hxnull, "no active hxtest_object_fixture");
	return *hxs_object_current;
}

// -- hxtest_skip_asserts ------------------------------------------------------

int hxtest_skip_asserts::hxs_remaining = 0;
