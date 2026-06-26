#pragma once
// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

/// \file
/// Searching and set utilities for libhatchet. Includes support for callables
/// when defining custom key operations. Otherwise `T::operator<(const T&)` and
/// `T::operator==(const T&)` are used.
///
/// See `hxsort.hpp` for sorting algorithms including `hxinsertion_sort`,
/// `hxheapsort` and `hxsort`. `hxbinary_search` is also over there.

#include "libhatchet.h"

// HX_USE_MODULE allows including macros in addition to the hx module.
#if HX_USE_MODULE
#error Header does not provide macros only.
#endif

#include "hxkey.hpp"

HX_NS_BEGIN_

/// `hxcount_if` - Returns the number of elements in `[begin, end)` for which
/// `predicate` returns true.
/// - `begin` : Iterator to the beginning of the range to search.
/// - `end` : Iterator to one past the last element in the range to search.
/// - `predicate` : A callable returning true for elements to count.
template<typename iterator_t_, typename predicate_t_> hxattr_hot hxattr_nodiscard hxconstexpr
hxsize_t hxcount_if(iterator_t_ begin_, iterator_t_ end_, const predicate_t_& predicate_) {
	hxsize_t count_ = hxsize_t{0};
	for(iterator_t_ it_ = begin_; it_ != end_; ++it_) {
		if(predicate_(*it_)) {
			++count_;
		}
	}
	return count_;
}

/// `hxexchange` - Replaces `obj` with `new_value` and returns the old value of
/// `obj`. `T` must be move-constructible and `U` must be assignable to `T`.
/// Useful in constructor initialization lists.
/// - `obj` : The object whose value is replaced.
/// - `new_value` : The value to assign to `obj`.
template<typename T_, typename U_> hxattr_nodiscard hxconstexpr
T_ hxexchange(T_& obj_, U_&& new_value_) noexcept {
	T_ old_(hxmove(obj_));
	obj_ = hxforward<U_>(new_value_);
	return old_;
}

/// `hxfind_if` - Returns an iterator to the first element in `[begin, end)` for
/// which `predicate` returns true. Returns `end` if no element matches.
/// - `begin` : Iterator to the beginning of the range to search.
/// - `end` : Iterator to one past the last element in the range to search.
/// - `predicate` : A callable returning true for the sought element.
template<typename iterator_t_, typename predicate_t_> hxattr_hot hxattr_nodiscard hxconstexpr
iterator_t_ hxfind_if(iterator_t_ begin_, iterator_t_ end_, const predicate_t_& predicate_) {
	for(iterator_t_ it_ = begin_; it_ != end_; ++it_) {
		if(predicate_(*it_)) {
			return it_;
		}
	}
	return end_;
}

