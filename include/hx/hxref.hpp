#pragma once
// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

/// \file
/// An optional reference type. Implements `std::optional<T&>`.

#include "libhatchet.h"

static_assert(HX_CPLUSPLUS >= 202302L, "hxref.hpp requires C++23");

// HX_USE_MACROS_WITH_MODULE allows including macros alongside the module.
#if HX_USE_MACROS_WITH_MODULE
#error Header does not provide macros alone.
#endif

#include "hxexpected.hpp"

HX_NS_BEGIN_

/// \cond HIDDEN
template<typename T_> class hxref;

// Used to prevent the lvalue binding constructor from hijacking converting
// construction paths.
template<typename> struct hxis_hxref_ : hxfalse_t { };
template<typename U_> struct hxis_hxref_<hxref<U_>> : hxtrue_t { };
/// \endcond

/// `hxref<T>` - Holds a reference to a value of type `T`, or null. Implements
/// the non-owning pointer `std::optional<T&>`. A null reference compares equal
/// to `hxnil`. Assignment rebinds the reference instead of assigning through to
/// the referent, matching pointer semantics. Use `hxref<const T>` where write
/// through should be prevented.
/// - `T` : The referenced value type. Must not be an array, reference or
///   pointer type.
template<typename T_>
class hxref {
public:
	// Use hxremove_reference_t<T> if needed.
	static_assert(!hxis_reference<T_>(), "hxref does not support reference types");

	/// `value_type` - Publishes the referenced value type. Doesn't end with
	/// `_t` because of the standard.
	using value_t = T_&;

	/// Constructs a null `hxref`.
	hxref(void) : m_value_(hxnull) { }

	/// Constructs a null `hxref` from `hxnil`.
	/// - `hxnil` : The error sentinel.
	hxref(hxnil_t) : m_value_(hxnull) { }

	/// Binds a reference to `value`. `U&` must bind to `T&` without creating a
	/// temporary.
	/// - `value` : The lvalue to reference.
	template<typename U_, hxenable_if_t<
		!hxis_hxref_<hxremove_cvref_t<U_>>::value &&
		!hxis_hxexpected_<hxremove_cvref_t<U_>>::value &&
		hxbinds_directly<T_, U_>(), bool> = true>
	hxref(U_& value_);

	/// Binds a reference to the value of `x`. Accepts an non-null or
	/// erroneous `hxexpected<T, E>`.
	/// - `x` : The `hxexpected` whose value to bind.
	template<typename E_>
	hxref(hxexpected<T_, E_>& x_);

	/// Returns a reference to the referenced value. The reference must be
	/// non-null.
	hxattr_nodiscard T_& operator*(void) const;

	/// Returns a pointer to the referenced value. The reference must be
	/// non-null.
	hxattr_nodiscard T_* operator->(void) const;

	/// Returns `true` if the reference is non-null.
	hxattr_nodiscard explicit operator bool(void) const { return m_value_ != hxnull; }

	/// Nulls the reference. The referent is not affected.
	/// - `hxnil` : The error sentinel.
	hxref& operator=(hxnil_t) { m_value_ = hxnull; return *this; }

	/// Rebinds the reference to `value`.
	/// - `value` : The lvalue to reference.
	template<typename U_, hxenable_if_t<
		!hxis_hxref_<hxremove_cvref_t<U_>>::value &&
		!hxis_hxexpected_<hxremove_cvref_t<U_>>::value &&
		hxbinds_directly<T_, U_>(), bool> = true>
	hxref& operator=(U_& value_);

	/// Returns `true` if both references are null or both reference equal
	/// values.
	/// - `x` : Right-hand side reference.
	hxattr_nodiscard bool operator==(const hxref& x_) const { return this->equal(x_); }

	/// Returns `true` if this reference is null.
	/// - `hxnil` : The error sentinel.
	hxattr_nodiscard bool operator==(hxnil_t) const { return m_value_ == hxnull; }

	/// Returns `true` if this reference is non-null and its referent equals
	/// `value`.
	/// - `value` : The value to compare against.
	hxattr_nodiscard bool operator==(const T_& value_) const;

	/// Returns the result of calling `callable` with the referenced value if
	/// non-null, otherwise returns `hxnil`. Use `and_then` to return `hxptr`,
	/// `hxref` or `hxexpected`.
	/// - `callable` : The function to call with the referenced value.
	template<typename self_t_, typename callable_t_>
	hxattr_nodiscard auto and_then(this self_t_&& self_, callable_t_&& callable_)
		-> hxremove_cvref_t<decltype(hxforward<callable_t_>(callable_)(hxdeclval<T_&>()))>;

	/// Returns `true` if both references are null or both reference equal
	/// values.
	/// - `x` : The `hxref` to compare against.
	hxattr_nodiscard bool equal(const hxref& x_) const;

	/// Returns `true` if the reference is non-null.
	hxattr_nodiscard bool has_value(void) const { return m_value_ != hxnull; }

	/// Returns the hash of the referenced value if non-null, otherwise `31u`.
	hxattr_nodiscard hxhash_t hash(void) const;

	/// Returns a copy of this reference if non-null, otherwise returns the
	/// result of calling `callable`. `callable` must return an `hxref<T>`.
	/// Implements `std::optional::or_else`.
	/// - `callable` : The function to call when null.
	template<typename self_t_, typename callable_t_>
	hxattr_nodiscard hxref or_else(this self_t_&& self_, callable_t_&& callable_);

	/// Nulls the reference. The referent is not affected.
	void reset(void) { m_value_ = hxnull; }

	/// Exchanges the bound pointers with `x`.
	/// - `x` : The `hxref` to swap with.
	void swap(hxref& x_) noexcept;

	/// Returns a reference to the referenced value. The reference must be
	/// non-null.
	hxattr_nodiscard T_& value(void) const;

	/// Returns the referenced value if non-null, otherwise returns a `T`
	/// constructed from `args`.
	/// - `args` : The arguments used to construct the value when null.
	template<typename self_t_, typename... args_t_>
	hxattr_nodiscard hxremove_cv_t<T_> value_or(
		this self_t_&& self_, args_t_&&... args_);

private:
	T_* m_value_;
};

/// `hxmake_ref<T>` - Returns an `hxref<T>` bound to `value`. `U&` must bind to
/// `T&` without creating a temporary.
/// - `value` : The lvalue to reference.
template<typename T_, typename U_=T_, hxenable_if_t<hxbinds_directly<T_, U_>(), bool> = true>
hxattr_nodiscard hxref<T_> hxmake_ref(U_& value_) { return hxref<T_>(value_); }

/// `hxkey_equal_t<hxref<T>>` - Compares `x` and `y` for equivalence.
template<typename T_>
class hxkey_equal_t<hxref<T_> > {
public:
	hxattr_nodiscard hxinline hxattr_flatten
	bool operator()(
			const hxref<T_>& x_, const hxref<T_>& y_) const {
		return x_.equal(y_);
	}
};

/// `hxkey_hash_t<hxref<T>>` - Returns the hash of the referenced value if
/// non-null, otherwise `31u`.
template<typename T_>
class hxkey_hash_t<hxref<T_> > {
public:
	hxattr_nodiscard hxinline hxattr_flatten
	hxhash_t operator()(const hxref<T_>& ref_) const {
		return ref_.hash();
	}
};

#include "detail/hxref.inl"
HX_NS_END_
