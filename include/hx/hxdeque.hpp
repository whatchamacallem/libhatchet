#pragma once
// SPDX-FileCopyrightText: © 2025 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

/// \file
/// A fixed-capacity deque backed by a power-of-two ring buffer

#include "libhatchet.h"

// HX_USE_MACROS_WITH_MODULE allows including macros alongside the module.
#if HX_USE_MACROS_WITH_MODULE
#error Header does not provide macros alone.
#endif

#include "hxallocator.hpp"
#include "hxalgorithm.hpp"
#include "hxutility.h"

HX_NS_BEGIN_

#if HX_CPLUSPLUS >= 202002L
/// \cond HIDDEN
template<typename T_>
concept hxdeque_concept_ = requires(T_& x_) {
	sizeof(T_);
	x_.~T_();
};
/// \endcond
#else
#define hxdeque_concept_ typename
#endif

/// `hxdeque` - A fixed-capacity deque backed by a power-of-two ring buffer. All
/// operations are `O(1)`. The effective capacity must be a power of two and
/// positive once storage is allocated. This is designed to do one thing very
/// well and that is all.
/// - `T` : The element type stored in the deque.
/// - `capacity` : Maximum element count or `hxallocator_dynamic_capacity` for
///   dynamic storage.
template<hxdeque_concept_ T_, hxsize_t capacity_=hxallocator_dynamic_capacity>
class hxdeque : private hxallocator<T_, capacity_> {
public:
	/// `const_iterator` - A const random-access iterator over logical indices.
	/// Iterators are invalidated by any operation that changes the deque.
	class const_iterator {
	public:
		/// Constructs an end iterator.
		const_iterator(void) : m_deque_(hxnull), m_index_(0) { }

		/// Returns a const reference to the element at the current position.
		const T_& operator*(void) const { return (*m_deque_)[m_index_]; }

		/// Returns an iterator advanced by `n` positions.
		/// - `n` : The number of positions to advance.
		const_iterator operator+(hxsize_t n_) const { return const_iterator(m_deque_, m_index_ + n_); }

		/// Returns an iterator retreated by `n` positions.
		/// - `n` : The number of positions to retreat.
		const_iterator operator-(hxsize_t n_) const { return const_iterator(m_deque_, m_index_ - n_); }

		/// Returns the signed distance between two iterators.
		/// - `x` : The iterator to subtract.
		hxsize_t operator-(const const_iterator& x_) const { return m_index_ - x_.m_index_; }

		/// Returns a const reference to the element `n` positions ahead of this
		/// one.
		/// - `n` : The offset from this position.
		const T_& operator[](hxsize_t n_) const { return *operator+(n_); }

		/// Advances the iterator to the next element.
		const_iterator& operator++(void) { ++m_index_; return *this; }

		/// Advances the iterator to the next element (post-increment).
		const_iterator operator++(int) { const_iterator t_(*this); ++m_index_; return t_; }

		/// Retreats the iterator to the previous element.
		const_iterator& operator--(void) { --m_index_; return *this; }

		/// Retreats the iterator to the previous element (post-decrement).
		const_iterator operator--(int) { const_iterator t_(*this); operator--(); return t_; }

		/// Compares two iterators for equality.
		/// - `x` : The iterator to compare against.
		bool operator==(const const_iterator& x_) const { return m_index_ == x_.m_index_; }

		/// Returns true when this iterator is ordered before `x`.
		/// - `x` : The iterator to compare against.
		bool operator<(const const_iterator& x_) const { return m_index_ < x_.m_index_; }

#if HX_CPLUSPLUS < 202002L
		/// Compares two iterators for inequality.
		/// - `x` : The iterator to compare against.
		bool operator!=(const const_iterator& x_) const { return m_index_ != x_.m_index_; }
#endif

	protected:
		/// \cond HIDDEN
		friend class hxdeque;
		const_iterator(const hxdeque* deque_, hxsize_t index_)
			: m_deque_(deque_), m_index_(index_) { }
		const hxdeque* m_deque_;
		hxsize_t m_index_;
		/// \endcond
	};

	/// `iterator` - A mutable random-access iterator over logical indices.
	class iterator : public const_iterator {
	public:
		/// Constructs an end iterator.
		iterator(void) { }

		/// Returns a reference to the element at the current position.
		T_& operator*(void) const { return (*const_cast<hxdeque*>(this->m_deque_))[this->m_index_]; }

		/// Returns an iterator advanced by `n` positions.
		/// - `n` : The number of positions to advance.
		iterator operator+(hxsize_t n_) const { return iterator(const_cast<hxdeque*>(this->m_deque_), this->m_index_ + n_); }

		/// Returns an iterator retreated by `n` positions.
		/// - `n` : The number of positions to retreat.
		iterator operator-(hxsize_t n_) const { return iterator(const_cast<hxdeque*>(this->m_deque_), this->m_index_ - n_); }

