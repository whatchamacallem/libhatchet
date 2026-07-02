#pragma once
// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

/// \file
/// Searching and set utilities for libhatchet. Includes support for callables
/// when defining custom key operations. Otherwise `T::operator<(const T&)` and
/// `T::operator==(const T&)` are used. The other relational operators are
/// not used.
///
/// See `hxsort.hpp` for sorting algorithms including `hxinsertion_sort`,
/// `hxheapsort` and `hxsort`. `hxbinary_search` is also over there.

#include "libhatchet.h"

// HX_USE_MACROS_WITH_MODULE allows including macros alongside the module.
#if HX_USE_MACROS_WITH_MODULE
#error Header does not provide macros alone.
#endif

#include "hxkey.hpp"

HX_NS_BEGIN_

/// `hxall_of` - Returns true if the predicate `callable` returns true for every
/// element in `[begin, end)`. Returns true for an empty range. Requires a
/// `forward-iterator`.
template<typename iterator_t_, typename callable_t_> hxattr_hot hxattr_nodiscard hxconstexpr
bool hxall_of(iterator_t_ begin_, iterator_t_ end_, callable_t_&& callable_) {
	for(; begin_ != end_; ++begin_) {
		if(!hxforward<callable_t_>(callable_)(*begin_)) {
			return false;
		}
	}
	return true;
}

/// `hxany_of` - Returns true if the predicate `callable` returns true for at
/// least one element in `[begin, end)`. Returns false for an empty range.
/// Requires a `forward-iterator`.
template<typename iterator_t_, typename callable_t_> hxattr_hot hxattr_nodiscard hxconstexpr
bool hxany_of(iterator_t_ begin_, iterator_t_ end_, callable_t_&& callable_) {
	for(; begin_ != end_; ++begin_) {
		if(hxforward<callable_t_>(callable_)(*begin_)) {
			return true;
		}
	}
	return false;
}

/// `hxcount_if` - Returns the number of elements in `[begin, end)` for which
/// the predicate `callable` returns true. Requires a `forward-iterator`.
template<typename iterator_t_, typename callable_t_> hxattr_hot hxattr_nodiscard hxconstexpr
hxsize_t hxcount_if(iterator_t_ begin_, iterator_t_ end_, callable_t_&& callable_) {
	hxsize_t count_ = hxsize_t{0};
	for(; begin_ != end_; ++begin_) {
		if(hxforward<callable_t_>(callable_)(*begin_)) {
			++count_;
		}
	}
	return count_;
}

/// `hxexchange` - Replaces `obj` with `new_value` and returns the old value of
/// `obj`. `T` must be move-constructible and `U` must be assignable to `T`.
/// Useful in constructor initialization lists.
template<typename T_, typename U_> hxattr_nodiscard hxconstexpr
T_ hxexchange(T_& obj_, U_&& new_value_) noexcept {
	T_ old_(hxmove(obj_));
	obj_ = hxforward<U_>(new_value_);
	return old_;
}

/// `hxfind_if` - Returns an iterator to the first element in `[begin, end)` for
/// which the predicate `callable` returns true. Returns `end` if no element
/// matches. Requires a `forward-iterator`.
template<typename iterator_t_, typename callable_t_> hxattr_hot hxattr_nodiscard hxconstexpr
iterator_t_ hxfind_if(iterator_t_ begin_, iterator_t_ end_, callable_t_&& callable_) {
	for(; begin_ != end_; ++begin_) {
		if(hxforward<callable_t_>(callable_)(*begin_)) {
			return begin_;
		}
	}
	return end_;
}

/// `hxfor_each` - Applies the `callable` to each element in `[begin, end)` in
/// order. Returns the `callable` after it has been applied to every element.
/// Requires a `forward-iterator`.
template<typename iterator_t_, typename callable_t_> hxattr_hot hxconstexpr
callable_t_ hxfor_each(iterator_t_ begin_, iterator_t_ end_, callable_t_&& callable_) {
	for(; begin_ != end_; ++begin_) {
		hxforward<callable_t_>(callable_)(*begin_);
	}
	return hxforward<callable_t_>(callable_);
}

