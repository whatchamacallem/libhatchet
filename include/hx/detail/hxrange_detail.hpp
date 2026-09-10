#pragma once
// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

HX_NS_BEGIN_

/// \cond HIDDEN
// hxcompare_ - C++11 compatibility wrapper that avoids calling unused three way
// calls. Required because hxif_constexpr is only a fallback in C++11.
template<bool three_way_> struct hxcompare_ { };
template<> struct hxcompare_<false> {
	template<typename compare_t_, typename A_, typename B_>
	hxattr_nodiscard static hxinline hxconstexpr hxattr_flatten
	bool before(const compare_t_& compare_, const A_& a_, const B_& b_) { return compare_(a_, b_); }
	template<typename compare_t_, typename A_, typename B_>
	hxattr_nodiscard static hxinline hxconstexpr hxattr_flatten
	bool after(const compare_t_& compare_, const A_& a_, const B_& b_) { return !compare_(a_, b_); }
	template<typename compare_t_, typename A_, typename B_>
	hxattr_nodiscard static hxinline hxconstexpr hxattr_flatten
	bool equal(const compare_t_& compare_, const A_& a_, const B_& b_) { return !compare_(a_, b_); }
	template<typename compare_t_, typename A_, typename B_>
	hxattr_nodiscard static hxinline hxconstexpr hxattr_flatten
	hxpair<bool, bool> step(const compare_t_& compare_, const A_& a_, const B_& b_) {
		return { before(compare_, a_, b_), false };
	}
};

template<> struct hxcompare_<true> {
	template<typename compare_t_, typename A_, typename B_>
	hxattr_nodiscard static hxinline hxconstexpr hxattr_flatten
	bool before(const compare_t_& compare_, const A_& a_, const B_& b_) { return compare_(a_, b_) < 0; }
	template<typename compare_t_, typename A_, typename B_>
	hxattr_nodiscard static hxinline hxconstexpr hxattr_flatten
	bool after(const compare_t_& compare_, const A_& a_, const B_& b_) { return compare_(a_, b_) >= 0; }
	template<typename compare_t_, typename A_, typename B_>
	hxattr_nodiscard static hxinline hxconstexpr hxattr_flatten
	bool equal(const compare_t_& compare_, const A_& a_, const B_& b_) { return compare_(a_, b_) == 0; }
	template<typename compare_t_, typename A_, typename B_>
	hxattr_nodiscard static hxinline hxconstexpr hxattr_flatten
	hxpair<bool, bool> step(const compare_t_& compare_, const A_& a_, const B_& b_) {
		const auto cmp_ = compare_(a_, b_);
		return { cmp_ < 0, cmp_ == 0 };
	}
};

template<typename range_t_, typename value_t_, typename compare_t_, int traits_>
hxattr_nodiscard hxinline hxconstexpr hxattr_flatten
auto hxlower_bound_iterator_(range_t_&& range_, const value_t_& value_, const compare_t_& compare_)
		-> hxrestrict_t<decltype(range_.begin())> {
	using iterator_t_ = hxrestrict_t<decltype(range_.begin())>;
	iterator_t_ begin_ = range_.begin();
	// Does not dereference null pointer args.
	hxsize_t count_ = range_.end() - begin_;
	while(count_ > hxsize_t{0}) {
		const hxsize_t step_ = count_ >> 1;
		const iterator_t_ mid_ = begin_ + step_;
		if(hxcompare_<(traits_ & hxtrait_three_way) != 0>::before(compare_, *mid_, value_)) {
			begin_ = mid_ + hxsize_t{1};
			count_ -= step_ + hxsize_t{1};
		}
		else {
			count_ = step_;
		}
	}
	return begin_;
}

template<typename range_t_, typename value_t_, typename compare_t_, int traits_>
hxattr_nodiscard hxinline hxconstexpr hxattr_flatten
auto hxlower_bound_pair_(range_t_&& range_, const value_t_& value_, const compare_t_& compare_)
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
		const hxpair<bool, bool> cmp_ = hxcompare_<(traits_ & hxtrait_three_way) != 0>::step(compare_, *mid_, value_);
		if(cmp_.a) {
			begin_ = mid_ + hxsize_t{1};
			count_ -= step_ + hxsize_t{1};
		}
		else {
			found_ = cmp_.b;
			count_ = step_;
		}
	}
	hxif_constexpr((traits_ & hxtrait_three_way) == 0) {
		found_ = begin_ != end_ && hxcompare_<false>::equal(compare_, value_, *begin_);
	}
	return { begin_, found_ };
}

template<typename range_t_, typename value_t_, typename compare_t_, int traits_>
hxattr_nodiscard hxinline hxconstexpr hxattr_flatten
bool hxbinary_search_(range_t_&& range_, const value_t_& value_, const compare_t_& compare_) {
	return hxlower_bound_pair_<range_t_, value_t_, compare_t_, traits_>(
		hxforward<range_t_>(range_), value_, compare_).b;
}

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
		if(hxcompare_<(traits_ & hxtrait_three_way) != 0>::after(compare_, value_, *mid_)) {
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
