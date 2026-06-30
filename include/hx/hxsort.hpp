#pragma once
// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

/// \file
/// Standard sorting utilities. For scalar key sorting see `hxradix_sort.hpp`.

#include "libhatchet.h"

// HX_USE_MACROS_WITH_MODULE allows including macros alongside the module.
#if HX_USE_MACROS_WITH_MODULE
#error Header does not provide macros alone.
#endif

#include "hxkey.hpp"

HX_NS_BEGIN_

#include "detail/hxsort_detail.hpp"

/// `hxbinary_search` - Performs a binary search for `value` in the range
/// `[first, last)`. Returns `end` if the value is not found. Unsorted data will
/// lead to errors. Non-unique values will be selected arbitrarily. The `less`
/// callable returns true if the first argument is ordered before (i.e., is less
/// than) the second.
template<typename iterator_t_, typename value_t_, typename less_t_> hxattr_hot hxattr_nodiscard hxconstexpr
iterator_t_ hxbinary_search(iterator_t_ begin_, iterator_t_ end_, const value_t_& value_, const less_t_& less_) {
	// don't operate on null pointer args. unallocated containers have this.
	if(begin_ == end_) { return end_; }
	iterator_t_ first_ = begin_;
	iterator_t_ last_ = end_;
	while(first_ < last_) {
		iterator_t_ mid_ = first_ + ((last_ - first_) >> 1);
		if(less_(*mid_, value_)) {
			first_ = mid_ + ptrdiff_t{1};
		}
		else if(less_(value_, *mid_)) {
			last_ = mid_;
		}
		else {
			return mid_;
		}
	}
	return end_;
}

/// `hxbinary_search` (specialization) - An overload of `hxbinary_search` that
/// searches for `value` in the range `[begin, end)` using `hxkey_less`.
template<typename iterator_t_, typename value_t_> hxattr_hot hxattr_nodiscard hxconstexpr
iterator_t_ hxbinary_search(iterator_t_ begin_, iterator_t_ end_, const value_t_& value_) {
	return hxbinary_search<iterator_t_>(begin_, end_, value_,
		hxkey_less_t<decltype(*begin_)>{});
}

/// `hxinsertion_sort` - Sorts the elements in the range `[begin, end)` in
/// comparison order using the insertion sort algorithm. The `end` parameter
/// points just past the end of the array. Exception handling during operation
/// is undefined. Declare your copy constructor and assignment operator
/// `noexcept` or turn off exceptions. All of `T::T(&&)`, `T::~T()` and
/// `T::operator=(T&&)` are used. The `less` callable defines the less-than
/// ordering relationship.
template<typename iterator_t_, typename less_t_> hxattr_hot hxconstexpr
void hxinsertion_sort(iterator_t_ begin_, iterator_t_ end_, const less_t_& less_) {
	// Address sanitizer: Avoids adding 1 to null iterators.
	if(begin_ == end_) { return; }
	for(iterator_t_ i_ = begin_, j_ = begin_ + ptrdiff_t{1}; j_ < end_; i_ = j_, ++j_) {
		if(less_(*j_, *i_)) {
			// Move construct. Use hxmove instead of hxswap because it should be
			// more efficient for simple types. Complex types will require an
			// T::operator=(T&&) to be efficient.
			auto t_ = hxmove(*j_);
			*j_ = hxmove(*i_);
			while(begin_ < i_ && less_(t_, *(i_ - ptrdiff_t{1}))) {
				*i_ = hxmove(*(i_ - ptrdiff_t{1}));
				--i_;
			}
			*i_ = hxmove(t_);
		}
	}
}

/// `hxinsertion_sort` (specialization) - An overload of `hxinsertion_sort` over
/// the range `[begin, end)` that uses `hxkey_less`.
template<typename iterator_t_> hxattr_hot hxconstexpr
void hxinsertion_sort(iterator_t_ begin_, iterator_t_ end_) {
	hxinsertion_sort<iterator_t_>(begin_, end_, hxkey_less_t<decltype(*begin_)>{});
}

/// `hxheapsort` - Sorts the elements in the range `[begin, end)` using the
/// heapsort algorithm. The `less` callable defines the less-than ordering
/// relationship.
template<typename iterator_t_, typename less_t_> hxattr_hot hxconstexpr
void hxheapsort(iterator_t_ begin_, iterator_t_ end_, const less_t_& less_) {
	if(begin_ == end_) { return; } // Prevents UB.
	// This is std::make_heap.
	hxdetail_::hxmake_heap_<iterator_t_>(begin_, end_, less_);
	// Swaps the largest values to the end of the array. These two implement
	// std::pop_heap.
	for(iterator_t_ it_ = end_ - ptrdiff_t{1}; begin_ < it_; --it_) {
		hxswap(*begin_, *it_);
		hxdetail_::hxheapsort_heapify_<iterator_t_>(begin_, begin_, it_, less_);
	}
}

/// `hxheapsort` (specialization) - An overload of `hxheapsort` over the range
/// `[begin, end)` that uses `hxkey_less`.
template<typename iterator_t_> hxattr_hot hxconstexpr
void hxheapsort(iterator_t_ begin_, iterator_t_ end_) {
	hxheapsort<iterator_t_>(begin_, end_, hxkey_less_t<decltype(*begin_)>{});
}

/// `hxsort` - A general purpose sort routine using `T::T(&&)`, `T::~T()`,
/// `T::operator=(&&)`, the `hxswap` overloads and a `less` callable which
/// defaults to `hxkey_less`. Sorts the range `[begin, end)`. This version is
/// intended for sorting large numbers of small objects.
template<typename iterator_t_, typename less_t_> hxattr_hot hxconstexpr
void hxsort(iterator_t_ begin_, iterator_t_ end_, const less_t_& less_) {
	// hxlog2i(0) is undefined but unused in this case.
	hxdetail_::hxintro_sort_<iterator_t_>(begin_, end_, less_, 2 * hxlog2i(static_cast<uint32_t>(end_ - begin_)));
}

/// `hxsort` (specialization) - An overload of `hxsort` over the range `[begin,
/// end)` that uses `hxkey_less`. This version is intended for sorting large
/// numbers of small objects.
template<typename iterator_t_> hxattr_hot hxconstexpr
void hxsort(iterator_t_ begin_, iterator_t_ end_) {
	// hxlog2i(0) is undefined but unused in this case.
	hxdetail_::hxintro_sort_<iterator_t_>(begin_, end_, hxkey_less_t<decltype(*begin_)>{},
		2 * hxlog2i(static_cast<uint32_t>(end_ - begin_)));
}

HX_NS_END_
