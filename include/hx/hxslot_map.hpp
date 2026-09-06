#pragma once
// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

/// \file
/// A fixed-size table mapping 64-bit generational handles to values stored in
/// a contiguous array. Also known by some as a "slot map".

#include "libhatchet.h"

// HX_USE_MACROS_WITH_MODULE allows including macros alongside the module.
#if HX_USE_MACROS_WITH_MODULE
#error Header does not provide macros alone.
#endif

#include "hxallocator.hpp"
#include "hxutility.h"

HX_NS_BEGIN_

/// \cond HIDDEN
// Makes the mask a compile time constant when non-0.
template<hxsize_t mask_>
class hxslot_map_mask_ {
public:
	hxinline_constexpr uint32_t get_mask_(void) const { return static_cast<uint32_t>(mask_); }
	hxinline void set_mask_(uint32_t) const { } // GCOVR_EXCL_LINE. Unreachable code below c++17.
};

template<>
class hxslot_map_mask_<hxallocator_dynamic_capacity> {
public:
	hxinline hxattr_nodiscard uint32_t get_mask_(void) const { return m_mask_; }
	hxinline void set_mask_(uint32_t mask_) { m_mask_ = mask_; }
private:
	uint32_t m_mask_;
};
/// \endcond

#if HX_CPLUSPLUS >= 202002L
/// \cond HIDDEN
template<typename T_>
concept hxslot_map_concept_ = requires(T_& x_) {
	sizeof(T_);
	x_.~T_();
};
/// \endcond
#else
#define hxslot_map_concept_ typename
#endif

/// `hxslot_map` - A table that maps 64-bit generational handles to values
/// kept in a contiguous array of size `2^table_size_bits - 1` without
/// reallocating memory. `data()` always references `size()` contiguous values.
/// Pointers into `data` are invalidated by `reset` while handles are not.
/// Handle 0 is never valid. If non-zero, the template parameter
/// `table_size_bits` configures the capacity to `2^table_size_bits - 1` values.
/// Otherwise use `set_size_bits` to configure the size dynamically.
/// - `T` : The value type stored in the map.
/// - `table_size_bits` : If non-zero, sets the capacity to
///   `2^table_size_bits - 1` values. Otherwise the size is dynamic.
template<hxslot_map_concept_ T_, uint32_t table_size_bits_=hxallocator_dynamic_capacity>
class hxslot_map {
public:
	static_assert(table_size_bits_ < 31u, "Slot index must fit in the low 31-bits of the handle");

	/// `iterator` - Random access iterator.
	using iterator = T_*;

	/// `const_iterator` - Const random access iterator.
	using const_iterator = const T_*;

	using value_t = T_;

	/// Constructs an empty map with a capacity of `2^table_size_bits - 1`.
	hxslot_map(void);

	/// Destructs the map and destroys all values.
	~hxslot_map();

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

	/// Returns a `const T*` to the beginning of the values (alias for `data`).
	/// Iterators are invalidated by modification of the table.
	hxinline hxattr_flatten const T_* begin(void) const { return this->data(); }

	hxinline hxattr_flatten T_* begin(void) { return this->data(); }

	/// Returns the number of values that can be stored.
	hxattr_nodiscard hxsize_t capacity(void) const;

	/// Returns a `const T*` to the beginning of the values (alias for `begin`).
	/// Iterators are invalidated by modification of the table.
	hxattr_nodiscard hxinline hxattr_flatten
	const T_* cbegin(void) const { return this->data(); }

	/// Returns a `const T*` to the end of the values. Iterators are invalidated
	/// by modification of the table.
	hxattr_nodiscard const T_* cend(void) const;

	/// Destroys all values and invalidates their handles.
	void clear(void) noexcept;

	/// Returns a const pointer to the first of `size` contiguous values.
	hxattr_nodiscard hxinline hxattr_flatten
	const T_* data(void) const { return m_values_.data(); }

	hxattr_nodiscard hxinline hxattr_flatten
	T_* data(void) { return m_values_.data(); }

