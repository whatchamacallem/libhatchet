#pragma once
// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

/// \file
/// An optional value type.

#include "libhatchet.h"

// HX_USE_MODULE allows including macros in addition to the hx module.
#if HX_USE_MODULE
#error Header does not provide macros only.
#endif

#include "hxutility.h"

HX_NS_BEGIN_

/// \cond HIDDEN
template<typename T_> class hxoptional;

namespace hxdetail_ {
// Used to prevent forwarding constructors from hijacking converting-copy
// construction paths.
template<typename> struct hxis_hxoptional_ : hxfalse_t { };
template<typename U_> struct hxis_hxoptional_<hxoptional<U_>> : hxtrue_t { };
}
/// \endcond

/// `hxnullopt_t` - A tag type used to construct a disengaged `hxoptional`.
struct hxnullopt_t {
	/// Explicit constructor prevents implicit construction from `{}`.
	explicit constexpr hxnullopt_t(int) { }
};

/// `hxnullopt` - A sentinel value of type `hxnullopt_t` representing a
/// disengaged `hxoptional`. Implements `std::nullopt`.
hxinline_constexpr hxnullopt_t hxnullopt{0};

/// `hxoptional<T>` - Holds either a value of type `T` or nothing. Implements
/// `std::optional`. The value is stored in aligned internal storage without
/// dynamic allocation. A disengaged optional compares equal to `hxnullopt`.
/// - `T` : The contained value type. Must not be a reference or array type.
template<typename T_>
class hxoptional {
public:
	/// `value_type` - Publishes the contained value type. Doesn't end with `_t`
	/// because of the standard.
	using value_type = T_;

	/// Constructs a disengaged `hxoptional`.
	hxoptional(void) : m_engaged_(false) { }

	/// Constructs a disengaged `hxoptional` from `hxnullopt`.
	/// - `nullopt` : The disengaged sentinel.
	hxoptional(hxnullopt_t) : m_engaged_(false) { }

	/// Copy constructor. Copies the engaged state and value from `other`.
	/// - `other` : The `hxoptional` to copy from.
	hxoptional(const hxoptional& other_) noexcept;

	/// Move constructor. Moves the engaged state and value from `other`.
	/// `other` is left disengaged.
	/// - `other` : The `hxoptional` to move from.
	hxoptional(hxoptional&& other_) noexcept;

	/// Constructs by copying the engaged state and converting the value of
	/// `other_` to `T_`. `U_` must be convertible to `T_`.
	/// - `other_` : The `hxoptional` to copy from.
	template<typename U_>
	hxoptional(const hxoptional<U_>& other_) noexcept;

	/// Constructs by moving the engaged state and converting the value of
	/// `other_` to `T_`. `other_` is left disengaged. `U_` must be
	/// convertible to `T_`.
	/// - `other_` : The `hxoptional` to move from.
	template<typename U_>
	hxoptional(hxoptional<U_>&& other_) noexcept;

	/// Constructs an engaged `hxoptional` by forwarding `value` into storage.
	/// - `value` : The value to construct from. Must be convertible to `T`.
	template<typename U_=T_, hxenable_if_t<
		!hxis_same<hxremove_cvref_t<U_>, hxoptional>::value &&
		!hxdetail_::hxis_hxoptional_<hxremove_cvref_t<U_>>::value, bool> = true>
	hxoptional(U_&& value_) noexcept;

	/// Destroys the contained value if engaged.
	~hxoptional(void) { reset(); }

	/// Returns a reference to the contained value. The optional must be engaged.
	hxattr_nodiscard T_& operator*(void);

	/// Returns a const reference to the contained value. The optional must be
	/// engaged.
	hxattr_nodiscard const T_& operator*(void) const;

	/// Returns a pointer to the contained value. The optional must be engaged.
	hxattr_nodiscard T_* operator->(void);

	/// Returns a const pointer to the contained value. The optional must be
	/// engaged.
	hxattr_nodiscard const T_* operator->(void) const;

