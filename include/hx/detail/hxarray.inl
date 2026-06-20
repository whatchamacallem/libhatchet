#pragma once
// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

static_assert(LIBHATCHET_VER, "Internal. Do not include this file directly.");

template<hxarray_concept_ T_, size_t capacity_>
hxarray<T_, capacity_>::hxarray(void) : m_end_(this->data()) { }

template<hxarray_concept_ T_, size_t capacity_>
hxarray<T_, capacity_>::hxarray(size_t size_) : hxarray() {
	this->resize(size_);
}

template<hxarray_concept_ T_, size_t capacity_>
hxarray<T_, capacity_>::hxarray(size_t size_, const T_& x_) : hxarray() {
	this->resize(size_, x_);
}

template<hxarray_concept_ T_, size_t capacity_>
hxarray<T_, capacity_>::hxarray(const hxarray& x_) : hxarray() {
	this->assign<const T_*>(x_.data(), x_.m_end_);
}

template<hxarray_concept_ T_, size_t capacity_>
template<size_t capacity_x_>
hxarray<T_, capacity_>::hxarray(const hxarray<T_, capacity_x_>& x_) : hxarray() {
	this->assign<const T_*>(x_.data(), x_.end());
}

template<hxarray_concept_ T_, size_t capacity_>
hxarray<T_, capacity_>::hxarray(hxarray&& x_) noexcept {
	static_assert(capacity_ == hxallocator_dynamic_capacity,
		"Capacity hxallocator_dynamic_capacity required for temporaries.");
	::memcpy((void*)this, &x_, sizeof x_); // NOLINT
	::memset((void*)&x_, 0x00, sizeof x_); // NOLINT
}

template<hxarray_concept_ T_, size_t capacity_>
template<typename other_value_t_, size_t array_length_>
hxarray<T_, capacity_>::hxarray(const other_value_t_(&array_)[array_length_]) : hxarray() {
	this->assign(array_ + 0, array_ + array_length_);
}

template<hxarray_concept_ T_, size_t capacity_>
template<typename other_value_t_>
hxarray<T_, capacity_>::hxarray(std::initializer_list<other_value_t_> x_) : hxarray() {
	this->assign(x_.begin(), x_.end());
}

template<hxarray_concept_ T_, size_t capacity_>
hxarray<T_, capacity_>::~hxarray(void) {
	this->destruct_(this->data(), m_end_);
}

template<hxarray_concept_ T_, size_t capacity_>
void hxarray<T_, capacity_>::operator=(const hxarray& x_) {
	hxassertmsg((const void*)this != (const void*)&x_, "invalid_reference Assignment to self.");
	this->assign<const T_*>(x_.data(), x_.m_end_);
}

template<hxarray_concept_ T_, size_t capacity_>
template<size_t capacity_x_>
void hxarray<T_, capacity_>::operator=(const hxarray<T_, capacity_x_>& x_) {
	hxassertmsg((const void*)this != (const void*)&x_, "invalid_reference Assignment to self.");
	this->assign<const T_*>(x_.data(), x_.end());
}

template<hxarray_concept_ T_, size_t capacity_>
void hxarray<T_, capacity_>::operator=(hxarray&& x_) noexcept {
	hxassertmsg((const void*)this != (const void*)&x_, "invalid_reference Assignment to self.");
	this->swap(x_);
}

template<hxarray_concept_ T_, size_t capacity_>
template<typename other_value_t_, size_t array_length_>
void hxarray<T_, capacity_>::operator=(const other_value_t_(&array_)[array_length_]) {
	this->assign(array_ + 0, array_ + array_length_);
}

template<hxarray_concept_ T_, size_t capacity_>
const T_& hxarray<T_, capacity_>::operator[](size_t index_) const {
	hxassert_hard(index_ < this->size(), "invalid_index %zu", index_);
	return this->data()[index_];
}

template<hxarray_concept_ T_, size_t capacity_>
T_& hxarray<T_, capacity_>::operator[](size_t index_) {
	hxassert_hard(index_ < this->size(), "invalid_index %zu", index_);
	return this->data()[index_];
}

