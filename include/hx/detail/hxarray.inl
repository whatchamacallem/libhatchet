#pragma once
// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

#ifndef LIBHATCHET_VER
#error Internal. Do not include this file directly.
#endif

#ifndef HX_DOXYGEN_PARSER
HX_INL_BEGIN_

template<hxarray_concept_ T_, hxsize_t capacity_>
hxinline hxattr_flatten hxarray<T_, capacity_>::hxarray(void) noexcept {
	hxif_constexpr(capacity_ != hxallocator_dynamic_capacity) {
		T_* it_ = this->data();
		for(const T_*const end_ = it_ + capacity_; it_ != end_; ++it_) {
			::new(it_) T_;
		}
	}
}

template<hxarray_concept_ T_, hxsize_t capacity_>
hxinline hxattr_flatten hxarray<T_, capacity_>::hxarray(const T_& x_) noexcept {
	static_assert(capacity_ > hxallocator_dynamic_capacity,
		"A known capacity is required for value initialization");
	T_* hxrestrict it_ = this->data();
	for(const T_*const end_ = it_ + capacity_; it_ != end_; ++it_) {
		::new(it_) T_(x_);
	}
}

template<hxarray_concept_ T_, hxsize_t capacity_>
hxinline hxattr_flatten hxarray<T_, capacity_>::hxarray(const hxarray& x_) noexcept {
	const hxsize_t c_ = x_.capacity();
	hxif_constexpr(capacity_ == hxallocator_dynamic_capacity) {
		this->reserve_storage(c_);
	}
	const T_* hxrestrict src_ = x_.data();
	T_* hxrestrict dst_ = this->data();
	for(const T_*const end_ = dst_ + c_; dst_ != end_; ++dst_, ++src_) {
		::new(dst_) T_(*src_);
	}
}

template<hxarray_concept_ T_, hxsize_t capacity_>
hxinline hxattr_flatten hxarray<T_, capacity_>::hxarray(hxarray&& x_) noexcept {
	static_assert(capacity_ == hxallocator_dynamic_capacity,
		"hxallocator_dynamic_capacity required for temporaries");
	::memcpy(static_cast<void*>(this), &x_, sizeof x_); // NOLINT(bugprone-undefined-memory-manipulation)
	::memset(static_cast<void*>(&x_), 0x00, sizeof x_);
}

template<hxarray_concept_ T_, hxsize_t capacity_>
template<typename other_value_t_, hxsize_t array_length_>
hxinline hxattr_flatten hxarray<T_, capacity_>::hxarray(
		const other_value_t_(&array_)[array_length_]) noexcept {
	// Will assert on a size mismatch.
	this->reserve_storage(array_length_);
	const other_value_t_* hxrestrict src_ = +array_;
	T_* hxrestrict dst_ = this->data();
	for(const T_*const end_ = dst_ + array_length_; dst_ != end_; ++dst_, ++src_) {
		::new(dst_) T_(*src_);
	}
}

template<hxarray_concept_ T_, hxsize_t capacity_>
template<typename other_value_t_>
hxinline hxattr_flatten hxarray<T_, capacity_>::hxarray(
		std::initializer_list<other_value_t_> x_) noexcept {
	// Will assert on a size mismatch.
	this->reserve_storage(static_cast<hxsize_t>(x_.size()));
	const other_value_t_* hxrestrict src_ = x_.begin();
	T_* hxrestrict dst_ = this->data();
	for(const T_*const end_ = dst_ + x_.size(); dst_ != end_; ++dst_, ++src_) {
		::new(dst_) T_(*src_);
	}
}

