// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

#pragma once

#include <hx/hxkey.hpp>
#include <hx/hxtest.hpp>

HX_NS_USE
namespace hxtest_util {

class hxtest_object_fixture :
	public testing::Test
{
public:
	hxtest_object_fixture(void);
	~hxtest_object_fixture();

	bool check_stats(int constructed, int destructed,
		int default_construct, int value_construct,
		int copy_construct, int move_construct,
		int copy_assign, int move_assign,
		int equal_to, int less_than);

	bool check_no_stats(void);

	static hxtest_object_fixture& get(void);

	uint16_t next_ticket(void);

	int m_constructed;
	int m_destructed;
	int m_copy_construct;
	int m_move_construct;
	int m_copy_assign;
	int m_move_assign;
	int m_default_construct;
	int m_equal_to;
	int m_less_than;
	int m_value_construct;
	bool m_check_stats_called;
	uint16_t m_next_ticket;
};

enum class hxtest_object_state : uint16_t {
	valid = 0xbeef,
	moved = 0xf00d,
	destructed = 0xdead
};

class hxtest_object {
public:
	hxtest_object(void);
	hxtest_object(const hxtest_object& x);
	explicit hxtest_object(int32_t value);
	hxtest_object(int32_t first, int32_t second);
	hxtest_object(hxtest_object&& x) noexcept;
	~hxtest_object(void);
	explicit operator bool(void) const { return this->m_value != 0; }
	void operator=(const hxtest_object& x);
	hxtest_object& operator=(hxtest_object&& x) noexcept;
	bool operator==(int32_t x) const;
	bool operator==(const hxtest_object& x) const;
	bool operator<(const hxtest_object& x) const;
	hxtest_object_state state(void) const;
	uint16_t ticket(void) const;
	int32_t& value(void);
	const int32_t& value(void) const;
private:
	int32_t m_value;
	uint16_t m_ticket;
	hxtest_object_state m_state;
};

bool hxtest_value_less(const hxtest_object& a, const hxtest_object& b);
bool hxtest_value_greater(const hxtest_object& a, const hxtest_object& b);
bool hxtest_value_equal(const hxtest_object& a, const hxtest_object& b);

template<typename derived_t>
class hxtest_iterator_api_base_t {
public:
	hxtest_object& operator*(void) const { return *m_pointer; }
	derived_t& operator++(void) { ++m_pointer; return static_cast<derived_t&>(*this); }
	derived_t operator++(int) { derived_t it(static_cast<derived_t&>(*this)); ++m_pointer; return it; }
	bool operator==(const derived_t& x) const { return m_pointer == x.m_pointer; }
	bool operator!=(const derived_t& x) const { return m_pointer != x.m_pointer; }

	hxtest_iterator_api_base_t(int) = delete;
	hxtest_iterator_api_base_t(hxnil_t) = delete;
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
	explicit hxtest_iterator_api_base_t(hxtest_object* pointer) : m_pointer(pointer) {
		hxassertf(m_pointer != hxnull, "sys_err");
	}

	friend derived_t;

protected:
	hxtest_object* m_pointer;
};

class hxtest_forward_iterator_api_t : public hxtest_iterator_api_base_t<hxtest_forward_iterator_api_t> {
public:
	explicit hxtest_forward_iterator_api_t(hxtest_object* pointer)
		: hxtest_iterator_api_base_t<hxtest_forward_iterator_api_t>(pointer) { }
};

class hxtest_bidirectional_iterator_api_t : public hxtest_iterator_api_base_t<hxtest_bidirectional_iterator_api_t> {
public:
	explicit hxtest_bidirectional_iterator_api_t(hxtest_object* pointer)
		: hxtest_iterator_api_base_t<hxtest_bidirectional_iterator_api_t>(pointer) { }
	hxtest_bidirectional_iterator_api_t& operator--(void);
	hxtest_bidirectional_iterator_api_t operator--(int);
};

class hxtest_rand_iterator_api_t : public hxtest_iterator_api_base_t<hxtest_rand_iterator_api_t> {
public:
	explicit hxtest_rand_iterator_api_t(hxtest_object* pointer)
		: hxtest_iterator_api_base_t<hxtest_rand_iterator_api_t>(pointer) { }
	hxtest_rand_iterator_api_t& operator--(void);
	hxtest_rand_iterator_api_t operator--(int);
	hxtest_rand_iterator_api_t operator+(ptrdiff_t offset) const;
	hxtest_rand_iterator_api_t operator-(ptrdiff_t offset) const;
	ptrdiff_t operator-(const hxtest_rand_iterator_api_t& x) const { return m_pointer - x.m_pointer; }
	bool operator<(const hxtest_rand_iterator_api_t& x) const { return m_pointer < x.m_pointer; }
	hxtest_object& operator[](ptrdiff_t offset) const { return m_pointer[offset]; }
};

// `first` and `last` must bound at least two elements.
template<typename iterator_t>
bool hxtest_check_forward_iterator_api(iterator_t first, iterator_t last) {
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
bool hxtest_check_bidirectional_iterator_api(iterator_t first, iterator_t last) {
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
bool hxtest_check_rand_iterator_api(iterator_t first, iterator_t last) {
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

} // namespace hxtest_util
using namespace hxtest_util;

#if defined HX_USE_NAMESPACE
namespace HX_USE_NAMESPACE {
#endif
template<>
class hxkey_hash_t<hxtest_util::hxtest_object> {
public:
	hxhash_t operator()(const hxtest_util::hxtest_object& x) const;
};
#if defined HX_USE_NAMESPACE
} // namespace HX_USE_NAMESPACE {
#endif