template<hxarray_concept_ T_, size_t capacity_>
void hxarray<T_, capacity_>::operator+=(const T_& x_) {
	::new(this->push_back_unconstructed_()) T_(x_);
}

template<hxarray_concept_ T_, size_t capacity_>
void hxarray<T_, capacity_>::operator+=(T_&& x_) {
	::new(this->push_back_unconstructed_()) T_(hxmove(x_));
}

template<hxarray_concept_ T_, size_t capacity_>
template<size_t capacity_x_>
void hxarray<T_, capacity_>::operator+=(const hxarray<T_, capacity_x_>& x_) {
	hxassertmsg((const void*)this != (const void*)&x_, "invalid_reference Assignment to self.");
	for(const T_* hxrestrict it_ = x_.data(), *end_ = x_.end(); it_ != end_; ++it_) {
		::new(this->push_back_unconstructed_()) T_(*it_);
	}
}

template<hxarray_concept_ T_, size_t capacity_>
template<size_t capacity_x_>
void hxarray<T_, capacity_>::operator+=(hxarray<T_, capacity_x_>&& x_) {
	hxassertmsg((const void*)this != (const void*)&x_, "invalid_reference Assignment to self.");
	// Non-const mutable operation.
	for(T_* hxrestrict it_ = x_.data(), *end_ = x_.end(); it_ != end_; ++it_) {
		::new(this->push_back_unconstructed_()) T_(hxmove(*it_));
	}
}

template<hxarray_concept_ T_, size_t capacity_>
hxarray_back_inserter_<hxarray<T_, capacity_>> hxarray<T_, capacity_>::operator*(void) {
	return hxarray_back_inserter_<hxarray<T_, capacity_>>(*this);
}

template<hxarray_concept_ T_, size_t capacity_>
template<typename callable_t_>
bool hxarray<T_, capacity_>::all_of(callable_t_&& fn_) const {
	for(const T_* it_ = this->data(), *end_ = m_end_; it_ != end_; ++it_) {
		if(!hxforward<callable_t_>(fn_)(*it_)) {
			return false;
		}
	}
	return true;
}

template<hxarray_concept_ T_, size_t capacity_>
template<typename callable_t_>
bool hxarray<T_, capacity_>::all_of(callable_t_&& fn_) {
	for(T_* it_ = this->data(), *end_ = m_end_; it_ != end_; ++it_) {
		if(!hxforward<callable_t_>(fn_)(*it_)) {
			return false;
		}
	}
	return true;
}

template<hxarray_concept_ T_, size_t capacity_>
template<typename callable_t_>
bool hxarray<T_, capacity_>::any_of(callable_t_&& fn_) const {
	for(const T_* it_ = this->data(), *end_ = m_end_; it_ != end_; ++it_) {
		if(hxforward<callable_t_>(fn_)(*it_)) {
			return true;
		}
	}
	return false;
}

template<hxarray_concept_ T_, size_t capacity_>
template<typename callable_t_>
bool hxarray<T_, capacity_>::any_of(callable_t_&& fn_) {
	for(T_* it_ = this->data(), *end_ = m_end_; it_ != end_; ++it_) {
		if(hxforward<callable_t_>(fn_)(*it_)) {
			return true;
		}
	}
	return false;
}

template<hxarray_concept_ T_, size_t capacity_>
template<typename iter_t_>
void hxarray<T_, capacity_>::assign(iter_t_ begin_, iter_t_ end_) {
	hxassert_hard((end_ - begin_) >= 0, "invalid_iterator");
	this->reserve(static_cast<size_t>(end_ - begin_));
	T_* hxrestrict it0_ = this->data();
	this->destruct_(it0_, m_end_);
	iter_t_ it1_(begin_); // begin_ may be a reference.
	while(it1_ != end_) {
		::new(it0_++) T_(*it1_++);
	}
	m_end_ = it0_;
}

