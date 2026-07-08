#pragma once
// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

/// \file
/// An optional reference type. Implements `std::optional<T&>`. See `hxoptional`
/// for a `std::optional<T>` equivalent.

#include "libhatchet.h"

// HX_USE_MACROS_WITH_MODULE allows including macros alongside the module.
#if HX_USE_MACROS_WITH_MODULE
#error Header does not provide macros alone.
#endif

#include "hxoptional.hpp"

HX_NS_BEGIN_

/// \cond HIDDEN
template<typename T_> class hxref;

namespace hxdetail_ {
// Used to prevent the lvalue binding constructor from hijacking converting
// construction paths.
template<typename> struct hxis_hxref_ : hxfalse_t { };
template<typename U_> struct hxis_hxref_<hxref<U_>> : hxtrue_t { };
}
/// \endcond

/// `hxref<T>` - Holds a reference to a value of type `T`, or nothing. Implements
/// `std::optional<T&>`. The reference is stored as a `T*` without dynamic
/// allocation. A disengaged reference compares equal to `hxnullopt`. Assignment
/// rebinds the reference instead of assigning through to the referent, so the
/// post condition is independent of the engaged state. Const is shallow,
/// matching the rebinding behavior. Use `hxref<const T>` where write through
/// should be prevented. This is weird but so is the standard.
/// - `T` : The referenced value type. Must not be a reference or pointer type.
template<typename T_>
class hxref {
public:
	/// `value_type` - Publishes the referenced value type. Doesn't end with `_t`
	/// because of the standard.
	using value_type = T_&;

	/// Constructs a disengaged `hxref`.
	hxref(void) : m_value_(hxnull) { }

	/// Constructs a disengaged `hxref` from `hxnullopt`.
	/// - `nullopt` : The disengaged sentinel.
	hxref(hxnullopt_t) : m_value_(hxnull) { }

	/// Copy constructor. Copies the bound pointer from `other`.
	/// - `other` : The `hxref` to copy from.
	hxref(const hxref& other_) : m_value_(other_.m_value_) { }

	/// Binds a reference to `value`. `U&` must bind to `T&` without creating a
	/// temporary.
	/// - `value` : The lvalue to reference.
	template<typename U_, hxenable_if_t<
		!hxdetail_::hxis_hxref_<hxremove_cvref_t<U_>>::value &&
		!hxdetail_::hxis_hxoptional_<hxremove_cvref_t<U_>>::value &&
		hxis_lvalue_reference<U_&>::value, bool> = true>
	hxref(U_& value_);

	/// Binds a reference by converting the referent of `other`. The referent of
	/// `other` must bind to `T&` without creating a temporary.
	/// - `other` : The `hxref` whose referent to bind.
	template<typename U_, hxenable_if_t<
		!hxis_same<U_, T_>::value, bool> = true>
	hxref(const hxref<U_>& other_);

	/// Binds a reference by converting the value of `other`. The referent of
	/// `other` must bind to `T&` without creating a temporary. Accepts an engaged
	/// or disengaged `hxoptional<U>`.
	/// - `other` : The `hxoptional` whose value to bind.
	template<typename U_>
	hxref(hxoptional<U_>& other_);

	/// Destroys the reference. The referent is not affected.
	~hxref(void) { }

	/// Returns a reference to the referenced value. The reference must be engaged.
	hxattr_nodiscard T_& operator*(void) const;

	/// Returns a pointer to the referenced value. The reference must be engaged.
	hxattr_nodiscard T_* operator->(void) const;

	/// Returns `true` if the reference is engaged.
	hxattr_nodiscard operator bool(void) const { return m_value_ != hxnull; }

	/// Rebinds the reference from `other`. Only the pointer is copied.
	/// - `other` : The `hxref` to copy from.
	hxref& operator=(const hxref& other_);

	/// Disengages the reference. The referent is not affected.
	/// - `nullopt` : The disengaged sentinel.
	hxref& operator=(hxnullopt_t) { m_value_ = hxnull; return *this; }

	/// Rebinds the reference to `value`.
	/// - `value` : The lvalue to reference.
	template<typename U_, hxenable_if_t<
		!hxdetail_::hxis_hxref_<hxremove_cvref_t<U_>>::value &&
		!hxdetail_::hxis_hxoptional_<hxremove_cvref_t<U_>>::value &&
		hxis_lvalue_reference<U_&>::value, bool> = true>
	hxref& operator=(U_& value_);

