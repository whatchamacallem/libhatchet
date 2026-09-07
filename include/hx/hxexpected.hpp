#pragma once
// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

/// \file
/// An expected value type. Implements `std::expected<T, E>`.

#include "libhatchet.h"

static_assert(HX_CPLUSPLUS >= 202302L, "hxexpected.hpp requires C++23");

// HX_USE_MACROS_WITH_MODULE allows including macros alongside the module.
#if HX_USE_MACROS_WITH_MODULE
#error Header does not provide macros alone.
#endif

#include "hxallocator.hpp"
#include "hxkey.hpp"

HX_NS_BEGIN_

/// \cond HIDDEN
template<typename T_>
concept hxexpected_concept_ = requires(T_& x_, const T_& y_) {
	sizeof(T_);
	T_();
	T_(y_);
	T_(hxmove(x_));
	x_ = y_;
	x_ = hxmove(x_);
	x_.~T_();
	{ x_ == y_ } -> hxsame_as<bool>;
};

template<typename E_>
concept hxunexpected_concept_ = requires(E_& x_, const E_& y_) {
	sizeof(E_);
	E_(true);
	E_(false);
	E_(y_);
	E_(hxmove(x_));
	x_ = y_;
	x_ = hxmove(x_);
	x_.~E_();
	{ static_cast<bool>(y_) };
	{ x_ == y_ } -> hxsame_as<bool>;
};

template<hxexpected_concept_ T_, hxunexpected_concept_ E_=bool> class hxexpected;

// Used to prevent forwarding constructors from hijacking converting-copy
// construction paths.
template<typename> class hxis_hxexpected_ : public hxfalse_t { };
template<typename T_, typename E_>
class hxis_hxexpected_<hxexpected<T_, E_>> : public hxtrue_t { };
/// \endcond

/// `hxexpected<T, E=bool>` - Holds either a value of type `T` or an error of
/// type `E`. Implements `std::expected<T, E>`. A false equivalent error means
/// the value is valid and a true equivalent error means the value is
/// uninitialized storage. `E` defaults to `bool`.
/// - `T` : The contained value type. Must not be a reference or pointer type.
///   Use `hxref<T>` for reference or pointer types.
/// - `E` : The error type. Must support `static_cast<bool>`.
template<hxexpected_concept_ T_, hxunexpected_concept_ E_>
class hxexpected : private hxallocator<T_, 1> {
public:
	// These are for safety. Use hxref for pointer and reference types.
	static_assert(!hxis_reference<T_>(), "hxexpected does not support reference types");
	static_assert(!hxis_pointer<T_>(), "hxexpected does not support pointer types");

	/// `error_t` - Publishes the error type.
	using error_t = E_;

	/// `value_t` - Publishes the contained value type.
	using value_t = T_;

	/// Default constructs an error using `E(true)`.
	hxexpected(void) : m_error_(true) { }

	/// Constructs an error from `hxnil` using `E(true)`.
	/// - `hxnil` : The error sentinel.
	hxexpected(hxnil_t) : m_error_(true) { }

	/// Copy constructor. Copies the state, value or error from `x`.
	/// - `x` : The `hxexpected` to copy from.
	hxexpected(const hxexpected& x_) noexcept;

	/// Move constructor. Moves the state, value or error from `x`.
	/// - `x` : The `hxexpected` to move from.
	hxexpected(hxexpected&& x_) noexcept;

	/// Constructs an `hxexpected` with `error`. If `error` is false then the
	/// value is also constructed by forwarding `args`. The standard idiom for
	/// loading a known value with no error is `hxexpected(false, args...)`.
	/// - `error` : An error that converts to bool.
	/// - `args` : The args to construct the value from. Must be convertible to `T`.
	template<typename error_t_=E_, typename... args_t_, hxenable_if_t<
		!hxis_hxexpected_<hxremove_cvref_t<error_t_>>::value, bool> = true>
	explicit hxexpected(error_t_&& error_, args_t_&&... args_) noexcept;

	/// Destroys the contained value if non-null.
	~hxexpected(void);

	/// Returns a const reference to the contained value. The expected value
	/// must not contain an error.
	hxattr_nodiscard const T_& operator*(void) const;

	hxattr_nodiscard T_& operator*(void);

	/// Returns a const pointer to the contained value. The expected value must
	/// not contain an error.
	hxattr_nodiscard const T_* operator->(void) const;

	hxattr_nodiscard T_* operator->(void);

	/// Returns `true` if the expected value contains a value.
	hxattr_nodiscard explicit operator bool(void) const { return !static_cast<bool>(m_error_); }

	/// Copy assignment. Copies the state, value or error from `x`.
	/// - `x` : The `hxexpected` to copy from.
	hxexpected& operator=(const hxexpected& x_) noexcept;

	/// Move assignment. Moves the state, value or error from `x`.
	/// - `x` : The `hxexpected` to move from.
	hxexpected& operator=(hxexpected&& x_) noexcept;

	/// Assigns a default error using `T(true)`.
	/// - `hxnil` : The error sentinel.
	hxexpected& operator=(hxnil_t) { this->reset(); return *this; }

	/// Assigns `value`, engaging the expected value.
	/// - `value` : The value to assign from. Must be convertible to `T`.
	template<typename U_=T_, hxenable_if_t<
		!hxis_hxexpected_<hxremove_cvref_t<U_>>::value &&
		!hxis_same<hxremove_cvref_t<U_>, E_>(), bool> = true>
	hxexpected& operator=(U_&& value_) noexcept;

