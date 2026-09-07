#pragma once
// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

/// \file
/// Random iterator based set algorithms. Includes support for callables when
/// defining custom key operations. Otherwise `T::operator<(const T&)` and
/// `T::operator==(const T&)` are used. The other relational operators are not
/// used.
///
/// See `hxrange.hpp` for range algorithms. See `hxsort.hpp` for sorting
/// algorithms including `hxinsertion_sort`, `hxheapsort` and `hxsort`.

#include "hxrange.hpp"

// HX_USE_MACROS_WITH_MODULE allows including macros alongside the module.
#if HX_USE_MACROS_WITH_MODULE
#error Header does not provide macros alone.
#endif

HX_NS_BEGIN_

/// `hxmerge` - Performs a stable merge of the two ordered ranges `range0` and
/// `range1` -> `output`. The input ranges must not overlap each other or the
/// destination array. Elements are copy-assigned out of an input range passed
/// as an l-value and move-assigned out of an input range passed as an
/// r-value. Passing a hxvector as an output iterator like this
/// `hxmerge<const int*, hxvector<int>&>(...)` will append to the array.
/// Assumes both `range0` and `range1` are ordered by the `less` callable.
/// Returns an output iterator positioned one past the last element written.
/// Requires a `random-iterator`.
template<hxrandom_range_concept_ range0_t_, hxrandom_range_concept_ range1_t_,
		typename output_iterator_t_, typename less_t_>
inline hxconstexpr hxattr_flatten
output_iterator_t_ hxmerge(range0_t_&& range0_, range1_t_&& range1_,
		output_iterator_t_&& output_, const less_t_& less_) noexcept {
	hxrestrict_t<decltype(range0_.begin())> src0_(range0_.begin());
	const auto end0_ = range0_.end();
	hxrestrict_t<decltype(range1_.begin())> src1_(range1_.begin());
	const auto end1_ = range1_.end();
	hxrestrict_t<output_iterator_t_> output_r_(hxforward<output_iterator_t_>(output_));
	while(src0_ != end0_ && src1_ != end1_) {
		const bool take1_ = less_(*src1_, *src0_);
		if(take1_) { *output_r_ = hxforward_like<range1_t_>(*src1_); }
		else { *output_r_ = hxforward_like<range0_t_>(*src0_); }
		++output_r_;
		src1_ = src1_ + static_cast<ptrdiff_t>(take1_);
		src0_ = src0_ + static_cast<ptrdiff_t>(!take1_);
	}
	while(src0_ != end0_) {
		*output_r_ = hxforward_like<range0_t_>(*src0_);
		++output_r_; ++src0_;
	}
	while(src1_ != end1_) {
		*output_r_ = hxforward_like<range1_t_>(*src1_);
		++output_r_; ++src1_;
	}
	return output_r_;
}

/// `hxmerge` (specialization) - An overload of `hxmerge` that uses
/// `hxkey_less` to order `range0` and `range1`. Requires a
/// `random-iterator`.
template<hxsorted_range_concept_ range0_t_, hxsorted_range_concept_ range1_t_,
		typename output_iterator_t_>
