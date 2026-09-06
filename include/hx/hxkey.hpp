#pragma once
// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

/// \file
/// User-specializable key-equal, key-less, and key-hash callables. By default
/// these compare with `==` and `<`, which works with a defaulted `<=>`
/// operator, and hash with an xxhash32-derived mix. Override key operations
/// for a type by explicitly specializing `hxkey_equal_t`, `hxkey_less_t`, or
/// `hxkey_hash_t`, the same way `std::hash` is specialized. Specializations
/// are evaluated when and where the derived container is instantiated and
/// must be consistently available.

#include "libhatchet.h"

// HX_USE_MACROS_WITH_MODULE allows including macros alongside the module.
#if HX_USE_MACROS_WITH_MODULE
#error Header does not provide macros alone.
#endif

#include "hxutility.h"

HX_NS_BEGIN_

/// \cond HIDDEN
// UTF-8 string detection. Does not match signed/unsigned char.
template<typename T_> struct hxis_string_ : public hxfalse_t { };
template<> struct hxis_string_<char*> : public hxtrue_t { };
template<> struct hxis_string_<const char*> : public hxtrue_t { };
template<> struct hxis_string_<volatile char*> : public hxtrue_t { };
template<> struct hxis_string_<const volatile char*> : public hxtrue_t { };
template<size_t size_> struct hxis_string_<char[size_]> : public hxtrue_t { };
template<size_t size_> struct hxis_string_<const char[size_]> : public hxtrue_t { };
template<size_t size_> struct hxis_string_<volatile char[size_]> : public hxtrue_t { };
template<size_t size_> struct hxis_string_<const volatile char[size_]> : public hxtrue_t { };
template<> struct hxis_string_<char[]> : public hxtrue_t { };
template<> struct hxis_string_<const char[]> : public hxtrue_t { };
template<> struct hxis_string_<volatile char[]> : public hxtrue_t { };
template<> struct hxis_string_<const volatile char[]> : public hxtrue_t { };
/// \endcond

/// `hxis_string<T>` - Checks if T is some kind of `char*` or `char` array,
/// ignoring any top level const, volatile or reference qualifiers on `T`
/// itself.
template<typename T_>
hxattr_nodiscard constexpr hxinline
bool hxis_string(void) {
	return hxis_string_<hxremove_cvref_t<T_>>::value;
}

/// `hxkey_equal_t<T>` - A `constexpr` callable that returns true if two
/// objects are equivalent. Invokes `operator==` by default. Specialize for a
/// key type `T` to resolve equality without a global `operator==`.
/// - `T` : The type to compare.
template<typename T_=void, typename enabled_t=void>
class hxkey_equal_t {
public:
	template<typename A_, typename B_>
	hxattr_nodiscard hxinline hxconstexpr hxattr_flatten
	bool operator()(const A_& a_, const B_& b_) const { return a_ == b_; }
};

/// `hxkey_equal_t<T>` for a C string pointer `T` - Compares two C strings with
/// `strcmp(a, b) == 0`.
template<typename T_>
class hxkey_equal_t<T_, hxenable_if_t<hxis_string<T_>()>> {
public:
	hxattr_nodiscard hxinline bool operator()(const volatile char* a_, const volatile char* b_) const {
		return ::strcmp(const_cast<const char*>(a_), const_cast<const char*>(b_)) == 0;
	}
};

/// `hxkey_equal` - Returns true if `a` and `b` are equivalent, deducing `A`
/// and invoking `hxkey_equal_t<A>`.
/// - `a` : The first value to compare.
/// - `b` : The second value to compare.
template<typename A_, typename B_>
hxattr_nodiscard hxinline hxconstexpr hxattr_flatten
bool hxkey_equal(const A_& a_, const B_& b_) {
	return hxkey_equal_t<A_>{}(a_, b_);
}

/// `hxkey_less_t<T>` - A `constexpr` callable that returns true if the first
/// object is less than the second. Invokes `operator<` by default. All the
/// other comparison operators can be written using `operator<`. However
/// `hxkey_equal_t` is also used for efficiency. Specialize for a key type `T`
/// to resolve custom ordering.
/// - `T` : The type to compare.
template<typename T_=void, typename enabled_t=void>
class hxkey_less_t {
public:
	template<typename A_, typename B_>
	hxattr_nodiscard hxinline hxconstexpr hxattr_flatten
	bool operator()(const A_& a_, const B_& b_) const { return a_ < b_; }
};

