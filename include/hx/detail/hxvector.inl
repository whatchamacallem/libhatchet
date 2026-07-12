#pragma once
// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

#ifndef LIBHATCHET_VER
#error Internal. Do not include this file directly.
#endif

#ifndef HX_DOXYGEN_PARSER
HX_BEGIN_INL_

template<hxvector_concept_ T_, hxsize_t capacity_>
hxinline hxattr_flatten hxvector<T_, capacity_>::hxvector(void) : m_end_(this->data()) { }

template<hxvector_concept_ T_, hxsize_t capacity_>
hxinline hxattr_flatten hxvector<T_, capacity_>::hxvector(hxsize_t size_) noexcept : hxvector() {
	this->resize(size_);
}

template<hxvector_concept_ T_, hxsize_t capacity_>
hxinline hxattr_flatten hxvector<T_, capacity_>::hxvector(hxsize_t size_,
		const T_& x_) noexcept : hxvector() {
	this->resize(size_, x_);
}

template<hxvector_concept_ T_, hxsize_t capacity_>
hxinline hxattr_flatten hxvector<T_, capacity_>::hxvector(const hxvector& x_) noexcept : hxvector() {
	this->assign<const T_*>(x_.data(), x_.m_end_);
}

template<hxvector_concept_ T_, hxsize_t capacity_>
template<hxsize_t capacity_x_>
hxinline hxattr_flatten hxvector<T_, capacity_>::hxvector(
		const hxvector<T_, capacity_x_>& x_) noexcept : hxvector() {
	this->assign<const T_*>(x_.data(), x_.end());
}

template<hxvector_concept_ T_, hxsize_t capacity_>
hxinline hxattr_flatten hxvector<T_, capacity_>::hxvector(hxvector&& x_) noexcept {
	static_assert(capacity_ == hxallocator_dynamic_capacity,
		"hxallocator_dynamic_capacity required for temporaries");
	::memcpy(static_cast<void*>(this), &x_, sizeof x_); // NOLINT(bugprone-undefined-memory-manipulation)
	::memset(static_cast<void*>(&x_), 0x00, sizeof x_);
}

template<hxvector_concept_ T_, hxsize_t capacity_>
template<typename other_value_t_, hxsize_t array_length_>
hxinline hxattr_flatten hxvector<T_, capacity_>::hxvector(
		const other_value_t_(&array_)[array_length_]) noexcept : hxvector() {
	this->assign(array_ + 0, array_ + array_length_);
}

template<hxvector_concept_ T_, hxsize_t capacity_>
template<typename other_value_t_>
hxinline hxattr_flatten hxvector<T_, capacity_>::hxvector(
		std::initializer_list<other_value_t_> x_) noexcept : hxvector() {
	this->assign(x_.begin(), x_.end());
}

template<hxvector_concept_ T_, hxsize_t capacity_>
hxinline hxattr_flatten hxvector<T_, capacity_>::~hxvector(void) noexcept {
	this->destruct_(this->data(), m_end_);
}

template<hxvector_concept_ T_, hxsize_t capacity_>
hxinline hxattr_flatten void hxvector<T_, capacity_>::operator=(const hxvector& x_) noexcept {
	hxassertmsg(static_cast<const void*>(this) != static_cast<const void*>(&x_),
		"invalid_reference");
	this->assign<const T_*>(x_.data(), x_.m_end_);
}

template<hxvector_concept_ T_, hxsize_t capacity_>
template<hxsize_t capacity_x_>
hxinline hxattr_flatten void hxvector<T_, capacity_>::operator=(const hxvector<T_,
		capacity_x_>& x_) noexcept {
	hxassertmsg(static_cast<const void*>(this) != static_cast<const void*>(&x_),
		"invalid_reference");
	this->assign<const T_*>(x_.data(), x_.end());
}

template<hxvector_concept_ T_, hxsize_t capacity_>
hxinline hxattr_flatten void hxvector<T_, capacity_>::operator=(hxvector&& x_) noexcept {
	hxassertmsg(static_cast<const void*>(this) != static_cast<const void*>(&x_),
		"invalid_reference");
	this->swap(x_);
}