#if HX_CPLUSPLUS >= 202002L
template<hxarray_concept_ T_, size_t capacity_>
template<typename range_t_>
void hxarray<T_, capacity_>::assign_range(range_t_& range_) {
	this->assign(range_.begin(), range_.end());
}

template<hxarray_concept_ T_, size_t capacity_>
template<typename range_t_>
requires(!hxis_lvalue_reference<range_t_>::value)
void hxarray<T_, capacity_>::assign_range(range_t_&& range_) {
	this->clear();
	// Sorry, std::begin and std::end may not exist.
	for(auto it_ = range_.begin(), end_ = range_.end(); it_ != end_; ++it_) {
		::new(this->push_back_unconstructed_()) T_(hxmove(*it_));
	}
}
#endif

template<hxarray_concept_ T_, size_t capacity_>
const T_& hxarray<T_, capacity_>::back(void) const {
	hxassert_hard(!this->empty(), "invalid_reference");
	return m_end_[-1];
}

template<hxarray_concept_ T_, size_t capacity_>
T_& hxarray<T_, capacity_>::back(void) {
	hxassert_hard(!this->empty(), "invalid_reference");
	return m_end_[-1];
}

template<hxarray_concept_ T_, size_t capacity_>
const T_* hxarray<T_, capacity_>::binary_search(const T_& value_) const {
	return hxbinary_search<const T_*>(this->data(), m_end_, value_, hxkey_less_t<const T_&>{});
}

template<hxarray_concept_ T_, size_t capacity_>
T_* hxarray<T_, capacity_>::binary_search(const T_& value_) {
	return hxbinary_search<T_*>(this->data(), m_end_, value_, hxkey_less_t<const T_&>{});
}

template<hxarray_concept_ T_, size_t capacity_>
void hxarray<T_, capacity_>::clear(void) {
	this->destruct_(this->data(), m_end_);
	m_end_ = this->data();
}

template<hxarray_concept_ T_, size_t capacity_>
template<typename... args_t_>
T_& hxarray<T_, capacity_>::emplace_back(args_t_&&... args_) {
	hxassert_hard(!this->full(), "stack_overflow");
	return *::new(m_end_++) T_(hxforward<args_t_>(args_)...);
}

template<hxarray_concept_ T_, size_t capacity_>
template<size_t capacity_x_>
bool hxarray<T_, capacity_>::equal(const hxarray<T_, capacity_x_>& x_) const {
	if(this->size() != x_.size()) {
		return false;
	}
	for(const T_* it0_ = this->data(), *it1_ = x_.data(), *end_ = m_end_;
			it0_ != end_; ++it0_, ++it1_) {
		if(!hxkey_equal(*it0_, *it1_)) {
			return false;
		}
	}
	return true;
}

template<hxarray_concept_ T_, size_t capacity_>
void hxarray<T_, capacity_>::erase(T_* pos_) {
	hxassert_hard(pos_ >= this->data() && pos_ < m_end_, "invalid_iterator");
	while((pos_ + 1) != m_end_) {
		*pos_ = hxmove(*(pos_ + 1));
		++pos_;
	}
	(--m_end_)->~T_();
}

template<hxarray_concept_ T_, size_t capacity_>
void hxarray<T_, capacity_>::erase(size_t index_) {
	hxassert_hard(index_ < this->size(), "invalid_index %zu", index_);
	this->erase(this->data() + index_);
}

template<hxarray_concept_ T_, size_t capacity_>
template<typename callable_t_>
size_t hxarray<T_, capacity_>::erase_if(callable_t_&& fn_) {
	size_t removed_ = 0u;
	T_* data_ = this->data();
	for(size_t index_ = this->size(); index_--;) {
		if(hxforward<callable_t_>(fn_)(data_[index_])) {
			this->erase_unordered(index_);
			++removed_;
		}
	}
	return removed_;
}

