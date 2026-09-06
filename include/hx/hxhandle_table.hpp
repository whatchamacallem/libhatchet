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

#include "hxallocator.hpp"
#include "hxptr.hpp"

HX_NS_BEGIN_

#include "detail/hxpow2_allocator.hpp"

#if HX_CPLUSPLUS >= 202002L
/// \cond HIDDEN
template<typename T_>
concept hxhandle_table_concept_ = requires(T_& x_) {
	sizeof(T_);
	x_.~T_();
};
/// \endcond
#else
#define hxhandle_table_concept_ typename
#endif

/// `hxhandle_table` - A table that maps 64-bit handles to owned pointers
/// without reallocating memory or moving values. The table is a fixed
/// `2^table_size_bits - 1` array of handles. Use `set_size_bits` to configure
/// the size dynamically.
/// - `T` : The pointed-to type stored in the table.
/// - `deleter_t` : A class type invoked as `deleter(T*)` to free the owned
///    pointer. See also `hxdo_not_delete`.
/// - `table_size_bits` : If non-zero, fixes the table to `2^table_size_bits-1`
///    slots. Otherwise the size is dynamic.
template<hxhandle_table_concept_ T_,
	typename deleter_t_=hxdefault_delete,
	uint32_t table_size_bits_=hxallocator_dynamic_capacity>
class hxhandle_table : private deleter_t_ {
public:
	// The capacity and a guard bit must fit in 32-bits to avoid 64-bit emulation.
	static_assert(table_size_bits_ <= 30u, "hxhandle_table is designed for 2^30 or less slots");

	using value_t = T_;

	/// Constructs an empty table with a capacity of `2^table_size_bits - 1`
	/// slots and an optional deleter instance.
	/// - `deleter` : Callable with signature `bool deleter(T*)`.
	explicit hxhandle_table(deleter_t_ deleter_=deleter_t_());

	/// Destructs the table and deletes all owned values.
	~hxhandle_table(void);

#if HX_CPLUSPLUS >= 202302L
	/// Returns the result of calling `callable` with the value referenced by
	/// `handle`, otherwise returns `hxnil`. Use `and_then` to return `hxptr`,
	/// `hxref` or `hxexpected`.
	/// - `handle` : The handle identifying the value to look up.
	/// - `callable` : The function to call with the value.
	template<typename self_t_, typename callable_t_>
	hxattr_nodiscard auto and_then(
		this self_t_&& self_, hxhandle_t handle_, callable_t_&& callable_)
		-> hxremove_cvref_t<decltype(
			hxforward<callable_t_>(callable_)(
				hxforward_like<self_t_>(hxdeclval<T_&>())))>;
#endif // HX_CPLUSPLUS >= 202302L

	/// Returns the number of slots in the table.
	hxattr_nodiscard hxsize_t capacity(void) const {
		return static_cast<hxsize_t>(m_table_.get_mask_());
	}

	/// Removes all values and if `deleter` is true then calls `deleter()` on
	/// every value.
	/// - `deleter` : Callable with signature `bool deleter(T*)`.
	template<typename deleter_u_>
	void clear(deleter_u_&& deleter_) noexcept;

	/// Removes all values and calls the stored deleter on every value.
	void clear(void) noexcept { this->clear(this->deleter()); }

	/// Returns a const reference to the stored deleter.
	hxattr_nodiscard const deleter_t_& deleter(void) const;

	hxattr_nodiscard deleter_t_& deleter(void);

	/// Checks if the table is empty.
	hxattr_nodiscard bool empty(void) const { return m_size_ == 0; }

	/// Returns the number of values destroyed. Calls `callable` with a `T&`
	/// for every value and calls the stored deleter on those for which it
	/// returns true.
	/// - `callable` : Returns true for values to erase.
	template<typename callable_t_>
	hxsize_t erase_if(callable_t_&& callable_) noexcept;

	/// Returns an `hxptr` owning the value referenced by `handle`, or an empty
	/// `hxptr` if `handle` does not resolve. The slot is freed.
	/// - `handle` : The handle identifying the value to extract.
	hxptr<T_, deleter_t_> extract(hxhandle_t handle_) noexcept;

