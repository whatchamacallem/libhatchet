#pragma once
// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

/// \file
/// A few utility functions and most standard C++ meta programming functions.

#include "libhatchet.h"

#if HX_CPLUSPLUS
extern "C" {

// HX_USE_MACROS_WITH_MODULE allows including macros alongside the module.
#if HX_USE_MACROS_WITH_MODULE
#error Header does not provide macros alone.
#endif
#endif

// -- C Utilities --------------------------------------------------------------

/// `hxbasename` - Returns a pointer to those characters following the last
/// `'\'` or `'/'` character or `path` if those are not present.
/// - `path` : Non-null null-terminated path string.
hxattr_nodiscard const char* hxbasename(const char* path_) hxattr_nonnull(1);

/// `hxfloat_dump` - Prints an array of floating point values.
/// - `address` : Non-null pointer to the start of the float array.
/// - `floats` : The number of floats to print.
void hxfloat_dump(const float* address_, size_t floats_) hxattr_nonnull(1) hxattr_cold;

/// `hxhex_dump` - Prints an array of bytes formatted in hexadecimal. Additional
/// information provided when pretty is non-zero.
/// - `address` : Non-null pointer to the start of the byte array.
/// - `bytes` : The number of bytes to print.
/// - `pretty` : Set non-zero to include extended visualization.
void hxhex_dump(const void* address_, size_t bytes_, bool pretty_) hxattr_nonnull(1) hxattr_cold;

#if HX_CPLUSPLUS
} // extern "C"

HX_NS_BEGIN_

// -- C and C++ Utilities ------------------------------------------------------

/// `hxsize` - Returns the size of a C array. Rejects pointer arguments at
/// compile time.
template<typename T_, hxsize_t N_> hxattr_nodiscard constexpr
hxsize_t hxsize(T_ (&)[N_]) { return N_; }

// -- C++ SFINAE ---------------------------------------------------------------
// Substitution Failure Is Not An Error. enable_if checks.

/// \cond HIDDEN
template<bool condition_, typename type_=void> struct hxenable_if_ { };
template<typename type_> struct hxenable_if_<true, type_> { using type = type_; };
/// \endcond

/// \cond HIDDEN
/// `hxenable_if_t<condition>` - Implements `std::enable_if_t`. This is available
/// instead of the `requires` keyword when backwards compatibility is required.
template<bool condition_, typename type_=void>
using hxenable_if_t = typename hxenable_if_<condition_, type_>::type;
/// \endcond

/// `hxnullptr_t` - A class that will only convert to a null `T` pointer. Useful
/// when an integer constant arg would be ambiguous or otherwise break template
/// code. `hxnullptr` is a `hxnullptr_t`. Use plain `hxnull` for comparisons.
class hxnullptr_t {
public:
	/// Null `T` pointer.
	template<typename T_> constexpr operator T_*() const { return 0; }
	/// Null `T` method pointer.
	template<typename T_, typename M_> constexpr operator M_ T_::*() const { return 0; }
private:
	// No address-of operator.
	void operator&(void) const = delete;
};

/// `hxnullptr` - An instance of a class that will only convert to a null
/// pointer. Useful when an integer constant arg would be ambiguous or otherwise
/// break template code. `hxnullptr` is a `hxnullptr_t`. Use plain `hxnull` for
/// `==` and `!=` comparisons.
hxinline_constexpr hxnullptr_t hxnullptr{};

/// Internal. Implements `std::remove_cv`.
/// \cond HIDDEN
template<typename T_> struct hxremove_cv_ { using type = T_; };
template<typename T_> struct hxremove_cv_<const T_> { using type = T_; };
template<typename T_> struct hxremove_cv_<volatile T_> { using type = T_; };
template<typename T_> struct hxremove_cv_<const volatile T_> { using type = T_; };
/// \endcond

/// `hxremove_cv_t` - Removes const and volatile from a type. Implements
/// `std::remove_cv_t`. This is used to maintain semantic compatibility with the
/// standard.
template<typename T_> using hxremove_cv_t = typename hxremove_cv_<T_>::type;

/// \cond HIDDEN
template<typename T_> struct hxremove_pointer_ { using type = T_; };
template<typename T_> struct hxremove_pointer_<T_*> { using type = T_; };
template<typename T_> struct hxremove_pointer_<T_* const> { using type = T_; };
template<typename T_> struct hxremove_pointer_<T_* volatile> { using type = T_; };
template<typename T_> struct hxremove_pointer_<T_* const volatile> { using type = T_; };
/// \endcond

/// `hxremove_pointer_t<T>` - Returns `T` with one pointer level removed.
template<typename T_> using hxremove_pointer_t = typename hxremove_pointer_<T_>::type;

