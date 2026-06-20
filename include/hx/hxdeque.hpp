#pragma once
// SPDX-FileCopyrightText: © 2025 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

#include "libhatchet.h"
#include "hxallocator.hpp"
#include "hxutility.h"

/// A fixed-capacity deque backed by a power-of-two ring buffer. All operations
/// are `O(1)`. The capacity must be a power of two and greater than zero.
/// This designed to do one thing very well and that is all.
/// - `T` : The element type stored in the deque.
/// - `capacity` : Maximum element count or `hxallocator_dynamic_capacity` for dynamic storage.
template<typename T_, size_t capacity_=hxallocator_dynamic_capacity>
class hxdeque : public hxallocator<T_, capacity_> {
public:
	/// Constructs an empty hxdeque. When using static storage the capacity is
	/// fixed at compile time. Asserts that the capacity is a power of two
	/// and greater than zero.
	/// - `dynamic_capacity` : Element capacity for dynamic storage.
	explicit hxdeque(size_t dynamic_capacity_=0u);

	/// Destroys all elements in the deque.
	~hxdeque(void);

	/// Returns a reference to the element at logical index `index`. Asserts if
	/// `index` is out of range or the deque is unallocated.
	/// - `index` : Zero-based index from the front.
	hxattr_nodiscard T_& operator[](size_t index_);

	/// Returns a const reference to the element at logical index `index`.
	/// Asserts if `index` is out of range or the deque is unallocated.
	/// - `index` : Zero-based index from the front.
	hxattr_nodiscard const T_& operator[](size_t index_) const;

	/// Returns a reference to the element at logical index `index`. Asserts if
	/// `index` is out of range or the deque is unallocated.
	/// - `index` : Zero-based index from the front.
	hxattr_nodiscard T_& at(size_t index_);

	/// Returns a const reference to the element at logical index `index`.
	/// Asserts if `index` is out of range or the deque is unallocated.
	/// - `index` : Zero-based index from the front.
	hxattr_nodiscard const T_& at(size_t index_) const;

	/// Returns a reference to the back element.
	hxattr_nodiscard T_& back(void);

	/// Returns a const reference to the back element.
	hxattr_nodiscard const T_& back(void) const;

	/// Destroys all elements and resets the deque to empty without deallocating.
	void clear(void);

	/// Constructs an element in place at the back using forwarded arguments.
	/// Returns `false` if the deque is full or unallocated. Exactly the same
	/// as `push_back`.
	/// - `args` : Arguments forwarded to `T`'s constructor.
	template<typename... args_t_>
	hxattr_nodiscard bool emplace_back(args_t_&&... args_);

	/// Constructs an element in place at the front using forwarded arguments.
	/// Returns `false` if the deque is full or unallocated. Exactly the same
	/// as `push_front`.
	/// - `args` : Arguments forwarded to `T`'s constructor.
	template<typename... args_t_>
	hxattr_nodiscard bool emplace_front(args_t_&&... args_);

	/// Returns `true` if the deque contains no elements.
	hxattr_nodiscard bool empty(void) const;

	/// Returns a reference to the front element.
	hxattr_nodiscard T_& front(void);

	/// Returns a const reference to the front element.
	hxattr_nodiscard const T_& front(void) const;

	/// Returns `true` if the deque is at capacity.
	hxattr_nodiscard bool full(void) const;

	/// Removes and destroys the back element, returning it in `out`. Returns
	/// `false` if empty. Asserts if unallocated.
	/// - `out` : Receives the removed element.
	hxattr_nodiscard bool pop_back(T_& out_);

	/// Removes and destroys the front element, returning it in `out`. Returns
	/// `false` if empty. Asserts if unallocated.
	/// - `out` : Receives the removed element.
	hxattr_nodiscard bool pop_front(T_& out_);

	/// Appends an element at the back using forwarded arguments. Returns `false`
	/// if the deque is full or unallocated. Exactly the same as `emplace_back`.
	/// - `args` : Arguments forwarded to `T`'s constructor.
	template<typename... args_t_>
	hxattr_nodiscard bool push_back(args_t_&&... args_);