template<hxvector_concept_ T_, hxsize_t capacity_>
template<typename other_value_t_, hxsize_t array_length_>
hxinline hxattr_flatten void hxvector<T_, capacity_>::operator=(
		const other_value_t_(&array_)[array_length_]) noexcept {
	this->assign(array_ + 0, array_ + array_length_);
}

template<hxvector_concept_ T_, hxsize_t capacity_>
hxinline hxattr_flatten const T_& hxvector<T_, capacity_>::operator[](hxsize_t index_) const {
	hxassert_hard(static_cast<size_t>(index_) < static_cast<size_t>(this->size()), "invalid_index %zu", index_);
	return this->data()[index_];
}

template<hxvector_concept_ T_, hxsize_t capacity_>
hxinline hxattr_flatten T_& hxvector<T_, capacity_>::operator[](hxsize_t index_) {
	hxassert_hard(static_cast<size_t>(index_) < static_cast<size_t>(this->size()), "invalid_index %zu", index_);
	return this->data()[index_];
}

template<hxvector_concept_ T_, hxsize_t capacity_>
hxinline hxattr_flatten void hxvector<T_, capacity_>::operator+=(const T_& x_) noexcept {
	::new(this->push_back_unconstructed_()) T_(x_);
}

template<hxvector_concept_ T_, hxsize_t capacity_>
hxinline hxattr_flatten void hxvector<T_, capacity_>::operator+=(T_&& x_) noexcept {
	::new(this->push_back_unconstructed_()) T_(hxmove(x_));
}

template<hxvector_concept_ T_, hxsize_t capacity_>
template<hxsize_t capacity_x_>
hxinline hxattr_flatten void hxvector<T_, capacity_>::operator+=(const hxvector<T_, capacity_x_>& x_) noexcept {
	hxassertmsg(static_cast<const void*>(this) != static_cast<const void*>(&x_),
		"invalid_reference");
	const hxsize_t size_x_ = x_.size();
	hxassert_hard(this->size() + size_x_ <= this->capacity(), "stack_overflow");
	const T_* hxrestrict src_ = x_.data();
	T_* hxrestrict dst_ = m_end_;
	for(const T_*const end_ = src_ + size_x_; src_ != end_; ++src_, ++dst_) {
		::new(dst_) T_(*src_);
	}
	m_end_ = dst_;
}

template<hxvector_concept_ T_, hxsize_t capacity_>
template<hxsize_t capacity_x_>
hxinline hxattr_flatten void hxvector<T_, capacity_>::operator+=(hxvector<T_, capacity_x_>&& x_) noexcept {
	hxassertmsg(static_cast<const void*>(this) != static_cast<const void*>(&x_),
		"invalid_reference");
	const hxsize_t size_x_ = x_.size();
	hxassert_hard(this->size() + size_x_ <= this->capacity(), "stack_overflow");
	T_* hxrestrict src_ = x_.data();
	T_* hxrestrict dst_ = m_end_;
	for(const T_*const end_ = src_ + size_x_; src_ != end_; ++src_, ++dst_) {
		::new(dst_) T_(hxmove(*src_));
	}
	m_end_ = dst_;
}

template<hxvector_concept_ T_, hxsize_t capacity_>
hxinline hxattr_flatten hxarray_back_inserter_<hxvector<T_, capacity_>> hxvector<T_, capacity_>::operator*(void) {
	return hxarray_back_inserter_<hxvector<T_, capacity_>>(*this);
}

template<hxvector_concept_ T_, hxsize_t capacity_>
template<typename callable_t_>
hxinline hxattr_flatten bool hxvector<T_, capacity_>::all_of(callable_t_&& callable_) const {
	for(const T_* it_ = this->data(), *const end_ = m_end_; it_ != end_; ++it_) {
		if(!hxforward<callable_t_>(callable_)(*it_)) {
			return false;
		}
	}
	return true;
}

template<hxvector_concept_ T_, hxsize_t capacity_>
template<typename callable_t_>
hxinline hxattr_flatten bool hxvector<T_, capacity_>::all_of(callable_t_&& callable_) {
	for(T_* it_ = this->data(), *const end_ = m_end_; it_ != end_; ++it_) {
		if(!hxforward<callable_t_>(callable_)(*it_)) {
			return false;
		}
	}
	return true;
}

