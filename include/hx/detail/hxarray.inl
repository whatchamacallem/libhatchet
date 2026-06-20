#pragma once
// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

#ifndef LIBHATCHET_VER
#error Internal. Do not include this file directly.
#endif

#ifndef HX_DOXYGEN_PARSER
HX_BEGIN_INL_

template<hxarray_element_concept_ T_, hxsize_t capacity_>
hxarray<T_, capacity_>::hxarray(void) noexcept {
	T_* it_ = this->data();
	for(const T_*const end_ = it_ + capacity_; it_ != end_; ++it_) {
		::new(it_) T_;
	}
}

template<hxarray_element_concept_ T_, hxsize_t capacity_>
hxarray<T_, capacity_>::hxarray(const T_& x_) noexcept {
	static_assert(capacity_ > 0, "A known capacity is required for value initialization");
	T_* it_ = this->data();
	for(const T_*const end_ = it_ + capacity_; it_ != end_; ++it_) {
		::new(it_) T_(x_);
	}
}

template<hxarray_element_concept_ T_, hxsize_t capacity_>
hxarray<T_, capacity_>::hxarray(const hxarray& x_) noexcept {
	const hxsize_t n_ = x_.capacity();
	hxif_constexpr(capacity_ == 0) {
		this->reserve_storage(n_);
	}
	const T_* hxrestrict src_ = x_.data();
	T_* hxrestrict dst_ = this->data();
	for(const T_*const end_ = dst_ + n_; dst_ != end_; ++dst_, ++src_) {
		::new(dst_) T_(*src_);
	}
}

template<hxarray_element_concept_ T_, hxsize_t capacity_>
hxarray<T_, capacity_>::hxarray(hxarray&& x_) noexcept {
	static_assert(capacity_ == hxallocator_dynamic_capacity,
		"hxallocator_dynamic_capacity required for temporaries.");
	::memcpy(static_cast<void*>(this), &x_, sizeof x_); // NOLINT(bugprone-undefined-memory-manipulation)
	::memset(static_cast<void*>(&x_), 0x00, sizeof x_);
}

template<hxarray_element_concept_ T_, hxsize_t capacity_>
template<typename other_value_t_, hxsize_t array_length_>
hxarray<T_, capacity_>::hxarray(const other_value_t_(&array_)[array_length_]) noexcept {
	static_assert(capacity_ == 0 || capacity_ == array_length_, "array_length mismatch");
	hxif_constexpr(capacity_ == 0) {
		this->reserve_storage(array_length_);
	}
	const other_value_t_* hxrestrict src_ = array_ + 0;
	T_* hxrestrict dst_ = this->data();
	for(const T_*const end_ = dst_ + array_length_; dst_ != end_; ++dst_, ++src_) {
		::new(dst_) T_(*src_);
	}
}

template<hxarray_element_concept_ T_, hxsize_t capacity_>
template<typename other_value_t_>
hxarray<T_, capacity_>::hxarray(std::initializer_list<other_value_t_> x_) noexcept {
	hxassert_hard(capacity_ == 0 || capacity_ == x_.size(), "array_length mismatch");
	hxif_constexpr(capacity_ == 0) {
		this->reserve_storage(static_cast<hxsize_t>(x_.size()));
	}
	const other_value_t_* hxrestrict src_ = x_.begin();
	T_* hxrestrict dst_ = this->data();
	for(const T_*const end_ = dst_ + x_.size(); dst_ != end_; ++dst_, ++src_) {
		::new(dst_) T_(*src_);
	}
}

template<hxarray_element_concept_ T_, hxsize_t capacity_>
hxarray<T_, capacity_>::~hxarray(void) noexcept {
	this->destruct_(this->data(), this->data() + this->capacity());
}

template<hxarray_element_concept_ T_, hxsize_t capacity_>
void hxarray<T_, capacity_>::operator=(const hxarray& x_) noexcept {
	hxassertmsg(static_cast<const void*>(this) != static_cast<const void*>(&x_),
		"invalid_reference");
	this->assign(x_.begin(), x_.end());
}

