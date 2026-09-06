#pragma once
// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

/// \file
/// Read-only range utilities for libhatchet. Includes support for callables
/// when defining custom key operations. Otherwise `T::operator<(const T&)` and
/// `T::operator==(const T&)` are used. The other relational operators are
/// not used.
///
/// See `hxset_operations.hpp` for iterator based set algorithms. See `hxsort.hpp` for
/// sorting algorithms including `hxinsertion_sort`, `hxheapsort` and `hxsort`.
///
/// All the `hxrestrict_t` sprinkled around has measurable benefits in certain
/// cases. Clang is better at utilizing that hint.

#include "libhatchet.h"

// HX_USE_MACROS_WITH_MODULE allows including macros alongside the module.
#if HX_USE_MACROS_WITH_MODULE
#error Header does not provide macros alone.
#endif

#include "hxkey.hpp"

HX_NS_BEGIN_

#if HX_CPLUSPLUS >= 202002L

/// \cond HIDDEN
// These are much simpler than the standard.
template<typename T_>
concept hxforward_iterator_concept_ = requires(T_& x_, const T_& y_) {
	*x_;
	{ ++x_ } -> hxsame_as<T_&>;
	{ x_++ } -> hxsame_as<T_>;
	{ x_ == y_ } -> hxconvertible_to<bool>;
	{ x_ != y_ } -> hxconvertible_to<bool>;
};
template<typename T_>
concept hxbidirectional_iterator_concept_ = hxforward_iterator_concept_<T_> && requires(T_& x_) {
	{ --x_ } -> hxsame_as<T_&>;
	{ x_-- } -> hxsame_as<T_>;
};
template<typename T_>
concept hxrandom_iterator_concept_ = hxbidirectional_iterator_concept_<T_> && requires(
		const T_& x_, const T_& y_, ptrdiff_t n_) {
	{ x_ + n_ } -> hxsame_as<T_>;
	{ x_ - n_ } -> hxsame_as<T_>;
	{ x_ - y_ } -> hxsame_as<ptrdiff_t>;
	{ x_ < y_ } -> hxconvertible_to<bool>;
};
template<typename T_>
concept hxsorted_iterator_concept_ = hxrandom_iterator_concept_<T_> && requires(const T_& x_) {
	{ x_ < x_ } -> hxconvertible_to<bool>;
	{ x_ == x_ } -> hxconvertible_to<bool>;
};
template<typename T_>
concept hxrange_concept_ = requires(T_& x_) {
	{ x_.begin() } -> hxforward_iterator_concept_;
	{ x_.end() } -> hxforward_iterator_concept_;
};
template<typename T_>
concept hxrandom_range_concept_ = requires(T_& x_) {
	{ x_.begin() } -> hxrandom_iterator_concept_;
	{ x_.end() } -> hxrandom_iterator_concept_;
};
template<typename T_>
concept hxsorted_range_concept_ = requires(T_& x_) {
	{ x_.begin() } -> hxsorted_iterator_concept_;
	{ x_.end() } -> hxsorted_iterator_concept_;
};
/// \endcond
#else
#define hxforward_iterator_concept_ typename
#define hxbidirectional_iterator_concept_ typename
#define hxrandom_iterator_concept_ typename
#define hxrange_concept_ typename
#define hxrandom_range_concept_ typename
#define hxsorted_iterator_concept_ typename
#define hxsorted_range_concept_ typename
#endif

// Forward declaration.
template<hxrandom_range_concept_ range_t_, typename value_t_, typename less_t_> hxconstexpr
auto hxlower_bound(range_t_&& range_, const value_t_& value_, const less_t_& less_)
	-> decltype(range_.begin());