template<hxarray_concept_ T_, size_t capacity_>
template<typename callable_t_>
size_t hxarray<T_, capacity_>::erase_if_heap(callable_t_&& fn_) {
	T_* hxrestrict src_ = this->data();
	T_* hxrestrict dst_ = src_;
	for(T_* end_ = m_end_; src_ != end_; ++src_) {
		if(!hxforward<callable_t_>(fn_)(*src_)) {
			// Survivor: move into the next free slot, or leave in place.
			if(src_ != dst_) {
				*dst_ = hxmove(*src_);
			}
			++dst_;
		}
		else {
			src_->~T_(); // Removed: destroy without freeing storage.
		}
	}
	const size_t removed_ = static_cast<size_t>(m_end_ - dst_);
	m_end_ = dst_;
	if(removed_) {
		hxdetail_::hxmake_heap_<T_*>(this->data(), m_end_, hxkey_less_t<T_>{});
	}
	return removed_;
}

template<hxarray_concept_ T_, size_t capacity_>
void hxarray<T_, capacity_>::erase_unordered(const T_* pos_) {
	hxassert_hard(pos_ >= this->data() && pos_ < m_end_, "invalid_iterator");
	if(pos_ != --m_end_) {
		// Having a non-const this pointer provides valid write access.
		*const_cast<T_*>(pos_) = hxmove(*m_end_);
	}
	m_end_->~T_();
}

template<hxarray_concept_ T_, size_t capacity_>
void hxarray<T_, capacity_>::erase_unordered(size_t index_) {
	hxassert_hard(index_ < this->size(), "invalid_index %zu", index_);
	this->erase_unordered(this->data() + index_);
}

template<hxarray_concept_ T_, size_t capacity_>
const T_* hxarray<T_, capacity_>::find(const T_& value_) const {
	for(const T_* it_ = this->data(), *end_ = m_end_; it_ != end_; ++it_) {
		if(hxkey_equal(*it_, value_)) {
			return it_;
		}
	}
	return m_end_;
}

template<hxarray_concept_ T_, size_t capacity_>
T_* hxarray<T_, capacity_>::find(const T_& value_) {
	for(T_* it_ = this->data(), *end_ = m_end_; it_ != end_; ++it_) {
		if(hxkey_equal(*it_, value_)) {
			return it_;
		}
	}
	return m_end_;
}

template<hxarray_concept_ T_, size_t capacity_>
template<typename callable_t_>
const T_* hxarray<T_, capacity_>::find_if(callable_t_&& fn_) const {
	for(const T_* it_ = this->data(), *end_ = m_end_; it_ != end_; ++it_) {
		if(hxforward<callable_t_>(fn_)(*it_)) {
			return it_;
		}
	}
	return m_end_;
}

template<hxarray_concept_ T_, size_t capacity_>
template<typename callable_t_>
T_* hxarray<T_, capacity_>::find_if(callable_t_&& fn_) {
	for(T_* it_ = this->data(), *end_ = m_end_; it_ != end_; ++it_) {
		if(hxforward<callable_t_>(fn_)(*it_)) {
			return it_;
		}
	}
	return m_end_;
}

template<hxarray_concept_ T_, size_t capacity_>
template<typename callable_t_>
void hxarray<T_, capacity_>::for_each(callable_t_&& fn_) const {
	for(const T_* it_ = this->data(), *end_ = m_end_; it_ != end_; ++it_) {
		hxforward<callable_t_>(fn_)(*it_);
	}
}

template<hxarray_concept_ T_, size_t capacity_>
template<typename callable_t_>
void hxarray<T_, capacity_>::for_each(callable_t_&& fn_) {
	for(T_* it_ = this->data(), *end_ = m_end_; it_ != end_; ++it_) {
		hxforward<callable_t_>(fn_)(*it_);
	}
}

template<hxarray_concept_ T_, size_t capacity_>
const T_& hxarray<T_, capacity_>::front(void) const {
	hxassert_hard(!this->empty(), "invalid_reference");
	return *this->data();
}

template<hxarray_concept_ T_, size_t capacity_>
T_& hxarray<T_, capacity_>::front(void) {
	hxassert_hard(!this->empty(), "invalid_reference");
	return *this->data();
}

