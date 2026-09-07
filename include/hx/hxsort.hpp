#pragma once
// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

/// \file
/// Standard sorting utilities. For scalar key sorting see `hxradix_sort.hpp`.
/// Includes support for callables when defining custom key operations.
/// Otherwise `T::operator<(const T&)` and `T::operator==(const T&)` are used.
/// The other relational operators are not used.

#include "libhatchet.h"

// HX_USE_MACROS_WITH_MODULE allows including macros alongside the module.
#if HX_USE_MACROS_WITH_MODULE
#error Header does not provide macros alone.
#endif

#include "hxkey.hpp"
#include "hxrange.hpp"

HX_NS_BEGIN_

#include "detail/hxsort_detail.hpp"

/// `hxinsertion_sort` - Sorts the elements in the range `[begin, end)` in
/// comparison order using the insertion sort algorithm. The `end` parameter
/// points just past the end of the array. Exception handling during operation
/// is undefined. Declare your copy constructor and assignment operator
/// `noexcept` or turn off exceptions. All of `T::T(&&)`, `T::~T()` and
/// `T::operator=(T&&)` are used. The `less` callable defines the less-than
/// ordering relationship. Requires a `random-iterator`.
template<hxrandom_iterator_concept_ iterator_t_, typename less_t_>
hxinline hxconstexpr hxattr_flatten
void hxinsertion_sort(iterator_t_ begin_, iterator_t_ end_, const less_t_& less_) {
	// Address sanitizer: Avoids adding 1 to null iterators.
	if(begin_ == end_) { return; }
	const hxrestrict_t<iterator_t_> begin_r_(begin_);
	for(iterator_t_ i_ = begin_r_, j_ = begin_r_ + ptrdiff_t{1}; j_ < end_; i_ = j_, ++j_) {
		if(less_(*j_, *i_)) {
			auto t_ = hxmove(*j_);
			*j_ = hxmove(*i_);
			while(begin_r_ < i_ && less_(t_, *(i_ - ptrdiff_t{1}))) {
				*i_ = hxmove(*(i_ - ptrdiff_t{1}));
				--i_;
			}
			*i_ = hxmove(t_);
		}
	}
}

/// `hxinsertion_sort` (specialization) - An overload of `hxinsertion_sort` over
/// the range `[begin, end)` that uses `hxkey_less`. Requires a
/// `random-iterator`.
template<hxsorted_iterator_concept_ iterator_t_>
hxinline hxconstexpr hxattr_flatten
void hxinsertion_sort(iterator_t_ begin_, iterator_t_ end_) {
	hxinsertion_sort<iterator_t_>(begin_, end_, hxkey_less_t<decltype(*begin_)>{});
}

/// `hxheapsort` - Sorts the elements in the range `[begin, end)` using the
/// heapsort algorithm. The `less` callable defines the less-than ordering
/// relationship. Requires a `random-iterator`.
template<hxrandom_iterator_concept_ iterator_t_, typename less_t_>
hxinline hxconstexpr void hxheapsort(iterator_t_ begin_, iterator_t_ end_, const less_t_& less_) {
	if(begin_ == end_) { return; } // Prevents UB.
	hxdetail_::hxmake_heap_<iterator_t_>(begin_, end_, less_);
	const hxrestrict_t<iterator_t_> begin_r_(begin_);
	for(iterator_t_ it_ = end_ - ptrdiff_t{1}; begin_r_ < it_; --it_) {
		hxswap(*begin_r_, *it_);
		hxdetail_::hxheapsort_heapify_<iterator_t_>(begin_r_, begin_r_, it_, less_);
	}
}

/// `hxheapsort` (specialization) - An overload of `hxheapsort` over the range
/// `[begin, end)` that uses `hxkey_less`. Requires a `random-iterator`.
template<hxsorted_iterator_concept_ iterator_t_>
hxinline hxconstexpr void hxheapsort(iterator_t_ begin_, iterator_t_ end_) {
	hxheapsort<iterator_t_>(begin_, end_, hxkey_less_t<decltype(*begin_)>{});
}

/// `hxsort` - A general purpose sort routine using `T::T(&&)`, `T::~T()`,
/// `T::operator=(&&)`, the `hxswap` overloads and a `less` callable which
/// defaults to `hxkey_less`. Sorts the range `[begin, end)`. This version is
/// intended for sorting large numbers of small objects. Requires a
/// `random-iterator`.
template<hxrandom_iterator_concept_ iterator_t_, typename less_t_>
hxinline hxconstexpr void hxsort(iterator_t_ begin_, iterator_t_ end_, const less_t_& less_) {
	const hxsize_t size_ = end_ - begin_;
	hxdetail_::hxintro_sort_<iterator_t_>(begin_, end_, less_,
		size_ > 0 ? 2 * hxlog2i(static_cast<uint32_t>(size_)) : 0);
}

template<hxsorted_iterator_concept_ iterator_t_>
hxinline hxconstexpr void hxsort(iterator_t_ begin_, iterator_t_ end_) {
	const hxsize_t size_ = end_ - begin_;
	hxdetail_::hxintro_sort_<iterator_t_>(begin_, end_, hxkey_less_t<decltype(*begin_)>{},
		size_ > 0 ? 2 * hxlog2i(static_cast<uint32_t>(size_)) : 0);
}

HX_NS_END_
