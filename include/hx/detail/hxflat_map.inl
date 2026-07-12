#pragma once
// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

#ifndef LIBHATCHET_VER
#error Internal. Do not include this file directly.
#endif

#ifndef HX_DOXYGEN_PARSER
HX_BEGIN_INL_

template<typename key_t_, typename mapped_t_, typename compare_t_, bool multi_t_, hxsize_t capacity_>
hxinline auto hxflat_map<key_t_, mapped_t_, compare_t_, multi_t_, capacity_>::const_iterator::operator+=(
		hxsize_t n_) -> const_iterator& {
	hxassertmsg(m_map_ != hxnull, "invalid_iterator");
	m_index_ += n_;
	hxassertmsg(static_cast<size_t>(m_index_) <= static_cast<size_t>(m_map_->m_size_), "invalid_iterator");
	return *this;
}

template<typename key_t_, typename mapped_t_, typename compare_t_, bool multi_t_, hxsize_t capacity_>
hxinline hxsize_t hxflat_map<key_t_, mapped_t_, compare_t_, multi_t_, capacity_>::const_iterator::operator-(
		const const_iterator& x_) const {
	hxassertmsg(m_map_ != hxnull && m_map_ == x_.m_map_, "invalid_iterator");
	return m_index_ - x_.m_index_;
}

template<typename key_t_, typename mapped_t_, typename compare_t_, bool multi_t_, hxsize_t capacity_>
hxinline bool hxflat_map<key_t_, mapped_t_, compare_t_, multi_t_, capacity_>::const_iterator::operator==(
		const const_iterator& x_) const {
	hxassertmsg(m_map_ != hxnull && m_map_ == x_.m_map_, "invalid_iterator");
	return m_index_ == x_.m_index_;
}

template<typename key_t_, typename mapped_t_, typename compare_t_, bool multi_t_, hxsize_t capacity_>
hxinline bool hxflat_map<key_t_, mapped_t_, compare_t_, multi_t_, capacity_>::const_iterator::operator<(
		const const_iterator& x_) const {
	hxassertmsg(m_map_ != hxnull && m_map_ == x_.m_map_, "invalid_iterator");
	return m_index_ < x_.m_index_;
}

#if HX_CPLUSPLUS < 202002L
template<typename key_t_, typename mapped_t_, typename compare_t_, bool multi_t_, hxsize_t capacity_>
hxinline bool hxflat_map<key_t_, mapped_t_, compare_t_, multi_t_, capacity_>::const_iterator::operator!=(
		const const_iterator& x_) const {
	hxassertmsg(m_map_ != hxnull && m_map_ == x_.m_map_, "invalid_iterator");
	return m_index_ != x_.m_index_;
}
#endif

template<typename key_t_, typename mapped_t_, typename compare_t_, bool multi_t_, hxsize_t capacity_>
hxinline hxattr_flatten const key_t_& hxflat_map<key_t_, mapped_t_, compare_t_, multi_t_, capacity_>::const_iterator::key(void) const {
	hxassertmsg(m_map_ != hxnull && static_cast<size_t>(m_index_) < static_cast<size_t>(m_map_->m_size_),
		"invalid_iterator");
	return m_map_->m_keys_.data()[m_index_]; // NOLINT(clang-analyzer-security.ArrayBound)
}

template<typename key_t_, typename mapped_t_, typename compare_t_, bool multi_t_, hxsize_t capacity_>
hxinline hxattr_flatten const mapped_t_& hxflat_map<key_t_, mapped_t_, compare_t_, multi_t_, capacity_>::const_iterator::value(void) const {
	hxassertmsg(m_map_ != hxnull && static_cast<size_t>(m_index_) < static_cast<size_t>(m_map_->m_size_),
		"invalid_iterator");
	return m_map_->m_values_.data()[m_index_];
}