/// `hxmerge` - Performs a stable merge of two ordered ranges `[begin0, end0)`
/// and `[begin1, end1)` -> `output`. The input arrays must not overlap each
/// other or the destination array. Elements are move-assigned out of the input
/// ranges. Passing a hxvector as an output iterator like this `hxmerge<const
/// int*, hxvector<int>&>(...)` will append to the array. Assumes both `[begin0,
/// end0)` and `[begin1, end1)` are ordered by the `less` callable. Returns an
/// output iterator positioned one past the last element written. Requires a
/// `random-iterator`.
template<typename iterator_t_, typename output_iterator_t_, typename less_t_> hxattr_hot hxconstexpr
output_iterator_t_ hxmerge(iterator_t_ begin0_, iterator_t_ end0_, iterator_t_ begin1_, iterator_t_ end1_,
		output_iterator_t_&& output_, const less_t_& less_) noexcept {
	hxrestrict_t<iterator_t_> src0_(begin0_);
	hxrestrict_t<iterator_t_> src1_(begin1_);
	hxrestrict_t<output_iterator_t_> output_r_(output_);
	while(src0_ != end0_ && src1_ != end1_) {
		const bool take1_ = less_(*src1_, *src0_);
		const hxrestrict_t<iterator_t_> sel_ = take1_ ? src1_ : src0_;
		*output_r_ = hxmove(*sel_);
		++output_r_;
		src1_ = src1_ + static_cast<ptrdiff_t>(take1_);
		src0_ = src0_ + static_cast<ptrdiff_t>(!take1_);
	}
	while(src0_ != end0_) {
		*output_r_ = hxmove(*src0_);
		++output_r_; ++src0_;
	}
	while(src1_ != end1_) {
		*output_r_ = hxmove(*src1_);
		++output_r_; ++src1_;
	}
	return output_r_;
}

/// `hxmerge` (specialization) - Performs a stable merge of two ordered ranges
/// `[begin0, end0)` and `[begin1, end1)` -> `output`. The input arrays must not
/// overlap the destination array. Assumes both `[begin0, end0)` and `[begin1,
/// end1)` are ordered by `hxless(a,b)`.  Passing a hxvector as an output
/// iterator like this `hxmerge<const int*, hxvector<int>&>(...)` will append to
/// the array. Returns an output iterator positioned one past the last element
/// written. Requires a `random-iterator`.
template<typename iterator_t_, typename output_iterator_t_> hxattr_hot hxconstexpr
output_iterator_t_ hxmerge(iterator_t_ begin0_, iterator_t_ end0_, iterator_t_ begin1_,
		iterator_t_ end1_, output_iterator_t_&& output_) noexcept {
	return hxmerge<iterator_t_, output_iterator_t_>(begin0_, end0_, begin1_, end1_,
		hxforward<output_iterator_t_>(output_), hxkey_less_t<decltype(*begin0_)>{});
}

/// `hxminmax_result` - Return type of `hxminmax` holding iterators to the
/// minimum and maximum elements of a range.
template<typename iterator_t_>
class hxminmax_result {
public:
	iterator_t_ min;
	iterator_t_ max;
};

/// `hxminmax` - Returns an `hxminmax_result` with iterators to the smallest and
/// largest elements in `[begin, end)` as determined by `less`. Both `min` and
/// `max` equal `end` for an empty range. When multiple elements compare equal
/// and minimal, the first is returned. When multiple elements compare equal and
/// maximal, the first is returned. The `less` callable defines the less-than
/// ordering relationship. Requires a `forward-iterator`.
template<typename iterator_t_, typename less_t_> hxattr_hot hxattr_nodiscard hxconstexpr
hxminmax_result<iterator_t_> hxminmax(iterator_t_ begin_, iterator_t_ end_, const less_t_& less_) {
	hxminmax_result<iterator_t_> result_{begin_, begin_};
	if(begin_ == end_) { return result_; }
	for(++begin_; begin_ != end_; ++begin_) {
		if(less_(*begin_, *result_.min)) {
			result_.min = begin_;
		}
		else if(less_(*result_.max, *begin_)) {
			result_.max = begin_;
		}
	}
	return result_;
}

/// `hxminmax` (specialization) - An overload of `hxminmax` that uses
/// `hxkey_less` over the range `[begin, end)`. Requires a `forward-iterator`.
template<typename iterator_t_> hxattr_hot hxattr_nodiscard hxconstexpr
hxminmax_result<iterator_t_> hxminmax(iterator_t_ begin_, iterator_t_ end_) {
	return hxminmax<iterator_t_>(begin_, end_, hxkey_less_t<decltype(*begin_)>{});
}