	/// Returns `true` if both expected values contain equal values or equal
	/// errors.
	/// - `x` : Right hand side expected value.
	hxattr_nodiscard bool operator==(const hxexpected& x_) const { return this->equal(x_); }

	/// Returns `true` if this expected value contains an error.
	/// - `hxnil` : The error sentinel.
	hxattr_nodiscard bool operator==(hxnil_t) const { return static_cast<bool>(m_error_); }

	/// Returns `true` if this expected value contains a value equal to `value`.
	/// - `value` : The value to compare against.
	hxattr_nodiscard bool operator==(const T_& value_) const;

	/// Returns the result of calling `callable` with the contained value when
	/// there is no error, otherwise returns `hxnil`. Use `and_then` to return
	/// `hxptr`, `hxref` or `hxexpected`.
	/// - `callable` : The function to call with the contained value.
	template<typename self_t_, typename callable_t_>
	hxattr_nodiscard auto and_then(this self_t_&& self_, callable_t_&& callable_)
		-> hxremove_cvref_t<decltype(hxforward<callable_t_>(callable_)(
			hxforward_like<self_t_>(hxdeclval<T_&>())))>;

	/// Constructs the contained value in place from `args`, clearing the error.
	/// - `args` : Arguments forwarded to the constructor of `T`.
	template<typename... args_t_>
	T_& emplace(args_t_&&... args_) noexcept;

	/// Returns `true` if both expected values contain equal values or equal
	/// errors.
	/// - `x` : The `hxexpected` to compare against.
	hxattr_nodiscard bool equal(const hxexpected& x_) const;

	/// Returns a reference to the error with the constness and value category
	/// of `self`. The expected value must contain an error.
	template<typename self_t_>
	hxattr_nodiscard auto error(this self_t_&& self_)
		-> decltype(hxforward_like<self_t_>(hxdeclval<E_&>()));

	/// Returns `true` if the expected value contains a value.
	hxattr_nodiscard bool has_value(void) const { return !static_cast<bool>(m_error_); }

	/// Returns the hash of the contained value if non-null, otherwise `31u`.
	hxattr_nodiscard hxhash_t hash(void) const;

	/// Returns this expected value if non-null, otherwise returns the result of
	/// calling `callable`.
	/// - `callable` : The function to call when an error is present.
	template<typename self_t_, typename callable_t_>
	hxattr_nodiscard hxexpected or_else(this self_t_&& self_, callable_t_&& callable_);

	/// Destroys the contained value and assigns a true error.
	void reset(void) noexcept;

	/// Changes the error. A false error engages a default constructed value and
	/// a true error destroys any contained value.
	/// - `error` : The error determining whether the value is non-null.
	void reset(const E_& error_) noexcept;

	/// Exchanges the values and errors with `x`.
	/// - `x` : The `hxexpected` to swap with.
	void swap(hxexpected& x_) noexcept;

	/// Returns a reference to the contained value with the constness and value
	/// category of `self`. The expected value must not contain an error.
	template<typename self_t_>
	hxattr_nodiscard auto value(this self_t_&& self_)
		-> decltype(hxforward_like<self_t_>(hxdeclval<T_&>()));

	/// Returns the contained value if non-null, otherwise returns a `T`
	/// constructed from `args`.
	/// - `args` : The arguments used to construct the value when an error is
	///    present.
	template<typename self_t_, typename... args_t_>
	hxattr_nodiscard T_ value_or(this self_t_&& self_, args_t_&&... args_);

private:
	E_ m_error_;
};

/// `hxmake_expected<T>` - Returns an non-null `hxexpected<T>` constructed by
/// copying `value`.
/// - `value` : The value to copy into the expected value.
template<typename T_>
hxattr_nodiscard hxexpected<T_> hxmake_expected(const T_& value_) {
	return hxexpected<T_>(false, value_);
}

/// `hxmake_expected<T, E>` - Returns an non-null `hxexpected<T, E>` constructed
/// in place by forwarding `args` to the constructor of `T`.
/// - `args` : Arguments forwarded to the constructor of `T`.
template<typename T_, typename E_, typename... args_t_>
hxattr_nodiscard hxexpected<T_, E_> hxmake_expected(args_t_&&... args_) {
	hxexpected<T_, E_> result_;
	result_.emplace(hxforward<args_t_>(args_)...);
	return result_;
}

/// `hxkey_equal_t<hxexpected<T, E>>` - Compares `x` and `y` for equivalence.
template<typename T_, typename E_>
class hxkey_equal_t<hxexpected<T_, E_> > {
public:
	hxattr_nodiscard hxinline hxattr_flatten
	bool operator()(
			const hxexpected<T_, E_>& x_, const hxexpected<T_, E_>& y_) const {
		return x_.equal(y_);
	}
};

/// `hxkey_hash_t<hxexpected<T, E>>` - Returns the hash of the contained value
/// if non-null, otherwise `31u`.
template<typename T_, typename E_>
class hxkey_hash_t<hxexpected<T_, E_> > {
public:
	hxattr_nodiscard hxinline hxattr_flatten
	hxhash_t operator()(const hxexpected<T_, E_>& expected_) const {
		return expected_.hash();
	}
};

#include "detail/hxexpected.inl"
HX_NS_END_
