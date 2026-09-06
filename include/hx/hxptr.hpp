#pragma once
// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

/// \file
/// A unique owning pointer.

#include "libhatchet.h"

// HX_USE_MACROS_WITH_MODULE allows including macros alongside the module.
#if HX_USE_MACROS_WITH_MODULE
#error Header does not provide macros alone.
#endif

#include "hxkey.hpp"

HX_NS_BEGIN_

/// `hxptr<T, deleter_t>` - A unique owning pointer. Owns a single dynamically
/// allocated object of type `T` and invokes `deleter_t` on destruction or
/// `reset`. Only one `hxptr` may own a given object at a time. Move
/// construction and move assignment transfer ownership. Copy construction and
/// copy assignment are deleted.
/// - `T` : The pointed-to type. Must not be an array, reference or pointer
///    type.
/// - `deleter_t` : A class type invoked as `bool deleter(T*)` to potentially
///    free the owned pointer. Use `hxconsteval_delete` for `consteval` work.
template<typename T_, typename deleter_t_=hxdefault_delete>
class hxptr : private deleter_t_ {
public:
	// No arrays allowed because there is no delete[] equivalent. Use hxref for
	// pointer and reference types..
	static_assert(!hxis_array<T_>(), "hxptr does not support array types");
	static_assert(!hxis_reference<T_>(), "hxptr does not support reference types");
	static_assert(!hxis_pointer<T_>(), "hxptr does not support pointer types");

	/// `element_t` - Publishes the pointed-to type.
	using element_t = T_;

	/// Constructs an `hxptr` that takes ownership of `ptr` with a specific
	/// deleter instance.
	/// - `ptr` : The pointer to take ownership of. May be null.
	/// - `deleter` : Callable with signature `bool deleter(T*)`.
	hxconstexpr explicit hxptr(T_* ptr_=hxnull, deleter_t_ deleter_=deleter_t_()) noexcept;

	/// Constructs a null `hxptr` from `hxnil`.
	/// - `hxnil` : The error sentinel.
	hxconstexpr hxptr(hxnil_t) noexcept : hxptr(hxnull) { } // GCOVR_EXCL_LINE. Constant folded.

	/// Move constructor. Transfers ownership from `x` to this. `x` is
	/// left null.
	/// - `x` : The `hxptr` to transfer ownership from.
	hxconstexpr hxptr(hxptr&& x_) noexcept;

	/// Destroys the owned object using `deleter_t` if non-null.
	hxconstexpr ~hxptr(void) noexcept;

	/// Move assignment. Destroys the currently owned object, then transfers
	/// ownership from `x`. `x` is left null. Self-assignment is not
	/// supported.
	/// - `x` : The `hxptr` to transfer ownership from.
	hxconstexpr hxptr& operator=(hxptr&& x_) noexcept;

	/// Returns a reference to the owned object. The pointer must not be null.
	hxattr_nodiscard hxconstexpr T_& operator*(void) const;

	/// Returns the owned pointer. The pointer must not be null.
	hxattr_nodiscard hxconstexpr T_* operator->(void) const;

	/// Returns `true` if the owned pointer is non-null.
	hxattr_nodiscard hxconstexpr explicit operator bool(void) const;

	/// Returns `true` if this and `x` point to the same object.
	/// - `x` : The `hxptr` to compare against.
	hxattr_nodiscard hxconstexpr bool operator==(const hxptr& x_) const { return this->equal(x_); }

	/// Returns `true` if the owned pointer is null.
	hxattr_nodiscard hxconstexpr bool operator==(hxnil_t) const;

#if HX_CPLUSPLUS < 202002L // C++20 defaults != from ==.
	/// Returns `true` if this and `x` point to different objects.
	/// - `x` : The `hxptr` to compare against.
	hxattr_nodiscard hxconstexpr bool operator!=(const hxptr& x_) const;

	/// Returns `true` if the owned pointer is non-null.
	hxattr_nodiscard hxconstexpr bool operator!=(hxnil_t) const;
#endif

#if HX_CPLUSPLUS >= 202302L
	/// Returns the result of calling `callable` with the owned object if
	/// non-null, otherwise returns `hxnil`. Use `and_then` to return `hxptr`,
	/// `hxref` or `hxexpected`.
	/// - `callable` : The function to call with the referenced value.
	template<typename self_t_, typename callable_t_>
	hxattr_nodiscard hxconstexpr auto and_then(this self_t_&& self_, callable_t_&& callable_)
		-> hxremove_cvref_t<decltype(hxforward<callable_t_>(callable_)(hxdeclval<T_&>()))>;
#endif // HX_CPLUSPLUS >= 202302L

	/// Returns a const reference to the stored deleter.
	hxattr_nodiscard hxconstexpr const deleter_t_& deleter(void) const;

	hxattr_nodiscard hxconstexpr deleter_t_& deleter(void);

