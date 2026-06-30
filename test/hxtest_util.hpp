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
	bool operator==(const hxtest_ref_tracker_t& other) const {
		return value == other.value;
	}
	int value;
private:
	hxtest_ref_tracker_t() = delete;
	hxtest_ref_tracker_t(const hxtest_ref_tracker_t&) = delete;
	hxtest_ref_tracker_t& operator=(const hxtest_ref_tracker_t&) = delete;
	bool operator!=(const hxtest_ref_tracker_t&) const = delete;
	bool operator>(const hxtest_ref_tracker_t&) const = delete;
	bool operator>=(const hxtest_ref_tracker_t&) const = delete;
	bool operator<=(const hxtest_ref_tracker_t&) const = delete;
	bool operator!(void) const = delete;
	operator bool(void) const = delete;
};

template<typename derived_t>
class hxtest_iter_api_base_t {
public:
	hxtest_ref_tracker_t& operator*(void) const { return *m_pointer; }
	derived_t& operator++(void) { ++m_pointer; return static_cast<derived_t&>(*this); }
	derived_t operator++(int) { derived_t it(static_cast<derived_t&>(*this)); ++m_pointer; return it; }
	bool operator==(const derived_t& other) const { return m_pointer == other.m_pointer; }
	bool operator!=(const derived_t& other) const { return m_pointer != other.m_pointer; }

	hxtest_iter_api_base_t(int) = delete;
	hxtest_iter_api_base_t(hxnullptr_t) = delete;
	derived_t& operator--(void) = delete;
	derived_t operator--(int) = delete;
	derived_t operator+(ptrdiff_t) const = delete;
	derived_t operator-(ptrdiff_t) const = delete;
	ptrdiff_t operator-(const derived_t&) const = delete;
	bool operator<(const derived_t&) const = delete;
	bool operator>(const derived_t&) const = delete;
	bool operator<=(const derived_t&) const = delete;
	bool operator>=(const derived_t&) const = delete;
	hxtest_ref_tracker_t& operator[](ptrdiff_t) const = delete;
	bool operator!(void) = delete;
	operator bool(void) = delete;
	bool operator&&(const hxtest_ref_tracker_t&) = delete;
	bool operator||(const hxtest_ref_tracker_t&) = delete;
private:
	explicit hxtest_iter_api_base_t(hxtest_ref_tracker_t* pointer) : m_pointer(pointer) {
		hxassert(m_pointer != hxnull);
	}

	friend derived_t;

protected:
	hxtest_ref_tracker_t* m_pointer;
};

class hxtest_forward_iter_api_t : public hxtest_iter_api_base_t<hxtest_forward_iter_api_t> {
public:
	explicit hxtest_forward_iter_api_t(hxtest_ref_tracker_t* pointer)
		: hxtest_iter_api_base_t<hxtest_forward_iter_api_t>(pointer) { }
};

class hxtest_bidirectional_iter_api_t : public hxtest_iter_api_base_t<hxtest_bidirectional_iter_api_t> {
public:
	explicit hxtest_bidirectional_iter_api_t(hxtest_ref_tracker_t* pointer)
		: hxtest_iter_api_base_t<hxtest_bidirectional_iter_api_t>(pointer) { }
	hxtest_bidirectional_iter_api_t& operator--(void) { --m_pointer; return *this; }
	hxtest_bidirectional_iter_api_t operator--(int) {
		hxtest_bidirectional_iter_api_t it(*this); --m_pointer; return it;
	}
};

class hxtest_rand_iter_api_t : public hxtest_iter_api_base_t<hxtest_rand_iter_api_t> {
public:
	explicit hxtest_rand_iter_api_t(hxtest_ref_tracker_t* pointer)
		: hxtest_iter_api_base_t<hxtest_rand_iter_api_t>(pointer) { }
	hxtest_rand_iter_api_t& operator--(void) { --m_pointer; return *this; }
	hxtest_rand_iter_api_t operator--(int) {
		hxtest_rand_iter_api_t it(*this); --m_pointer; return it;
	}
	hxtest_rand_iter_api_t operator+(ptrdiff_t offset) const { return hxtest_rand_iter_api_t(m_pointer + offset); }
	hxtest_rand_iter_api_t operator-(ptrdiff_t offset) const { return hxtest_rand_iter_api_t(m_pointer - offset); }
	ptrdiff_t operator-(const hxtest_rand_iter_api_t& other) const { return m_pointer - other.m_pointer; }
	bool operator<(const hxtest_rand_iter_api_t& other) const { return m_pointer < other.m_pointer; }
	hxtest_ref_tracker_t& operator[](ptrdiff_t offset) const { return m_pointer[offset]; }
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
		// GCOVR_EXCL_START
		return false;
		// GCOVR_EXCL_STOP
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

// `first` and `last` must bound at least two elements.
template<typename iterator_t>
bool hxtest_check_forward_iter_api(iterator_t first, iterator_t last) {
	iterator_t second = first;
	++second;
	(void)*first;
	iterator_t post = first++;
	bool ok = (post != second) && (first == second);
	ok = ok && (first != last) && (post != last);
	return ok;
}

// `first` and `last` must bound at least two elements.
template<typename iterator_t>
bool hxtest_check_bidirectional_iter_api(iterator_t first, iterator_t last) {
	iterator_t second = first;
	++second;
	(void)*first;
	iterator_t walked = second;
	--walked;
	iterator_t post = second--;
	bool ok = (walked == first) && (second == first) && (post != first);
	ok = ok && (first != last) && (post != last);
	return ok;
}

// `first` and `last` must bound at least two elements.
template<typename iterator_t>
bool hxtest_check_rand_iter_api(iterator_t first, iterator_t last) {
	iterator_t second = first;
	++second;
	(void)*first;
	++first;
	--first;
	iterator_t forward = first + ptrdiff_t{1};
	iterator_t backward = second - ptrdiff_t{1};
	const ptrdiff_t distance = second - first;
	bool ok = (forward == second) && (backward == first);
	ok = ok && (distance == ptrdiff_t{1}) && (first < second);
	ok = ok && (first[ptrdiff_t{1}] == *second) && (first[ptrdiff_t{0}] == *first);
	ok = ok && (first != last) && (second != last);
	return ok;
}

inline bool hxtest_value_less(const hxtest_ref_tracker_t& lhs, const hxtest_ref_tracker_t& rhs) {
	return lhs.value < rhs.value;
}
inline bool hxtest_value_greater(const hxtest_ref_tracker_t& lhs, const hxtest_ref_tracker_t& rhs) {
	return lhs.value > rhs.value;
}
inline bool hxtest_value_equal(const hxtest_ref_tracker_t& lhs, const hxtest_ref_tracker_t& rhs) {
	return lhs.value == rhs.value;
}
