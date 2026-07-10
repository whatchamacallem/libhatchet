#pragma once
// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

/// \file
/// A fixed-size table mapping 64-bit generational handles to pointers.

#include "libhatchet.h"

// HX_USE_MACROS_WITH_MODULE allows including macros alongside the module.
#if HX_USE_MACROS_WITH_MODULE
#error Header does not provide macros alone.
#endif

#include "hxptr.hpp"
#include "hxutility.h"
#include "hxallocator.hpp"

HX_NS_BEGIN_

#include "detail/hxpow2_table_allocator.hpp"

/// `hxhandle_t` - An opaque 64-bit handle. Low bits are the index.
using hxhandle_t = uint64_t;

/// `hxnull_handle` - A handle that does not refer to a valid object.
hxinline_constexpr hxhandle_t hxnull_handle = 0u;

/// `hxhandle_table` - A table that maps 64-bit handles to owned pointers without
/// reallocating memory or moving values. The table is a fixed power-of-two
/// array of `{ key, ptr }` slots. If non-zero, `table_size_bits` configures the
/// table size to `2^table_size_bits`. Otherwise use `set_table_size_bits` to
/// configure the size dynamically. WARNING: `deleter` must accept `hxnull`.
/// - `T` : The pointed-to type stored in the table.
/// - `deleter_t` : A class type invoked as `deleter(T*)` to free the owned
///    pointer. See also `hxdo_not_delete`.
/// - `table_size_bits` : If non-zero, fixes the table to `2^table_size_bits`
///   slots. Otherwise the size is dynamic.
template<typename T_,
	typename deleter_t_=hxdefault_delete,
	uint32_t table_size_bits_=hxallocator_dynamic_capacity>
class hxhandle_table : private deleter_t_ {
public:
	// The capacity and a guard bit must fit in 32-bits to avoid 64-bit emulation.
	static_assert(table_size_bits_ <= 30u, "hxhandle_table is designed for 2^30 or less slots");

	using value_t = T_;

	/// Constructs an empty table with a capacity of `2^table_size_bits` and an
	/// optional deleter instance.
	/// - `deleter` : Callable with signature `bool deleter(T*)`.
	explicit hxhandle_table(deleter_t_ deleter_=deleter_t_());

	/// Destructs the table and deletes all owned values.
	~hxhandle_table(void);

	/// Returns the number of slots in the table.
	hxattr_nodiscard hxsize_t capacity(void) const { return m_table_.capacity(); }

	/// Removes all values and if `deleter` is true then calls `deleter()` on
	/// every value.
	/// - `deleter` : Callable with signature `bool deleter(T*)`.
	template<typename deleter_u_>
	void clear(deleter_u_&& deleter_) noexcept;

	/// Removes all values and calls the stored deleter on every value.
	void clear(void) noexcept { this->clear(this->deleter()); }

	/// Checks if the table is empty.
	hxattr_nodiscard bool empty(void) const { return m_size_ == 0; }

	/// Releases the value referenced by `handle` if `deleter` is true then
	/// calls `deleter` on it. - `handle` : The handle identifying the value to
	/// remove. - `deleter` : Callable with signature `bool deleter(T*)`.
	template<typename deleter_u_>
	bool erase(hxhandle_t handle_, deleter_u_&& deleter_) noexcept;

	/// Removes and calls the stored deleter on the value referenced by
	/// `handle`.
	/// - `handle` : The handle identifying the value to remove.
	bool erase(hxhandle_t handle_) noexcept { return this->erase(handle_, this->deleter()); }

	/// `extract` - Returns an `hxptr` owning the value referenced by `handle`,
	/// or an empty `hxptr` if `handle` does not resolve. The slot is freed.
	/// - `handle` : The handle identifying the value to extract.
	hxptr<T_, deleter_t_> extract(hxhandle_t handle_) noexcept;

	/// Returns the value referenced by `handle` if it resolves, otherwise hxnull.
	/// - `handle` : The handle identifying the value to look up.
	hxattr_nodiscard hxattr_hot T_* get(hxhandle_t handle_) noexcept;

	/// `const` version of `get`.
	/// - `handle` : The handle identifying the value to look up.
	hxattr_nodiscard hxattr_hot const T_* get(hxhandle_t handle_) const noexcept;

	/// Returns a const reference to the stored deleter.
	hxattr_nodiscard const deleter_t_& deleter(void) const;

	/// Returns a reference to the stored deleter.
	hxattr_nodiscard deleter_t_& deleter(void);

	/// `insert` - Takes ownership of `ptr` and returns a non-zero handle
	/// referencing it. The table must not be full and `ptr` must not be null.
	/// - `ptr` : The value to insert.
	hxhandle_t insert(T_* ptr_) noexcept;

	/// `insert` - Takes ownership of the value owned by `ptr` and returns a
	/// non-zero handle referencing it. The table must not be full and `ptr`
	/// must not be null.
	/// - `ptr` : The `hxptr` owning the value to insert.
	template<typename deleter_u_>
	hxhandle_t insert(hxptr<T_, deleter_u_>&& ptr_) noexcept;

	/// Clears the table without deleting any values.
	void release_all(void) noexcept { this->clear(hxdo_not_delete()); }

	/// `replace` - Stores `ptr` in the slot referenced by `handle` and returns
	/// an `hxptr` owning the previously stored pointer, or an empty `hxptr` if
	/// `handle` does not resolve. Does not invoke the deleter.
	/// - `handle` : The handle identifying the slot to replace.
	/// - `ptr` : The new value to store. Must not be null.
	hxptr<T_, deleter_t_> replace(hxhandle_t handle_, T_* ptr_) noexcept;

	/// Sets the number of index bits and allocates memory for the table (only for
	/// dynamic capacity).
	/// - `bits` : The number of index bits to set for the table.
	void set_table_size_bits(uint32_t bits_);

	/// Returns the number of values in the table.
	hxattr_nodiscard hxsize_t size(void) const { return static_cast<hxsize_t>(m_size_); }

private:
	class hxhandle_slot_ {
	public:
		uint64_t key_;
		union {
			T_* ptr_;
			hxhandle_slot_* next_;
		};
	};

	// Deleted for being bug prone.
	hxhandle_table(const hxhandle_table&) = delete;
	hxhandle_table& operator=(const hxhandle_table&) = delete;

	void build_free_list_(void);

	uint32_t m_size_;
	hxhandle_slot_* m_free_head_;
	hxdetail_::hxpow2_table_allocator_<hxhandle_slot_, table_size_bits_, false> m_table_;
};

#include "detail/hxhandle_table.inl"
HX_NS_END_