/// `hxkey_less_t<T>` for a C string pointer `T` - Returns true if the first C
/// string is lexicographically less than the second by ASCII. UTF-8 is assigned
/// a stable ordering without looking up a locale. Uses (`strcmp(a, b) < 0`).
template<typename T_>
class hxkey_less_t<T_, hxenable_if_t<hxis_string<T_>()>> {
public:
	hxattr_nodiscard hxinline bool operator()(const volatile char* a_, const volatile char* b_) const {
		return ::strcmp(const_cast<const char*>(a_), const_cast<const char*>(b_)) < 0;
	}
};

/// `hxkey_less` - Returns true if `a` is less than `b`, deducing `A` and
/// invoking `hxkey_less_t<A>`.
/// - `a` : The first value to compare.
/// - `b` : The second value to compare.
template<typename A_, typename B_>
hxattr_nodiscard hxinline hxconstexpr hxattr_flatten
bool hxkey_less(const A_& a_, const B_& b_) {
	return hxkey_less_t<A_>{}(a_, b_);
}

/// \cond HIDDEN
// xxhash32 prime constants and avalanche mixing. Useful when hashing sequential
// data. These are used by hxkey_hash_t specializations below.
hxinline_constexpr hxhash_t hxhash_prime1_ = hxhash_t{0x9E3779B1u};
hxinline_constexpr hxhash_t hxhash_prime2_ = hxhash_t{0x85EBCA77u};
hxinline_constexpr hxhash_t hxhash_prime3_ = hxhash_t{0xC2B2AE3Du};
hxinline_constexpr hxhash_t hxhash_prime4_ = hxhash_t{0x27D4EB2Fu};
hxinline_constexpr hxhash_t hxhash_prime5_ = hxhash_t{0x165667B1u};

// xxhash32 avalanche: x ^= x >> 15, x *= prime2, x ^= x >> 13, x *= prime3, x ^= x >> 16.
hxattr_nodiscard hxinline hxconstexpr
hxhash_t hxhash_avalanche_(hxhash_t x_) {
	x_ ^= x_ >> 15u;
	x_ *= hxhash_prime2_;
	x_ ^= x_ >> 13u;
	x_ *= hxhash_prime3_;
	x_ ^= x_ >> 16u;
	return x_;
}
/// \endcond

/// `hxkey_hash_t<T>` - A callable that returns the xxhash32 of a numeric value
/// cast to 32 bits. Used by the base hash table node. Specialize for custom
/// key types, the same way `std::hash` is specialized. Uses the xxhash32
/// short-input path for a single 4-byte word.
/// - `T` : The type to hash.
template<typename T_, typename enabled_t=void>
class hxkey_hash_t {
public:
	hxattr_nodiscard hxinline hxhash_t operator()(const T_& x_) const {
		// xxhash32 short-input path: seed=0, length=4, single 4-byte word.
		hxhash_t h_ = hxhash_prime5_ + hxhash_t{4u};
		// Did you write a custom hxkey_hash_t specialization?
		h_ += static_cast<hxhash_t>(x_) * hxhash_prime3_;
		h_  = ((h_ << 17u) | (h_ >> 15u)) * hxhash_prime4_;
		return hxhash_avalanche_(h_);
	}
};

/// `hxkey_hash_t<T>` for a C string pointer `T` - Returns the xxhash32 style
/// hash of a C string. Mixes each byte with the xxhash32 primes in a single
/// pass and applies the xxhash32 avalanche.
template<typename T_>
class hxkey_hash_t<T_, hxenable_if_t<hxis_string<T_>()>> {
public:
	hxattr_nodiscard hxinline hxhash_t operator()(const volatile char* s_) const {
		const char* t_ = const_cast<const char*>(s_);
		hxhash_t h_ = hxhash_prime5_;
		while(*t_ != '\0') {
			h_ += static_cast<hxhash_t>(static_cast<unsigned char>(*t_++)) * hxhash_prime5_;
			h_ = ((h_ << 11u) | (h_ >> 21u)) * hxhash_prime1_;
		}
		return hxhash_avalanche_(h_);
	}
};

/// `hxkey_hash` - Returns the hash of `x`, deducing `T` and invoking
/// `hxkey_hash_t<T>`.
/// - `x` : The value to hash.
template<typename T_>
hxattr_nodiscard hxinline hxattr_flatten hxhash_t hxkey_hash(const T_& x_) {
	return hxkey_hash_t<T_>{}(x_);
}

HX_NS_END_