template<hxarray_concept_ T_, size_t capacity_>
template<typename callable_t_>
void hxarray<T_, capacity_>::generate_n(size_t size_, callable_t_&& fn_) {
	while(size_--) {
		::new(this->push_back_unconstructed_()) T_(hxforward<callable_t_>(fn_)());
	}
}

template<hxarray_concept_ T_, size_t capacity_>
const T_* hxarray<T_, capacity_>::get(size_t index_) const {
	// Casting a signed index is well defined. Comparing pointers would be undefined.
	return index_ < this->size() ? this->data() + index_ : hxnull;
}

template<hxarray_concept_ T_, size_t capacity_>
T_* hxarray<T_, capacity_>::get(size_t index_) {
	// Casting a signed index is well defined. Comparing pointers would be undefined.
	return index_ < this->size() ? this->data() + index_ : hxnull;
}

template<hxarray_concept_ T_, size_t capacity_>
template<typename ref_t_>
void hxarray<T_, capacity_>::insert(const T_* pos_, ref_t_&& x_) {
	hxassert_hard(pos_ >= this->data() && pos_ <= m_end_, "invalid_insert");
	if(pos_ == m_end_) {
		// Single constructor call for last element.
		::new(this->push_back_unconstructed_()) T_(hxforward<ref_t_>(x_));
	}
	else {
		// A move constructor for a new end element followed by a series of
		// assignment operations.
		T_* it_ = static_cast<T_*>(this->push_back_unconstructed_());
		::new(it_) T_(hxmove(it_[-1]));
		while(pos_ < --it_) {
			*it_ = it_[-1];
		}
		*it_ = hxforward<ref_t_>(x_);
	}
}

template<hxarray_concept_ T_, size_t capacity_>
template<typename ref_t_>
void hxarray<T_, capacity_>::insert(size_t index_, ref_t_&& x_) {
	hxassert_hard(index_ <= this->size(), "invalid_index %zu", index_);
	this->insert(this->data() + index_, hxforward<ref_t_>(x_));
}

template<hxarray_concept_ T_, size_t capacity_>
void hxarray<T_, capacity_>::insertion_sort(void) {
	hxinsertion_sort<T_*>(this->data(), m_end_, hxkey_less_t<T_>{});
}

template<hxarray_concept_ T_, size_t capacity_>
template<size_t capacity_x_>
bool hxarray<T_, capacity_>::less(const hxarray<T_, capacity_x_>& x_) const {
	const size_t size_ = hxmin(this->size(), x_.size());
	for(const T_* it0_ = this->data(), *it1_ = x_.data(), *end_ = it0_ + size_;
			it0_ != end_; ++it0_, ++it1_) {
		// Use `a == b` instead of `a < b && b < a` for performance.
		if(!hxkey_equal(*it0_, *it1_)) {
			return hxkey_less(*it0_, *it1_);
		}
	}
	// Order the prefix before the other.
	return this->size() < x_.size();
}

template<hxarray_concept_ T_, size_t capacity_>
void hxarray<T_, capacity_>::make_heap(void) {
	hxdetail_::hxmake_heap_<T_*>(this->data(), m_end_, hxkey_less_t<T_>{});
}

template<hxarray_concept_ T_, size_t capacity_>
template<size_t capacity_x_>
void hxarray<T_, capacity_>::memcpy(const hxarray<T_, capacity_x_>& x_) {
	this->resize(x_.size());
	::memcpy(static_cast<void*>(this->data()), x_.data(), x_.size_bytes()); // NOLINT
}

template<hxarray_concept_ T_, size_t capacity_>
void hxarray<T_, capacity_>::memset(int byte_) {
	::memset(static_cast<void*>(this->data()), byte_, this->size_bytes()); // NOLINT
}

template<hxarray_concept_ T_, size_t capacity_>
void hxarray<T_, capacity_>::pop_back(void) {
	hxassert_hard(!this->empty(), "stack_underflow");
	(--m_end_)->~T_();
}