#if HX_CPLUSPLUS >= 202002L
template<hxarray_concept_ T_, hxsize_t capacity_>
template<hxrange_concept_ range_t_>
requires(!hxis_same<hxremove_cvref_t<range_t_>, hxarray<T_, capacity_> >())
hxinline hxattr_flatten hxarray<T_, capacity_>::hxarray(range_t_& range_) noexcept {
	hxrestrict_t<decltype(range_.begin())> src_(range_.begin());
	const auto end_ = range_.end();
	static_assert(capacity_ != hxallocator_dynamic_capacity
			|| requires(const decltype(src_)& a_, const decltype(src_)& b_) { { b_ - a_ }; },
		"hxallocator_dynamic_capacity requires a range supporting subtraction");
	if constexpr(requires(const decltype(src_)& a_, const decltype(src_)& b_) { { b_ - a_ }; }) {
		const hxsize_t size_ = static_cast<hxsize_t>(end_ - src_);
		this->reserve_storage(size_);
		T_* hxrestrict dst_ = this->data();
		for(; src_ != end_; ++dst_, ++src_) {
			::new(dst_) T_(*src_);
		}
	}
	else {
		T_* hxrestrict dst_ = this->data();
		for(const T_*const dst_end_ = dst_ + this->capacity();  src_ != end_ && dst_ != dst_end_; ++dst_, ++src_) {
			::new(dst_) T_(*src_);
		}
		hxassert_hard(src_ == end_ && dst_ == this->end(), "array_size mismatch %zd", this->capacity());
	}
}

template<hxarray_concept_ T_, hxsize_t capacity_>
template<hxrange_concept_ range_t_>
requires(!hxis_lvalue_reference<range_t_>()
		&& !hxis_same<hxremove_cvref_t<range_t_>, hxarray<T_, capacity_> >())
hxinline hxattr_flatten hxarray<T_, capacity_>::hxarray(range_t_&& range_) noexcept {
	hxrestrict_t<decltype(range_.begin())> src_(range_.begin());
	const auto end_ = range_.end();
	static_assert(capacity_ != hxallocator_dynamic_capacity
			|| requires(const decltype(src_)& a_, const decltype(src_)& b_) { { b_ - a_ }; },
		"hxallocator_dynamic_capacity requires a range supporting subtraction");
	if constexpr(requires(const decltype(src_)& a_, const decltype(src_)& b_) { { b_ - a_ }; }) {
		const hxsize_t size_ = static_cast<hxsize_t>(end_ - src_);
		this->reserve_storage(size_);
		T_* hxrestrict dst_ = this->data();
		for(; src_ != end_; ++dst_, ++src_) {
			::new(dst_) T_(hxmove(*src_));
		}
	}
	else {
		T_* hxrestrict dst_ = this->data();
		for(const T_*const dst_end_ = dst_ + this->capacity();  src_ != end_ && dst_ != dst_end_; ++dst_, ++src_) {
			::new(dst_) T_(hxmove(*src_));
		}
		hxassert_hard(src_ == end_ && dst_ == this->end(), "array_size mismatch %zd", this->capacity());
	}
}
#endif

template<hxarray_concept_ T_, hxsize_t capacity_>
hxinline hxattr_flatten hxarray<T_, capacity_>::~hxarray(void) noexcept {
	this->destruct_(this->data(), this->data() + this->capacity());
}

#if HX_CPLUSPLUS >= 202302L
template<hxarray_concept_ T_, hxsize_t capacity_>
template<typename self_t_, typename callable_t_>
hxinline hxattr_flatten auto hxarray<T_, capacity_>::and_then(
		this self_t_&& self_, hxsize_t index_, callable_t_&& callable_)
		-> hxremove_cvref_t<decltype(
			hxforward<callable_t_>(callable_)(
				hxforward_like<self_t_>(hxdeclval<T_&>())))> {
	if(static_cast<size_t>(index_) < static_cast<size_t>(self_.size())) {
		auto& value_ = self_.data()[index_];
		return hxforward<callable_t_>(callable_)(hxforward_like<self_t_>(value_));
	}
	return hxnil;
}