template<hxarray_element_concept_ T_, hxsize_t capacity_>
template<typename other_value_t_, hxsize_t array_length_>
void hxarray<T_, capacity_>::operator=(const other_value_t_(&array_)[array_length_]) noexcept {
	this->assign(array_ + 0, array_ + array_length_);
}

template<hxarray_element_concept_ T_, hxsize_t capacity_>
const T_& hxarray<T_, capacity_>::operator[](hxsize_t index_) const {
	hxassert_hard(static_cast<size_t>(index_) < static_cast<size_t>(this->capacity()), "invalid_index %zu", index_);
	return this->data()[index_];
}

template<hxarray_element_concept_ T_, hxsize_t capacity_>
T_& hxarray<T_, capacity_>::operator[](hxsize_t index_) {
	hxassert_hard(static_cast<size_t>(index_) < static_cast<size_t>(this->capacity()), "invalid_index %zu", index_);
	return this->data()[index_];
}

template<hxarray_element_concept_ T_, hxsize_t capacity_>
template<typename callable_t_>
bool hxarray<T_, capacity_>::all_of(callable_t_&& fn_) const {
	for(const T_* it_ = this->data(), *const end_ = it_ + this->capacity(); it_ != end_; ++it_) {
		if(!hxforward<callable_t_>(fn_)(*it_)) {
			return false;
		}
	}
	return true;
}

template<hxarray_element_concept_ T_, hxsize_t capacity_>
template<typename callable_t_>
bool hxarray<T_, capacity_>::all_of(callable_t_&& fn_) {
	for(T_* it_ = this->data(), *const end_ = it_ + this->capacity(); it_ != end_; ++it_) {
		if(!hxforward<callable_t_>(fn_)(*it_)) {
			return false;
		}
	}
	return true;
}

template<hxarray_element_concept_ T_, hxsize_t capacity_>
template<typename callable_t_>
bool hxarray<T_, capacity_>::any_of(callable_t_&& fn_) const {
	for(const T_* it_ = this->data(), *const end_ = it_ + this->capacity(); it_ != end_; ++it_) {
		if(hxforward<callable_t_>(fn_)(*it_)) {
			return true;
		}
	}
	return false;
}

template<hxarray_element_concept_ T_, hxsize_t capacity_>
template<typename callable_t_>
bool hxarray<T_, capacity_>::any_of(callable_t_&& fn_) {
	for(T_* it_ = this->data(), *const end_ = it_ + this->capacity(); it_ != end_; ++it_) {
		if(hxforward<callable_t_>(fn_)(*it_)) {
			return true;
		}
	}
	return false;
}

template<hxarray_element_concept_ T_, hxsize_t capacity_>
template<typename iter_t_>
void hxarray<T_, capacity_>::assign(iter_t_ begin_, iter_t_ end_) noexcept {
	hxassertmsg(!(end_ < begin_), "invalid_range");
	const hxsize_t n_ = static_cast<hxsize_t>(end_ - begin_);
	hxassert_hard(this->capacity() == 0 || this->capacity() == n_, "array_length mismatch");
	hxif_constexpr(capacity_ == 0) {
		if(this->capacity() == 0) {
			this->reserve_storage(n_);
			T_* hxrestrict dst_ = this->data();
			for(iter_t_ src_(begin_); src_ != end_; ++dst_, ++src_) {
				::new(dst_) T_(*src_);
			}
			return;
		}
	}
	T_* hxrestrict dst_ = this->data();
	iter_t_ src_(begin_);
	while(src_ != end_) {
		*dst_++ = *src_++;
	}
}

#if HX_CPLUSPLUS >= 202002L
template<hxarray_element_concept_ T_, hxsize_t capacity_>
template<typename range_t_>
void hxarray<T_, capacity_>::assign_range(range_t_& range_) noexcept {
	this->assign(range_.begin(), range_.end());
}

