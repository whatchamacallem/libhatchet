#pragma once
// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

/// \file hxkey.hpp User-overloadable key-equal, key-less, and key-hash
/// functions. By default this code uses only the `==` and `<` operators, which
/// works with either the default or a custom `<=>` operator. Alternatively,
/// these functions can be overloaded to resolve key operations without global
/// operator overloads. This code uses C++20 concepts when available and
/// provides no fallbacks for SFINAE otherwise. Partial specialization does not
/// work before C++20. As an alternative, callables are recommended and
/// supported for complex use cases because they are relatively easy to debug.
/// See `hxkey_equal_t` and `hxkey_less_t` for generating default callables.

#include "libhatchet.h"

// HX_USE_MODULE allows including macros in addition to the hx module.
#if HX_USE_MODULE
#error Header does not provide macros only.
#endif

#include "hxutility.h"

/// \cond HIDDEN
// Used for readability.
using hxcstring_const_ = const char*;
using hxcstring_ = char*;
/// \endcond

#if HX_CPLUSPLUS >= 202002L
/// A concept that requires one type to be convertible to another. See usage
/// below. The compiler applies some unintuitive rules when evaluating this.
/// - `from_t` : The source type.
/// - `to_t` : The target type.
template<typename from_t_, typename to_t_>
concept hxconvertible_to = requires(from_t_ (&&from_)()) {
	// from_ is an unexecuted function pointer used to provide a from_t_&&. This
	// is what the standard does and avoids requiring other operators.
    requires requires { static_cast<to_t_>(from_()); };
};

// Used with the `->` return-type constraint syntax in requires-expressions to
// enforce that a return type is exactly `to_t_` rather than merely convertible.
template<typename from_t_, typename to_t_>
concept hxsame_as = hxis_same<from_t_, to_t_>::value;
#endif

/// `hxkey_equal(const A& a, const B& b)` - Returns true if two objects are
/// equivalent. If your key type does not support `operator==`, then this
/// function may need to be overridden for your key type(s). Function overloads
/// are evaluated when and where the derived container is instantiated and must
/// be consistently available. Uses `operator==`.
template<typename A_, typename B_>
#if HX_CPLUSPLUS >= 202002L
requires requires(const A_& a_, const B_& b_) { { a_ == b_ } -> hxconvertible_to<bool>; }
#endif
hxattr_nodiscard constexpr bool hxkey_equal(const A_& a_, const B_& b_) {
    return a_ == b_;
}

/// `hxkey_equal(const char* a, const char* b)` is `strcmp(a, b) == 0`.
/// Returns true if two C strings are equal (`strcmp(a, b) == 0`).
/// - `a` : The first C string.
/// - `b` : The second C string.
hxattr_nodiscard inline bool hxkey_equal(const hxcstring_const_& a_, const hxcstring_const_& b_) {
    return ::strcmp(a_, b_) == 0;
}

/// Non-const overload.
hxattr_nodiscard inline bool hxkey_equal(const hxcstring_& a_, const hxcstring_& b_) {
    return ::strcmp(a_, b_) == 0;
}

/// `hxkey_equal_t<T>` - A `constexpr` callable that invokes `hxkey_equal`.
/// - `T` : The type to compare.
template<typename T_>
class hxkey_equal_t {
public:
	hxattr_nodiscard constexpr bool operator()(const hxremove_cvref_t<T_>& a_,
			const hxremove_cvref_t<T_>& b_) const {
		return hxkey_equal(a_, b_);
	}
};

/// `hxkey_less(const T&, const T&)` - User-overloadable function for performing
/// comparisons. Invokes `operator<` by default. All the other comparison
/// operators can be written using `operator<`. However `hxkey_equal` is also used
/// for efficiency. Returns true if the first object is less than the second.
/// Override for custom ordering.
/// - `a` : The first object.
/// - `b` : The second object.
template<typename A_, typename B_>
#if HX_CPLUSPLUS >= 202002L
requires requires(const A_& a_, const B_& b_) { { a_ < b_ } -> hxconvertible_to<bool>; }
#endif
hxattr_nodiscard constexpr bool hxkey_less(const A_& a_, const B_& b_) {
    return a_ < b_;
}