template<hxvector_concept_ T_, hxsize_t capacity_>
template<typename callable_t_>
hxinline hxattr_flatten bool hxvector<T_, capacity_>::any_of(callable_t_&& callable_) const {
	for(const T_* it_ = this->data(), *const end_ = m_end_; it_ != end_; ++it_) {
		if(hxforward<callable_t_>(callable_)(*it_)) {
			return true;
		}
	}
	return false;
}

template<hxvector_concept_ T_, hxsize_t capacity_>
template<typename callable_t_>
hxinline hxattr_flatten bool hxvector<T_, capacity_>::any_of(callable_t_&& callable_) {
	for(T_* it_ = this->data(), *const end_ = m_end_; it_ != end_; ++it_) {
		if(hxforward<callable_t_>(callable_)(*it_)) {
			return true;
		}
	}
	return false;
}

template<hxvector_concept_ T_, hxsize_t capacity_>
template<typename iterator_t_>
hxinline hxattr_flatten void hxvector<T_, capacity_>::assign(iterator_t_ begin_, iterator_t_ end_) noexcept {
	hxassert_hard((end_ - begin_) >= 0, "invalid_iterator");
	this->reserve(static_cast<hxsize_t>(end_ - begin_));
	T_* hxrestrict it_ = this->data();
	this->destruct_(it_, m_end_);
	while(begin_ != end_) {
		::new(it_++) T_(*begin_++);
	}
	m_end_ = it_;
}

#if HX_CPLUSPLUS >= 202002L
template<hxvector_concept_ T_, hxsize_t capacity_>
template<typename range_t_>
hxinline hxattr_flatten void hxvector<T_, capacity_>::assign_range(range_t_& range_) noexcept {
	this->assign(range_.begin(), range_.end());
}

template<hxvector_concept_ T_, hxsize_t capacity_>
template<typename range_t_>
requires(!hxis_lvalue_reference<range_t_>::value)
hxinline hxattr_flatten void hxvector<T_, capacity_>::assign_range(range_t_&& range_) noexcept {
	this->clear();
	hxassert_hard(static_cast<hxsize_t>(range_.end() - range_.begin()) <= this->capacity(),
		"stack_overflow");
	T_* hxrestrict dst_ = this->data();
	// Sorry, hxbegin and hxend are not in use.
	for(auto it_ = range_.begin(), end_ = range_.end(); it_ != end_; ++it_) {
		::new(dst_++) T_(hxmove(*it_));
	}
	m_end_ = dst_;
}
#endif

template<hxvector_concept_ T_, hxsize_t capacity_>
hxinline hxattr_flatten const T_& hxvector<T_, capacity_>::back(void) const {
	hxassert_hard(!this->empty(), "invalid_reference");
	return m_end_[-1];
}

template<hxvector_concept_ T_, hxsize_t capacity_>
hxinline hxattr_flatten T_& hxvector<T_, capacity_>::back(void) {
	hxassert_hard(!this->empty(), "invalid_reference");
	return m_end_[-1];
}

template<hxvector_concept_ T_, hxsize_t capacity_>
hxinline const T_* hxvector<T_, capacity_>::binary_search(const T_& value_) const {
	return hxbinary_search<const T_*>(this->data(), m_end_, value_, hxkey_less_t<const T_&>{});
}

template<hxvector_concept_ T_, hxsize_t capacity_>
hxinline T_* hxvector<T_, capacity_>::binary_search(const T_& value_) {
	return hxbinary_search<T_*>(this->data(), m_end_, value_, hxkey_less_t<const T_&>{});
}

template<hxvector_concept_ T_, hxsize_t capacity_>
hxinline hxattr_flatten hxsize_t hxvector<T_, capacity_>::capacity(void) const {
	return hxallocator<T_, capacity_>::capacity();
}

template<hxvector_concept_ T_, hxsize_t capacity_>
hxinline hxattr_flatten void hxvector<T_, capacity_>::clear(void) noexcept {
	this->destruct_(this->data(), m_end_);
	m_end_ = this->data();
}