inline hxconstexpr hxattr_flatten
output_iterator_t_ hxmerge(range0_t_&& range0_, range1_t_&& range1_,
		output_iterator_t_&& output_) noexcept {
	const auto begin0_ = range0_.begin();
	return hxmerge<range0_t_, range1_t_, output_iterator_t_>(hxforward<range0_t_>(range0_),
		hxforward<range1_t_>(range1_), hxforward<output_iterator_t_>(output_),
		hxkey_less_t<decltype(*begin0_)>{});
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
/// largest elements in `range` as determined by `less`. Both `min` and `max`
/// equal `range.end()` for an empty range. When multiple elements compare equal
/// and minimal, the first is returned. When multiple elements compare equal and
/// maximal, the first is returned. The `less` callable defines the less-than
/// ordering relationship. Requires a `forward-iterator`.
template<hxrange_concept_ range_t_, typename less_t_>
hxattr_nodiscard inline hxconstexpr hxattr_flatten
auto hxminmax(range_t_&& range_, const less_t_& less_) -> hxminmax_result<decltype(range_.begin())> {
	hxrestrict_t<decltype(range_.begin())> it_(range_.begin());
	const auto end_ = range_.end();
	hxminmax_result<decltype(range_.begin())> result_{it_, it_};
	if(it_ == end_) { return result_; }
	for(++it_; it_ != end_; ++it_) {
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
/// `hxkey_less` over `range`. Requires a `forward-iterator`.
template<hxrange_concept_ range_t_>
hxattr_nodiscard inline hxconstexpr hxattr_flatten
auto hxminmax(range_t_&& range_) -> hxminmax_result<decltype(range_.begin())> {
	const auto begin_ = range_.begin();
	return hxminmax<range_t_>(hxforward<range_t_>(range_), hxkey_less_t<decltype(*begin_)>{});
}

/// `hxset_difference` - Forms the difference of the two ordered ranges
/// `range0` and `range1` into `output`. The output contains keys that appear
/// in the first range but not the second. The input ranges must not overlap
/// each other or the destination array. Elements from the first range are
/// copy-assigned into the output if `range0` is an l-value and move-assigned
/// if it is an r-value. Passing a hxvector as an output iterator
/// like this `hxset_difference<const int*, hxvector<int>&>(...)` will append to
/// the array. Assumes both ranges are ordered by the `less` callable. Returns
/// an output iterator positioned one past the last element written. Requires a
/// `random-iterator`.
template<hxrandom_range_concept_ range0_t_, hxrandom_range_concept_ range1_t_,
		typename output_iterator_t_, typename less_t_>
inline hxconstexpr hxattr_flatten
output_iterator_t_ hxset_difference(range0_t_&& range0_, range1_t_&& range1_,
		output_iterator_t_&& output_, const less_t_& less_) noexcept {
	hxrestrict_t<decltype(range0_.begin())> src0_(range0_.begin());
	const auto end0_ = range0_.end();
	hxrestrict_t<decltype(range1_.begin())> src1_(range1_.begin());
	const auto end1_ = range1_.end();
	hxrestrict_t<output_iterator_t_> output_r_(hxforward<output_iterator_t_>(output_));
	while(src0_ != end0_ && src1_ != end1_) {
		const bool lt_ = less_(*src0_, *src1_);
		const bool gt_ = less_(*src1_, *src0_);
		if(lt_) {
			*output_r_ = hxforward_like<range0_t_>(*src0_);
			++output_r_;
		}
		src0_ = src0_ + static_cast<ptrdiff_t>(!gt_);
		src1_ = src1_ + static_cast<ptrdiff_t>(!lt_);
	}
	while(src0_ != end0_) {
		*output_r_ = hxforward_like<range0_t_>(*src0_);
		++output_r_; ++src0_;
	}
	return output_r_;
}

/// `hxset_difference` (specialization) - An overload of `hxset_difference`
/// that uses `hxkey_less` to order `range0` and `range1`. Requires a
/// `random-iterator`.
template<hxsorted_range_concept_ range0_t_, hxsorted_range_concept_ range1_t_,
		typename output_iterator_t_>
inline hxconstexpr hxattr_flatten
output_iterator_t_ hxset_difference(range0_t_&& range0_, range1_t_&& range1_,
		output_iterator_t_&& output_) noexcept {
	const auto begin0_ = range0_.begin();
	return hxset_difference<range0_t_, range1_t_, output_iterator_t_>(hxforward<range0_t_>(range0_),
		hxforward<range1_t_>(range1_), hxforward<output_iterator_t_>(output_),
		hxkey_less_t<decltype(*begin0_)>{});
}

/// `hxset_intersection` - Forms the intersection of the two ordered ranges
/// `range0` and `range1` into `output`. Only keys present in both ranges
/// appear in the output. The input ranges must not overlap each other or the
/// destination array. Elements from the first range are copy-assigned into
/// the output if `range0` is an l-value and move-assigned if it is an
/// r-value. Passing a hxvector as an output iterator like this
/// `hxset_intersection<const int*, hxvector<int>&>(...)` will append to the
/// array. Assumes both ranges are ordered by the `less` callable. Returns an
/// output iterator positioned one past the last element written. Requires a
/// `random-iterator`.
template<hxrandom_range_concept_ range0_t_, hxrandom_range_concept_ range1_t_,
		typename output_iterator_t_, typename less_t_>
inline hxconstexpr hxattr_flatten
output_iterator_t_ hxset_intersection(range0_t_&& range0_, range1_t_&& range1_,
		output_iterator_t_&& output_, const less_t_& less_) noexcept {
	hxrestrict_t<decltype(range0_.begin())> src0_(range0_.begin());
	const auto end0_ = range0_.end();
	hxrestrict_t<decltype(range1_.begin())> src1_(range1_.begin());
	const auto end1_ = range1_.end();
	hxrestrict_t<output_iterator_t_> output_r_(hxforward<output_iterator_t_>(output_));
	while(src0_ != end0_ && src1_ != end1_) {
		const bool lt_ = less_(*src0_, *src1_);
		const bool gt_ = less_(*src1_, *src0_);
		if(!lt_ && !gt_) {
			*output_r_ = hxforward_like<range0_t_>(*src0_);
			++output_r_;
		}
		src0_ = src0_ + static_cast<ptrdiff_t>(!gt_);
		src1_ = src1_ + static_cast<ptrdiff_t>(!lt_);
	}
	return output_r_;
}

/// `hxset_intersection` (specialization) - An overload of `hxset_intersection`
/// that uses `hxkey_less` to order `range0` and `range1`. Requires a
/// `random-iterator`.
template<hxsorted_range_concept_ range0_t_, hxsorted_range_concept_ range1_t_,
		typename output_iterator_t_>
inline hxconstexpr hxattr_flatten
output_iterator_t_ hxset_intersection(range0_t_&& range0_, range1_t_&& range1_,
		output_iterator_t_&& output_) noexcept {
	const auto begin0_ = range0_.begin();
	return hxset_intersection<range0_t_, range1_t_, output_iterator_t_>(hxforward<range0_t_>(range0_),
		hxforward<range1_t_>(range1_), hxforward<output_iterator_t_>(output_),
		hxkey_less_t<decltype(*begin0_)>{});
}

/// `hxset_union` - Forms the union of the two ordered ranges `range0` and
/// `range1` into `output`. A key appearing `m` times in the first range and
/// `n` times in the second appears `max(m, n)` times in the output. The input
/// ranges must not overlap each other or the destination array. Elements are
/// copy-assigned out of an input range passed as an l-value and move-assigned
/// out of an input range passed as an r-value. Passing a hxvector as an output
/// iterator like this `hxset_union<const int*, hxvector<int>&>(...)` will
/// append to the array. Assumes both ranges are ordered by the `less` callable.
/// Returns an output iterator positioned one past the last element written.
/// Requires a `random-iterator`.
template<hxrandom_range_concept_ range0_t_, hxrandom_range_concept_ range1_t_,
		typename output_iterator_t_, typename less_t_>
inline hxconstexpr hxattr_flatten
output_iterator_t_ hxset_union(range0_t_&& range0_, range1_t_&& range1_,
		output_iterator_t_&& output_, const less_t_& less_) noexcept {
	hxrestrict_t<decltype(range0_.begin())> src0_(range0_.begin());
	const auto end0_ = range0_.end();
	hxrestrict_t<decltype(range1_.begin())> src1_(range1_.begin());
	const auto end1_ = range1_.end();
	hxrestrict_t<output_iterator_t_> output_r_(hxforward<output_iterator_t_>(output_));
	while(src0_ != end0_ && src1_ != end1_) {
		const bool take1_ = less_(*src1_, *src0_);
		const bool less0_ = less_(*src0_, *src1_);
		if(take1_) { *output_r_ = hxforward_like<range1_t_>(*src1_); }
		else { *output_r_ = hxforward_like<range0_t_>(*src0_); }
		++output_r_;
		src0_ = src0_ + static_cast<ptrdiff_t>(!take1_);
		src1_ = src1_ + static_cast<ptrdiff_t>(!less0_);
	}
	while(src0_ != end0_) {
		*output_r_ = hxforward_like<range0_t_>(*src0_);
		++output_r_; ++src0_;
	}
	while(src1_ != end1_) {
		*output_r_ = hxforward_like<range1_t_>(*src1_);
		++output_r_; ++src1_;
	}
	return output_r_;
}

/// `hxset_union` (specialization) - An overload of `hxset_union` that uses
/// `hxkey_less` to order `range0` and `range1`. Requires a
/// `random-iterator`.
template<hxsorted_range_concept_ range0_t_, hxsorted_range_concept_ range1_t_,
		typename output_iterator_t_>
inline hxconstexpr hxattr_flatten
output_iterator_t_ hxset_union(range0_t_&& range0_, range1_t_&& range1_,
		output_iterator_t_&& output_) noexcept {
	const auto begin0_ = range0_.begin();
	return hxset_union<range0_t_, range1_t_, output_iterator_t_>(hxforward<range0_t_>(range0_),
		hxforward<range1_t_>(range1_), hxforward<output_iterator_t_>(output_),
		hxkey_less_t<decltype(*begin0_)>{});
}

/// `hxunique` - Removes consecutive duplicate elements from the range `[begin,
/// end)` using `equal` to identify duplicates. Elements are moved into place
/// using move-assignment. Returns an iterator to the new end of the range.
/// Elements past the returned iterator are in a valid but unspecified state.
/// The range must be sorted to remove all duplicates. The `equal` callable
/// returns true when two elements are considered equal. Requires a
/// `random-iterator`.
template<hxrandom_iterator_concept_ iterator_t_, typename equal_t_>
hxattr_nodiscard inline hxconstexpr hxattr_flatten
iterator_t_ hxunique(iterator_t_ begin_, iterator_t_ end_, const equal_t_& equal_) noexcept {
	if(begin_ == end_) { return end_; }
	iterator_t_ dst_ = begin_;
	for(++begin_; begin_ != end_; ++begin_, ++dst_) {
		if(equal_(*dst_, *begin_)) { break; }
	}
	if(begin_ == end_) { return end_; }
	hxrestrict_t<iterator_t_> dst_r_(dst_);
	hxrestrict_t<iterator_t_> src_r_(begin_);
	for(++src_r_; src_r_ != end_; ++src_r_) {
		if(!equal_(*dst_r_, *src_r_)) {
			++dst_r_;
			*dst_r_ = hxmove(*src_r_);
		}
	}
	return ++dst_r_;
}

/// `hxunique` (specialization) - An overload of `hxunique` that uses
/// `hxkey_equal` over the range `[begin, end)`. Requires a `random-iterator`.
template<hxsorted_iterator_concept_ iterator_t_>
hxattr_nodiscard inline hxconstexpr hxattr_flatten
iterator_t_ hxunique(iterator_t_ begin_, iterator_t_ end_) noexcept {
	return hxunique<iterator_t_>(begin_, end_, hxkey_equal_t<decltype(*begin_)>{});
}

HX_NS_END_