/// `hxset_difference` - Forms the difference of two ordered ranges `[begin0,
/// end0)` and `[begin1, end1)` into `output`. The output contains keys that
/// appear in the first range but not the second. The input arrays must not
/// overlap each other or the destination array. Elements from the first range
/// are move-assigned into the output. Passing a hxvector as an output iterator
/// like this `hxset_difference<const int*, hxvector<int>&>(...)` will append to
/// the array. Assumes both ranges are ordered by the `less` callable. Returns
/// an output iterator positioned one past the last element written. Requires a
/// `random-iterator`.
template<typename iterator_t_, typename output_iterator_t_, typename less_t_> hxattr_hot hxconstexpr
output_iterator_t_ hxset_difference(iterator_t_ begin0_, iterator_t_ end0_, iterator_t_ begin1_,
		iterator_t_ end1_, output_iterator_t_&& output_, const less_t_& less_) noexcept {
	hxrestrict_t<iterator_t_> src0_(begin0_);
	hxrestrict_t<iterator_t_> src1_(begin1_);
	hxrestrict_t<output_iterator_t_> output_r_(output_);
	while(src0_ != end0_ && src1_ != end1_) {
		const bool lt_ = less_(*src0_, *src1_);
		const bool gt_ = less_(*src1_, *src0_);
		if(lt_) {
			*output_r_ = hxmove(*src0_);
			++output_r_;
		}
		src0_ = src0_ + static_cast<ptrdiff_t>(!gt_);
		src1_ = src1_ + static_cast<ptrdiff_t>(!lt_);
	}
	while(src0_ != end0_) {
		*output_r_ = hxmove(*src0_);
		++output_r_; ++src0_;
	}
	return output_r_;
}

/// `hxset_difference` (specialization) - Forms the difference of two ordered
/// ranges `[begin0, end0)` and `[begin1, end1)` into `output` using `hxless`.
/// The output contains keys that appear in the first range but not the second.
/// The input arrays must not overlap the destination array. Passing a hxvector
/// as an output iterator like this `hxset_difference<const int*,
/// hxvector<int>&>(...)` will append to the array. Returns an output iterator
/// positioned one past the last element written. Requires a `random-iterator`.
template<typename iterator_t_, typename output_iterator_t_> hxattr_hot hxconstexpr
output_iterator_t_ hxset_difference(iterator_t_ begin0_, iterator_t_ end0_, iterator_t_ begin1_,
		iterator_t_ end1_, output_iterator_t_&& output_) noexcept {
	return hxset_difference<iterator_t_, output_iterator_t_>(begin0_, end0_, begin1_, end1_,
		hxforward<output_iterator_t_>(output_), hxkey_less_t<decltype(*begin0_)>{});
}

/// `hxset_intersection` - Forms the intersection of two ordered ranges
/// `[begin0, end0)` and `[begin1, end1)` into `output`. Only keys present in
/// both ranges appear in the output. The input arrays must not overlap each
/// other or the destination array. Elements from the first range are
/// move-assigned into the output. Passing a hxvector as an output iterator like
/// this `hxset_intersection<const int*, hxvector<int>&>(...)` will append to
/// the array. Assumes both ranges are ordered by the `less` callable. Returns
/// an output iterator positioned one past the last element written. Requires a
/// `random-iterator`.
template<typename iterator_t_, typename output_iterator_t_, typename less_t_> hxattr_hot hxconstexpr
output_iterator_t_ hxset_intersection(iterator_t_ begin0_, iterator_t_ end0_, iterator_t_ begin1_,
		iterator_t_ end1_, output_iterator_t_&& output_, const less_t_& less_) noexcept {
	hxrestrict_t<iterator_t_> src0_(begin0_);
	hxrestrict_t<iterator_t_> src1_(begin1_);
	hxrestrict_t<output_iterator_t_> output_r_(output_);
	while(src0_ != end0_ && src1_ != end1_) {
		const bool lt_ = less_(*src0_, *src1_);
		const bool gt_ = less_(*src1_, *src0_);
		if(!lt_ && !gt_) {
			*output_r_ = hxmove(*src0_);
			++output_r_;
		}
		src0_ = src0_ + static_cast<ptrdiff_t>(!gt_);
		src1_ = src1_ + static_cast<ptrdiff_t>(!lt_);
	}
	return output_r_;
}

/// `hxset_intersection` (specialization) - Forms the intersection of two
/// ordered ranges `[begin0, end0)` and `[begin1, end1)` into `output` using
/// `hxless`. Only keys present in both ranges appear in the output. The input
/// arrays must not overlap the destination array. Passing a hxvector as an
/// output iterator like this `hxset_intersection<const int*,
/// hxvector<int>&>(...)` will append to the array. Returns an output iterator
/// positioned one past the last element written. Requires a `random-iterator`.
template<typename iterator_t_, typename output_iterator_t_> hxattr_hot hxconstexpr
output_iterator_t_ hxset_intersection(iterator_t_ begin0_, iterator_t_ end0_, iterator_t_ begin1_,
		iterator_t_ end1_, output_iterator_t_&& output_) noexcept {
	return hxset_intersection<iterator_t_, output_iterator_t_>(begin0_, end0_, begin1_, end1_,
		hxforward<output_iterator_t_>(output_), hxkey_less_t<decltype(*begin0_)>{});
}