template<hxarray_concept_ T_, hxsize_t capacity_>
template<typename self_t_, typename callable_t_>
hxinline hxattr_flatten auto hxarray<T_, capacity_>::and_then(
		this self_t_&& self_, const T_* it_, callable_t_&& callable_)
		-> hxremove_cvref_t<decltype(
			hxforward<callable_t_>(callable_)(hxforward_like<self_t_>(
				hxdeclval<const T_&>())))> {
	if(it_ != self_.end()) {
		const T_& value_ = *it_;
		return hxforward<callable_t_>(callable_)(hxforward_like<self_t_>(value_));
	}
	return hxnil;
}
#endif // HX_CPLUSPLUS >= 202302L

template<hxarray_concept_ T_, hxsize_t capacity_>
hxinline hxattr_flatten void hxarray<T_, capacity_>::operator=(const hxarray& x_) noexcept {
	hxassertf(static_cast<const void*>(this) != static_cast<const void*>(&x_), "bad_ref");
	hxif_constexpr(capacity_ == hxallocator_dynamic_capacity) {
		if(this->capacity() == 0) {
			const hxsize_t c_ = x_.capacity();
			this->reserve_storage(c_);
			const T_* hxrestrict src_ = x_.data();
			T_* hxrestrict dst_ = this->data();
			for(const T_*const end_ = dst_ + c_; dst_ != end_; ++dst_, ++src_) {
				::new(dst_) T_(*src_);
			}
			return;
		}
	}
	hxassert_hard(this->capacity() == x_.capacity(),
		"array_size mismatch %zd %zd", this->capacity(), x_.capacity());
	const T_* hxrestrict src_ = x_.data();
	T_* hxrestrict dst_ = this->data();
	for(const T_*const end_ = dst_ + this->capacity(); dst_ != end_; ++dst_, ++src_) {
		*dst_ = *src_;
	}
}

template<hxarray_concept_ T_, hxsize_t capacity_>
hxinline hxattr_flatten void hxarray<T_, capacity_>::operator=(hxarray&& x_) noexcept {
	static_assert(capacity_ == hxallocator_dynamic_capacity,
		"hxallocator_dynamic_capacity required for temporaries");
	hxassertf(static_cast<const void*>(this) != static_cast<const void*>(&x_), "bad_ref");
	this->swap(x_);
}

template<hxarray_concept_ T_, hxsize_t capacity_>
template<typename other_value_t_, hxsize_t array_length_>
hxinline hxattr_flatten void hxarray<T_, capacity_>::operator=(
		const other_value_t_(&array_)[array_length_]) noexcept {
	hxif_constexpr(capacity_ == hxallocator_dynamic_capacity) {
		if(this->capacity() == 0) {
			this->reserve_storage(array_length_);
			const other_value_t_* hxrestrict src_ = +array_;
			T_* hxrestrict dst_ = this->data();
			for(const T_*const end_ = dst_ + array_length_; dst_ != end_; ++dst_, ++src_) {
				::new(dst_) T_(*src_);
			}
			return;
		}
	}
	hxassert_hard(this->capacity() == array_length_,
		"array_size mismatch %zd %zd", this->capacity(), hxsize_t{array_length_});
	const other_value_t_* hxrestrict src_ = +array_;
	T_* hxrestrict dst_ = this->data();
	for(const T_*const end_ = dst_ + this->capacity(); dst_ != end_; ++dst_, ++src_) {
		*dst_ = *src_;
	}
}

template<hxarray_concept_ T_, hxsize_t capacity_>
hxinline hxattr_flatten const T_& hxarray<T_, capacity_>::operator[](hxsize_t index_) const {
	hxassert_hard(static_cast<size_t>(index_) < static_cast<size_t>(this->capacity()),
		"bad_index %zd", index_);
	return this->data()[index_];
}

template<hxarray_concept_ T_, hxsize_t capacity_>
hxinline hxattr_flatten T_& hxarray<T_, capacity_>::operator[](hxsize_t index_) {
	hxassert_hard(static_cast<size_t>(index_) < static_cast<size_t>(this->capacity()),
		"bad_index %zd", index_);
	return this->data()[index_];
}