/// \cond HIDDEN
template<typename T_> struct hxremove_reference_       { using type = T_; };
template<typename T_> struct hxremove_reference_<T_&>  { using type = T_; };
template<typename T_> struct hxremove_reference_<T_&&> { using type = T_; };
/// \endcond

/// `hxremove_reference_t<T>` - Returns `T` with references removed.
template<typename T_> using hxremove_reference_t = typename hxremove_reference_<T_>::type;

/// `hxremove_cvref_t<T>` - Returns `T` with const, volatile, and references removed.
template<typename T_> using hxremove_cvref_t = hxremove_cv_t<hxremove_reference_t<T_>>;

/// `hxdeclval` - Implements `std::declval`. Returns a `T&&` reference for use in
/// unevaluated contexts such as `decltype`. Must not be called or defined.
template<typename T_> T_&& hxdeclval(void) noexcept;

/// `hxtrue_t` - Implements `std::true_type`.
struct hxtrue_t { constexpr static bool value = true; };
/// `hxfalse_t` - Implements `std::false_type`.
struct hxfalse_t { constexpr static bool value = false; };

/// `hxis_array` - Implements `std::is_array`.
template<typename T_> struct hxis_array : public hxfalse_t { };
template<typename T_, size_t size_> struct hxis_array<T_[size_]> : public hxtrue_t { };
template<typename T_> struct hxis_array<T_[]> : public hxtrue_t { };

/// `hxis_const` - Implements `std::is_const`.
template<typename T_> struct hxis_const : public hxfalse_t { };
template<typename T_> struct hxis_const<const T_> : public hxtrue_t { };

/// \cond HIDDEN
template<typename T_> struct hxis_floating_point_ : public hxfalse_t { };
template<> struct hxis_floating_point_<float> : public hxtrue_t { };
template<> struct hxis_floating_point_<double> : public hxtrue_t { };
template<> struct hxis_floating_point_<long double> : public hxtrue_t { };
/// \endcond

/// `hxis_floating_point` - Implements `std::is_floating_point`.
template<typename T_>
struct hxis_floating_point : public hxis_floating_point_<hxremove_cv_t<T_>> { };

/// \cond HIDDEN
template<typename T_> struct hxis_integral_ : public hxfalse_t { };
template<> struct hxis_integral_<bool> : public hxtrue_t { };
template<> struct hxis_integral_<char> : public hxtrue_t { };
template<> struct hxis_integral_<signed char> : public hxtrue_t { };
template<> struct hxis_integral_<unsigned char> : public hxtrue_t { };
template<> struct hxis_integral_<wchar_t> : public hxtrue_t { };
template<> struct hxis_integral_<char16_t> : public hxtrue_t { };
template<> struct hxis_integral_<char32_t> : public hxtrue_t { };
#if HX_CPLUSPLUS >= 202002L
template<> struct hxis_integral_<char8_t> : public hxtrue_t { };
#endif
template<> struct hxis_integral_<short> : public hxtrue_t { };
template<> struct hxis_integral_<unsigned short> : public hxtrue_t { };
template<> struct hxis_integral_<int> : public hxtrue_t { };
template<> struct hxis_integral_<unsigned int> : public hxtrue_t { };
template<> struct hxis_integral_<long> : public hxtrue_t { };
template<> struct hxis_integral_<unsigned long> : public hxtrue_t { };
template<> struct hxis_integral_<long long> : public hxtrue_t { };
template<> struct hxis_integral_<unsigned long long> : public hxtrue_t { };
/// \endcond

/// `hxis_integral` - Implements `std::is_integral`.
template<typename T_>
struct hxis_integral : public hxis_integral_<hxremove_cv_t<T_>> { };

/// `hxis_lvalue_reference` - Implements `std::is_lvalue_reference`.
template<typename T_> struct hxis_lvalue_reference : public hxfalse_t { };
template<typename T_> struct hxis_lvalue_reference<T_&> : public hxtrue_t { };

/// \cond HIDDEN
template<typename T_> struct hxis_pointer_ : public hxfalse_t { };
template<typename T_> struct hxis_pointer_<T_*> : public hxtrue_t { };
/// \endcond

/// `hxis_pointer` - Returns whether T is a pointer type as
/// `hxis_pointer<T>::value`. Implements `std::is_pointer`.
template<typename T> struct hxis_pointer : hxis_pointer_<hxremove_cv_t<T>> { };

/// `hxis_reference` - Implements `std::is_reference`.
template<typename T_> struct hxis_reference : public hxfalse_t { };
template<typename T_> struct hxis_reference<T_&>  : public hxtrue_t { };
template<typename T_> struct hxis_reference<T_&&> : public hxtrue_t { };

/// `hxis_rvalue_reference` - Implements `std::is_rvalue_reference`.
template<typename T_> struct hxis_rvalue_reference : public hxfalse_t { };
template<typename T_> struct hxis_rvalue_reference<T_&&> : public hxtrue_t { };

