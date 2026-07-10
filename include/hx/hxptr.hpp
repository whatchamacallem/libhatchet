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

#include "hxutility.h"

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
	static_assert(!hxis_array<T_>::value, "hxptr does not support array types");
	static_assert(!hxis_reference<T_>::value, "hxptr does not support reference types");
	static_assert(!hxis_pointer<T_>::value, "hxptr does not support pointer types");

	/// `element_t` - Publishes the pointed-to type.
	using element_t = T_;

	/// Constructs an `hxptr` that takes ownership of `ptr` with a specific
	/// deleter instance.
	/// - `ptr` : The pointer to take ownership of. May be null.
	/// - `deleter` : Callable with signature `bool deleter(T*)`.
	hxconstexpr explicit hxptr(T_* ptr_=hxnull, deleter_t_ deleter_=deleter_t_()) noexcept;

	/// Move constructor. Transfers ownership from `other` to this. `other` is
	/// left null.
	/// - `other` : The `hxptr` to transfer ownership from.
	hxconstexpr hxptr(hxptr&& other_) noexcept;

	/// Destroys the owned object using `deleter_t` if non-null.
	hxconstexpr ~hxptr(void) noexcept;

	/// Move assignment. Destroys the currently owned object, then transfers
	/// ownership from `other`. `other` is left null. Self-assignment is not
	/// supported.
	/// - `other` : The `hxptr` to transfer ownership from.
	hxconstexpr hxptr& operator=(hxptr&& other_) noexcept;

	/// Returns a reference to the owned object. The pointer must not be null.
	hxattr_nodiscard hxconstexpr T_& operator*(void) const;

	/// Returns the owned pointer. The pointer must not be null.
	hxattr_nodiscard hxconstexpr T_* operator->(void) const;

	/// Returns `true` if the owned pointer is non-null.
	hxattr_nodiscard hxconstexpr operator bool(void) const;

	/// Returns `true` if this and `other` point to the same object.
	/// - `other` : The `hxptr` to compare against.
	hxattr_nodiscard hxconstexpr bool operator==(const hxptr& other_) const;

	/// Returns `true` if the owned pointer is null.
	hxattr_nodiscard hxconstexpr bool operator==(hxnullptr_t) const;

#if HX_CPLUSPLUS < 202002L // C++20 defaults != from ==.
	/// Returns `true` if this and `other` point to different objects.
	/// - `other` : The `hxptr` to compare against.
	hxattr_nodiscard hxconstexpr bool operator!=(const hxptr& other_) const;

	/// Returns `true` if the owned pointer is non-null.
	hxattr_nodiscard hxconstexpr bool operator!=(hxnullptr_t) const;
#endif

	/// Returns the result of calling `callable` with the owned object if
	/// non-null, otherwise returns a null `hxptr` of the same type. `callable`
	/// must return an `hxptr`.
	/// - `callable` : The function to call with the referenced value.
	template<typename function_t_>
	hxattr_nodiscard hxconstexpr auto and_then(function_t_&& callable_) const
		-> hxremove_cvref_t<decltype(hxforward<function_t_>(callable_)(hxdeclval<T_&>()))>;

	/// Returns a const reference to the stored deleter.
	hxattr_nodiscard hxconstexpr const deleter_t_& deleter(void) const;

	/// Returns a reference to the stored deleter.
	hxattr_nodiscard hxconstexpr deleter_t_& deleter(void);

	/// Returns the owned pointer without releasing ownership.
	hxattr_nodiscard hxconstexpr T_* get(void) const { return m_ptr_; }

	/// Returns this `hxptr` moved out if non-null, otherwise returns the result
	/// of calling `callable`. `callable` must return an `hxptr`. Rvalue
	/// qualified because ownership is transferred out of this `hxptr`.
	/// - `callable` : The function to call when null.
	template<typename function_t_>
	hxattr_nodiscard hxconstexpr hxptr or_else(function_t_&& callable_) &&;

	// An lvalue `hxptr` cannot yield ownership without leaving a dangling copy.
	template<typename function_t_>
	hxptr or_else(function_t_&& callable_) const& = delete;

	/// Releases ownership and returns the previously owned pointer without
	/// invoking the deleter. The caller takes responsibility for freeing it.
	hxattr_nodiscard hxconstexpr T_* release(void);

	/// Destroys the currently owned object using `deleter_t` if non-null, then
	/// takes ownership of `ptr`.
	/// - `ptr` : The new pointer to own. May be null.
	hxconstexpr void reset(T_* ptr_=hxnull) noexcept;

	/// Exchanges ownership with `other`. Neither pointer is deleted.
	/// - `other` : The `hxptr` to swap with.
	hxconstexpr void swap(hxptr& other_) noexcept;

	/// Returns the owned value if non-null, otherwise returns `default_value`
	/// forwarded and converted to `T`.
	/// - `default_value` : The value to return when null.
	template<typename U_=hxremove_cv_t<T_>>
	hxattr_nodiscard hxconstexpr hxremove_cv_t<T_> value_or(U_&& default_value_) const;

private:
	hxptr(const hxptr&) = delete;
	hxptr& operator=(const hxptr&) = delete;

	T_* m_ptr_;
};

/// `hxmake_ptr<T, allocator, align>(args...)` - Allocates and constructs an
/// object of type `T` and returns it wrapped in an `hxptr`. Equivalent to
/// `hxptr<T>(hxnew<T, allocator, align>(args...))`. Will not return on
/// failure.
/// - `allocator` : The memory manager ID to use for allocation. Defaults to
///    `hxsystem_allocator_current`.
/// - `align` : Alignment to use when allocating. Defaults to `hxalignment`.
template<typename T_, hxsystem_allocator_t allocator_=hxsystem_allocator_current,
	hxalignment_t align_=hxalignment, typename... args_t_>
hxattr_nodiscard hxptr<T_> hxmake_ptr(args_t_&&... args_) noexcept {
	return hxptr<T_>(hxnew<T_, allocator_, align_>(hxforward<args_t_>(args_)...));
}

#include "detail/hxptr.inl"
HX_NS_END_