template<hxarray_concept_ T_, hxsize_t capacity_>
template<typename callable_t_>
hxinline hxattr_flatten bool hxarray<T_, capacity_>::all_of(callable_t_&& callable_) const {
	for(const T_* it_ = this->data(), *const end_ = it_ + this->capacity(); it_ != end_; ++it_) {
		if(!hxforward<callable_t_>(callable_)(*it_)) {
			return false;
		}
	}
	return true;
}

template<hxarray_concept_ T_, hxsize_t capacity_>
template<typename callable_t_>
hxinline hxattr_flatten bool hxarray<T_, capacity_>::all_of(callable_t_&& callable_) {
	for(T_* it_ = this->data(), *const end_ = it_ + this->capacity(); it_ != end_; ++it_) {
		if(!hxforward<callable_t_>(callable_)(*it_)) {
			return false;
		}
	}
	return true;
}

template<hxarray_concept_ T_, hxsize_t capacity_>
template<typename callable_t_>
hxinline hxattr_flatten bool hxarray<T_, capacity_>::any_of(callable_t_&& callable_) const {
	for(const T_* it_ = this->data(), *const end_ = it_ + this->capacity(); it_ != end_; ++it_) {
		if(hxforward<callable_t_>(callable_)(*it_)) {
			return true;
		}
	}
	return false;
}

template<hxarray_concept_ T_, hxsize_t capacity_>
template<typename callable_t_>
hxinline hxattr_flatten bool hxarray<T_, capacity_>::any_of(callable_t_&& callable_) {
	for(T_* it_ = this->data(), *const end_ = it_ + this->capacity(); it_ != end_; ++it_) {
		if(hxforward<callable_t_>(callable_)(*it_)) {
			return true;
		}
	}
	return false;
}

template<hxarray_concept_ T_, hxsize_t capacity_>
hxinline hxattr_flatten const T_* hxarray<T_, capacity_>::binary_search(const T_& value_) const {
	return hxbinary_search(*this, value_, hxkey_less_t<const T_&>{});
}

template<hxarray_concept_ T_, hxsize_t capacity_>
hxinline hxattr_flatten T_* hxarray<T_, capacity_>::binary_search(const T_& value_) {
	return hxbinary_search(*this, value_, hxkey_less_t<const T_&>{});
}

template<hxarray_concept_ T_, hxsize_t capacity_>
hxinline hxattr_flatten hxsize_t hxarray<T_, capacity_>::capacity(void) const {
	return hxallocator<T_, capacity_>::capacity();
}

template<hxarray_concept_ T_, hxsize_t capacity_>
hxinline hxattr_flatten bool hxarray<T_, capacity_>::equal(const hxarray& x_) const {
	const hxsize_t c_ = this->capacity();
	if(c_ != x_.capacity()) { return false; }
	for(const T_* it0_ = this->data(), *it1_ = x_.data(), *const end_ = it0_ + c_;
			it0_ != end_; ++it0_, ++it1_) {
		if(!hxkey_equal(*it0_, *it1_)) {
			return false;
		}
	}
	return true;
}

template<hxarray_concept_ T_, hxsize_t capacity_>
hxinline hxattr_flatten const T_* hxarray<T_, capacity_>::find(const T_& value_) const {
	const T_* it_ = this->data();
	const T_*const end_ = it_ + this->capacity();
	for(; it_ != end_; ++it_) {
		if(hxkey_equal(*it_, value_)) {
			return it_;
		}
	}
	return end_;
}

template<hxarray_concept_ T_, hxsize_t capacity_>
hxinline hxattr_flatten T_* hxarray<T_, capacity_>::find(const T_& value_) {
	return const_cast<T_*>(const_cast<const hxarray<T_, capacity_>*>(this)->find(value_));
}