/// `hxset_union` - Forms the union of two ordered ranges `[begin0, end0)` and
/// `[begin1, end1)` into `output`. A key appearing `m` times in the first range
/// and `n` times in the second appears `max(m, n)` times in the output. The
/// input arrays must not overlap each other or the destination array. Elements
/// are move-assigned out of the input ranges. Passing a hxvector as an output
/// iterator like this `hxset_union<const int*, hxvector<int>&>(...)` will
/// append to the array. Assumes both ranges are ordered by the `less` callable.
/// Returns an output iterator positioned one past the last element written.
/// Requires a `random-iterator`.
template<typename iterator_t_, typename output_iterator_t_, typename less_t_> hxattr_hot hxconstexpr
output_iterator_t_ hxset_union(iterator_t_ begin0_, iterator_t_ end0_, iterator_t_ begin1_,
		iterator_t_ end1_, output_iterator_t_&& output_, const less_t_& less_) noexcept {
	hxrestrict_t<iterator_t_> src0_(begin0_);
	hxrestrict_t<iterator_t_> src1_(begin1_);
	hxrestrict_t<output_iterator_t_> output_r_(output_);
	while(src0_ != end0_ && src1_ != end1_) {
		const bool take1_ = less_(*src1_, *src0_);
		const bool less0_ = less_(*src0_, *src1_);
		const hxrestrict_t<iterator_t_> sel_ = take1_ ? src1_ : src0_;
		*output_r_ = hxmove(*sel_);
		++output_r_;
		src0_ = src0_ + static_cast<ptrdiff_t>(!take1_);
		src1_ = src1_ + static_cast<ptrdiff_t>(!less0_);
	}
	while(src0_ != end0_) {
		*output_r_ = hxmove(*src0_);
		++output_r_; ++src0_;
	}
	while(src1_ != end1_) {
		*output_r_ = hxmove(*src1_);
		++output_r_; ++src1_;
	}
	return output_r_;
}

/// `hxset_union` (specialization) - Forms the union of two ordered ranges
/// `[begin0, end0)` and `[begin1, end1)` into `output` using `hxless`. A key
/// appearing `m` times in the first range and `n` times in the second appears
/// `max(m, n)` times in the output. The input arrays must not overlap the
/// destination array. Passing a hxvector as an output iterator like this
/// `hxset_union<const int*, hxvector<int>&>(...)` will append to the array.
/// Returns an output iterator positioned one past the last element written.
/// Requires a `random-iterator`.
template<typename iterator_t_, typename output_iterator_t_> hxattr_hot hxconstexpr
output_iterator_t_ hxset_union(iterator_t_ begin0_, iterator_t_ end0_, iterator_t_ begin1_,
		iterator_t_ end1_, output_iterator_t_&& output_) noexcept {
	return hxset_union<iterator_t_, output_iterator_t_>(begin0_, end0_, begin1_, end1_,
		hxforward<output_iterator_t_>(output_), hxkey_less_t<decltype(*begin0_)>{});
}

/// `hxunique` - Removes consecutive duplicate elements from the range `[begin,
/// end)` using `equal` to identify duplicates. Elements are moved into place
/// using move-assignment. Returns an iterator to the new end of the range.
/// Elements past the returned iterator are in a valid but unspecified state.
/// The range must be sorted to remove all duplicates. The `equal` callable
/// returns true when two elements are considered equal. Requires a
/// `forward-iterator`.
template<typename iterator_t_, typename equal_t_> hxattr_hot hxattr_nodiscard hxconstexpr
iterator_t_ hxunique(iterator_t_ begin_, iterator_t_ end_, const equal_t_& equal_) noexcept {
	if(begin_ == end_) { return end_; }
	iterator_t_ dst_ = begin_;
	for(++begin_; begin_ != end_; ++begin_, ++dst_) {
		if(equal_(*dst_, *begin_)) { break; }
	}
	if(begin_ == end_) { return end_; }
	for(++begin_; begin_ != end_; ++begin_) {
		if(!equal_(*dst_, *begin_)) {
			++dst_;
			*dst_ = hxmove(*begin_);
		}
	}
	return ++dst_;
}

/// `hxunique` (specialization) - An overload of `hxunique` that uses
/// `hxkey_equal` over the range `[begin, end)`. Requires a `forward-iterator`.
template<typename iterator_t_> hxattr_hot hxattr_nodiscard hxconstexpr
iterator_t_ hxunique(iterator_t_ begin_, iterator_t_ end_) noexcept {
	return hxunique<iterator_t_>(begin_, end_, hxkey_equal_t<decltype(*begin_)>{});
}

HX_NS_END_