template<hxarray_element_concept_ T_, hxsize_t capacity_>
template<typename range_t_>
requires(!hxis_lvalue_reference<range_t_>::value)
void hxarray<T_, capacity_>::assign_range(range_t_&& range_) noexcept {
	hxif_constexpr(capacity_ == 0) {
		if(this->capacity() == 0) {
			const hxsize_t n_ = static_cast<hxsize_t>(range_.end() - range_.begin());
			this->reserve_storage(n_);
			T_* hxrestrict dst_ = this->data();
			for(auto it_ = range_.begin(), rend_ = range_.end(); it_ != rend_; ++it_, ++dst_) {
				::new(dst_) T_(hxmove(*it_));
			}
			return;
		}
	}
	T_* dst_ = this->data();
	const T_*const end_ = this->data() + this->capacity();
	for(auto it_ = range_.begin(), rend_ = range_.end(); it_ != rend_; ++it_, ++dst_) {
		hxassertmsg(dst_ != end_, "array_length mismatch");
		*dst_ = hxmove(*it_);
	}
	hxassertmsg(dst_ == end_, "array_length mismatch"); (void)end_;
}
#endif

template<hxarray_element_concept_ T_, hxsize_t capacity_>
const T_* hxarray<T_, capacity_>::binary_search(const T_& value_) const {
	return hxbinary_search<const T_*>(this->data(), this->data() + this->capacity(),
		value_, hxkey_less_t<const T_&>{});
}

template<hxarray_element_concept_ T_, hxsize_t capacity_>
T_* hxarray<T_, capacity_>::binary_search(const T_& value_) {
	return hxbinary_search<T_*>(this->data(), this->data() + this->capacity(),
		value_, hxkey_less_t<const T_&>{});
}

template<hxarray_element_concept_ T_, hxsize_t capacity_>
bool hxarray<T_, capacity_>::equal(const hxarray& x_) const {
	const hxsize_t n_ = this->capacity();
	if(n_ != x_.capacity()) { return false; }
	for(const T_* it0_ = this->data(), *it1_ = x_.data(), *const end_ = it0_ + n_;
			it0_ != end_; ++it0_, ++it1_) {
		if(!hxkey_equal(*it0_, *it1_)) {
			return false;
		}
	}
	return true;
}

template<hxarray_element_concept_ T_, hxsize_t capacity_>
const T_* hxarray<T_, capacity_>::find(const T_& value_) const {
	const T_* it_ = this->data();
	const T_*const end_ = it_ + this->capacity();
	for(; it_ != end_; ++it_) {
		if(hxkey_equal(*it_, value_)) {
			return it_;
		}
	}
	return end_;
}

template<hxarray_element_concept_ T_, hxsize_t capacity_>
T_* hxarray<T_, capacity_>::find(const T_& value_) {
	T_* it_ = this->data();
	T_*const end_ = it_ + this->capacity();
	for(; it_ != end_; ++it_) {
		if(hxkey_equal(*it_, value_)) {
			return it_;
		}
	}
	return end_;
}

template<hxarray_element_concept_ T_, hxsize_t capacity_>
template<typename callable_t_>
const T_* hxarray<T_, capacity_>::find_if(callable_t_&& fn_) const {
	const T_* it_ = this->data();
	const T_*const end_ = it_ + this->capacity();
	for(; it_ != end_; ++it_) {
		if(hxforward<callable_t_>(fn_)(*it_)) {
			return it_;
		}
	}
	return end_;
}

template<hxarray_element_concept_ T_, hxsize_t capacity_>
template<typename callable_t_>
T_* hxarray<T_, capacity_>::find_if(callable_t_&& fn_) {
	T_* it_ = this->data();
	T_*const end_ = it_ + this->capacity();
	for(; it_ != end_; ++it_) {
		if(hxforward<callable_t_>(fn_)(*it_)) {
			return it_;
		}
	}
	return end_;
}

