#pragma once
// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

/// \file
/// Similar to `std::allocator`. Supports static or dynamic allocation.

#include "libhatchet.h"

// HX_USE_MACROS_WITH_MODULE allows including macros alongside the module.
#if HX_USE_MACROS_WITH_MODULE
#error Header does not provide macros alone.
#endif

HX_NS_BEGIN_

/// `hxallocator_dynamic_capacity` - A capacity value that allows for dynamic
/// allocation.
hxinline_constexpr hxsize_t hxallocator_dynamic_capacity = 0;

/// `hxallocator<1+>` - Provides static allocation when capacity is greater than
/// zero.
template<typename T_, hxsize_t fixed_capacity_>
class hxallocator {
public:
	/// `value_t` - Publishes the value type.
	using value_t = T_;

	// Template specialization below should have been selected.
	static_assert(fixed_capacity_ > 0, "Fixed capacity must be > 0.");

	/// Initializes memory to `0xab` when `HX_HARDENING_MODE ==
	/// HX_HARDENING_MODE_DEBUG`.
	hxallocator(void) {
#if (HX_HARDENING_MODE) == HX_HARDENING_MODE_DEBUG
		::memset(m_data_, 0xab, sizeof m_data_);
#endif
	}

	/// Returns the number of elements of `T` allocated.
	hxattr_nodiscard hxsize_t capacity(void) const { return fixed_capacity_; }

	/// Returns a pointer to a const and potentially uninitialized array of `T`.
	const T_* data(void) const { return reinterpret_cast<const T_*>(m_data_); }

	/// Returns a pointer to a potentially uninitialized array of `T`.
	T_* data(void) { return reinterpret_cast<T_*>(m_data_); }

	/// Used to ensure initial capacity as `reserve_storage` will not reallocate.
	/// Provided for interface compatibility with the dynamic allocator.
	/// - `size` : The number of elements of type `T` to ensure are available.
	/// - `allocator` : Ignored.
	/// - `alignment` : Ignored.
	void reserve_storage(hxsize_t size_,
			hxsystem_allocator_t allocator_=hxsystem_allocator_current,
			hxalignment_t alignment_=hxalignment) {
		(void)size_; (void)allocator_; (void)alignment_;
		hxassertmsg(static_cast<size_t>(size_) <= static_cast<size_t>(fixed_capacity_),
			"overflowing_fixed_capacity Buffer overflow.");
	}

private:
	hxallocator(const hxallocator&) = delete;
	void operator=(const hxallocator&) = delete;

	alignas(T_) char m_data_[fixed_capacity_ * hxsizeof<T_>()];
};

/// `hxallocator<0>` - Capacity is set by first call to `reserve_storage` and may
/// not be extended. May be moved using `hxswap_memcpy`.
template<typename T_>
class hxallocator<T_, hxallocator_dynamic_capacity> {
public:
	/// `value_t` - Publishes the value type.
	using value_t = T_;

	/// Does not allocate until `reserve_storage` is called.
	hxallocator(void) {
		m_data_ = hxnull;
		m_capacity_ = 0;
	}

	/// Calls `hxfree` with any allocated memory.
	~hxallocator(void) {
		if(m_data_) {
			m_capacity_ = 0;
			hxfree(m_data_);
			m_data_ = hxnull;
		}
	}

	/// Returns the number of elements of `T` allocated.
	hxattr_nodiscard hxsize_t capacity(void) const { return m_capacity_; }

	/// Returns a pointer to a const and potentially uninitialized array of `T`.
	const T_* data(void) const { return m_data_; }

	/// Returns a pointer to a potentially uninitialized array of `T`.
	T_* data(void) { return m_data_; }

	/// Capacity is set by first call to `reserve_storage` and may not be extended.
	/// - `size` : The number of elements of type `T` to allocate space for.
	/// - `allocator` : The memory manager ID to use for allocation (default: `hxsystem_allocator_current`)
	/// - `alignment` : The alignment to use for the allocation. (default: `hxalignment`)
	void reserve_storage(hxsize_t size_,
			hxsystem_allocator_t allocator_=hxsystem_allocator_current,
			hxalignment_t alignment_=hxalignment) {
		if(size_ <= m_capacity_) { return; }
		hxassert_always(m_capacity_ == 0, "reallocation_disallowed");
		m_data_ = static_cast<T_*>(hxmalloc_ext(sizeof(T_) * static_cast<size_t>(size_), allocator_, alignment_));
		m_capacity_ = size_;
	}

private:
	hxallocator(const hxallocator&) = delete;
	void operator=(const hxallocator&) = delete;

	hxsize_t m_capacity_;
	T_* m_data_;
};

HX_NS_END_