	/// Returns a non-zero handle referencing a value constructed at the end of
	/// `data` from `args`. The map must not be full.
	/// - `args` : Arguments forwarded to `T`'s constructor.
	template<typename... args_t_>
	hxhandle_t emplace(args_t_&&... args_) noexcept;

	/// Checks if the map is empty.
	hxinline hxattr_nodiscard bool empty(void) const { return m_size_ == 0u; }

	/// Returns a `const T*` to the end of the values. Iterators are invalidated
	/// by modification of the table.
	hxattr_nodiscard const T_* end(void) const;

	hxattr_nodiscard hxinline hxattr_flatten
	T_* end(void) { return this->data() + this->size(); }

	/// Returns the number of values destroyed. Calls `callable` with a `T&`
	/// for every value and destroys those for which it returns true.
	/// - `callable` : Returns true for values to erase.
	template<typename callable_t_>
	hxsize_t erase_if(callable_t_&& callable_) noexcept;

	/// Checks if the map is full.
	hxattr_nodiscard bool full(void) const;

	/// Returns the handle for the value at `begin() + index`.
	/// - `index` : The offset of the value in `[0, size())` to get a handle
	///   for.
	hxattr_nodiscard hxattr_flatten hxhandle_t handle_at(hxsize_t index_) const noexcept;

	/// Returns a non-zero handle referencing a value constructed at the end of
	/// `data` from `args`. The map must not be full.
	/// - `args` : Arguments forwarded to `T`'s constructor.
	template<typename... args_t_>
	hxhandle_t insert(args_t_&&... args_) noexcept;

	/// Returns the maximum number of values that can be stored.
	hxinline hxattr_nodiscard hxsize_t max_size(void) const { return this->capacity(); }

#if HX_CPLUSPLUS >= 202302L
	/// Returns a pointer to the value referenced by `handle`, or the
	/// result of calling `callable` when the handle does not resolve.
	/// - `handle` : The handle identifying the value to look up.
	/// - `callable` : The function to call when lookup fails. Must return an
	///   iterator.
	template<typename self_t_, typename callable_t_>
	hxattr_nodiscard auto or_else(
		this self_t_&& self_, hxhandle_t handle_, callable_t_&& callable_)
		-> decltype(self_.end());
#endif // HX_CPLUSPLUS >= 202302L

	/// Returns true if `handle` resolved and its value was destroyed. The last
	/// value is move-assigned into the hole to keep the values contiguous.
	/// - `handle` : The handle identifying the value to remove.
	bool reset(hxhandle_t handle_) noexcept;

	/// Sets the number of index bits and allocates memory for the map (only for
	/// dynamic capacity).
	/// - `bits` : The number of index bits to set for the map, in the range
	///   [1, 30].
	void set_size_bits(uint32_t bits_);

	/// Returns the number of values in the map.
	hxinline hxattr_nodiscard hxsize_t size(void) const { return static_cast<hxsize_t>(m_size_); }

	/// Returns a pointer to the value referenced by `handle` if it resolves,
	/// otherwise `end()`.
	/// - `handle` : The handle identifying the value to look up.
	hxattr_nodiscard T_* value(hxhandle_t handle_) noexcept;

	hxattr_nodiscard const T_* value(hxhandle_t handle_) const noexcept;

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
	// See detail/hxslot_map.inl for the design.
	static hxinline_constexpr hxsize_t s_capacity_ =
		table_size_bits_ == hxallocator_dynamic_capacity
			? hxallocator_dynamic_capacity
			: static_cast<hxsize_t>(1) << table_size_bits_;

	static hxinline_constexpr hxsize_t s_value_capacity_ =
		table_size_bits_ == hxallocator_dynamic_capacity
			? hxallocator_dynamic_capacity
			: s_capacity_ - 1;

	class slot_t_ {
	public:
		uint64_t m_handle_;
		uint32_t m_index_;
		uint32_t m_backref_;
	};

	void build_free_list_(void);

	uint32_t m_size_;
	hxslot_map_mask_<s_value_capacity_> m_mask_;
	hxallocator<slot_t_, s_capacity_> m_slots_;
	hxallocator<T_, s_value_capacity_> m_values_;
};

#include "detail/hxslot_map.inl"
HX_NS_END_