template<typename key_t_, typename mapped_t_, typename compare_t_, bool multi_t_, hxsize_t capacity_>
hxinline hxattr_flatten mapped_t_& hxflat_map<key_t_, mapped_t_, compare_t_, multi_t_, capacity_>::iterator::value(void) const {
	hxassertmsg(this->m_map_ != hxnull && static_cast<size_t>(this->m_index_) < static_cast<size_t>(this->m_map_->m_size_),
		"invalid_iterator");
	return const_cast<hxflat_map*>(this->m_map_)->m_values_.data()[this->m_index_];
}

template<typename key_t_, typename mapped_t_, typename compare_t_, bool multi_t_, hxsize_t capacity_>
hxinline hxflat_map<key_t_, mapped_t_, compare_t_, multi_t_, capacity_>::hxflat_map(void)
	: m_size_(0) {
}

template<typename key_t_, typename mapped_t_, typename compare_t_, bool multi_t_, hxsize_t capacity_>
hxinline hxattr_flatten hxflat_map<key_t_, mapped_t_, compare_t_, multi_t_, capacity_>::hxflat_map(const hxflat_map& x_) noexcept
	: m_size_(0) {
	this->operator=(x_);
}

template<typename key_t_, typename mapped_t_, typename compare_t_, bool multi_t_, hxsize_t capacity_>
hxinline hxflat_map<key_t_, mapped_t_, compare_t_, multi_t_, capacity_>::hxflat_map(hxflat_map&& x_) noexcept {
	static_assert(capacity_ == hxallocator_dynamic_capacity,
		"hxallocator_dynamic_capacity required for temporaries");
	::memcpy(static_cast<void*>(this), &x_, sizeof x_); // NOLINT(bugprone-undefined-memory-manipulation)
	::memset(static_cast<void*>(&x_), 0x00, sizeof x_);
}

template<typename key_t_, typename mapped_t_, typename compare_t_, bool multi_t_, hxsize_t capacity_>
hxinline hxattr_flatten hxflat_map<key_t_, mapped_t_, compare_t_, multi_t_, capacity_>::~hxflat_map(void) noexcept {
	this->clear();
}

template<typename key_t_, typename mapped_t_, typename compare_t_, bool multi_t_, hxsize_t capacity_>
hxinline hxattr_flatten void hxflat_map<key_t_, mapped_t_, compare_t_, multi_t_, capacity_>::operator=(
		const hxflat_map& x_) noexcept {
	hxassertmsg(static_cast<const void*>(this) != static_cast<const void*>(&x_),
		"invalid_reference");
	this->clear();
	const hxsize_t size_ = x_.m_size_;
	const key_t_* hxrestrict ks_ = x_.m_keys_.data();
	const mapped_t_* hxrestrict vs_ = x_.m_values_.data();
	key_t_* hxrestrict kd_ = m_keys_.data();
	mapped_t_* hxrestrict vd_ = m_values_.data();
	hxassert_hard(size_ <= m_keys_.capacity(), "hxflat_map capacity exceeded");
	for(hxsize_t i_ = 0; i_ < size_; ++i_) {
		::new(kd_ + i_) key_t_(ks_[i_]);
		::new(vd_ + i_) mapped_t_(vs_[i_]);
	}
	m_size_ = size_;
}

template<typename key_t_, typename mapped_t_, typename compare_t_, bool multi_t_, hxsize_t capacity_>
template<hxsize_t capacity_x_>
hxinline hxattr_flatten void hxflat_map<key_t_, mapped_t_, compare_t_, multi_t_, capacity_>::operator=(
		const hxflat_map<key_t_, mapped_t_, compare_t_, multi_t_, capacity_x_>& x_) noexcept {
	hxassertmsg(static_cast<const void*>(this) != static_cast<const void*>(&x_),
		"invalid_reference");
	this->clear();
	const hxsize_t size_ = x_.m_size_;
	const key_t_* hxrestrict ks_ = x_.m_keys_.data();
	const mapped_t_* hxrestrict vs_ = x_.m_values_.data();
	key_t_* hxrestrict kd_ = m_keys_.data();
	mapped_t_* hxrestrict vd_ = m_values_.data();
	hxassert_hard(size_ <= m_keys_.capacity(), "hxflat_map capacity exceeded");
	for(hxsize_t i_ = 0; i_ < size_; ++i_) {
		::new(kd_ + i_) key_t_(ks_[i_]);
		::new(vd_ + i_) mapped_t_(vs_[i_]);
	}
	m_size_ = size_;
}

