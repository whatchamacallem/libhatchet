// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

#pragma once
#include <hx/libhatchet.h>
#include <hx/hxtest.hpp>
#include <hx/hxutility.h>

HX_NS_USE

class hxtest_ref_tracker_t {
public:
	explicit hxtest_ref_tracker_t(int x) : value(x) { }
	hxtest_ref_tracker_t(hxtest_ref_tracker_t&& other) noexcept : value(other.value) {
		hxassert(this != &other);
	}
	~hxtest_ref_tracker_t() {
		value = 0xbad;
	}
	hxtest_ref_tracker_t& operator=(hxtest_ref_tracker_t&& other) noexcept {
		hxassert(this != &other);
		value = other.value;
		other.value = 0xbad;
		return *this;
	}
	bool operator<(const hxtest_ref_tracker_t& other) const {
		hxassert(this != &other);
		return value < other.value;
	}
	int value;
private:
	hxtest_ref_tracker_t() = delete;
	hxtest_ref_tracker_t(const hxtest_ref_tracker_t&) = delete;
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
	hxtest_iter_api_t(int null) = delete;
	hxtest_iter_api_t(hxnullptr_t null) = delete;
	void operator[](int index) const = delete;
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

class hxtest_object_fixture :
	public testing::Test
{
public:
	hxtest_object_fixture(void);
	~hxtest_object_fixture();
	bool check_totals(hxsize_t total) const {
		return m_constructed == total && m_destructed == total;
	}
	static hxtest_object_fixture& get(void);
	hxsize_t m_constructed;
	hxsize_t m_destructed;
	int32_t m_next_id;
};

class hxtest_object {
public:
	hxtest_object(void) {
		++hxtest_object_fixture::get().m_constructed;
		id = hxtest_object_fixture::get().m_next_id--;
		moved_from = false;
	}
	hxtest_object(const hxtest_object& x) {
		++hxtest_object_fixture::get().m_constructed;
		id = x.id;
		moved_from = false;
	}
	explicit hxtest_object(int32_t x) {
		EXPECT_GE(x, 0);
		++hxtest_object_fixture::get().m_constructed;
		id = x;
		moved_from = false;
	}
	hxtest_object(hxtest_object&& x) noexcept {
		++hxtest_object_fixture::get().m_constructed;
		id = x.id;
		moved_from = false;
		x.id = 0xefef;
		x.moved_from = true;
	}
	~hxtest_object(void) {
		++hxtest_object_fixture::get().m_destructed;
		id = 0xefef;
		moved_from = true;
	}
	void operator=(const hxtest_object& x) {
		hxassert(this != &x);
		id = x.id;
		moved_from = false;
	}
	hxtest_object& operator=(hxtest_object&& x) noexcept {
		hxassert(this != &x);
		id = x.id;
		moved_from = false;
		x.id = 0xefef;
		x.moved_from = true;
		return *this;
	}
	bool operator==(int32_t x) const { return id == x; }
	bool operator==(const hxtest_object& x) const { return id == x.id; }
	bool operator<(const hxtest_object& x) const { return id < x.id; }
	bool moved_from;
	int32_t id;
};

class hxtest_skip_asserts {
public:
	explicit hxtest_skip_asserts(int count) {
		hxs_remaining = count;
		hxset_assert_handler(handler);
	}
	~hxtest_skip_asserts(void) {
		hxset_assert_handler(hxnull);
		hxs_remaining = 0;
	}
	static int remaining(void) { return hxs_remaining; }
	static bool handler(void) {
		if(hxs_remaining > 0) {
			--hxs_remaining;
			return true;
		}
		return false;
	}
private:
	hxtest_skip_asserts(const hxtest_skip_asserts&) = delete;
	hxtest_skip_asserts& operator=(const hxtest_skip_asserts&) = delete;
	static int hxs_remaining;
};

template<typename T>
class hxtest_pointer_range {
public:
	hxtest_pointer_range(T* b, T* e)
		: begin_ptr(b), end_ptr(e) { }
	T* begin(void) { return begin_ptr; }
	T* end(void) { return end_ptr; }
	const T& operator[](hxsize_t index) const { return begin_ptr[index]; }
	T& operator[](hxsize_t index) { return begin_ptr[index]; }
private:
	T* begin_ptr;
	T* end_ptr;
};

inline bool hxtest_value_less(const hxtest_ref_tracker_t& lhs, const hxtest_ref_tracker_t& rhs) {
	return lhs.value < rhs.value;
}
inline bool hxtest_value_greater(const hxtest_ref_tracker_t& lhs, const hxtest_ref_tracker_t& rhs) {
	return lhs.value > rhs.value;
}