/// `hxis_same` - Implements `std::is_same`.
template<typename A_, typename B_> struct hxis_same : public hxfalse_t { };
template<typename A_> struct hxis_same<A_, A_> : public hxtrue_t { };

/// \cond HIDDEN
template<typename T_> struct hxis_void_ : public hxfalse_t { };
template<> struct hxis_void_<void> : public hxtrue_t { };
/// \endcond

/// `hxis_void` - Implements `std::is_void`.
template<typename T_> struct hxis_void : public hxis_void_<hxremove_cv_t<T_>> { };

/// \cond HIDDEN
template<typename T_> struct hxrestrict_t_ { using type = T_; };
template<typename T_> struct hxrestrict_t_<T_*> { using type = T_* hxrestrict; };
template<typename T_> struct hxrestrict_t_<T_* const> { using type = T_* const hxrestrict; };
/// \endcond

/// `hxrestrict_t` - Adds the `__restrict` keyword to C++ pointers.
/// (Non-standard.)
template<typename T_> using hxrestrict_t = typename hxrestrict_t_<T_>::type;

/// `hxisgraph` - Implements standard `isgraph` for a locale where all non-ASCII
/// characters are considered graphical or mark making. This is compatible with
/// scanf-style parsing of UTF-8 string parameters. However, this is not
/// `en_US.UTF-8`, the default C locale or the POSIX locale.
hxattr_nodiscard constexpr bool hxisgraph(char ch_) {
	return ((static_cast<unsigned char>(ch_) - 0x21u) < 0x5eu)
		|| ((static_cast<unsigned char>(ch_) & 0x80u) != 0u);
}

/// `hxisspace` - Implements standard `isspace` for a locale where all non-ASCII
/// characters are considered graphical or mark making. Returns nonzero for
/// space and `\t
/// \n \v \f \r`. This is compatible with scanf-style parsing of UTF-8 string
/// parameters. However, this is not `en_US.UTF-8`. It does match the the
/// default C locale and the POSIX locale.
hxattr_nodiscard constexpr bool hxisspace(char ch_) {
	return ch_ == ' ' || (static_cast<unsigned char>(ch_) - 0x09u) < 0x05u;
}

/// `hxlog2i` - Returns `log2(n)` as an integer which is the power of 2 of the
/// largest bit in `n`. WARNING: `hxlog2i(0)` is currently -127 and is UB. `i` ≥
/// 2^32-128 will also round up returning an incorrect result.
/// - `i` : A `uint32_t`.
hxattr_nodiscard inline int hxlog2i(uint32_t i_) {
	// Use the floating point hardware because this isn't important enough.
	// The memcpy is an intrinsic.
	float f_ = static_cast<float>(i_);
	uint32_t bits_ = 0u; ::memcpy(&bits_, &f_, sizeof f_);
	return static_cast<int>((bits_ >> 23) & 0xffu) - 127;
}

/// `hxabs` - Returns the absolute value of `x` using a `<` comparison.
/// - `x` : The value to compute the absolute value for.
template<typename T_>
hxattr_nodiscard constexpr T_ hxabs(T_ x_) { return (x_ < T_()) ? (T_() - x_) : x_; }

/// `hxclamp` - Returns `x` clamped between the `minimum` and `maximum` using `<`
/// comparisons.
/// - `x` : The value to clamp.
/// - `minimum` : The minimum allowable value.
/// - `maximum` : The maximum allowable value.
template<typename T_>
hxattr_nodiscard constexpr T_ hxclamp(T_ x_, T_ minimum_, T_ maximum_) {
#if HX_CPLUSPLUS >= 201402L
	hxassertmsg(!(maximum_ < minimum_), "minimum <= maximum");
#endif
	return (x_ < minimum_) ? minimum_ : ((maximum_ < x_) ? maximum_ : x_);
}

/// `hxforward` - Implements `std::forward`. Call as `hxforward<T>(x)` **from
/// inside a templated forwarding function** where the parameter was declared
/// `T&&` and `T` was **deduced**. `T` must be explicitly specified. E.g.,
/// ```cpp
///   template<typename T>
///   void forwards_temp(T&&x) { requires_temp(hxforward<T>(x)); }
/// ```
/// This is the `T&&` version of hxforward<T>.
template<typename T_>
constexpr T_&& hxforward(hxremove_reference_t<T_>&& x_) {
	static_assert(!hxis_lvalue_reference<T_>::value, "T must be a `T&&` reference.");
	return static_cast<T_&&>(x_);
}

/// `hxforward` - This is the `T&` version of hxforward<T>. It gets invoked when
/// `T` turns out to be an l-value. This happens when a `T&` is passed as a
/// `T&&`.
template<typename T_>
constexpr T_&& hxforward(hxremove_reference_t<T_>& x_) {
	return static_cast<T_&&>(x_);
}