template<typename key_t_, typename mapped_t_, typename compare_t_, bool multi_t_, hxsize_t capacity_>
hxinline hxattr_flatten void hxflat_map<key_t_, mapped_t_, compare_t_, multi_t_, capacity_>::operator=(
		hxflat_map&& x_) noexcept {
	hxassertmsg(static_cast<const void*>(this) != static_cast<const void*>(&x_),
		"invalid_reference");
	this->swap(x_);
}

template<typename key_t_, typename mapped_t_, typename compare_t_, bool multi_t_, hxsize_t capacity_>
hxinline hxattr_flatten auto hxflat_map<key_t_, mapped_t_, compare_t_, multi_t_, capacity_>::operator[](
		hxsize_t index_) const -> const_iterator {
	hxassert_hard(static_cast<size_t>(index_) < static_cast<size_t>(m_size_), "invalid_index %zu", index_);
	return const_iterator(this, index_);
}

template<typename key_t_, typename mapped_t_, typename compare_t_, bool multi_t_, hxsize_t capacity_>
hxinline hxattr_flatten auto hxflat_map<key_t_, mapped_t_, compare_t_, multi_t_, capacity_>::operator[](
		hxsize_t index_) -> iterator {
	hxassert_hard(static_cast<size_t>(index_) < static_cast<size_t>(m_size_), "invalid_index %zu", index_);
	return iterator(this, index_);
}

template<typename key_t_, typename mapped_t_, typename compare_t_, bool multi_t_, hxsize_t capacity_>
hxinline hxattr_flatten void hxflat_map<key_t_, mapped_t_, compare_t_, multi_t_, capacity_>::clear(void) noexcept {
	key_t_* hxrestrict k_ = m_keys_.data();
	mapped_t_* hxrestrict v_ = m_values_.data();
	const hxsize_t n_ = m_size_;
	m_size_ = 0;
	for(hxsize_t i_ = 0; i_ < n_; ++i_) {
		k_[i_].key_t_::~key_t_();
		v_[i_].mapped_t_::~mapped_t_();
	}
}

template<typename key_t_, typename mapped_t_, typename compare_t_, bool multi_t_, hxsize_t capacity_>
hxinline hxattr_flatten hxsize_t hxflat_map<key_t_, mapped_t_, compare_t_, multi_t_, capacity_>::count(const key_t_& key_) const {
	const compare_t_ comp_;
	const key_t_* const keys_ = m_keys_.data();
	const key_t_* const end_ = keys_ + m_size_;
	const key_t_* const pos_ = hxlower_bound(keys_, end_, key_, comp_);
	hxif_constexpr(!multi_t_) {
		return (pos_ != end_ && !comp_(key_, *pos_)) ? 1 : 0;
	}
	else {
		return hxupper_bound(pos_, end_, key_, comp_) - pos_;
	}
}

template<typename key_t_, typename mapped_t_, typename compare_t_, bool multi_t_, hxsize_t capacity_>
template<hxsize_t capacity_x_>
hxinline hxattr_flatten bool hxflat_map<key_t_, mapped_t_, compare_t_, multi_t_, capacity_>::equal(
		const hxflat_map<key_t_, mapped_t_, compare_t_, multi_t_, capacity_x_>& x_) const {
	const hxsize_t size_ = m_size_;
	if(size_ != x_.m_size_) { return false; }
	const key_t_* k0_ = m_keys_.data();
	const key_t_* k1_ = x_.m_keys_.data();
	const mapped_t_* v0_ = m_values_.data();
	const mapped_t_* v1_ = x_.m_values_.data();
	for(hxsize_t i_ = 0; i_ < size_; ++i_) {
		if(!hxkey_equal(k0_[i_], k1_[i_]) || !hxkey_equal(v0_[i_], v1_[i_])) { return false; }
	}
	return true;
}