template<hxarray_concept_ T_, hxsize_t capacity_>
template<typename callable_t_>
hxinline hxattr_flatten const T_* hxarray<T_, capacity_>::find_if(callable_t_&& callable_) const {
	const T_* it_ = this->data();
	const T_*const end_ = it_ + this->capacity();
	for(; it_ != end_; ++it_) {
		if(hxforward<callable_t_>(callable_)(*it_)) {
			return it_;
		}
	}
	return end_;
}

template<hxarray_concept_ T_, hxsize_t capacity_>
template<typename callable_t_>
hxinline hxattr_flatten T_* hxarray<T_, capacity_>::find_if(callable_t_&& callable_) {
	T_* it_ = this->data();
	T_*const end_ = it_ + this->capacity();
	for(; it_ != end_; ++it_) {
		if(hxforward<callable_t_>(callable_)(*it_)) {
			return it_;
		}
	}
	return end_;
}

template<hxarray_concept_ T_, hxsize_t capacity_>
template<typename callable_t_>
hxinline hxattr_flatten void hxarray<T_, capacity_>::for_each(callable_t_&& callable_) const {
	for(const T_* it_ = this->data(), *const end_ = it_ + this->capacity(); it_ != end_; ++it_) {
		hxforward<callable_t_>(callable_)(*it_);
	}
}

template<hxarray_concept_ T_, hxsize_t capacity_>
template<typename callable_t_>
hxinline hxattr_flatten void hxarray<T_, capacity_>::for_each(callable_t_&& callable_) {
	for(T_* it_ = this->data(), *const end_ = it_ + this->capacity(); it_ != end_; ++it_) {
		hxforward<callable_t_>(callable_)(*it_);
	}
}

template<hxarray_concept_ T_, hxsize_t capacity_>
hxinline hxattr_flatten hxhash_t hxarray<T_, capacity_>::hash(void) const {
	hxhash_t h_ = hxhash_prime5_;
	for(const T_& element_ : *this) {
		h_ += hxkey_hash(element_) * hxhash_prime5_;
		h_ = ((h_ << 11u) | (h_ >> 21u)) * hxhash_prime1_;
	}
	return hxhash_avalanche_(h_);
}

template<hxarray_concept_ T_, hxsize_t capacity_>
hxinline hxattr_flatten void hxarray<T_, capacity_>::insertion_sort(void) noexcept {
	hxinsertion_sort<T_*>(this->data(), this->data() + this->capacity(), hxkey_less_t<T_>{});
}

template<hxarray_concept_ T_, hxsize_t capacity_>
hxinline hxattr_flatten bool hxarray<T_, capacity_>::less(const hxarray& x_) const {
	const hxsize_t c_ = this->capacity();
	const hxsize_t nx_ = x_.capacity();
	const hxsize_t min_ = c_ < nx_ ? c_ : nx_;
	for(const T_* it0_ = this->data(), *it1_ = x_.data(), *const end_ = it0_ + min_;
			it0_ != end_; ++it0_, ++it1_) {
		if(!hxkey_equal(*it0_, *it1_)) {
			return hxkey_less(*it0_, *it1_);
		}
	}
	return c_ < nx_;
}

template<hxarray_concept_ T_, hxsize_t capacity_>
hxinline hxattr_flatten void hxarray<T_, capacity_>::memcpy(const hxarray& x_) {
	hxassertf(static_cast<const void*>(this) != static_cast<const void*>(&x_), "bad_ref");
	hxassert_hard(this->capacity() == x_.capacity(),
		"array_size mismatch %zd %zd", this->capacity(), x_.capacity());
	::memcpy(static_cast<void*>(this->data()), x_.data(), static_cast<size_t>(this->size_bytes()));
}

template<hxarray_concept_ T_, hxsize_t capacity_>
hxinline hxattr_flatten void hxarray<T_, capacity_>::memset(int byte_) {
	::memset(static_cast<void*>(this->data()), byte_, static_cast<size_t>(this->size_bytes()));
}