template<hxvector_concept_ T_, hxsize_t capacity_>
template<typename... args_t_>
hxinline hxattr_flatten T_& hxvector<T_, capacity_>::emplace_back(args_t_&&... args_) noexcept {
	hxassert_hard(!this->full(), "stack_overflow");
	return *::new(m_end_++) T_(hxforward<args_t_>(args_)...);
}

template<hxvector_concept_ T_, hxsize_t capacity_>
template<hxsize_t capacity_x_>
hxinline hxattr_flatten bool hxvector<T_, capacity_>::equal(const hxvector<T_, capacity_x_>& x_) const {
	if(this->size() != x_.size()) {
		return false;
	}
	for(const T_*it0_ = this->data(), *it1_ = x_.data(), *const end_ = m_end_;
			it0_ != end_; ++it0_, ++it1_) {
		if(!hxkey_equal(*it0_, *it1_)) {
			return false;
		}
	}
	return true;
}

template<hxvector_concept_ T_, hxsize_t capacity_>
hxinline hxattr_flatten void hxvector<T_, capacity_>::erase(T_* pos_) noexcept {
	hxassert_hard(pos_ >= this->data() && pos_ < m_end_, "invalid_iterator");
	T_*const end_ = m_end_ - 1;
	while(pos_ != end_) {
		*pos_ = hxmove(pos_[1]);
		++pos_;
	}
	end_->T_::~T_();
	m_end_ = end_;
}

template<hxvector_concept_ T_, hxsize_t capacity_>
hxinline hxattr_flatten void hxvector<T_, capacity_>::erase(hxsize_t index_) noexcept {
	hxassert_hard(static_cast<size_t>(index_) < static_cast<size_t>(this->size()), "invalid_index %zu", index_);
	this->erase(this->data() + index_);
}

template<hxvector_concept_ T_, hxsize_t capacity_>
template<typename callable_t_>
hxinline hxattr_flatten hxsize_t hxvector<T_, capacity_>::erase_if_unordered(callable_t_&& callable_) noexcept {
	hxsize_t removed_ = 0;
	const T_* const begin_ = this->data();
	T_* hxrestrict end_ = m_end_;
	for(T_* hxrestrict it_ = end_; it_-- != begin_;) {
		if(hxforward<callable_t_>(callable_)(*it_)) {
			if(it_ != --end_) {
				*it_ = hxmove(*end_);
			}
			end_->T_::~T_();
			++removed_;
		}
	}
	m_end_ = end_;
	return removed_;
}

template<hxvector_concept_ T_, hxsize_t capacity_>
template<typename callable_t_>
inline hxsize_t hxvector<T_, capacity_>::erase_if_heap(callable_t_&& callable_) noexcept {
	T_* hxrestrict src_ = this->data();
	T_* hxrestrict dst_ = src_;
	for(const T_*const end_ = m_end_; src_ != end_; ++src_) {
		if(!hxforward<callable_t_>(callable_)(*src_)) {
			// Survivor: move into the next free slot, or leave in place.
			if(src_ != dst_) {
				*dst_ = hxmove(*src_);
			}
			++dst_;
		}
		else {
			src_->T_::~T_(); // Removed: destroy without freeing storage.
		}
	}
	const hxsize_t removed_ = m_end_ - dst_;
	m_end_ = dst_;
	if(removed_ != 0) {
		hxdetail_::hxmake_heap_<T_*>(this->data(), dst_, hxkey_less_t<T_>{});
	}
	return removed_;
}

template<hxvector_concept_ T_, hxsize_t capacity_>
hxinline hxattr_flatten void hxvector<T_, capacity_>::erase_unordered(const T_* pos_) noexcept {
	// Prevent reloading end and annotate known not to partially overlap.
	T_* hxrestrict end_ = --m_end_;
	hxassert_hard(pos_ >= this->data() && pos_ <= end_, "invalid_iterator");
	if(pos_ != end_) {
		// Having a non-const this pointer provides valid write access.
		*const_cast<T_*>(pos_) = hxmove(*end_);
	}
	end_->T_::~T_();
}