template<typename key_t_, typename mapped_t_, typename compare_t_, bool multi_t_, hxsize_t capacity_>
hxattr_flatten hxsize_t hxflat_map<key_t_, mapped_t_, compare_t_, multi_t_, capacity_>::erase(const key_t_& key_) noexcept {
	const compare_t_ comp_;
	const hxsize_t size_ = m_size_;
	key_t_* hxrestrict k_ = m_keys_.data();
	const hxsize_t index_ = hxlower_bound(k_, k_ + size_, key_, comp_) - k_;
	hxif_constexpr(!multi_t_) {
		if(index_ >= size_ || comp_(key_, k_[index_])) { return 0; }
		this->erase(iterator(this, index_));
		return 1;
	}
	else {
		mapped_t_* hxrestrict v_ = m_values_.data();
		const hxsize_t hi_ = hxupper_bound(k_ + index_, k_ + size_, key_, comp_) - k_;
		const hxsize_t count_ = hi_ - index_;
		if(count_ == 0) { return 0; }
		const hxsize_t tail_ = size_ - hi_;
		for(hxsize_t i_ = 0; i_ < tail_; ++i_) {
			k_[index_ + i_] = hxmove(k_[hi_ + i_]);
			v_[index_ + i_] = hxmove(v_[hi_ + i_]);
		}
		const hxsize_t new_size_ = size_ - count_;
		for(hxsize_t i_ = new_size_; i_ < size_; ++i_) {
			k_[i_].key_t_::~key_t_();
			v_[i_].mapped_t_::~mapped_t_();
		}
		m_size_ = new_size_;
		return count_;
	}
}

template<typename key_t_, typename mapped_t_, typename compare_t_, bool multi_t_, hxsize_t capacity_>
hxattr_flatten auto hxflat_map<key_t_, mapped_t_, compare_t_, multi_t_, capacity_>::erase(iterator pos_)
		noexcept -> iterator {
	const hxsize_t size_ = m_size_;
	hxassertmsg(pos_.m_map_ == this && static_cast<size_t>(pos_.m_index_) < static_cast<size_t>(size_),
		"invalid_iterator");
	const hxsize_t index_ = pos_.m_index_;
	key_t_* hxrestrict k_ = m_keys_.data();
	mapped_t_* hxrestrict v_ = m_values_.data();
	const hxsize_t new_size_ = size_ - 1;
	for(hxsize_t i_ = index_; i_ < new_size_; ++i_) {
		k_[i_] = hxmove(k_[i_ + 1]);
		v_[i_] = hxmove(v_[i_ + 1]);
	}
	k_[new_size_].key_t_::~key_t_();
	v_[new_size_].mapped_t_::~mapped_t_();
	m_size_ = new_size_;
	return iterator(this, index_);
}

template<typename key_t_, typename mapped_t_, typename compare_t_, bool multi_t_, hxsize_t capacity_>
hxinline hxattr_flatten const mapped_t_* hxflat_map<key_t_, mapped_t_, compare_t_, multi_t_, capacity_>::find(const key_t_& key_) const {
	const compare_t_ comp_;
	const key_t_* const keys_ = m_keys_.data();
	const hxsize_t index_ = hxlower_bound(keys_, keys_ + m_size_, key_, comp_) - keys_;
	if(index_ < m_size_ && !comp_(key_, keys_[index_])) {
		return m_values_.data() + index_;
	}
	return hxnull;
}

template<typename key_t_, typename mapped_t_, typename compare_t_, bool multi_t_, hxsize_t capacity_>
hxinline hxattr_flatten mapped_t_* hxflat_map<key_t_, mapped_t_, compare_t_, multi_t_, capacity_>::find(const key_t_& key_) {
	return const_cast<mapped_t_*>(const_cast<const hxflat_map*>(this)->find(key_));
}