template<hxarray_concept_ T_, size_t capacity_>
void hxarray<T_, capacity_>::pop_heap(void) {
	hxassert_hard(!this->empty(), "stack_underflow");
	T_* hxrestrict begin_ = this->data();
	--m_end_;
	if(begin_ == m_end_) {
		begin_->~T_();
		return;
	}
	*begin_ = hxmove(*m_end_);
	m_end_->~T_();
	hxdetail_::hxheapsort_heapify_(this->data(), begin_, m_end_, hxkey_less_t<T_>{});
}

template<hxarray_concept_ T_, size_t capacity_>
template<typename... args_t_>
T_& hxarray<T_, capacity_>::push_back(args_t_&&... args_) {
	hxassert_hard(!this->full(), "stack_overflow");
	return *::new(m_end_++) T_(hxforward<args_t_>(args_)...);
}

template<hxarray_concept_ T_, size_t capacity_>
template<typename ref_t_>
T_& hxarray<T_, capacity_>::push_heap(ref_t_&& arg_) {
	T_* begin_ = this->data();
	T_* node_ = static_cast<T_*>(this->push_back_unconstructed_());
	while(node_ != begin_) {
		T_* parent_ = begin_ + ((node_ - begin_ - 1) >> 1);
		// arg_ has to be comparable to T_.
		if(!hxkey_less(*parent_, arg_)) {
			break;
		}
		// Shifts unconstructed element (the hole) into position.
		::new(node_) T_(hxmove(*parent_));
		parent_->~T_();
		node_ = parent_;
	}
	// Construct new element.
	::new(node_) T_(hxmove(arg_));
	return *node_;
}

template<hxarray_concept_ T_, size_t capacity_>
void hxarray<T_, capacity_>::reserve(size_t size_,
		hxsystem_allocator_t allocator_,
		hxalignment_t alignment_) {
	this->reserve_storage_(size_, allocator_, alignment_);
	if(m_end_ == hxnull) {
		m_end_ = this->data();
	}
}

template<hxarray_concept_ T_, size_t capacity_>
void hxarray<T_, capacity_>::resize(size_t size_) {
	this->reserve(size_);
	T_* end_ = this->data() + size_;
	if(size_ >= this->size()) {
		while(m_end_ != end_) {
			// This version uses a default constructor. Note "T_()" is not being
			// called. That would default initialize arrays of integers to zero.
			::new(this->push_back_unconstructed_()) T_;
		}
	}
	else {
		this->destruct_(end_, m_end_);
	}
	m_end_ = end_;
}

template<hxarray_concept_ T_, size_t capacity_>
void hxarray<T_, capacity_>::resize(size_t size_, const T_& x_) {
	this->reserve(size_);
	T_* end_ = this->data() + size_;
	if(size_ >= this->size()) {
		while(m_end_ != end_) {
			// This version uses a copy constructor.
			::new(this->push_back_unconstructed_()) T_(x_);
		}
	}
	else {
		this->destruct_(end_, m_end_);
	}
	m_end_ = end_;
}

template<hxarray_concept_ T_, size_t capacity_>
void hxarray<T_, capacity_>::sort(void) {
	hxsort<T_*>(this->data(), m_end_, hxkey_less_t<T_>{});
}

template<hxarray_concept_ T_, size_t capacity_>
void hxarray<T_, capacity_>::swap(hxarray& x_) noexcept {
	static_assert(capacity_ == hxallocator_dynamic_capacity,
		"Dynamic capacity required for hxarray::swap");

	// hxarray has a dynamic allocator that allows memcpy.
	hxswap_memcpy(*this, x_);
}

template<hxarray_concept_ T_, size_t capacity_>
void* hxarray<T_, capacity_>::push_back_unconstructed_(void) {
	hxassert_hard(!this->full(), "stack_overflow");
	return static_cast<void*>(m_end_++);
}

template<hxarray_concept_ T_, size_t capacity_>
void hxarray<T_, capacity_>::destruct_(T_* begin_, T_* end_) {
	while(begin_ != end_) {
		begin_++->~T_();
	}
}