	/// Prepends an element at the front using forwarded arguments. Returns
	/// `false` if the deque is full or unallocated. Exactly the same as
	/// `emplace_front`.
	/// - `args` : Arguments forwarded to `T`'s constructor.
	template<typename... args_t_>
	hxattr_nodiscard bool push_front(args_t_&&... args_);

	/// Allocates storage for a dynamic deque. May only be called once and only
	/// when the deque has no storage. Asserts that the capacity is a power of
	/// two and greater than zero.
	/// - `dynamic_capacity` : Element capacity to allocate.
	void reserve(size_t dynamic_capacity_);

	/// Returns the number of elements currently in the deque.
	hxattr_nodiscard size_t size(void) const;

private:
	// Hide access to the raw data. This is raw underlying data and would not be
	// what was expected.
	using hxallocator<T_, capacity_>::data;

	hxdeque(const hxdeque&) = delete;
	void operator=(const hxdeque&) = delete;

	size_t m_mask_;
	size_t m_head_;
	size_t m_tail_;
	size_t m_count_;
};

// ----------------------------------------------------------------------------

template<typename T_, size_t capacity_>
hxdeque<T_, capacity_>::hxdeque(size_t dynamic_capacity_)
	: m_mask_(0u), m_head_(0u), m_tail_(0u), m_count_(0u)
{
	hxassertrelease(dynamic_capacity_ == 0u || (dynamic_capacity_ & (dynamic_capacity_ - 1u)) == 0u,
		"invalid_capacity capacity must be a power of 2");
	if(dynamic_capacity_ != 0u) {
		this->reserve_storage_(dynamic_capacity_);
	}
	m_mask_ = this->capacity() - 1u;
}

template<typename T_, size_t capacity_>
hxdeque<T_, capacity_>::~hxdeque(void) { clear(); }

template<typename T_, size_t capacity_>
T_& hxdeque<T_, capacity_>::operator[](size_t index_) {
	hxassertmsg(this->capacity() > 0u, "unallocated_deque");
	hxassertmsg(index_ < m_count_, "invalid_index %zu", index_);
	return this->data()[(m_head_ + index_) & m_mask_];
}

template<typename T_, size_t capacity_>
const T_& hxdeque<T_, capacity_>::operator[](size_t index_) const {
	hxassertmsg(this->capacity() > 0u, "unallocated_deque");
	hxassertmsg(index_ < m_count_, "invalid_index %zu", index_);
	return this->data()[(m_head_ + index_) & m_mask_];
}

template<typename T_, size_t capacity_>
T_& hxdeque<T_, capacity_>::at(size_t index_) {
	hxassertmsg(this->capacity() > 0u, "unallocated_deque");
	hxassertmsg(index_ < m_count_, "invalid_index %zu", index_);
	return this->data()[(m_head_ + index_) & m_mask_];
}

template<typename T_, size_t capacity_>
const T_& hxdeque<T_, capacity_>::at(size_t index_) const {
	hxassertmsg(this->capacity() > 0u, "unallocated_deque");
	hxassertmsg(index_ < m_count_, "invalid_index %zu", index_);
	return this->data()[(m_head_ + index_) & m_mask_];
}

template<typename T_, size_t capacity_>
T_& hxdeque<T_, capacity_>::back(void) {
	hxassertmsg(this->capacity() > 0u, "unallocated_deque");
	hxassertmsg(m_count_ > 0u, "empty_deque");
	return this->data()[(m_tail_ + m_mask_) & m_mask_];
}

template<typename T_, size_t capacity_>
const T_& hxdeque<T_, capacity_>::back(void) const {
	hxassertmsg(this->capacity() > 0u, "unallocated_deque");
	hxassertmsg(m_count_ > 0u, "empty_deque");
	return this->data()[(m_tail_ + m_mask_) & m_mask_];
}