	/// Returns `true` if the optional contains a value.
	hxattr_nodiscard operator bool(void) const { return m_engaged_; }

	/// Copy assignment. Destroys any current value, then copies from `other`.
	/// - `other` : The `hxoptional` to copy from.
	hxoptional& operator=(const hxoptional& other_) noexcept;

	/// Move assignment. Destroys any current value, then moves from `other`.
	/// `other` is left disengaged.
	/// - `other` : The `hxoptional` to move from.
	hxoptional& operator=(hxoptional&& other_) noexcept;

	/// Disengages the optional, destroying any contained value.
	/// - `nullopt` : The disengaged sentinel.
	hxoptional& operator=(hxnullopt_t) { reset(); return *this; }

	/// Assigns `value` by forwarding, engaging the optional.
	/// - `value` : The value to assign from. Must be convertible to `T`.
	template<typename U_=T_, hxenable_if_t<!hxis_same<hxremove_cvref_t<U_>, hxoptional>::value, bool> = true>
	hxoptional& operator=(U_&& value_) noexcept;

	/// Returns `true` if both optionals are disengaged or both are engaged with
	/// equal values.
	/// - `rhs` : Right-hand side optional.
	hxattr_nodiscard bool operator==(const hxoptional& rhs_) const;

	/// Returns `true` if this optional is disengaged.
	/// - `nullopt` : The disengaged sentinel.
	hxattr_nodiscard bool operator==(hxnullopt_t) const { return !m_engaged_; }

	/// Returns `true` if this optional is engaged and its value equals `value`.
	/// - `value` : The value to compare against.
	hxattr_nodiscard bool operator==(const T_& value_) const;

#if HX_CPLUSPLUS < 202002L // C++20 defaults != from ==.
	/// Returns `true` if the optionals are not equal.
	/// - `rhs` : Right-hand side optional.
	hxattr_nodiscard bool operator!=(const hxoptional& rhs_) const { return !(*this == rhs_); }

	/// Returns `true` if this optional is engaged.
	/// - `nullopt` : The disengaged sentinel.
	hxattr_nodiscard bool operator!=(hxnullopt_t) const { return m_engaged_; }

	/// Returns `true` if this optional is disengaged or its value does not
	/// equal `value`.
	/// - `value` : The value to compare against.
	hxattr_nodiscard bool operator!=(const T_& value_) const;
#endif

	/// Constructs the contained value in place from `args`, destroying any
	/// previous value.
	/// - `args` : Arguments forwarded to the constructor of `T`.
	template<typename... args_t_>
	T_& emplace(args_t_&&... args_) noexcept;

	/// Returns `true` if the optional contains a value.
	hxattr_nodiscard bool has_value(void) const { return m_engaged_; }

	/// Destroys the contained value if engaged and leaves the optional
	/// disengaged.
	void reset(void) noexcept;

	/// Exchanges the contents with `other`. Both engaged states and values are
	/// swapped.
	/// - `other` : The `hxoptional` to swap with.
	void swap(hxoptional& other_) noexcept;

	/// Returns a reference to the contained value. The optional must be engaged.
	hxattr_nodiscard T_& value(void);

	/// Returns a const reference to the contained value. The optional must be
	/// engaged.
	hxattr_nodiscard const T_& value(void) const;

	/// Returns the contained value if engaged, otherwise returns `default_value`.
	/// - `default_value` : The value to return when disengaged.
	hxattr_nodiscard T_ value_or(const T_& default_value_) const;

private:
	alignas(T_) char m_storage_[sizeof(T_)];
	bool m_engaged_;
};

/// `hxmake_optional` - Returns an engaged `hxoptional<hxremove_cvref_t<T>>`
/// constructed by forwarding `value`.
/// - `value_` : The value to forward into the optional.
template<typename T_>
hxoptional<hxremove_cvref_t<T_>> hxmake_optional(T_&& value_) {
	return hxoptional<hxremove_cvref_t<T_>>(hxforward<T_>(value_));
}

#include "detail/hxoptional.inl"
HX_NS_END_