#if HX_CPLUSPLUS >= 202302L
template<hxarray_concept_ T_, hxsize_t capacity_>
template<typename self_t_, typename callable_t_>
hxinline hxattr_flatten auto hxarray<T_, capacity_>::or_else(
		this self_t_&& self_, hxsize_t index_, callable_t_&& callable_)
		-> decltype(self_.end()) {
	if(static_cast<size_t>(index_) < static_cast<size_t>(self_.size())) {
		return self_.data() + index_;
	}
	return hxforward<callable_t_>(callable_)();
}

template<hxarray_concept_ T_, hxsize_t capacity_>
template<typename self_t_, typename callable_t_>
hxinline hxattr_flatten auto hxarray<T_, capacity_>::or_else(
		this self_t_&& self_, const T_* it_, callable_t_&& callable_)
		-> decltype(self_.end()) {
	if(it_ != self_.end()) {
		return it_;
	}
	return hxforward<callable_t_>(callable_)();
}
#endif // HX_CPLUSPLUS >= 202302L

template<hxarray_concept_ T_, hxsize_t capacity_>
hxinline hxattr_flatten void hxarray<T_, capacity_>::reserve(hxsize_t size_,
		hxsystem_allocator_t allocator_, hxalignment_t alignment_) noexcept {
	const hxsize_t c_ = this->capacity();
	this->reserve_storage(size_, allocator_, alignment_);
	hxif_constexpr(capacity_ == hxallocator_dynamic_capacity) {
		if(c_ == 0) {
			T_* it_ = this->data();
			for(const T_*const end_ = it_ + size_; it_ != end_; ++it_) {
				// Does not zero-fill.
				::new(it_) T_;
			}
		}
	}
}

template<hxarray_concept_ T_, hxsize_t capacity_>
hxinline hxattr_flatten void hxarray<T_, capacity_>::sort(void) noexcept {
	hxsort<T_*>(this->data(), this->data() + this->capacity(), hxkey_less_t<T_>{});
}

template<hxarray_concept_ T_, hxsize_t capacity_>
hxinline hxattr_flatten void hxarray<T_, capacity_>::swap(hxarray& x_) noexcept {
	static_assert(capacity_ == hxallocator_dynamic_capacity,
		"Dynamic capacity required for hxarray::swap");
	hxswap_memcpy(*this, x_);
}

#if HX_CPLUSPLUS >= 202302L
template<hxarray_concept_ T_, hxsize_t capacity_>
template<typename self_t_, typename... args_t_>
hxinline hxattr_flatten T_ hxarray<T_, capacity_>::value_or(
		this self_t_&& self_, hxsize_t index_, args_t_&&... args_) {
	if(static_cast<size_t>(index_) < static_cast<size_t>(self_.size())) {
		auto& value_ = self_.data()[index_];
		return static_cast<T_>(hxforward_like<self_t_>(value_));
	}
	return T_(hxforward<args_t_>(args_)...);
}

template<hxarray_concept_ T_, hxsize_t capacity_>
template<typename self_t_, typename... args_t_>
hxinline hxattr_flatten T_ hxarray<T_, capacity_>::value_or(
		this self_t_&& self_, const T_* it_, args_t_&&... args_) {
	if(it_ != self_.end()) {
		const T_& value_ = *it_;
		return static_cast<T_>(hxforward_like<self_t_>(value_));
	}
	return T_(hxforward<args_t_>(args_)...);
}
#endif // HX_CPLUSPLUS >= 202302L

template<hxarray_concept_ T_, hxsize_t capacity_>
hxinline hxattr_flatten void hxarray<T_, capacity_>::destruct_(T_* it_, T_* end_) noexcept {
	while(it_ != end_) {
		it_++->T_::~T_();
	}
}

HX_INL_END_
#endif // HX_DOXYGEN_PARSER