	/// Checks if the table is full.
	hxattr_nodiscard hxinline hxattr_flatten
	bool full(void) const {
		return m_free_head_ == m_table_.data();
	}

	/// Takes ownership of `ptr` and returns a non-zero handle referencing it,
	/// or `hxnull_handle` if `ptr` is null. The table must not be full.
	/// - `ptr` : The value to insert.
	hxhandle_t insert(T_* ptr_) noexcept;

	/// Takes ownership of the value owned by `ptr` and returns a non-zero
	/// handle referencing it, or `hxnull_handle` if `ptr` is empty. The table
	/// must not be full.
	/// - `ptr` : The `hxptr` owning the value to insert.
	template<typename deleter_u_>
	hxhandle_t insert(hxptr<T_, deleter_u_>&& ptr_) noexcept;

	/// Returns the maximum number of values that can be stored.
	hxattr_nodiscard hxinline hxattr_flatten
	hxsize_t max_size(void) const {
		return static_cast<hxsize_t>(m_table_.get_mask_());
	}

#if HX_CPLUSPLUS >= 202302L
	/// Returns a pointer to the value referenced by `handle`, or the result
	/// of calling `callable` when the handle does not resolve.
	/// - `handle` : The handle identifying the value to look up.
	/// - `callable` : The function to call when lookup fails. Must return a
	///   `T` pointer.
	template<typename self_t_, typename callable_t_>
	hxattr_nodiscard auto or_else(
		this self_t_&& self_, hxhandle_t handle_, callable_t_&& callable_)
		-> decltype(self_.value(handle_));
#endif // HX_CPLUSPLUS >= 202302L

	/// Clears the table without deleting any values.
	void release_all(void) noexcept { this->clear(hxdo_not_delete()); }

	/// Releases the value referenced by `handle` if `deleter` is true then
	/// calls `deleter` on it.
	/// - `handle` : The handle identifying the value to remove.
	/// - `deleter` : Callable with signature `bool deleter(T*)`.
	template<typename deleter_u_>
	bool reset(hxhandle_t handle_, deleter_u_&& deleter_) noexcept;

	/// Removes and calls the stored deleter on the value referenced by
	/// `handle`.
	/// - `handle` : The handle identifying the value to remove.
	bool reset(hxhandle_t handle_) noexcept { return this->reset(handle_, this->deleter()); }

	/// Sets the number of index bits and allocates memory for the table (only
	/// for dynamic capacity).
	/// - `bits` : The number of index bits to set for the table, in the range
	///   [1, 31].
	void set_size_bits(uint32_t bits_);

	/// Returns the number of values in the table.
	hxattr_nodiscard hxsize_t size(void) const { return static_cast<hxsize_t>(m_size_); }

	/// Returns the value referenced by `handle` if it resolves, otherwise
	/// hxnull.
	/// - `handle` : The handle identifying the value to look up.
	hxattr_nodiscard const T_* value(hxhandle_t handle_) const noexcept;

	hxattr_nodiscard T_* value(hxhandle_t handle_) noexcept;

#if HX_CPLUSPLUS >= 202302L
	/// Returns a copy of the value referenced by `handle`, or a value
	/// constructed from `args` when the handle does not resolve.
	/// - `handle` : The handle identifying the value to look up.
	/// - `args` : The arguments used to construct the value when lookup fails.
	template<typename self_t_, typename... args_t_>
	hxattr_nodiscard T_ value_or(
		this self_t_&& self_, hxhandle_t handle_, args_t_&&... args_);
#endif // HX_CPLUSPLUS >= 202302L

private:
	class slot_t_ {
	public:
		uint64_t m_key_;
		union {
			T_* m_ptr_;
			slot_t_* m_next_;
		};
	};

	void build_free_list_(void);

	hxdetail_::hxpow2_allocator_<slot_t_, table_size_bits_, false> m_table_;
	uint32_t m_size_;
	slot_t_* m_free_head_;
};

#include "detail/hxhandle_table.inl"
HX_NS_END_