template<hxvector_concept_ T_, hxsize_t capacity_>
hxinline hxattr_flatten void hxvector<T_, capacity_>::erase_unordered(hxsize_t index_) noexcept {
	hxassert_hard(static_cast<size_t>(index_) < static_cast<size_t>(this->size()), "invalid_index %zu", index_);
	this->erase_unordered(this->data() + index_);
}

template<hxvector_concept_ T_, hxsize_t capacity_>
hxinline hxattr_flatten const T_* hxvector<T_, capacity_>::find(const T_& value_) const {
	for(const T_* it_ = this->data(), *const end_ = m_end_; it_ != end_; ++it_) {
		if(hxkey_equal(*it_, value_)) {
			return it_;
		}
	}
	return m_end_;
}

template<hxvector_concept_ T_, hxsize_t capacity_>
hxinline hxattr_flatten T_* hxvector<T_, capacity_>::find(const T_& value_) {
	return const_cast<T_*>(const_cast<const hxvector<T_, capacity_>*>(this)->find(value_));
}

template<hxvector_concept_ T_, hxsize_t capacity_>
template<typename callable_t_>
hxinline hxattr_flatten const T_* hxvector<T_, capacity_>::find_if(callable_t_&& callable_) const {
	for(const T_* it_ = this->data(), *const end_ = m_end_; it_ != end_; ++it_) {
		if(hxforward<callable_t_>(callable_)(*it_)) {
			return it_;
		}
	}
	return m_end_;
}

template<hxvector_concept_ T_, hxsize_t capacity_>
template<typename callable_t_>
hxinline hxattr_flatten T_* hxvector<T_, capacity_>::find_if(callable_t_&& callable_) {
	for(T_* it_ = this->data(), *const end_ = m_end_; it_ != end_; ++it_) {
		if(hxforward<callable_t_>(callable_)(*it_)) {
			return it_;
		}
	}
	return m_end_;
}

template<hxvector_concept_ T_, hxsize_t capacity_>
template<typename callable_t_>
hxinline hxattr_flatten void hxvector<T_, capacity_>::for_each(callable_t_&& callable_) const {
	for(const T_* it_ = this->data(), *const end_ = m_end_; it_ != end_; ++it_) {
		hxforward<callable_t_>(callable_)(*it_);
	}
}

template<hxvector_concept_ T_, hxsize_t capacity_>
template<typename callable_t_>
hxinline hxattr_flatten void hxvector<T_, capacity_>::for_each(callable_t_&& callable_) {
	for(T_* it_ = this->data(), *const end_ = m_end_; it_ != end_; ++it_) {
		hxforward<callable_t_>(callable_)(*it_);
	}
}

template<hxvector_concept_ T_, hxsize_t capacity_>
hxinline hxattr_flatten const T_& hxvector<T_, capacity_>::front(void) const {
	hxassert_hard(!this->empty(), "invalid_reference");
	return *this->data();
}

template<hxvector_concept_ T_, hxsize_t capacity_>
hxinline hxattr_flatten T_& hxvector<T_, capacity_>::front(void) {
	hxassert_hard(!this->empty(), "invalid_reference");
	return *this->data();
}

template<hxvector_concept_ T_, hxsize_t capacity_>
template<typename callable_t_>
hxinline hxattr_flatten void hxvector<T_, capacity_>::generate_n(hxsize_t size_, callable_t_&& callable_) noexcept {
	hxassert_hard(this->size() + size_ <= this->capacity(), "stack_overflow");
	T_* hxrestrict dst_ = m_end_;
	while(size_--) {
		::new(dst_++) T_(hxforward<callable_t_>(callable_)());
	}
	m_end_ = dst_;
}

template<hxvector_concept_ T_, hxsize_t capacity_>
hxinline hxattr_flatten const T_* hxvector<T_, capacity_>::get(hxsize_t index_) const {
	// Casting a signed index is well defined. Comparing pointers would be undefined.
	return index_ < this->size() ? this->data() + index_ : hxnull;
}

template<hxvector_concept_ T_, hxsize_t capacity_>
hxinline hxattr_flatten T_* hxvector<T_, capacity_>::get(hxsize_t index_) {
	// Casting a signed index is well defined. Comparing pointers would be undefined.
	return index_ < this->size() ? this->data() + index_ : hxnull;
}