/// `hxrange` - Wraps an iterator pair `[begin, end)` for use with algorithms
/// on that pair. Intended for use with raw pointers.
/// - `iter_t` : The iterator type.
template<typename iterator_t_>
class hxrange {
public:
	/// Constructs from the C array `array`.
	/// - `array` : The array to wrap.
	template<typename T_, size_t N_>
	hxinline hxconstexpr hxrange(T_ (&array_)[N_])
		: m_begin_(array_ + 0), m_end_(array_ + N_) { }

	/// Constructs from the iterator pair `[begin, end)`.
	/// - `begin` : Iterator to the first element.
	/// - `end` : Iterator one past the last element.
	hxinline hxconstexpr hxrange(iterator_t_ begin_, iterator_t_ end_)
		: m_begin_(begin_), m_end_(end_) { }

	/// Constructs from an iterator and a length.
	/// - `begin` : Iterator to the first element.
	/// - `size` : The number of elements.
	hxinline hxconstexpr hxrange(iterator_t_ begin_, hxsize_t size_)
		: m_begin_(begin_), m_end_(begin_ + size_) { }

	/// Constructs from an iterator and a length.
	/// - `begin` : Iterator to the first element.
	/// - `size` : The number of elements.
	hxinline hxconstexpr hxrange(iterator_t_ begin_, size_t size_)
		: m_begin_(begin_), m_end_(begin_ + size_) { }

	/// Returns an iterator to the first element.
	hxattr_nodiscard hxinline hxconstexpr
	iterator_t_ begin(void) const { return m_begin_; }

	/// Returns an iterator one past the last element.
	hxattr_nodiscard hxinline hxconstexpr
	iterator_t_ end(void) const { return m_end_; }

private:
	iterator_t_ m_begin_;
	iterator_t_ m_end_;
};

/// `hxmake_range` - Returns an `hxrange` deduced from the iterator pair
/// `[begin, end)`.
/// - `begin` : Iterator to the first element.
/// - `end` : Iterator one past the last element.
template<typename iterator_t_>
hxattr_nodiscard hxinline hxconstexpr hxattr_flatten
hxrange<iterator_t_> hxmake_range(iterator_t_ begin_, iterator_t_ end_) {
	return hxrange<iterator_t_>(begin_, end_);
}

template<typename T_, size_t N_>
hxattr_nodiscard hxinline hxconstexpr hxattr_flatten
hxrange<T_*> hxmake_range(T_(&array_)[N_]) { return hxrange<T_*>(array_ + 0, array_ + N_); }

/// `hxall_of` - Returns true if the predicate `callable` returns true for every
/// element of `range`. Returns true for an empty range. Requires a
/// `forward-iterator`.
template<hxrange_concept_ range_t_, typename callable_t_>
hxattr_nodiscard hxinline hxconstexpr hxattr_flatten
bool hxall_of(range_t_&& range_, callable_t_&& callable_) {
	hxrestrict_t<decltype(range_.begin())> it_(range_.begin());
	for(const auto end_ = range_.end(); it_ != end_; ++it_) {
		if(!hxforward<callable_t_>(callable_)(*it_)) {
			return false;
		}
	}
	return true;
}

/// `hxany_of` - Returns true if the predicate `callable` returns true for at
/// least one element of `range`. Returns false for an empty range. Requires a
/// `forward-iterator`.
template<hxrange_concept_ range_t_, typename callable_t_>
hxattr_nodiscard hxinline hxconstexpr hxattr_flatten
bool hxany_of(range_t_&& range_, callable_t_&& callable_) {
	hxrestrict_t<decltype(range_.begin())> it_(range_.begin());
	for(const auto end_ = range_.end(); it_ != end_; ++it_) {
		if(hxforward<callable_t_>(callable_)(*it_)) {
			return true;
		}
	}
	return false;
}