template<typename key_t_, typename mapped_t_, typename compare_t_, bool multi_t_, hxsize_t capacity_>
hxinline hxattr_flatten auto hxflat_map<key_t_, mapped_t_, compare_t_, multi_t_, capacity_>::get(hxsize_t index_) const
		-> const_iterator {
	return const_iterator(this, index_ < m_size_ ? index_ : m_size_);
}

template<typename key_t_, typename mapped_t_, typename compare_t_, bool multi_t_, hxsize_t capacity_>
hxinline hxattr_flatten auto hxflat_map<key_t_, mapped_t_, compare_t_, multi_t_, capacity_>::get(hxsize_t index_) -> iterator {
	return iterator(this, index_ < m_size_ ? index_ : m_size_);
}

// Fixes gcc + optimizer + sanitizer -Wmaybe-uninitialized bug.
#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#endif

template<typename key_t_, typename mapped_t_, typename compare_t_, bool multi_t_, hxsize_t capacity_>
hxinline hxattr_flatten auto hxflat_map<key_t_, mapped_t_, compare_t_, multi_t_, capacity_>::insert(
		const key_t_& key_, const mapped_t_& mapped_) noexcept -> iterator {
	const compare_t_ comp_;
	const key_t_* const keys_ = m_keys_.data();
	const hxsize_t index_ = hxlower_bound(keys_, keys_ + m_size_, key_, comp_) - keys_;
	hxif_constexpr(!multi_t_) {
		if(index_ < m_size_ && !comp_(key_, keys_[index_])) {
			return iterator(this, index_);
		}
	}
	return this->insert_at_(index_, key_, mapped_);
}

template<typename key_t_, typename mapped_t_, typename compare_t_, bool multi_t_, hxsize_t capacity_>
hxinline hxattr_flatten auto hxflat_map<key_t_, mapped_t_, compare_t_, multi_t_, capacity_>::insert(
		const key_t_& key_, mapped_t_&& mapped_) noexcept -> iterator {
	const compare_t_ comp_;
	const key_t_* const keys_ = m_keys_.data();
	const hxsize_t index_ = hxlower_bound(keys_, keys_ + m_size_, key_, comp_) - keys_;
	hxif_constexpr(!multi_t_) {
		if(index_ < m_size_ && !comp_(key_, keys_[index_])) {
			return iterator(this, index_);
		}
	}
	return this->insert_at_(index_, key_, hxmove(mapped_));
}

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic pop
#endif

template<typename key_t_, typename mapped_t_, typename compare_t_, bool multi_t_, hxsize_t capacity_>
template<hxsize_t capacity_x_>
hxinline hxattr_flatten bool hxflat_map<key_t_, mapped_t_, compare_t_, multi_t_, capacity_>::less(
		const hxflat_map<key_t_, mapped_t_, compare_t_, multi_t_, capacity_x_>& x_) const {
	const hxsize_t size_ = hxmin(m_size_, x_.m_size_);
	const key_t_* k0_ = m_keys_.data();
	const key_t_* k1_ = x_.m_keys_.data();
	const mapped_t_* v0_ = m_values_.data();
	const mapped_t_* v1_ = x_.m_values_.data();
	for(hxsize_t i_ = 0; i_ < size_; ++i_) {
		if(!hxkey_equal(k0_[i_], k1_[i_])) { return hxkey_less(k0_[i_], k1_[i_]); }
		if(!hxkey_equal(v0_[i_], v1_[i_])) { return hxkey_less(v0_[i_], v1_[i_]); }
	}
	return m_size_ < x_.m_size_;
}

template<typename key_t_, typename mapped_t_, typename compare_t_, bool multi_t_, hxsize_t capacity_>
hxinline hxattr_flatten auto hxflat_map<key_t_, mapped_t_, compare_t_, multi_t_, capacity_>::lower_bound(const key_t_& key_) const
		-> const_iterator {
	const compare_t_ comp_;
	const key_t_* const keys_ = m_keys_.data();
	return const_iterator(this, hxlower_bound(keys_, keys_ + m_size_, key_, comp_) - keys_);
}

