// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

#pragma once
#include <hx/libhatchet.h>
#include <hx/hxtest.hpp>
#include <hx/hxutility.h>

HX_NS_USE

class hxtest_object_fixture :
	public testing::Test
{
public:
	hxtest_object_fixture(void);
	~hxtest_object_fixture();

	bool check_stats(int constructed, int destructed, int copy_construct, int move_construct,
		int copy_assign, int move_assign, int equal_to, int less_than);

	static hxtest_object_fixture& get(void);

	int m_constructed;
	int m_destructed;
	int m_copy_construct;
	int m_move_construct;
	int m_copy_assign;
	int m_move_assign;
	int m_equal_to;
	int m_less_than;
	bool m_check_stats_called;
};

class hxtest_object {
public:
	hxtest_object(void);
	hxtest_object(const hxtest_object& x);
	explicit hxtest_object(int32_t value);
	hxtest_object(hxtest_object&& x) noexcept;
	~hxtest_object(void);
	void operator=(const hxtest_object& x);
	hxtest_object& operator=(hxtest_object&& x) noexcept;
	bool operator==(int32_t x) const { return this->value == x; }
	bool operator==(const hxtest_object& x) const;
	bool operator<(const hxtest_object& x) const;
	int32_t value;
	int32_t moved_from;
};

bool hxtest_value_less(const hxtest_object& lhs, const hxtest_object& rhs);
bool hxtest_value_greater(const hxtest_object& lhs, const hxtest_object& rhs);
bool hxtest_value_equal(const hxtest_object& lhs, const hxtest_object& rhs);

template<typename derived_t>
class hxtest_iter_api_base_t {
public:
	hxtest_object& operator*(void) const { return *m_pointer; }
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
	hxtest_object& operator[](ptrdiff_t) const = delete;
	bool operator!(void) = delete;
	operator bool(void) = delete;
	bool operator&&(const hxtest_object&) = delete;
	bool operator||(const hxtest_object&) = delete;
private:
	explicit hxtest_iter_api_base_t(hxtest_object* pointer) : m_pointer(pointer) {
		hxassert(m_pointer != hxnull);
	}

	friend derived_t;

protected:
	hxtest_object* m_pointer;
};

class hxtest_forward_iter_api_t : public hxtest_iter_api_base_t<hxtest_forward_iter_api_t> {
public:
	explicit hxtest_forward_iter_api_t(hxtest_object* pointer)
		: hxtest_iter_api_base_t<hxtest_forward_iter_api_t>(pointer) { }
};

class hxtest_bidirectional_iter_api_t : public hxtest_iter_api_base_t<hxtest_bidirectional_iter_api_t> {
public:
	explicit hxtest_bidirectional_iter_api_t(hxtest_object* pointer)
		: hxtest_iter_api_base_t<hxtest_bidirectional_iter_api_t>(pointer) { }
	hxtest_bidirectional_iter_api_t& operator--(void);
	hxtest_bidirectional_iter_api_t operator--(int);
};

class hxtest_rand_iter_api_t : public hxtest_iter_api_base_t<hxtest_rand_iter_api_t> {
public:
	explicit hxtest_rand_iter_api_t(hxtest_object* pointer)
		: hxtest_iter_api_base_t<hxtest_rand_iter_api_t>(pointer) { }
	hxtest_rand_iter_api_t& operator--(void);
	hxtest_rand_iter_api_t operator--(int);
	hxtest_rand_iter_api_t operator+(ptrdiff_t offset) const;
	hxtest_rand_iter_api_t operator-(ptrdiff_t offset) const;
	ptrdiff_t operator-(const hxtest_rand_iter_api_t& other) const { return m_pointer - other.m_pointer; }
	bool operator<(const hxtest_rand_iter_api_t& other) const { return m_pointer < other.m_pointer; }
	hxtest_object& operator[](ptrdiff_t offset) const { return m_pointer[offset]; }
};

template<typename T>
class hxtest_pointer_range {
public:
	hxtest_pointer_range(T* b, T* e)
		: m_begin(b), m_end(e) { }
	T* begin(void) { return m_begin; }
	T* end(void) { return m_end; }
	const T& operator[](hxsize_t index) const { return m_begin[index]; }
	T& operator[](hxsize_t index) { return m_begin[index]; }

private:
	T* m_begin;
	T* m_end;
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

class hxtest_skip_asserts {
public:
	explicit hxtest_skip_asserts(int count);
	~hxtest_skip_asserts(void);
	static int remaining(void) { return hxs_remaining; }
	static bool handler(void);
private:
	hxtest_skip_asserts(const hxtest_skip_asserts&) = delete;
	hxtest_skip_asserts& operator=(const hxtest_skip_asserts&) = delete;
	static int hxs_remaining;
};