/// `hxbinary_search` - Performs a binary search for `value` in the sorted
/// range `[begin, end)`. Returns `end` if the value is not found. Unsorted
/// data will lead to errors. The first of non-unique values is returned. The
/// `less` callable returns true if the first argument is ordered before
/// (i.e., is less than) the second. Requires a `random-iterator`.
template<hxrandom_iterator_concept_ iterator_t_, typename value_t_, typename less_t_>
hxattr_nodiscard hxinline hxconstexpr hxattr_flatten
iterator_t_ hxbinary_search(iterator_t_ begin_, iterator_t_ end_, const value_t_& value_,
		const less_t_& less_) {
	const hxrestrict_t<iterator_t_> it_ = hxlower_bound(hxmake_range(begin_, end_), value_, less_);
	return (it_ != end_ && !less_(value_, *it_)) ? it_ : end_;
}

/// `hxbinary_search` - An overload of `hxbinary_search` over `range`.
/// Requires a `random-iterator`.
template<hxrandom_range_concept_ range_t_, typename value_t_, typename less_t_>
hxattr_nodiscard hxinline hxconstexpr hxattr_flatten
auto hxbinary_search(range_t_&& range_, const value_t_& value_, const less_t_& less_)
		-> decltype(range_.begin()) {
	const auto end_ = range_.end();
	const hxrestrict_t<decltype(end_)> it_ = hxlower_bound(hxforward<range_t_>(range_), value_, less_);
	return (it_ != end_ && !less_(value_, *it_)) ? it_ : end_;
}

/// `hxbinary_search` (specialization) - An overload of `hxbinary_search` that
/// searches for `value` in the sorted `range` using `hxkey_less`. Requires a
/// `sorted-iterator`.
template<hxsorted_range_concept_ range_t_, typename value_t_>
hxattr_nodiscard hxinline hxconstexpr hxattr_flatten
auto hxbinary_search(range_t_&& range_, const value_t_& value_) -> decltype(range_.begin()) {
	using less_t_ = hxkey_less_t<decltype(*range_.begin())>;
	const auto end_ = range_.end();
	const hxrestrict_t<decltype(end_)> it_ = hxlower_bound(hxforward<range_t_>(range_), value_, less_t_{});
	return (it_ != end_ && !less_t_{}(value_, *it_)) ? it_ : end_;
}

/// `hxcount_if` - Returns the number of elements of `range` for which the
/// predicate `callable` returns true. Requires a `forward-iterator`.
template<hxrange_concept_ range_t_, typename callable_t_>
hxattr_nodiscard hxinline hxconstexpr hxattr_flatten
hxsize_t hxcount_if(range_t_&& range_, callable_t_&& callable_) {
	hxrestrict_t<decltype(range_.begin())> it_(range_.begin());
	hxsize_t count_ = hxsize_t{0};
	for(const auto end_ = range_.end(); it_ != end_; ++it_) {
		if(hxforward<callable_t_>(callable_)(*it_)) {
			++count_;
		}
	}
	return count_;
}

/// `hxequal_range` - Returns true if `range0` and `range1` have the same
/// length and `equal` returns true for every pair of elements at
/// corresponding positions. Requires a `forward-iterator`.
template<hxrange_concept_ range0_t_, hxrange_concept_ range1_t_, typename equal_t_>
hxattr_nodiscard hxinline hxconstexpr hxattr_flatten
bool hxequal_range(range0_t_&& range0_, range1_t_&& range1_, const equal_t_& equal_) {
	hxrestrict_t<decltype(range0_.begin())> begin0_(range0_.begin());
	const auto end0_ = range0_.end();
	hxrestrict_t<decltype(range1_.begin())> begin1_(range1_.begin());
	const auto end1_ = range1_.end();
#if HX_CPLUSPLUS >= 202002L
	if constexpr(requires(const decltype(begin0_)& a_, const decltype(begin0_)& b_) { { b_ - a_ }; }
			&& requires(const decltype(begin1_)& a_, const decltype(begin1_)& b_) { { b_ - a_ }; }) {
		if((end0_ - begin0_) != (end1_ - begin1_)) { return false; }
		while(begin0_ != end0_) {
			if(!equal_(*begin0_, *begin1_)) { return false; }
			++begin0_; ++begin1_;
		}
		return true;
	}
	else
#endif
	{
		while(begin0_ != end0_ && begin1_ != end1_) {
			if(!equal_(*begin0_, *begin1_)) { return false; }
			++begin0_; ++begin1_;
		}
		return begin0_ == end0_ && begin1_ == end1_;
	}
}