template<typename key_t_, typename mapped_t_, typename compare_t_, bool multi_t_, hxsize_t capacity_>
hxinline hxattr_flatten auto hxflat_map<key_t_, mapped_t_, compare_t_, multi_t_, capacity_>::lower_bound(const key_t_& key_)
		-> iterator {
	return iterator(this, const_cast<const hxflat_map*>(this)->lower_bound(key_).m_index_);
}

template<typename key_t_, typename mapped_t_, typename compare_t_, bool multi_t_, hxsize_t capacity_>
hxinline hxattr_flatten void hxflat_map<key_t_, mapped_t_, compare_t_, multi_t_, capacity_>::reserve(hxsize_t cap_,
		hxsystem_allocator_t allocator_, hxalignment_t alignment_) noexcept {
	m_keys_.reserve_storage(cap_, allocator_, alignment_);
	m_values_.reserve_storage(cap_, allocator_, alignment_);
}

template<typename key_t_, typename mapped_t_, typename compare_t_, bool multi_t_, hxsize_t capacity_>
hxinline hxattr_flatten void hxflat_map<key_t_, mapped_t_, compare_t_, multi_t_, capacity_>::swap(
		hxflat_map& x_) noexcept {
	static_assert(capacity_ == hxallocator_dynamic_capacity,
		"hxallocator_dynamic_capacity required for hxflat_map::swap");
	hxswap_memcpy(*this, x_);
}

template<typename key_t_, typename mapped_t_, typename compare_t_, bool multi_t_, hxsize_t capacity_>
hxinline hxattr_flatten auto hxflat_map<key_t_, mapped_t_, compare_t_, multi_t_, capacity_>::upper_bound(const key_t_& key_) const
		-> const_iterator {
	const compare_t_ comp_;
	const key_t_* const keys_ = m_keys_.data();
	return const_iterator(this, hxupper_bound(keys_, keys_ + m_size_, key_, comp_) - keys_);
}

template<typename key_t_, typename mapped_t_, typename compare_t_, bool multi_t_, hxsize_t capacity_>
hxinline hxattr_flatten auto hxflat_map<key_t_, mapped_t_, compare_t_, multi_t_, capacity_>::upper_bound(const key_t_& key_)
		-> iterator {
	return iterator(this, const_cast<const hxflat_map*>(this)->upper_bound(key_).m_index_);
}

template<typename key_t_, typename mapped_t_, typename compare_t_, bool multi_t_, hxsize_t capacity_>
template<typename mapped_u_>
hxattr_flatten auto hxflat_map<key_t_, mapped_t_, compare_t_, multi_t_, capacity_>::insert_at_(
		hxsize_t index_, const key_t_& key_, mapped_u_&& mapped_) noexcept -> iterator {
	const hxsize_t size_ = m_size_;
	hxassert_hard(size_ < m_keys_.capacity(), "hxflat_map full");
	key_t_* hxrestrict k_ = m_keys_.data();
	mapped_t_* hxrestrict v_ = m_values_.data();
	if(size_ > index_) {
		::new(k_ + size_) key_t_(hxmove(k_[size_ - 1]));
		::new(v_ + size_) mapped_t_(hxmove(v_[size_ - 1]));
		for(hxsize_t i_ = size_ - 1; i_ > index_; --i_) {
			k_[i_] = hxmove(k_[i_ - 1]);
			v_[i_] = hxmove(v_[i_ - 1]);
		}
		k_[index_] = key_;
		v_[index_] = hxforward<mapped_u_>(mapped_);
	}
	else {
		::new(k_ + index_) key_t_(key_);
		::new(v_ + index_) mapped_t_(hxforward<mapped_u_>(mapped_));
	}
	m_size_ = size_ + 1;
	return iterator(this, index_);
}

HX_END_INL_
#endif // HX_DOXYGEN_PARSER
