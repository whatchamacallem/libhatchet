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

HX_NS_BEGIN_

#include "detail/hxsort_detail.hpp"

/// `hxlower_bound` - Returns the first position in the sorted range `[begin,
/// end)` where `value` could be inserted without violating the ordering.
/// Returns `end` if no element is ordered after or equivalent to `value`.
/// Unsorted data will lead to errors. The `less` callable returns true if the
/// first argument is ordered before (i.e., is less than) the second. Requires a
/// `random-iterator`.
template<typename iterator_t_, typename value_t_, typename less_t_>
hxinline hxattr_flatten hxattr_nodiscard hxconstexpr
iterator_t_ hxlower_bound(iterator_t_ begin_, iterator_t_ end_, const value_t_& value_, const less_t_& less_) {
	// Does not dereference null pointer args.
	hxsize_t count_ = end_ - begin_;
	while(count_ > hxsize_t{0}) {
		const hxsize_t step_ = count_ >> 1;
		const iterator_t_ mid_ = begin_ + step_;
		if(less_(*mid_, value_)) {
			begin_ = mid_ + hxsize_t{1};
			count_ -= step_ + hxsize_t{1};
		}
		else {
			count_ = step_;
		}
	}
	return begin_;
}

/// `hxlower_bound` (specialization) - An overload of `hxlower_bound` over the
/// range `[begin, end)` that uses `hxkey_less`. Requires a `random-iterator`.
template<typename iterator_t_, typename value_t_>
hxinline hxattr_flatten hxattr_nodiscard hxconstexpr
iterator_t_ hxlower_bound(iterator_t_ begin_, iterator_t_ end_, const value_t_& value_) {
	return hxlower_bound<iterator_t_>(begin_, end_, value_,
		hxkey_less_t<decltype(*begin_)>{});
}

/// `hxupper_bound` - Returns the first position in the sorted range `[begin,
/// end)` whose element is ordered after `value`. Returns `end` if no such
/// element exists. Unsorted data will lead to errors. The `less` callable
/// returns true if the first argument is ordered before (i.e., is less than)
/// the second. Requires a `random-iterator`.
template<typename iterator_t_, typename value_t_, typename less_t_>
hxinline hxattr_flatten hxattr_nodiscard hxconstexpr
iterator_t_ hxupper_bound(iterator_t_ begin_, iterator_t_ end_, const value_t_& value_, const less_t_& less_) {
	// Does not dereference null pointer args.
	hxsize_t count_ = end_ - begin_;
	while(count_ > hxsize_t{0}) {
		const hxsize_t step_ = count_ >> 1;
		const iterator_t_ mid_ = begin_ + step_;
		if(!less_(value_, *mid_)) {
			begin_ = mid_ + hxsize_t{1};
			count_ -= step_ + hxsize_t{1};
		}
		else {
			count_ = step_;
		}
	}
	return begin_;
}

/// `hxupper_bound` (specialization) - An overload of `hxupper_bound` over the
/// range `[begin, end)` that uses `hxkey_less`. Requires a `random-iterator`.
template<typename iterator_t_, typename value_t_>
hxinline hxattr_flatten hxattr_nodiscard hxconstexpr
iterator_t_ hxupper_bound(iterator_t_ begin_, iterator_t_ end_, const value_t_& value_) {
	return hxupper_bound<iterator_t_>(begin_, end_, value_,
		hxkey_less_t<decltype(*begin_)>{});
}

/// `hxbinary_search` - Performs a binary search for `value` in the range
/// `[first, last)`. Returns `end` if the value is not found. Unsorted data will
/// lead to errors. The first of non-unique values is returned. The `less`
/// callable returns true if the first argument is ordered before (i.e., is less
/// than) the second. Requires a `random-iterator`.
template<typename iterator_t_, typename value_t_, typename less_t_>
hxinline hxattr_flatten hxattr_nodiscard hxconstexpr
iterator_t_ hxbinary_search(iterator_t_ begin_, iterator_t_ end_, const value_t_& value_, const less_t_& less_) {
	const iterator_t_ pos_ = hxlower_bound<iterator_t_>(begin_, end_, value_, less_);
	return (pos_ != end_ && !less_(value_, *pos_)) ? pos_ : end_;
}

