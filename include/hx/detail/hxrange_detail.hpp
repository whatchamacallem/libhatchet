#pragma once
// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

HX_NS_BEGIN_

/// \cond HIDDEN
// Tag dispatch on traits_ & hxtrait_three_way so that a plain bool-returning
// less callable is never type-checked against `< 0`, and a three-way callable
// is never used as a bool. hxif_constexpr alone cannot discard the other
// branch before C++17. Used by hxflat_map and hxflat_set to share one binary
// search implementation between a strict weak order and a three-way
// compare_t.
template<bool three_way_> struct hxcompare_before_ { };
template<> struct hxcompare_before_<false> {
	template<typename compare_t_, typename A_, typename B_>
	hxattr_nodiscard static hxinline hxconstexpr hxattr_flatten
	bool compare(const compare_t_& compare_, const A_& a_, const B_& b_) { return compare_(a_, b_); }
};
template<> struct hxcompare_before_<true> {
	template<typename compare_t_, typename A_, typename B_>
	hxattr_nodiscard static hxinline hxconstexpr hxattr_flatten
	bool compare(const compare_t_& compare_, const A_& a_, const B_& b_) { return compare_(a_, b_) < 0; }
};
template<bool three_way_> struct hxcompare_after_ { };
template<> struct hxcompare_after_<false> {
	template<typename compare_t_, typename A_, typename B_>
	hxattr_nodiscard static hxinline hxconstexpr hxattr_flatten
	bool compare(const compare_t_& compare_, const A_& a_, const B_& b_) { return !compare_(a_, b_); }
};
template<> struct hxcompare_after_<true> {
	template<typename compare_t_, typename A_, typename B_>
	hxattr_nodiscard static hxinline hxconstexpr hxattr_flatten
	bool compare(const compare_t_& compare_, const A_& a_, const B_& b_) { return compare_(a_, b_) >= 0; }
};

// Tag dispatch on traits_ & hxtrait_three_way for the equality recheck after a
// hxlower_bound_search_ call, for the same reason as hxcompare_before_ above.
// Only three_way can report equality at zero extra cost as part of the
// search itself (see hxlower_bound_search_ above), the false case always
// needs this call.
template<bool three_way_> struct hxcompare_equal_ { };
template<> struct hxcompare_equal_<false> {
	template<typename compare_t_, typename A_, typename B_>
	hxattr_nodiscard static hxinline hxconstexpr hxattr_flatten
	bool compare(const compare_t_& compare_, const A_& a_, const B_& b_) { return !compare_(a_, b_); }
};
template<> struct hxcompare_equal_<true> {
	template<typename compare_t_, typename A_, typename B_>
	hxattr_nodiscard static hxinline hxconstexpr hxattr_flatten
	bool compare(const compare_t_& compare_, const A_& a_, const B_& b_) { return compare_(a_, b_) == 0; }
};

// One binary search loop shared by every hxlower_bound_ call site in
// hxflat_map.inl/hxflat_set.inl, in the form of an hxpair of the converged
// position and whether that position holds an element equivalent to value_.
// When traits_ & hxtrait_three_way is set the found flag comes from the last
// "not before" step at zero extra cost, reusing the comparison the search
// already made against the position it converges on. Otherwise a strict weak
// order cannot report equality from a single compare_ call, so one extra
// hxcompare_equal_ call is made once the search converges.
template<typename range_t_, typename value_t_, typename compare_t_, int traits_>
hxattr_nodiscard hxinline hxconstexpr hxattr_flatten
auto hxlower_bound_search_(range_t_&& range_, const value_t_& value_, const compare_t_& compare_)
		-> hxpair<hxrestrict_t<decltype(range_.begin())>, bool> {
	using iterator_t_ = hxrestrict_t<decltype(range_.begin())>;
	iterator_t_ begin_ = range_.begin();
	const iterator_t_ end_ = range_.end();
	// Does not dereference null pointer args.
	hxsize_t count_ = end_ - begin_;
	bool found_ = false;
	while(count_ > hxsize_t{0}) {
		const hxsize_t step_ = count_ >> 1;
		const iterator_t_ mid_ = begin_ + step_;
		bool before_;
		hxif_constexpr((traits_ & hxtrait_three_way) != 0) {
			const auto cmp_ = compare_(*mid_, value_);
			before_ = cmp_ < 0;
			// Only a "not before" step can narrow down to the final position,
			// so only that case may report equality. A later "before" step
			// never moves begin_ back to a position this step already ruled
			// out.
			if(!before_) { found_ = cmp_ == 0; }
		}
		else {
			before_ = hxcompare_before_<false>::compare(compare_, *mid_, value_);
		}
		if(before_) {
			begin_ = mid_ + hxsize_t{1};
			count_ -= step_ + hxsize_t{1};
		}
		else {
			count_ = step_;
		}
	}
	hxif_constexpr((traits_ & hxtrait_three_way) == 0) {
		found_ = begin_ != end_ && hxcompare_equal_<(traits_ & hxtrait_three_way) != 0>::compare(compare_, value_, *begin_);
	}
	else {
		found_ = found_ && begin_ != end_;
	}
	return { begin_, found_ };
}

// hxupper_bound_ shares hxrange.hpp's public hxupper_bound algorithm, but
// dispatches on traits_ & hxtrait_three_way instead of requiring a strict weak
// order, for use by hxflat_map and hxflat_set.
template<typename range_t_, typename value_t_, typename compare_t_, int traits_>
hxattr_nodiscard hxinline hxconstexpr hxattr_flatten
auto hxupper_bound_(range_t_&& range_, const value_t_& value_, const compare_t_& compare_)
		-> decltype(range_.begin()) {
	using iterator_t_ = hxrestrict_t<decltype(range_.begin())>;
	iterator_t_ begin_ = range_.begin();
	// Does not dereference null pointer args.
	hxsize_t count_ = range_.end() - begin_;
	while(count_ > hxsize_t{0}) {
		const hxsize_t step_ = count_ >> 1;
		const iterator_t_ mid_ = begin_ + step_;
		if(hxcompare_after_<(traits_ & hxtrait_three_way) != 0>::compare(compare_, value_, *mid_)) {
			begin_ = mid_ + hxsize_t{1};
			count_ -= step_ + hxsize_t{1};
		}
		else {
			count_ = step_;
		}
	}
	return begin_;
}
/// \endcond

HX_NS_END_