	/// Returns `true` if both references are disengaged or both reference equal
	/// values.
	/// - `rhs` : Right-hand side reference.
	hxattr_nodiscard bool operator==(const hxref& rhs_) const;

	/// Returns `true` if this reference is disengaged.
	/// - `nullopt` : The disengaged sentinel.
	hxattr_nodiscard bool operator==(hxnullopt_t) const { return m_value_ == hxnull; }

	/// Returns `true` if this reference is engaged and its referent equals `value`.
	/// - `value` : The value to compare against.
	hxattr_nodiscard bool operator==(const T_& value_) const;

#if HX_CPLUSPLUS < 202002L // C++20 defaults != from ==.
	/// Returns `true` if the references are not equal.
	/// - `rhs` : Right-hand side reference.
	hxattr_nodiscard bool operator!=(const hxref& rhs_) const { return !(*this == rhs_); }

	/// Returns `true` if this reference is engaged.
	/// - `nullopt` : The disengaged sentinel.
	hxattr_nodiscard bool operator!=(hxnullopt_t nullopt_) const { return !(*this == nullopt_); }

	/// Returns `true` if this reference is disengaged or its referent does not
	/// equal `value`.
	/// - `value` : The value to compare against.
	hxattr_nodiscard bool operator!=(const T_& value_) const { return !(*this == value_); }
#endif

	/// Returns the result of calling `callable` with the referenced value if
	/// engaged, otherwise returns a disengaged optional of the same type.
	/// `callable` must return an `hxoptional`. Implements
	/// `std::optional::and_then`.
	/// - `callable` : The function to call with the referenced value.
	template<typename function_t_>
	hxattr_nodiscard auto and_then(function_t_&& callable_) const
		-> hxremove_cvref_t<decltype(hxforward<function_t_>(callable_)(hxdeclval<T_&>()))>;

	/// Binds the reference to `value`, engaging the reference. `value` must bind
	/// to `T&` without creating a temporary. Returns the referenced value.
	/// - `value` : The lvalue to reference.
	template<typename U_>
	T_& emplace(U_& value_) { m_value_ = &static_cast<T_&>(value_); return *m_value_; }

	/// Returns `true` if the reference is engaged.
	hxattr_nodiscard bool has_value(void) const { return m_value_ != hxnull; }

	/// Returns a copy of this reference if engaged, otherwise returns the result
	/// of calling `callable`. `callable` must return an `hxref<T>`. Implements
	/// `std::optional::or_else`.
	/// - `callable` : The function to call when disengaged.
	template<typename function_t_>
	hxattr_nodiscard hxref or_else(function_t_&& callable_) const;

	/// Disengages the reference. The referent is not affected.
	void reset(void) { m_value_ = hxnull; }

	/// Exchanges the bound pointers with `other`.
	/// - `other` : The `hxref` to swap with.
	void swap(hxref& other_) noexcept;

	/// Returns a reference to the referenced value. The reference must be engaged.
	hxattr_nodiscard T_& value(void) const;

	/// Returns the referenced value if engaged, otherwise returns
	/// `default_value` forwarded and converted to `T`.
	/// - `default_value` : The value to return when disengaged.
	template<typename U_=hxremove_cv_t<T_>>
	hxattr_nodiscard hxremove_cv_t<T_> value_or(U_&& default_value_) const;

private:
	T_* m_value_;
};

/// `hxref<T&>` - Reference to a reference is not supported.
/// - `T` : The referenced value type.
template<typename T_>
class hxref<T_&> {
public:
	static_assert(sizeof(T_) == 0, "hxref does not support reference types");
};

/// `hxref<T*>` - Reference to a pointer is not supported.
/// - `T` : The referenced value type.
template<typename T_>
class hxref<T_*> {
public:
	static_assert(sizeof(T_) == 0, "hxref does not support pointer types");
};

/// `hxkey_hash(hxref<T>)` - Returns the hash of the referenced value if engaged,
/// otherwise `1u`.
/// - `ref` : The reference to hash.
template<typename T_>
hxattr_nodiscard hxhash_t hxkey_hash(const hxref<T_>& ref_) {
	return ref_.has_value() ? hxkey_hash(*ref_) : hxhash_t{1u};
}

#include "detail/hxref.inl"
HX_NS_END_