/// `hxbinary_search` (specialization) - An overload of `hxbinary_search` that
/// searches for `value` in the range `[begin, end)` using `hxkey_less`. Requires
/// a `random-iterator`.
template<typename iterator_t_, typename value_t_>
hxinline hxattr_flatten hxattr_nodiscard hxconstexpr
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
/// ordering relationship. Requires a `random-iterator`.
template<typename iterator_t_, typename less_t_>
hxinline hxattr_flatten hxconstexpr
void hxinsertion_sort(iterator_t_ begin_, iterator_t_ end_, const less_t_& less_) {
	// Address sanitizer: Avoids adding 1 to null iterators.
	if(begin_ == end_) { return; }
	for(iterator_t_ i_ = begin_, j_ = begin_ + ptrdiff_t{1}; j_ < end_; i_ = j_, ++j_) {
		if(less_(*j_, *i_)) {
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
/// the range `[begin, end)` that uses `hxkey_less`. Requires a
/// `random-iterator`.
template<typename iterator_t_>
hxinline hxattr_flatten hxconstexpr
void hxinsertion_sort(iterator_t_ begin_, iterator_t_ end_) {
	hxinsertion_sort<iterator_t_>(begin_, end_, hxkey_less_t<decltype(*begin_)>{});
}

/// `hxheapsort` - Sorts the elements in the range `[begin, end)` using the
/// heapsort algorithm. The `less` callable defines the less-than ordering
/// relationship. Requires a `random-iterator`.
template<typename iterator_t_, typename less_t_>
hxinline hxconstexpr
void hxheapsort(iterator_t_ begin_, iterator_t_ end_, const less_t_& less_) {
	if(begin_ == end_) { return; } // Prevents UB.
	hxdetail_::hxmake_heap_<iterator_t_>(begin_, end_, less_);
	for(iterator_t_ it_ = end_ - ptrdiff_t{1}; begin_ < it_; --it_) {
		hxswap(*begin_, *it_);
		hxdetail_::hxheapsort_heapify_<iterator_t_>(begin_, begin_, it_, less_);
	}
}

/// `hxheapsort` (specialization) - An overload of `hxheapsort` over the range
/// `[begin, end)` that uses `hxkey_less`. Requires a `random-iterator`.
template<typename iterator_t_>
hxinline hxconstexpr
void hxheapsort(iterator_t_ begin_, iterator_t_ end_) {
	hxheapsort<iterator_t_>(begin_, end_, hxkey_less_t<decltype(*begin_)>{});
}

/// `hxsort` - A general purpose sort routine using `T::T(&&)`, `T::~T()`,
/// `T::operator=(&&)`, the `hxswap` overloads and a `less` callable which
/// defaults to `hxkey_less`. Sorts the range `[begin, end)`. This version is
/// intended for sorting large numbers of small objects. Requires a
/// `random-iterator`.
template<typename iterator_t_, typename less_t_>
hxinline hxconstexpr
void hxsort(iterator_t_ begin_, iterator_t_ end_, const less_t_& less_) {
	const hxsize_t size_ = end_ - begin_;
	hxdetail_::hxintro_sort_<iterator_t_>(begin_, end_, less_,
		size_ > 0 ? 2 * hxlog2i(static_cast<uint32_t>(size_)) : 0);
}

/// `hxsort` (specialization) - An overload of `hxsort` over the range `[begin,
/// end)` that uses `hxkey_less`. This version is intended for sorting large
/// numbers of small objects. Requires a `random-iterator`.
template<typename iterator_t_>
hxinline hxconstexpr
void hxsort(iterator_t_ begin_, iterator_t_ end_) {
	const hxsize_t size_ = end_ - begin_;
	hxdetail_::hxintro_sort_<iterator_t_>(begin_, end_, hxkey_less_t<decltype(*begin_)>{},
		size_ > 0 ? 2 * hxlog2i(static_cast<uint32_t>(size_)) : 0);
}

HX_NS_END_