/// `hxmax` - Returns the maximum value of `x` and `y` using a `<` comparison.
/// - `x` : The first value.
/// - `y` : The second value.
template<typename T_>
hxattr_nodiscard constexpr T_ hxmax(T_ x_, T_ y_) { return (y_ < x_) ? x_ : y_; }

/// `hxmin` - Returns the minimum value of `x` and `y` using a `<` comparison.
/// - `x` : The first value.
/// - `y` : The second value.
template<typename T_>
hxattr_nodiscard constexpr T_ hxmin(T_ x_, T_ y_) { return (x_ < y_) ? x_ : y_; }

/// `hxmove` - Implements `std::move`. Converts either a `T&` or a `T&&` to a
/// `T&&`. Do not specify `T` explicitly as it will not work as expected. This
/// uses the rules about reference collapsing to handle both `T&` and `T&&`.
template<typename T_>
constexpr hxremove_reference_t<T_>&& hxmove(T_&& t_) {
	return static_cast<hxremove_reference_t<T_>&&>(t_);
}

/// `hxswap` - Exchanges the contents of `x` and `y` using a temporary. If `T`
/// has `T::T(T&&)` or `T::operator=(T&&)` then those will be used.
template<typename T_>
hxconstexpr void hxswap(T_& x_, T_& y_) {
	// Provides an optimization hint.
	hxassertmsg(&x_ != &y_, "hxswap No swapping with self.");
	T_ t_(hxmove(x_));
	x_ = hxmove(y_);
	y_ = hxmove(t_);
}

/// `hxswap_memcpy` - Exchanges the contents of `x` and `y` using `memcpy` and a
/// stack temporary. This is intended for internal use where `T` is known to be
/// trivially copyable.
/// - `x` : First `T&`.
/// - `y` : Second `T&`.
template<typename T_>
void hxswap_memcpy(T_& x_, T_& y_) {
	hxassertmsg(&x_ != &y_, "hxswap_memcpy No swapping with self.");
	char t_[sizeof x_];
	::memcpy(t_, &y_, sizeof x_); // NOLINT(bugprone-undefined-memory-manipulation)
	::memcpy(static_cast<void*>(&y_), &x_, sizeof x_); // NOLINT(bugprone-undefined-memory-manipulation)
	::memcpy(static_cast<void*>(&x_), t_, sizeof x_); // NOLINT(bugprone-undefined-memory-manipulation)
}

HX_NS_END_
#else // !HX_CPLUSPLUS

// -- C Macro Utility API - Does it all backwards in heels --------------------

/// `hxsize` - Returns the size of a C array. WARNING: Args will be multiply
/// instantiated.
#define hxsize(x_) (sizeof(x_) / sizeof(x_)[0])

/// `hxabs` - Returns the absolute value of `x` using a `<` comparison. WARNING:
/// Args will be multiply instantiated.
/// - `x` : The value to compute the absolute value for.
#define hxabs(x_) ((x_) < 0 ? (0 - (x_)) : (x_))

/// `hxclamp` - Returns `x` clamped between the `minimum` and `maximum` using `<`
/// comparisons. WARNING: Args will be multiply instantiated.
/// - `x` : The value to clamp.
/// - `minimum` : The minimum allowable value.
/// - `maximum` : The maximum allowable value.
#define hxclamp(x_, minimum_, maximum_) \
	((x_) < (minimum_) ? (minimum_) : ((maximum_) < (x_) ? (maximum_) : (x_)))

/// `hxmax` - Returns the maximum value of `x` and `y` using a `<` comparison.
/// WARNING: Args will be multiply instantiated.
/// - `x` : The first value.
/// - `y` : The second value.
#define hxmax(x_, y_) ((y_) < (x_) ? (x_) : (y_))

/// `hxmin` - Returns the minimum value of `x` and `y` using a `<` comparison.
/// WARNING: Args will be multiply instantiated.
/// - `x` : The first value.
/// - `y` : The second value.
#define hxmin(x_, y_) ((x_) < (y_) ? (x_) : (y_))

/// `hxswap_memcpy` - Exchanges the contents of `x` and `y` using `memcpy` and a
/// stack temporary. This is intended for internal use where it is known to be
/// safe to do so. WARNING: Args will be multiply instantiated.
/// - `x` : First object.
/// - `y` : Second object.
#define hxswap_memcpy(x_,y_) do { \
	char t_[sizeof(x_) == sizeof(y_) ? (int)sizeof(x_) : -1]; \
	memcpy(t_, &(y_), sizeof(x_)); \
	memcpy(&(y_), &(x_), sizeof(x_)); \
	memcpy(&(x_), t_, sizeof(x_)); } while(0)

#endif // !HX_CPLUSPLUS