/// `hxkey_less(const char*, const char*)` - Returns true if the first C string
/// is lexicographically less than the second by ASCII. UTF-8 is assigned a
/// stable ordering without looking up a locale. Uses (`strcmp(a, b) < 0`).
/// - `a` : The first C string.
/// - `b` : The second C string.
hxattr_nodiscard inline bool hxkey_less(const hxcstring_const_& a_, const hxcstring_const_& b_) {
    return ::strcmp(a_, b_) < 0;
}

/// Non-const overload.
hxattr_nodiscard inline bool hxkey_less(const hxcstring_& a_, const hxcstring_& b_) {
    return ::strcmp(a_, b_) < 0;
}

/// `hxkey_less_t<T>` - A `constexpr` callable that invokes `hxkey_less`.
/// - `T` : The type to compare.
template<typename T_>
class hxkey_less_t {
public:
	hxattr_nodiscard constexpr bool operator()(const hxremove_cvref_t<T_>& a_,
			const hxremove_cvref_t<T_>& b_) const {
		return hxkey_less(a_, b_);
	}
};

// xxhash32 prime constants and avalanche mixing. Useful when hashing sequential
// data. These are used by hxkey_hash overloads below.
hxinline_constexpr hxhash_t hxhash_prime1_ = hxhash_t{0x9E3779B1u};
hxinline_constexpr hxhash_t hxhash_prime2_ = hxhash_t{0x85EBCA77u};
hxinline_constexpr hxhash_t hxhash_prime3_ = hxhash_t{0xC2B2AE3Du};
hxinline_constexpr hxhash_t hxhash_prime4_ = hxhash_t{0x27D4EB2Fu};
hxinline_constexpr hxhash_t hxhash_prime5_ = hxhash_t{0x165667B1u};

// xxhash32 avalanche: x ^= x >> 15, x *= prime2, x ^= x >> 13, x *= prime3, x ^= x >> 16.
hxattr_nodiscard inline hxhash_t hxhash_avalanche_(hxhash_t x_) {
    x_ ^= x_ >> 15u;
    x_ *= hxhash_prime2_;
    x_ ^= x_ >> 13u;
    x_ *= hxhash_prime3_;
    x_ ^= x_ >> 16u;
    return x_;
}

/// `hxkey_hash(T)` - Returns the xxhash32 of a numeric value cast to 32 bits.
/// Used by the base hash table node. Override for custom key types. Overrides
/// are evaluated when and where the hash table is instantiated. Uses the
/// xxhash32 short-input path for a single 4-byte word.
/// - `x` : The input value.
template<typename T_>
hxattr_nodiscard inline hxhash_t hxkey_hash(T_ x_) {
    // xxhash32 short-input path: seed=0, length=4, single 4-byte word mix then avalanche.
    hxhash_t h_ = hxhash_prime5_ + hxhash_t{4u};
    h_ += static_cast<hxhash_t>(x_) * hxhash_prime3_;
    h_  = ((h_ << 17u) | (h_ >> 15u)) * hxhash_prime4_;
    return hxhash_avalanche_(h_);
}

/// `hxkey_hash(const char*)` - Returns the xxhash32 of a C string. Uses the
/// xxhash32 byte-by-byte short-input path with seed 0.
/// - `s` : The C string.
hxattr_nodiscard hxattr_hot inline hxhash_t hxkey_hash(const char* s_) {
    // xxhash32 short-input init: seed=0, length accumulated below.
    hxhash_t h_ = hxhash_prime5_;
    hxhash_t len_ = hxhash_t{0u};
    while(*s_ != '\0') {
        h_ += static_cast<hxhash_t>(static_cast<unsigned char>(*s_++)) * hxhash_prime5_;
        h_ = ((h_ << 11u) | (h_ >> 21u)) * hxhash_prime1_;
        ++len_;
    }
    return hxhash_avalanche_(h_ + len_);
}
