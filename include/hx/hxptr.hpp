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
/// - `T` : The pointed-to type.
/// - `deleter_t` : A callable that frees the owned pointer. Defaults to
///    `hxdefault_delete`. Use `hxconsteval_delete` for `consteval` work.
template<typename T_, typename deleter_t_=hxdefault_delete>
class hxptr {
public:
	/// `element_t` - Publishes the pointed-to type.
	using element_t = T_;

	/// Constructs an `hxptr` that takes ownership of `ptr` with a specific
	/// deleter instance.
	/// - `ptr` : The pointer to take ownership of. May be null.
	/// - `deleter` : The deleter instance to use when freeing `ptr`.
	hxconstexpr hxptr(T_* ptr_=hxnull, deleter_t_ deleter_=deleter_t_()) noexcept;

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

#if HX_CPLUSPLUS < 202002L // C++20 defaults != from ==.
	/// Returns `true` if this and `other` point to different objects.
	/// - `other` : The `hxptr` to compare against.
	hxattr_nodiscard hxconstexpr bool operator!=(const hxptr& other_) const;
#endif

	/// Returns `true` if the owned pointer is null.
	hxattr_nodiscard hxconstexpr bool operator==(hxnullptr_t) const;

#if HX_CPLUSPLUS < 202002L // C++20 defaults != from ==.
	/// Returns `true` if the owned pointer is non-null.
	hxattr_nodiscard hxconstexpr bool operator!=(hxnullptr_t) const;
#endif

	/// Returns the owned pointer without releasing ownership.
	hxattr_nodiscard hxconstexpr T_* get(void) const { return m_ptr_; }

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

private:
	hxptr(const hxptr&) = delete;
	hxptr& operator=(const hxptr&) = delete;

	T_* m_ptr_;
	deleter_t_ m_deleter_;
};

/// `hxptr<T[], deleter_t>` - Partial specialization of `hxptr` for array types.
/// Use is not allowed as delete[] is not required to exist.
template<typename T_, typename deleter_t_>
class hxptr<T_[], deleter_t_> {
public:
	static_assert(sizeof(T_) == 0, "hxptr does not support array types");
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
hxattr_nodiscard hxptr<T_> hxmake_ptr(args_t_&&... args_) {
	return hxptr<T_>(::new(hxmalloc_ext(sizeof(T_), allocator_, align_)) T_(static_cast<args_t_&&>(args_)...));
}

#include "detail/hxptr.inl"
HX_NS_END_