/// `hxequal_range` (specialization) - An overload of `hxequal_range` that uses
/// `hxkey_equal` over `range0` and `range1`. Requires a `forward-iterator`.
template<hxrange_concept_ range0_t_, hxrange_concept_ range1_t_>
hxattr_nodiscard hxinline hxconstexpr hxattr_flatten
bool hxequal_range(range0_t_&& range0_, range1_t_&& range1_) {
	const auto begin0_ = range0_.begin();
	return hxequal_range<range0_t_, range1_t_>(hxforward<range0_t_>(range0_),
		hxforward<range1_t_>(range1_), hxkey_equal_t<decltype(*begin0_)>{});
}

/// `hxexchange` - Replaces `obj` with a `T` constructed from `args` and
/// returns the old value of `obj`. `T` must be move-constructible. Useful in
/// constructor initialization lists.
template<typename T_, typename... args_t_>
hxattr_nodiscard hxinline hxconstexpr hxattr_flatten
T_ hxexchange(T_& obj_, args_t_&&... args_) noexcept {
	T_ old_(hxmove(obj_));
	obj_ = T_(hxforward<args_t_>(args_)...);
	return old_;
}

/// `hxfind_if` - Returns an iterator to the first element of `range` for
/// which the predicate `callable` returns true. Returns `range.end()` if no
/// element matches. Requires a `forward-iterator`.
template<hxrange_concept_ range_t_, typename callable_t_>
hxattr_nodiscard hxinline hxconstexpr hxattr_flatten
auto hxfind_if(range_t_&& range_, callable_t_&& callable_) -> decltype(range_.begin()) {
	hxrestrict_t<decltype(range_.begin())> it_(range_.begin());
	const auto end_ = range_.end();
	for(; it_ != end_; ++it_) {
		if(hxforward<callable_t_>(callable_)(*it_)) {
			return it_;
		}
	}
	return end_;
}

/// `hxfor_each` - Applies the `callable` to each element of `range` in order.
/// Returns the `callable` after it has been applied to every element.
/// Requires a `forward-iterator`.
template<hxrange_concept_ range_t_, typename callable_t_>
hxinline hxconstexpr hxattr_flatten
callable_t_ hxfor_each(range_t_&& range_, callable_t_&& callable_) {
	hxrestrict_t<decltype(range_.begin())> it_(range_.begin());
	for(const auto end_ = range_.end(); it_ != end_; ++it_) {
		hxforward<callable_t_>(callable_)(*it_);
	}
	return hxforward<callable_t_>(callable_);
}

/// `hxless_range` - Returns true if `range0` is lexicographically less than
/// `range1`, using `equal` to detect matching elements and `less` to order
/// the first pair that differs. A range that is a proper prefix of the other
/// is less than the other. Costs one `equal` call and, at the deciding
/// position only, one `less` call per element. Requires a `forward-iterator`.
template<hxrange_concept_ range0_t_, hxrange_concept_ range1_t_, typename equal_t_, typename less_t_>
hxattr_nodiscard hxinline hxconstexpr hxattr_flatten
bool hxless_range(range0_t_&& range0_, range1_t_&& range1_, const equal_t_& equal_,
		const less_t_& less_) {
	hxrestrict_t<decltype(range0_.begin())> begin0_(range0_.begin());
	const auto end0_ = range0_.end();
	hxrestrict_t<decltype(range1_.begin())> begin1_(range1_.begin());
	const auto end1_ = range1_.end();
	while(begin0_ != end0_ && begin1_ != end1_) {
		if(!equal_(*begin0_, *begin1_)) { return less_(*begin0_, *begin1_); }
		++begin0_; ++begin1_;
	}
	return begin0_ == end0_ && begin1_ != end1_;
}