template<typename T_, size_t capacity_>
void hxdeque<T_, capacity_>::clear(void) {
	T_* const data_ = this->data();
	for(size_t i_ = 0u; i_ < m_count_; ++i_) {
		data_[(m_head_ + i_) & m_mask_].~T_();
	}
	m_head_ = 0u;
	m_tail_ = 0u;
	m_count_ = 0u;
}

template<typename T_, size_t capacity_>
template<typename... args_t_>
bool hxdeque<T_, capacity_>::emplace_back(args_t_&&... args_) {
	return push_back(hxforward<args_t_>(args_)...);
}

template<typename T_, size_t capacity_>
template<typename... args_t_>
bool hxdeque<T_, capacity_>::emplace_front(args_t_&&... args_) {
	return push_front(hxforward<args_t_>(args_)...);
}

template<typename T_, size_t capacity_>
bool hxdeque<T_, capacity_>::empty(void) const { return m_count_ == 0u; }

template<typename T_, size_t capacity_>
T_& hxdeque<T_, capacity_>::front(void) {
	hxassertmsg(this->capacity() > 0u, "unallocated_deque");
	hxassertmsg(m_count_ > 0u, "empty_deque");
	return this->data()[m_head_];
}

template<typename T_, size_t capacity_>
const T_& hxdeque<T_, capacity_>::front(void) const {
	hxassertmsg(this->capacity() > 0u, "unallocated_deque");
	hxassertmsg(m_count_ > 0u, "empty_deque");
	return this->data()[m_head_];
}

template<typename T_, size_t capacity_>
bool hxdeque<T_, capacity_>::full(void) const { return m_count_ == this->capacity(); }

template<typename T_, size_t capacity_>
bool hxdeque<T_, capacity_>::pop_back(T_& out_) {
	hxassertmsg(this->capacity() > 0u, "unallocated_deque");
	if(m_count_ == 0u) { return false; }
	m_tail_ = (m_tail_ + m_mask_) & m_mask_;
	T_* slot_ = this->data() + m_tail_;
	out_ = hxmove(*slot_);
	slot_->~T_();
	--m_count_;
	return true;
}

template<typename T_, size_t capacity_>
bool hxdeque<T_, capacity_>::pop_front(T_& out_) {
	hxassertmsg(this->capacity() > 0u, "unallocated_deque");
	if(m_count_ == 0u) { return false; }
	T_* slot_ = this->data() + m_head_;
	m_head_ = (m_head_ + 1u) & m_mask_;
	out_ = hxmove(*slot_);
	slot_->~T_();
	--m_count_;
	return true;
}

template<typename T_, size_t capacity_>
template<typename... args_t_>
bool hxdeque<T_, capacity_>::push_back(args_t_&&... args_) {
	hxassertmsg(this->capacity() > 0u, "unallocated_deque");
	if(m_count_ >= this->capacity()) { return false; }
	T_* slot_ = this->data() + m_tail_;
	m_tail_ = (m_tail_ + 1u) & m_mask_; ++m_count_;
	::new(slot_) T_(hxforward<args_t_>(args_)...);
	return true;
}

template<typename T_, size_t capacity_>
template<typename... args_t_>
bool hxdeque<T_, capacity_>::push_front(args_t_&&... args_) {
	hxassertmsg(this->capacity() > 0u, "unallocated_deque");
	if(m_count_ >= this->capacity()) { return false; }
	m_head_ = (m_head_ + m_mask_) & m_mask_;
	T_* slot_ = this->data() + m_head_;
	++m_count_;
	::new(slot_) T_(hxforward<args_t_>(args_)...);
	return true;
}

template<typename T_, size_t capacity_>
void hxdeque<T_, capacity_>::reserve(size_t dynamic_capacity_) {
	hxassertrelease(dynamic_capacity_ > 0u && (dynamic_capacity_ & (dynamic_capacity_ - 1u)) == 0u,
		"invalid_capacity capacity must be a power of 2");
	this->reserve_storage_(dynamic_capacity_);
	m_mask_ = this->capacity() - 1u;
}

template<typename T_, size_t capacity_>
size_t hxdeque<T_, capacity_>::size(void) const { return m_count_; }