/// `hxmerge` - Performs a stable merge of two ordered ranges `[begin0,
/// end0)` and `[begin1, end1)` -> `output`. The input arrays must not overlap
/// each other or the destination array. Elements are move-assigned out of the
/// input ranges. Passing a hxvector as an output iterator like this
/// `hxmerge<const int*, hxvector<int>&>(...)` will append to the array.
///
/// Assumes both `[begin0, end0)` and `[begin1, end1)` are ordered by the `less`
/// callable.
/// - `begin0` : Pointer to the beginning of the first ordered range to merge.
/// - `end0` : Pointer to one past the last element of the first ordered range.
/// - `begin1` : Pointer to the beginning of the second ordered range to merge.
/// - `end1` : Pointer to one past the last element of the second ordered range.
/// - `output` : Destination output iterator receiving the merged output.
/// - `less` : Comparator defining the less-than ordering relationship.
/// Returns an output iterator positioned one past the last element written.
template<typename iterator_t_, typename output_iterator_t_, typename less_t_> hxattr_hot hxconstexpr
output_iterator_t_ hxmerge(iterator_t_ begin0_, iterator_t_ end0_, iterator_t_ begin1_, iterator_t_ end1_,
		output_iterator_t_&& output_, const less_t_& less_) noexcept {
	hxassertmsg(begin0_ <= end0_ && begin1_ <= end1_, "invalid_iterator");
	hxrestrict_t<iterator_t_> src0_(begin0_);
	hxrestrict_t<iterator_t_> src1_(begin1_);
	hxrestrict_t<output_iterator_t_> output_r_(output_);
	while(src0_ != end0_ && src1_ != end1_) {
		if(less_(*src1_, *src0_)) {
			*output_r_ = hxmove(*src1_);
			++output_r_; ++src1_;
		}
		else {
			*output_r_ = hxmove(*src0_);
			++output_r_; ++src0_;
		}
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

/// `hxmerge` (specialization) - Performs a stable merge of two ordered
/// ranges `[begin0, end0)` and `[begin1, end1)` -> `output`. The input arrays
/// must not overlap the destination array. Assumes both `[begin0, end0)` and
/// `[begin1, end1)` are ordered by `hxless(a,b)`.  Passing a hxvector as an
/// output iterator like this `hxmerge<const int*, hxvector<int>&>(...)` will
/// append to the array.
/// - `begin0` : Pointer to the beginning of the first ordered range to merge.
/// - `end0` : Pointer to one past the last element of the first ordered range.
/// - `begin1` : Pointer to the beginning of the second ordered range to merge.
/// - `end1` : Pointer to one past the last element of the second ordered range.
/// - `output` : Destination output iterator receiving the merged output.
/// Returns an output iterator positioned one past the last element written.
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

/// `hxminmax` - Returns an `hxminmax_result` with iterators to the smallest
/// and largest elements in `[begin, end)` as determined by `less`. Both
/// `min` and `max` equal `end` for an empty range. When multiple elements
/// compare equal and minimal, the first is returned. When multiple elements
/// compare equal and maximal, the first is returned.
/// - `begin` : Iterator to the beginning of the range to search.
/// - `end` : Iterator to one past the last element in the range to search.
/// - `less` : A key comparison callable defining a less-than ordering relationship.
template<typename iterator_t_, typename less_t_> hxattr_hot hxattr_nodiscard hxconstexpr
hxminmax_result<iterator_t_> hxminmax(iterator_t_ begin_, iterator_t_ end_, const less_t_& less_) {
	if(begin_ == end_) { return hxminmax_result<iterator_t_>{end_, end_}; }
	hxminmax_result<iterator_t_> result_{begin_, begin_};
	for(iterator_t_ it_ = begin_ + ptrdiff_t{1}; it_ != end_; ++it_) {
		if(less_(*it_, *result_.min)) {
			result_.min = it_;
		}
		else if(less_(*result_.max, *it_)) {
			result_.max = it_;
		}
	}
	return result_;
}

/// `hxminmax` (specialization) - An overload of `hxminmax` that uses
/// `hxkey_less`.
/// - `begin` : Iterator to the beginning of the range to search.
/// - `end` : Iterator to one past the last element in the range to search.
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
/// the array.
///
/// Assumes both ranges are ordered by the `less` callable.
/// - `begin0` : Pointer to the beginning of the first ordered range.
/// - `end0` : Pointer to one past the last element of the first ordered range.
/// - `begin1` : Pointer to the beginning of the second ordered range.
/// - `end1` : Pointer to one past the last element of the second ordered range.
/// - `output` : Destination output iterator receiving the difference.
/// - `less` : Comparator defining the less-than ordering relationship.
/// Returns an output iterator positioned one past the last element written.
template<typename iterator_t_, typename output_iterator_t_, typename less_t_> hxattr_hot hxconstexpr
output_iterator_t_ hxset_difference(iterator_t_ begin0_, iterator_t_ end0_, iterator_t_ begin1_,
		iterator_t_ end1_, output_iterator_t_&& output_, const less_t_& less_) noexcept {
	hxassertmsg(begin0_ <= end0_ && begin1_ <= end1_, "invalid_iterator");
	hxrestrict_t<iterator_t_> src0_(begin0_);
	hxrestrict_t<iterator_t_> src1_(begin1_);
	hxrestrict_t<output_iterator_t_> output_r_(output_);

	while(src0_ != end0_ && src1_ != end1_) {
		if(less_(*src0_, *src1_)) {
			*output_r_ = hxmove(*src0_);
			++output_r_; ++src0_;
		}
		else if(less_(*src1_, *src0_)) {
			++src1_;
		}
		else {
			++src0_; ++src1_;
		}
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
/// hxvector<int>&>(...)` will append to the array.
///
/// - `begin0` : Pointer to the beginning of the first ordered range.
/// - `end0` : Pointer to one past the last element of the first ordered range.
/// - `begin1` : Pointer to the beginning of the second ordered range.
/// - `end1` : Pointer to one past the last element of the second ordered range.
/// - `output` : Destination output iterator receiving the difference.
/// Returns an output iterator positioned one past the last element written.
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
/// this `hxset_intersection<const int*, hxvector<int>&>(...)` will append to the
/// array.
///
/// Assumes both ranges are ordered by the `less` callable.
/// - `begin0` : Pointer to the beginning of the first ordered range.
/// - `end0` : Pointer to one past the last element of the first ordered range.
/// - `begin1` : Pointer to the beginning of the second ordered range.
/// - `end1` : Pointer to one past the last element of the second ordered range.
/// - `output` : Destination output iterator receiving the intersection.
/// - `less` : Comparator defining the less-than ordering relationship.
/// Returns an output iterator positioned one past the last element written.
template<typename iterator_t_, typename output_iterator_t_, typename less_t_> hxattr_hot hxconstexpr
output_iterator_t_ hxset_intersection(iterator_t_ begin0_, iterator_t_ end0_, iterator_t_ begin1_,
		iterator_t_ end1_, output_iterator_t_&& output_, const less_t_& less_) noexcept {
	hxassertmsg(begin0_ <= end0_ && begin1_ <= end1_, "invalid_iterator");
	hxrestrict_t<iterator_t_> src0_(begin0_);
	hxrestrict_t<iterator_t_> src1_(begin1_);
	hxrestrict_t<output_iterator_t_> output_r_(output_);

	while(src0_ != end0_ && src1_ != end1_) {
		if(less_(*src0_, *src1_)) {
			++src0_;
		}
		else if(less_(*src1_, *src0_)) {
			++src1_;
		}
		else {
			*output_r_ = hxmove(*src0_);
			++output_r_; ++src0_; ++src1_;
		}
	}
	return output_r_;
}

/// `hxset_intersection` (specialization) - Forms the intersection of two
/// ordered ranges `[begin0, end0)` and `[begin1, end1)` into `output` using
/// `hxless`. Only keys present in both ranges appear in the output. The input
/// arrays must not overlap the destination array. Passing a hxvector as an
/// output iterator like this `hxset_intersection<const int*,
/// hxvector<int>&>(...)` will append to the array.
/// - `begin0` : Pointer to the beginning of the first ordered range.
/// - `end0` : Pointer to one past the last element of the first ordered range.
/// - `begin1` : Pointer to the beginning of the second ordered range.
/// - `end1` : Pointer to one past the last element of the second ordered range.
/// - `output` : Destination output iterator receiving the intersection.
/// Returns an output iterator positioned one past the last element written.
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
/// iterator like this `hxset_union<const int*, hxvector<int>&>(...)` will append
/// to the array.
///
/// Assumes both ranges are ordered by the `less` callable.
/// - `begin0` : Pointer to the beginning of the first ordered range.
/// - `end0` : Pointer to one past the last element of the first ordered range.
/// - `begin1` : Pointer to the beginning of the second ordered range.
/// - `end1` : Pointer to one past the last element of the second ordered range.
/// - `output` : Destination output iterator receiving the union.
/// - `less` : Comparator defining the less-than ordering relationship.
/// Returns an output iterator positioned one past the last element written.
template<typename iterator_t_, typename output_iterator_t_, typename less_t_> hxattr_hot hxconstexpr
output_iterator_t_ hxset_union(iterator_t_ begin0_, iterator_t_ end0_, iterator_t_ begin1_,
		iterator_t_ end1_, output_iterator_t_&& output_, const less_t_& less_) noexcept {
	hxassertmsg(begin0_ <= end0_ && begin1_ <= end1_, "invalid_iterator");
	hxrestrict_t<iterator_t_> src0_(begin0_);
	hxrestrict_t<iterator_t_> src1_(begin1_);
	hxrestrict_t<output_iterator_t_> output_r_(output_);

	while(src0_ != end0_ && src1_ != end1_) {
		if(less_(*src1_, *src0_)) {
			*output_r_ = hxmove(*src1_);
			++output_r_; ++src1_;
		}
		else if(less_(*src0_, *src1_)) {
			*output_r_ = hxmove(*src0_);
			++output_r_; ++src0_;
		}
		else {
			*output_r_ = hxmove(*src0_);
			++output_r_; ++src0_; ++src1_;
		}
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
/// - `begin0` : Pointer to the beginning of the first ordered range.
/// - `end0` : Pointer to one past the last element of the first ordered range.
/// - `begin1` : Pointer to the beginning of the second ordered range.
/// - `end1` : Pointer to one past the last element of the second ordered range.
/// - `output` : Destination output iterator receiving the union.
/// Returns an output iterator positioned one past the last element written.
template<typename iterator_t_, typename output_iterator_t_> hxattr_hot hxconstexpr
output_iterator_t_ hxset_union(iterator_t_ begin0_, iterator_t_ end0_, iterator_t_ begin1_,
		iterator_t_ end1_, output_iterator_t_&& output_) noexcept {
	return hxset_union<iterator_t_, output_iterator_t_>(begin0_, end0_, begin1_, end1_,
		hxforward<output_iterator_t_>(output_), hxkey_less_t<decltype(*begin0_)>{});
}

/// `hxunique` - Removes consecutive duplicate elements from the range
/// `[begin, end)` using `equal` to identify duplicates. Elements are moved
/// into place using move-assignment. Returns an iterator to the new end of
/// the range. Elements past the returned iterator are in a valid but
/// unspecified state. The range must be sorted to remove all duplicates.
/// - `begin` : Iterator to the beginning of the range.
/// - `end` : Iterator to one past the last element in the range.
/// - `equal` : A callable returning true when two elements are considered equal.
template<typename iterator_t_, typename equal_t_> hxattr_hot hxattr_nodiscard hxconstexpr
iterator_t_ hxunique(iterator_t_ begin_, iterator_t_ end_, const equal_t_& equal_) noexcept {
	if(begin_ == end_) { return end_; }
	iterator_t_ dst_ = begin_;
	for(iterator_t_ src_ = begin_ + ptrdiff_t{1}; src_ != end_; ++src_) {
		if(!equal_(*dst_, *src_)) {
			++dst_;
			*dst_ = hxmove(*src_);
		}
	}
	return dst_ + ptrdiff_t{1};
}

/// `hxunique` (specialization) - An overload of `hxunique` that uses
/// `hxkey_equal`.
/// - `begin` : Iterator to the beginning of the range.
/// - `end` : Iterator to one past the last element in the range.
template<typename iterator_t_> hxattr_hot hxattr_nodiscard hxconstexpr
iterator_t_ hxunique(iterator_t_ begin_, iterator_t_ end_) noexcept {
	return hxunique<iterator_t_>(begin_, end_, hxkey_equal_t<decltype(*begin_)>{});
}

HX_NS_END_