		/// Returns the signed distance from `x` to this position.
		/// - `x` : The iterator to subtract.
		hxsize_t operator-(const const_iterator& x_) const { return const_iterator::operator-(x_); }

		/// Returns a reference to the element `n` positions ahead of this one.
		/// - `n` : The offset from this position.
		T_& operator[](hxsize_t n_) const { return *operator+(n_); }

		/// Advances the iterator to the next element.
		iterator& operator++(void) { const_iterator::operator++(); return *this; }

		/// Advances the iterator to the next element (post-increment).
		iterator operator++(int) { iterator t_(*this); const_iterator::operator++(); return t_; }

		/// Retreats the iterator to the previous element.
		iterator& operator--(void) { const_iterator::operator--(); return *this; }

		/// Retreats the iterator to the previous element (post-decrement).
		iterator operator--(int) { iterator t_(*this); const_iterator::operator--(); return t_; }

	protected:
		/// \cond HIDDEN
		friend class hxdeque;
		iterator(hxdeque* deque_, hxsize_t index_) : const_iterator(deque_, index_) { }
		iterator(const const_iterator& x_) : const_iterator(x_) { }
		/// \endcond
	};

	/// Constructs an empty hxdeque. When using static storage the capacity is
	/// fixed at compile time and `dynamic_capacity` must be 0.
	/// - `dynamic_capacity` : Element capacity for dynamic storage.
	explicit hxdeque(hxsize_t dynamic_capacity_=0);

	/// Destroys all elements in the deque.
	~hxdeque(void);

	/// Returns a const reference to the element at logical index `index`.
	/// - `index` : Zero-based index from the front.
	hxattr_nodiscard const T_& operator[](hxsize_t index_) const;

	hxattr_nodiscard T_& operator[](hxsize_t index_);

	/// Returns `true` if this deque and `x` contain the same elements in the
	/// same order, using `hxkey_equal`.
	/// - `x` : The deque to compare against.
	hxattr_nodiscard bool operator==(const hxdeque& x_) const;

	/// Returns `true` if this deque compares less than `x` lexicographically,
	/// using `hxkey_equal` and `hxkey_less`.
	/// - `x` : The deque to compare against.
	hxattr_nodiscard bool operator<(const hxdeque& x_) const;

	/// Returns a const iterator pointing to the first element.
	const_iterator begin(void) const { return const_iterator(this, 0); }

	iterator begin(void) { return iterator(this, 0); }

	/// Appends elements from a range referenced by an lvalue, copying each
	/// element with `push_back`.
	/// - `range` : The range to copy elements from.
	template<hxrange_concept_ range_t_>
	void add_range(range_t_& range_) noexcept;

	/// Appends elements from a temporary range, moving each element with
	/// `push_back`. This overload enables moving the range elements into the
	/// deque when forwarding rvalues.
	/// - `range` : The range to move elements from.
	template<hxrange_concept_ range_t_,
		hxenable_if_t<!hxis_lvalue_reference<range_t_>(), int> = 0>
	void add_range(range_t_&& range_) noexcept;

	/// Returns a reference to the back element. The deque must not be empty.
	hxattr_nodiscard T_& back(void);

	hxattr_nodiscard const T_& back(void) const;

	/// Returns the capacity of the deque or 0 if unallocated.
	hxattr_nodiscard hxsize_t capacity(void) const;

	/// Returns a const iterator pointing to the first element (alias for
	/// `begin`).
	const_iterator cbegin(void) const { return const_iterator(this, 0); }

	/// Returns a const iterator pointing past the last element.
	const_iterator cend(void) const { return const_iterator(this, this->size()); }

	/// Destroys all elements and resets the deque to empty without deallocating.
	void clear(void) noexcept;

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

	/// Returns a const iterator pointing past the last element.
	const_iterator end(void) const { return const_iterator(this, this->size()); }

	iterator end(void) { return iterator(this, this->size()); }

	/// Returns a reference to the front element. The deque must not be empty.
	hxattr_nodiscard T_& front(void);

	hxattr_nodiscard const T_& front(void) const;

	/// Returns `true` if the deque is at capacity.
	hxattr_nodiscard bool full(void) const;

	/// Returns the capacity of the deque or 0 if unallocated.
	hxattr_nodiscard hxsize_t max_size(void) const { return this->capacity(); }

	/// Removes and destroys the back element. The deque must not be empty.
	void pop_back(void) noexcept;

	/// Removes and destroys the front element. The deque must not be empty.
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
	// This is raw underlying data and would not be what was expected.
	using hxallocator<T_, capacity_>::data;

	hxdeque(const hxdeque&) = delete;
	void operator=(const hxdeque&) = delete;

	size_t m_head_;
	size_t m_tail_;
};

#include "detail/hxdeque.inl"
HX_NS_END_