template<hxarray_element_concept_ T_, hxsize_t capacity_>
template<typename callable_t_>
void hxarray<T_, capacity_>::for_each(callable_t_&& fn_) const {
	for(const T_* it_ = this->data(), *end_ = it_ + this->capacity(); it_ != end_; ++it_) {
		hxforward<callable_t_>(fn_)(*it_);
	}
}

template<hxarray_element_concept_ T_, hxsize_t capacity_>
template<typename callable_t_>
void hxarray<T_, capacity_>::for_each(callable_t_&& fn_) {
	for(T_* it_ = this->data(), *const end_ = it_ + this->capacity(); it_ != end_; ++it_) {
		hxforward<callable_t_>(fn_)(*it_);
	}
}

template<hxarray_element_concept_ T_, hxsize_t capacity_>
const T_* hxarray<T_, capacity_>::get(hxsize_t index_) const {
	return index_ < this->capacity() ? this->data() + index_ : hxnull;
}

template<hxarray_element_concept_ T_, hxsize_t capacity_>
T_* hxarray<T_, capacity_>::get(hxsize_t index_) {
	return index_ < this->capacity() ? this->data() + index_ : hxnull;
}

template<hxarray_element_concept_ T_, hxsize_t capacity_>
void hxarray<T_, capacity_>::insertion_sort(void) noexcept {
	hxinsertion_sort<T_*>(this->data(), this->data() + this->capacity(), hxkey_less_t<T_>{});
}

template<hxarray_element_concept_ T_, hxsize_t capacity_>
bool hxarray<T_, capacity_>::less(const hxarray& x_) const {
	const hxsize_t n_ = this->capacity();
	const hxsize_t nx_ = x_.capacity();
	const hxsize_t min_ = n_ < nx_ ? n_ : nx_;
	for(const T_* hxrestrict it0_ = this->data(), *hxrestrict it1_ = x_.data(), *const end_ = it0_ + min_;
			it0_ != end_; ++it0_, ++it1_) {
		if(!hxkey_equal(*it0_, *it1_)) {
			return hxkey_less(*it0_, *it1_);
		}
	}
	return n_ < nx_;
}

template<hxarray_element_concept_ T_, hxsize_t capacity_>
void hxarray<T_, capacity_>::memcpy(const hxarray& x_) {
	hxassertmsg(static_cast<const void*>(this) != static_cast<const void*>(&x_),
		"invalid_reference");
	hxassert_hard(this->capacity() == x_.capacity(), "array_length mismatch");
	::memcpy(static_cast<void*>(this->data()), x_.data(), static_cast<size_t>(this->size_bytes()));
}

template<hxarray_element_concept_ T_, hxsize_t capacity_>
void hxarray<T_, capacity_>::memset(int byte_) {
	::memset(static_cast<void*>(this->data()), byte_, static_cast<size_t>(this->size_bytes()));
}

template<hxarray_element_concept_ T_, hxsize_t capacity_>
void hxarray<T_, capacity_>::reserve(hxsize_t size_,
		hxsystem_allocator_t allocator_, hxalignment_t alignment_) noexcept {
	const hxsize_t n_ = this->capacity();
	hxassert_hard(n_ == 0 || n_ == size_, "reserve can not resize");
	this->reserve_storage(size_, allocator_, alignment_);
	if(n_ == 0) {
		T_* it_ = this->data();
		for(const T_*const end_ = it_ + size_; it_ != end_; ++it_) {
			::new(it_) T_;
		}
	}
}

template<hxarray_element_concept_ T_, hxsize_t capacity_>
void hxarray<T_, capacity_>::sort(void) noexcept {
	hxsort<T_*>(this->data(), this->data() + this->capacity(), hxkey_less_t<T_>{});
}

template<hxarray_element_concept_ T_, hxsize_t capacity_>
void hxarray<T_, capacity_>::destruct_(T_* it_, T_* end_) noexcept {
	while(it_ != end_) {
		it_++->T_::~T_();
	}
}

HX_END_INL_
#endif // HX_DOXYGEN_PARSER
