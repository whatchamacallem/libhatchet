#pragma once
// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

/// \file
/// A pair of public fields with the key operations required by the keyed
/// containers.

#include "libhatchet.h"

// HX_USE_MACROS_WITH_MODULE allows including macros alongside the module.
#if HX_USE_MACROS_WITH_MODULE
#error Header does not provide macros alone.
#endif

#include "hxkey.hpp"

HX_NS_BEGIN_

/// `hxpair` - Provides C-style construction of a pair of public fields `a` and
/// `b`. Avoid API use and prefer `hxexpected`.
template<typename a_t_, typename b_t_> class hxpair {
public:
	/// The type of the first value.
	using a_t = a_t_;
	/// The type of the second value.
	using b_t = b_t_;

	/// Returns true if `a.a` is less than `b.a`, or if they are equal, if `a.b`
	/// is less than `b.b`. Ordering of each field uses `hxkey_less` so that a C
	/// string field is ordered by `strcmp`.
	hxattr_nodiscard friend hxconstexpr bool operator<(const hxpair& a_, const hxpair& b_) {
		return hxkey_less(a_.a, b_.a)
			|| (!hxkey_less(b_.a, a_.a) && hxkey_less(a_.b, b_.b));
	}

	/// Returns true if `a.a` and `a.b` are equal to `b.a` and `b.b`. Equality
	/// of each field uses `hxkey_equal` so that a C string field is compared by
	/// `strcmp`.
	hxattr_nodiscard friend hxconstexpr bool operator==(const hxpair& a_, const hxpair& b_) {
		return hxkey_equal(a_.a, b_.a) && hxkey_equal(a_.b, b_.b);
	}

#if HX_CPLUSPLUS >= 202002L
	/// Returns a negative value if `a` orders before `b`, a positive value if
	/// it orders after and zero otherwise. Compares `a.b` against `b.b` only
	/// when `a.a` and `b.a` are equivalent. Both fields must return a
	/// compatible type.
	hxattr_nodiscard friend hxconstexpr auto operator<=>(const hxpair& a_, const hxpair& b_)
			-> decltype(hxkey_three_way(hxdeclval<const a_t_&>(), hxdeclval<const a_t_&>())) {
		const auto c_ = hxkey_three_way(a_.a, b_.a);
		if(c_ != 0) { return c_; }
		return hxkey_three_way(a_.b, b_.b);
	}
#else
	/// Returns false if `a.a` and `a.b` are equal to `b.a` and `b.b`.
	hxattr_nodiscard friend hxconstexpr bool operator!=(const hxpair& a_, const hxpair& b_) {
		return !(a_ == b_);
	}
#endif

	/// Returns a negative value if `a` orders before `b`, a positive value if
	/// it orders after and zero otherwise. Provided as a C++11 fallback that
	/// will get picked up by `hxkey_three_way`. Both fields must return a
	/// compatible type.
	hxattr_nodiscard friend hxconstexpr auto operator-(const hxpair& a_, const hxpair& b_)
			-> decltype(hxkey_three_way(hxdeclval<const a_t_&>(), hxdeclval<const a_t_&>())) {
		const auto d_ = hxkey_three_way(a_.a, b_.a);
		if(d_ != 0) { return d_; }
		return hxkey_three_way(a_.b, b_.b);
	}

	/// The first value.
	a_t_ a;
	/// The second value.
	b_t_ b;
};

/// `hxkey_hash_t<hxpair<A, B>>` - Returns a hash mixing the hashes of both
/// fields in order.
template<typename a_t_, typename b_t_>
class hxkey_hash_t<hxpair<a_t_, b_t_> > {
public:
	hxattr_nodiscard hxinline constexpr hxattr_flatten
	hxhash_t operator()(const hxpair<a_t_, b_t_>& x_) const {
		hxhash_t h_ = hxhash_k5_;
		h_ += hxkey_hash(x_.a) * hxhash_k5_;
		h_ = ((h_ << 11u) | (h_ >> 21u)) * hxhash_k1_;
		h_ += hxkey_hash(x_.b) * hxhash_k5_;
		h_ = ((h_ << 11u) | (h_ >> 21u)) * hxhash_k1_;
		return hxhash_avalanche_(h_);
	}
};

HX_NS_END_
