// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

#pragma once

#include <hx/libhatchet.h>

HX_NS_USE

// This codebase has contracts about which operators are required and which are
// not. hxtest_ref_tracker_t enforces that only the correct operators are used
// when manipulating object instances. hxtest_iter_api_t does the same for
// iterators.

class hxtest_ref_tracker_t {
public:
	// This is not used by the sort code.
	explicit hxtest_ref_tracker_t(int x) : value(x) { }

	// This is what is being used.

	hxtest_ref_tracker_t(hxtest_ref_tracker_t&& other) : value(other.value) {
		// Callee may leave itself in an unusable state or crash.
		hxassert(this != &other);
	}

	~hxtest_ref_tracker_t() {
		value = 0xbad;
	}

	hxtest_ref_tracker_t& operator=(hxtest_ref_tracker_t&& other) {
		// Callee may leave itself in an unusable state or crash.
		hxassert(this != &other);
		value = other.value;
		other.value = 0xbad;
		return *this;
	}

	// Called by hxkey_less.
	bool operator<(const hxtest_ref_tracker_t& other) const {
		// Technically legal but indicates an optimization issue.
		hxassert(this != &other);
		return value < other.value;
	}

	int value;

private:
	// This is what is not being used.

	hxtest_ref_tracker_t() = delete;
	hxtest_ref_tracker_t(const hxtest_ref_tracker_t&) = delete;
	// Did you use hxmove?
	hxtest_ref_tracker_t& operator=(const hxtest_ref_tracker_t&) = delete;
	bool operator==(const hxtest_ref_tracker_t&) const = delete;
	bool operator!=(const hxtest_ref_tracker_t&) const = delete;
	bool operator>(const hxtest_ref_tracker_t&) const = delete;
	bool operator>=(const hxtest_ref_tracker_t&) const = delete;
	bool operator<=(const hxtest_ref_tracker_t&) const = delete;
	bool operator!(void) const = delete;
	operator bool(void) const = delete;
};

class hxtest_iter_api_t {
public:
	explicit hxtest_iter_api_t(hxtest_ref_tracker_t* pointer) : m_pointer(pointer) { }
	hxtest_iter_api_t(const hxtest_iter_api_t& x) = default;
	hxtest_iter_api_t& operator=(const hxtest_iter_api_t& x) = default;

	// Require only the standard pointer operations. No array notation.
	hxtest_ref_tracker_t& operator*(void) const { hxassert(m_pointer != hxnull); return *m_pointer; }

	hxtest_iter_api_t& operator++(void) { hxassert(m_pointer != hxnull); ++m_pointer; return *this; }
	hxtest_iter_api_t& operator--(void) { hxassert(m_pointer != hxnull); --m_pointer; return *this; }
	hxtest_iter_api_t operator+(ptrdiff_t offset) const { hxassert(m_pointer != hxnull); return hxtest_iter_api_t(m_pointer + offset); }
	hxtest_iter_api_t operator-(ptrdiff_t offset) const { hxassert(m_pointer != hxnull); return hxtest_iter_api_t(m_pointer - offset); }
	ptrdiff_t operator-(const hxtest_iter_api_t& other) const { hxassert(m_pointer != hxnull); return m_pointer - other.m_pointer; }

	bool operator==(const hxtest_iter_api_t& other) const { return m_pointer == other.m_pointer; }
	bool operator!=(const hxtest_iter_api_t& other) const { return m_pointer != other.m_pointer; }
	bool operator<(const hxtest_iter_api_t& other) const { return m_pointer < other.m_pointer; }
	bool operator>(const hxtest_iter_api_t& other) const { return m_pointer > other.m_pointer; }
	bool operator<=(const hxtest_iter_api_t& other) const { return m_pointer <= other.m_pointer; }
	bool operator>=(const hxtest_iter_api_t& other) const { return m_pointer >= other.m_pointer; }
private:
	// This is what is not being used.

	// Not hxnull, hxnullptr. Return the "end" instead.
	hxtest_iter_api_t(int null) = delete;
	hxtest_iter_api_t(hxnullptr_t null) = delete;
	void operator[](int index) const = delete;

	// No post-increment or -decrement.
	hxtest_iter_api_t operator++(int) = delete;
	hxtest_iter_api_t operator--(int) = delete;

	hxtest_ref_tracker_t& operator+=(const hxtest_ref_tracker_t&) = delete;
	hxtest_ref_tracker_t& operator-=(const hxtest_ref_tracker_t&) = delete;
	bool operator&&(const hxtest_ref_tracker_t&) const = delete;
	bool operator||(const hxtest_ref_tracker_t&) const = delete;
	bool operator!(void) const = delete;
	operator bool(void) const = delete;

	hxtest_ref_tracker_t* m_pointer;
};

inline bool hxtest_value_less(const hxtest_ref_tracker_t& lhs, const hxtest_ref_tracker_t& rhs) {
	return lhs.value < rhs.value;
}

inline bool hxtest_value_greater(const hxtest_ref_tracker_t& lhs, const hxtest_ref_tracker_t& rhs) {
	return lhs.value > rhs.value;
}