/// `hxless_range` (specialization) - An overload of `hxless_range` that uses
/// `hxkey_equal` and `hxkey_less` over `range0` and `range1`. Requires a
/// `forward-iterator`.
template<hxrange_concept_ range0_t_, hxrange_concept_ range1_t_>
hxattr_nodiscard hxinline hxconstexpr hxattr_flatten
bool hxless_range(range0_t_&& range0_, range1_t_&& range1_) {
	const auto begin0_ = range0_.begin();
	return hxless_range<range0_t_, range1_t_>(hxforward<range0_t_>(range0_),
		hxforward<range1_t_>(range1_), hxkey_equal_t<decltype(*begin0_)>{},
		hxkey_less_t<decltype(*begin0_)>{});
}

/// `hxlower_bound` - Returns the first position in the sorted `range` where
/// `value` could be inserted without violating the ordering. Returns
/// `range.end()` if no element is ordered after or equivalent to `value`.
/// Unsorted data will lead to errors. The `less` callable returns true if the
/// first argument is ordered before (i.e., is less than) the second. Requires a
/// `random-iterator`.
template<hxrandom_range_concept_ range_t_, typename value_t_, typename less_t_>
hxattr_nodiscard hxinline hxconstexpr hxattr_flatten
auto hxlower_bound(range_t_&& range_, const value_t_& value_, const less_t_& less_)
		-> decltype(range_.begin()) {
	using iterator_t_ = hxrestrict_t<decltype(range_.begin())>;
	iterator_t_ begin_ = range_.begin();
	// Does not dereference null pointer args.
	hxsize_t count_ = range_.end() - begin_;
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

/// `hxlower_bound` (specialization) - An overload of `hxlower_bound` over
/// `range` that uses `hxkey_less`. Requires a `random-iterator`.
template<hxrandom_range_concept_ range_t_, typename value_t_>
hxattr_nodiscard hxinline hxconstexpr hxattr_flatten
auto hxlower_bound(range_t_&& range_, const value_t_& value_) -> decltype(range_.begin()) {
	const auto begin_ = range_.begin();
	return hxlower_bound(hxforward<range_t_>(range_), value_,
		hxkey_less_t<decltype(*begin_)>{});
}

/// `hxupper_bound` - Returns the first position in the sorted `range` whose
/// element is ordered after `value`. Returns `range.end()` if no such element
/// exists. Unsorted data will lead to errors. The `less` callable returns
/// true if the first argument is ordered before (i.e., is less than) the
/// second. Requires a `random-iterator`.
template<hxrandom_range_concept_ range_t_, typename value_t_, typename less_t_>
hxattr_nodiscard hxinline hxconstexpr hxattr_flatten
auto hxupper_bound(range_t_&& range_, const value_t_& value_, const less_t_& less_)
		-> decltype(range_.begin()) {
	using iterator_t_ = hxrestrict_t<decltype(range_.begin())>;
	iterator_t_ begin_ = range_.begin();
	// Does not dereference null pointer args.
	hxsize_t count_ = range_.end() - begin_;
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

/// `hxupper_bound` (specialization) - An overload of `hxupper_bound` over
/// `range` that uses `hxkey_less`. Requires a `random-iterator`.
template<hxrandom_range_concept_ range_t_, typename value_t_>
hxattr_nodiscard hxinline hxconstexpr hxattr_flatten
auto hxupper_bound(range_t_&& range_, const value_t_& value_) -> decltype(range_.begin()) {
	const auto begin_ = range_.begin();
	return hxupper_bound(hxforward<range_t_>(range_), value_,
		hxkey_less_t<decltype(*begin_)>{});
}

HX_NS_END_