template<hxvector_concept_ T_, hxsize_t capacity_>
template<typename ref_t_>
hxinline hxattr_flatten void hxvector<T_, capacity_>::insert(const T_* pos_, ref_t_&& x_) noexcept {
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
			*it_ = hxmove(it_[-1]);
		}
		*it_ = hxforward<ref_t_>(x_);
	}
}

template<hxvector_concept_ T_, hxsize_t capacity_>
template<typename ref_t_>
hxinline hxattr_flatten void hxvector<T_, capacity_>::insert(hxsize_t index_, ref_t_&& x_) noexcept {
	hxassert_hard(static_cast<size_t>(index_) <= static_cast<size_t>(this->size()), "invalid_index %zu", index_);
	this->insert(this->data() + index_, hxforward<ref_t_>(x_));
}

template<hxvector_concept_ T_, hxsize_t capacity_>
hxinline hxattr_flatten void hxvector<T_, capacity_>::insertion_sort(void) noexcept {
	hxinsertion_sort<T_*>(this->data(), m_end_, hxkey_less_t<T_>{});
}

template<hxvector_concept_ T_, hxsize_t capacity_>
template<hxsize_t capacity_x_>
hxinline hxattr_flatten bool hxvector<T_, capacity_>::less(const hxvector<T_, capacity_x_>& x_) const {
	const hxsize_t size_ = hxmin(this->size(), x_.size());
	for(const T_* it0_ = this->data(), *it1_ = x_.data(), *const end_ = it0_ + size_;
			it0_ != end_; ++it0_, ++it1_) {
		// Use `a == b` instead of `a < b && b < a` for performance.
		if(!hxkey_equal(*it0_, *it1_)) {
			return hxkey_less(*it0_, *it1_);
		}
	}
	// Order the prefix before the other.
	return this->size() < x_.size();
}

template<hxvector_concept_ T_, hxsize_t capacity_>
hxinline void hxvector<T_, capacity_>::make_heap(void) noexcept {
	hxdetail_::hxmake_heap_<T_*>(this->data(), m_end_, hxkey_less_t<T_>{});
}

template<hxvector_concept_ T_, hxsize_t capacity_>
template<hxsize_t capacity_x_>
hxinline hxattr_flatten void hxvector<T_, capacity_>::memcpy(const hxvector<T_, capacity_x_>& x_) {
	hxassert_hard(static_cast<const void*>(this) != static_cast<const void*>(&x_),
		"invalid_reference");
	this->resize(x_.size());
	::memcpy(static_cast<void*>(this->data()), x_.data(), static_cast<size_t>(x_.size_bytes()));
}

template<hxvector_concept_ T_, hxsize_t capacity_>
hxinline hxattr_flatten void hxvector<T_, capacity_>::memset(int byte_) {
	::memset(static_cast<void*>(this->data()), byte_, static_cast<size_t>(this->size_bytes()));
}

template<hxvector_concept_ T_, hxsize_t capacity_>
hxinline hxattr_flatten void hxvector<T_, capacity_>::pop_back(void) noexcept {
	hxassert_hard(!this->empty(), "stack_underflow");
	(--m_end_)->T_::~T_();
}

template<hxvector_concept_ T_, hxsize_t capacity_>
inline hxattr_flatten void hxvector<T_, capacity_>::pop_heap(void) noexcept {
	hxassert_hard(!this->empty(), "stack_underflow");
	T_*const begin_ = this->data();
	T_*const end_ = m_end_ - 1;
	m_end_ = end_;
	if(begin_ == end_) {
		begin_->T_::~T_();
		return;
	}
	*begin_ = hxmove(*end_);
	end_->T_::~T_();
	hxdetail_::hxheapsort_heapify_(begin_, begin_, end_, hxkey_less_t<T_>{});
}

template<hxvector_concept_ T_, hxsize_t capacity_>
template<typename... args_t_>
hxinline hxattr_flatten T_& hxvector<T_, capacity_>::push_back(args_t_&&... args_) noexcept {
	hxassert_hard(!this->full(), "stack_overflow");
	return *::new(m_end_++) T_(hxforward<args_t_>(args_)...);
}