	/// Replaces the owned object with one constructed from `args` and returns a
	/// reference to it.
	/// - `args` : Arguments forwarded to the object constructor.
	template<hxsystem_allocator_t allocator_=hxsystem_allocator_current,
		hxalignment_t alignment_=hxalignment, typename... args_t_>
	T_& emplace(args_t_&&... args_) noexcept;

	/// Returns `true` if this and `x` point to the same object.
	/// - `x` : The `hxptr` to compare against.
	hxattr_nodiscard hxconstexpr bool equal(const hxptr& x_) const;

	/// Returns the owned pointer without releasing ownership.
	hxattr_nodiscard hxconstexpr T_* get(void) const { return m_ptr_; }

	/// Returns the hash of the owned value if non-null, otherwise `31u`.
	hxattr_nodiscard hxhash_t hash(void) const;

	/// Returns `true` if the owned pointer is non-null.
	hxattr_nodiscard hxconstexpr bool has_value(void) const { return m_ptr_ != hxnull; }

#if HX_CPLUSPLUS >= 202302L
	/// Returns this `hxptr` moved out if non-null, otherwise returns the result
	/// of calling `callable`. `callable` must return an `hxptr`. Rvalue
	/// qualified because ownership is transferred out of this `hxptr`.
	/// - `callable` : The function to call when null.
	template<typename self_t_, typename callable_t_>
	hxattr_nodiscard hxconstexpr hxptr or_else(
		this self_t_&& self_, callable_t_&& callable_)
		requires(hxis_rvalue_reference<self_t_&&>() &&
			!hxis_const<hxremove_reference_t<self_t_>>());
#endif // HX_CPLUSPLUS >= 202302L

	/// Releases ownership and returns the previously owned pointer without
	/// invoking the deleter. The caller takes responsibility for freeing it.
	hxattr_nodiscard hxconstexpr T_* release(void);

	/// Destroys the currently owned object using `deleter_t` if non-null, then
	/// takes ownership of `ptr`.
	/// - `ptr` : The new pointer to own. May be null.
	hxconstexpr void reset(T_* ptr_=hxnull) noexcept;

	/// Exchanges ownership with `x`. Neither pointer is deleted.
	/// - `x` : The `hxptr` to swap with.
	hxconstexpr void swap(hxptr& x_) noexcept;

	/// Returns a reference to the owned object without releasing ownership.
	hxattr_nodiscard hxconstexpr T_& value(void) const;

#if HX_CPLUSPLUS >= 202302L
	/// Returns the owned value if non-null, otherwise returns a `T` constructed
	/// from `args`.
	/// - `args` : The arguments used to construct the value when null.
	template<typename self_t_, typename... args_t_>
	hxattr_nodiscard hxconstexpr hxremove_cv_t<T_> value_or(
		this self_t_&& self_, args_t_&&... args_);
#endif // HX_CPLUSPLUS >= 202302L

private:
	hxptr(const hxptr&) = delete;
	hxptr& operator=(const hxptr&) = delete;

	T_* m_ptr_;
};

/// `hxmake_ptr<T, deleter, allocator, align>(value)` - Allocates and
/// constructs an object of type `T` from `value` and returns it wrapped in an
/// `hxptr`. Will not return on failure.
/// - `deleter` : The deleter type invoked on destruction. Defaults to
///    `hxdefault_delete`.
/// - `allocator` : The memory manager ID to use for allocation. Defaults to
///    `hxsystem_allocator_current`.
/// - `align` : Alignment to use when allocating. Defaults to `hxalignment`.
/// - `value` : The value used to construct `T`.
template<typename T_, typename deleter_t_=hxdefault_delete,
	hxsystem_allocator_t allocator_=hxsystem_allocator_current,
	hxalignment_t align_=hxalignment, typename U_=T_>
hxattr_nodiscard hxptr<T_, deleter_t_> hxmake_ptr(const U_& value_) noexcept {
	return hxptr<T_, deleter_t_>(::new(hxmalloc_ext(sizeof(T_), allocator_, align_)) T_(value_));
}

/// `hxkey_equal_t<hxptr<T>>` - Compares `x` and `y` for equivalence.
template<typename T_, typename deleter_t_>
class hxkey_equal_t<hxptr<T_, deleter_t_> > {
public:
	hxattr_nodiscard hxinline hxattr_flatten bool operator()(
			const hxptr<T_, deleter_t_>& x_, const hxptr<T_, deleter_t_>& y_) const {
		return x_.equal(y_);
	}
};

/// `hxkey_hash_t<hxptr<T>>` - Returns the hash of the owned value if
/// non-null, otherwise `31u`.
template<typename T_, typename deleter_t_>
class hxkey_hash_t<hxptr<T_, deleter_t_> > {
public:
	hxattr_nodiscard hxinline hxattr_flatten
	hxhash_t operator()(const hxptr<T_, deleter_t_>& ptr_) const {
		return ptr_.hash();
	}
};

#include "detail/hxptr.inl"
HX_NS_END_
