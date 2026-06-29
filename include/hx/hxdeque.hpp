#pragma once
// SPDX-FileCopyrightText: © 2025 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

/// \file
/// A fixed-capacity deque backed by a power-of-two ring buffer

#include "libhatchet.h"

// HX_USE_MODULE allows including macros in addition to the hx module.
#if HX_USE_MODULE
#error Header does not provide macros only.
#endif

#include "hxallocator.hpp"
#include "hxutility.h"

HX_NS_BEGIN_

/// `hxdeque` - A fixed-capacity deque backed by a power-of-two ring buffer. All
/// operations are `Θ(1)`. The capacity must be a power of two and greater than
/// zero. This is designed to do one thing very well and that is all.
/// - `T` : The element type stored in the deque.
/// - `capacity` : Maximum element count or `hxallocator_dynamic_capacity` for dynamic storage.
template<typename T_, hxsize_t capacity_=hxallocator_dynamic_capacity>
class hxdeque : private hxallocator<T_, capacity_> {
public:
	/// Constructs an empty hxdeque. When using static storage the capacity is
	/// fixed at compile time.
	/// - `dynamic_capacity` : Element capacity for dynamic storage.
	explicit hxdeque(hxsize_t dynamic_capacity_=0);

	/// Destroys all elements in the deque.
	~hxdeque(void);

	/// Returns a reference to the element at logical index `index`.
	/// - `index` : Zero-based index from the front.
	hxattr_nodiscard T_& operator[](hxsize_t index_);

	/// Returns a const reference to the element at logical index `index`.
	/// - `index` : Zero-based index from the front.
	hxattr_nodiscard const T_& operator[](hxsize_t index_) const;

	/// Returns a reference to the element at logical index `index`.
	/// - `index` : Zero-based index from the front.
	hxattr_nodiscard T_& at(hxsize_t index_);

	/// Returns a const reference to the element at logical index `index`.
	/// - `index` : Zero-based index from the front.
	hxattr_nodiscard const T_& at(hxsize_t index_) const;

	/// Returns a reference to the back element.
	hxattr_nodiscard T_& back(void);

	/// Returns a const reference to the back element.
	hxattr_nodiscard const T_& back(void) const;

	/// Returns the capacity of the deque or 0 if unallocated.
	hxattr_nodiscard hxsize_t capacity(void) const;

	/// Destroys all elements and resets the deque to empty without deallocating.
	void clear(void) noexcept;

	/// Returns a pointer to a const and potentially uninitialized array of `T`.
	const T_* data(void) const { return hxallocator<T_, capacity_>::data(); }

	/// Returns a pointer to a potentially uninitialized array of `T`.
	T_* data(void) { return hxallocator<T_, capacity_>::data(); }

	/// Constructs an element in place at the back using forwarded arguments.
	/// Exactly the same as `push_back`.
	/// - `args` : Arguments forwarded to `T`'s constructor.
	template<typename... args_t_>
	void emplace_back(args_t_&&... args_) noexcept;

	/// Constructs an element in place at the front using forwarded arguments.
	/// Exactly the same as `push_front`.
	/// - `args` : Arguments forwarded to `T`'s constructor.
	template<typename... args_t_>
	void emplace_front(args_t_&&... args_) noexcept;

	/// Returns `true` if the deque contains no elements.
	hxattr_nodiscard bool empty(void) const;

	/// Returns a reference to the front element.
	hxattr_nodiscard T_& front(void);

	/// Returns a const reference to the front element.
	hxattr_nodiscard const T_& front(void) const;

	/// Returns `true` if the deque is at capacity.
	hxattr_nodiscard bool full(void) const;

	/// Returns the capacity of the deque.
	hxattr_nodiscard hxsize_t max_size(void) const { return this->capacity(); }

	/// Removes and destroys the back element.
	void pop_back(void) noexcept;

	/// Removes and destroys the front element.
	void pop_front(void) noexcept;

	/// Appends an element at the back using forwarded arguments. Exactly the
	/// same as `emplace_back`.
	/// - `args` : Arguments forwarded to `T`'s constructor.
	template<typename... args_t_>
	void push_back(args_t_&&... args_) noexcept;

	/// Prepends an element at the front using forwarded arguments. Exactly the
	/// same as `emplace_front`.
	/// - `args` : Arguments forwarded to `T`'s constructor.
	template<typename... args_t_>
	void push_front(args_t_&&... args_) noexcept;

	/// Allocates storage for a dynamic deque. May only be called once and only
	/// when the deque has no storage.
	/// - `dynamic_capacity` : Element capacity to allocate.
	void reserve(hxsize_t dynamic_capacity_);

	/// Returns the number of elements currently in the deque.
	hxattr_nodiscard hxsize_t size(void) const;

private:
	// Hide access to the raw data. This is raw underlying data and would not be
	// what was expected.
	using hxallocator<T_, capacity_>::data;

	hxdeque(const hxdeque&) = delete;
	void operator=(const hxdeque&) = delete;

	hxsize_t m_mask_;
	hxsize_t m_head_;
	hxsize_t m_tail_;
	hxsize_t m_count_;
};

#include "detail/hxdeque.inl"
HX_NS_END_