// Fixes gcc + optimizer + sanitizer -Wmaybe-uninitialized bug.
#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#endif

template<hxvector_concept_ T_, hxsize_t capacity_>
template<typename ref_t_>
inline hxattr_flatten T_& hxvector<T_, capacity_>::push_heap(ref_t_&& arg_) noexcept {
	T_*const begin_ = this->data();
	T_* node_ = static_cast<T_*>(this->push_back_unconstructed_());
	hxsize_t index_ = node_ - begin_;
	if(index_ == 0) {
		::new(node_) T_(hxforward<ref_t_>(arg_));
		return *node_;
	}
	index_ = (index_ - 1) >> 1;
	T_* parent_ = begin_ + index_;
	if(!hxkey_less(*parent_, arg_)) {
		::new(node_) T_(hxforward<ref_t_>(arg_));
		return *node_;
	}
	::new(node_) T_(hxmove(*parent_));
	node_ = parent_;
	while(index_ != 0) {
		index_ = (index_ - 1) >> 1;
		parent_ = begin_ + index_;
		if(!hxkey_less(*parent_, arg_)) {
			break;
		}
		*node_ = hxmove(*parent_);
		node_ = parent_;
	}
	*node_ = hxforward<ref_t_>(arg_);
	return *node_;
}

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic pop
#endif

template<hxvector_concept_ T_, hxsize_t capacity_>
hxinline hxattr_flatten void hxvector<T_, capacity_>::reserve(hxsize_t size_, hxsystem_allocator_t allocator_,
		hxalignment_t alignment_) {
	if(size_ > this->capacity()) {
		// reserve_storage asserts unallocated or size is equivalent.
		this->reserve_storage(size_, allocator_, alignment_);
		hxassertmsg(m_end_ == hxnull, "internal_error");
		m_end_ = this->data();
	}
}

template<hxvector_concept_ T_, hxsize_t capacity_>
hxinline hxattr_flatten void hxvector<T_, capacity_>::resize(hxsize_t size_) noexcept {
	this->reserve(size_);
	T_* it_ = m_end_;
	T_*const end_ = this->data() + size_;
	while(it_ < end_) {
		// Note: "T_()" is not being called. That would default initialize
		// arrays of integers to zero.
		::new(it_++) T_;
	}
	this->destruct_(end_, it_);
	m_end_ = end_;
}

template<hxvector_concept_ T_, hxsize_t capacity_>
hxinline hxattr_flatten void hxvector<T_, capacity_>::resize(hxsize_t size_, const T_& x_) noexcept {
	this->reserve(size_);
	T_* hxrestrict it_ = m_end_;
	T_*const end_ = this->data() + size_;
	while(it_ < end_) {
		::new(it_++) T_(x_);
	}
	this->destruct_(end_, it_);
	m_end_ = end_;
}

template<hxvector_concept_ T_, hxsize_t capacity_>
hxinline hxattr_flatten void hxvector<T_, capacity_>::sort(void) noexcept {
	hxsort<T_*>(this->data(), m_end_, hxkey_less_t<T_>{});
}

template<hxvector_concept_ T_, hxsize_t capacity_>
hxinline hxattr_flatten void hxvector<T_, capacity_>::swap(hxvector& x_) noexcept {
	static_assert(capacity_ == hxallocator_dynamic_capacity,
		"Dynamic capacity required for hxvector::swap");
	hxswap_memcpy(*this, x_);
}

template<hxvector_concept_ T_, hxsize_t capacity_>
hxinline hxattr_flatten void* hxvector<T_, capacity_>::push_back_unconstructed_(void) {
	hxassert_hard(!this->full(), "stack_overflow");
	return static_cast<void*>(m_end_++);
}

template<hxvector_concept_ T_, hxsize_t capacity_>
hxinline hxattr_flatten void hxvector<T_, capacity_>::destruct_(T_* it_, T_* end_) noexcept {
	while(it_ != end_) {
		it_++->T_::~T_();
	}
}

HX_END_INL_
#endif // HX_DOXYGEN_PARSER
