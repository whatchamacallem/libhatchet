#pragma once
// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

#ifndef LIBHATCHET_VER
#error Internal. Do not include this file directly.
#endif

#ifndef HX_DOXYGEN_PARSER
HX_BEGIN_INL_

template<typename key_t_, typename compare_t_, bool multi_t_, hxsize_t capacity_>
hxinline hxattr_flatten hxflat_set<key_t_, compare_t_, multi_t_, capacity_>::hxflat_set(void)
	: m_end_(this->data()) {
}

template<typename key_t_, typename compare_t_, bool multi_t_, hxsize_t capacity_>
hxinline hxattr_flatten hxflat_set<key_t_, compare_t_, multi_t_, capacity_>::hxflat_set(const hxflat_set& x_) noexcept
	: m_end_(this->data()) {
	this->operator=(x_);
}

template<typename key_t_, typename compare_t_, bool multi_t_, hxsize_t capacity_>
hxinline hxattr_flatten hxflat_set<key_t_, compare_t_, multi_t_, capacity_>::hxflat_set(
		std::initializer_list<key_t_> x_) noexcept : m_end_(this->data()) {
	hxif_constexpr(capacity_ == hxallocator_dynamic_capacity) {
		this->reserve_storage(static_cast<hxsize_t>(x_.size()));
		m_end_ = this->data();
	}
	const key_t_* hxrestrict src_ = x_.begin();
	for(const key_t_*const end_ = x_.end(); src_ != end_; ++src_) {
		this->insert(*src_);
	}
}

template<typename key_t_, typename compare_t_, bool multi_t_, hxsize_t capacity_>
hxinline hxflat_set<key_t_, compare_t_, multi_t_, capacity_>::hxflat_set(hxflat_set&& x_) noexcept {
	static_assert(capacity_ == hxallocator_dynamic_capacity,
		"hxallocator_dynamic_capacity required for temporaries");
	::memcpy(static_cast<void*>(this), &x_, sizeof x_); // NOLINT(bugprone-undefined-memory-manipulation)
	::memset(static_cast<void*>(&x_), 0x00, sizeof x_);
}

template<typename key_t_, typename compare_t_, bool multi_t_, hxsize_t capacity_>
hxinline hxattr_flatten hxflat_set<key_t_, compare_t_, multi_t_, capacity_>::~hxflat_set(void) noexcept {
	this->clear();
}

template<typename key_t_, typename compare_t_, bool multi_t_, hxsize_t capacity_>
hxinline hxattr_flatten void hxflat_set<key_t_, compare_t_, multi_t_, capacity_>::operator=(
		const hxflat_set& x_) noexcept {
	hxassertmsg(static_cast<const void*>(this) != static_cast<const void*>(&x_),
		"invalid_reference");
	this->clear();
	const key_t_* hxrestrict src_ = x_.data();
	const key_t_* const src_end_ = x_.m_end_;
	key_t_* hxrestrict dst_ = this->data();
	hxassert_hard(src_end_ - src_ <= this->data() + this->capacity() - dst_,
		"hxflat_set capacity exceeded");
	for(; src_ != src_end_; ++src_, ++dst_) {
		::new(dst_) key_t_(*src_);
	}
	m_end_ = dst_;
}

template<typename key_t_, typename compare_t_, bool multi_t_, hxsize_t capacity_>
template<hxsize_t capacity_x_>
hxinline hxattr_flatten void hxflat_set<key_t_, compare_t_, multi_t_, capacity_>::operator=(
		const hxflat_set<key_t_, compare_t_, multi_t_, capacity_x_>& x_) noexcept {
	hxassertmsg(static_cast<const void*>(this) != static_cast<const void*>(&x_),
		"invalid_reference");
	this->clear();
	const key_t_* hxrestrict src_ = x_.data();
	const key_t_* const src_end_ = x_.m_end_;
	key_t_* hxrestrict dst_ = this->data();
	hxassert_hard(src_end_ - src_ <= this->data() + this->capacity() - dst_,
		"hxflat_set capacity exceeded");
	for(; src_ != src_end_; ++src_, ++dst_) {
		::new(dst_) key_t_(*src_);
	}
	m_end_ = dst_;
}

template<typename key_t_, typename compare_t_, bool multi_t_, hxsize_t capacity_>
hxinline hxattr_flatten void hxflat_set<key_t_, compare_t_, multi_t_, capacity_>::operator=(
		hxflat_set&& x_) noexcept {
	hxassertmsg(static_cast<const void*>(this) != static_cast<const void*>(&x_),
		"invalid_reference");
	this->swap(x_);
}

template<typename key_t_, typename compare_t_, bool multi_t_, hxsize_t capacity_>
hxinline hxattr_flatten auto hxflat_set<key_t_, compare_t_, multi_t_, capacity_>::operator[](
		hxsize_t index_) const -> const key_t_* {
	const key_t_* const p_ = this->data() + index_;
	hxassert_hard(p_ < m_end_, "invalid_index %zu", index_);
	return p_;
}

template<typename key_t_, typename compare_t_, bool multi_t_, hxsize_t capacity_>
hxinline hxattr_flatten hxsize_t hxflat_set<key_t_, compare_t_, multi_t_, capacity_>::capacity(void) const {
	return hxallocator<key_t_, capacity_>::capacity();
}

template<typename key_t_, typename compare_t_, bool multi_t_, hxsize_t capacity_>
hxinline hxattr_flatten void hxflat_set<key_t_, compare_t_, multi_t_, capacity_>::clear(void) noexcept {
	key_t_* it_ = this->data();
	const key_t_* const end_ = m_end_;
	m_end_ = this->data();
	for(; it_ != end_; ++it_) {
		it_->key_t_::~key_t_();
	}
}

template<typename key_t_, typename compare_t_, bool multi_t_, hxsize_t capacity_>
hxinline hxattr_flatten hxsize_t hxflat_set<key_t_, compare_t_, multi_t_, capacity_>::count(const key_t_& key_) const {
	const compare_t_ comp_;
	const key_t_* const end_ = m_end_;
	const key_t_* const pos_ = hxlower_bound(this->data(), end_, key_, comp_);
	hxif_constexpr(!multi_t_) {
		return (pos_ < end_ && !comp_(key_, *pos_)) ? 1 : 0;
	}
	else {
		return hxupper_bound(pos_, end_, key_, comp_) - pos_;
	}
}

template<typename key_t_, typename compare_t_, bool multi_t_, hxsize_t capacity_>
hxattr_flatten hxsize_t hxflat_set<key_t_, compare_t_, multi_t_, capacity_>::erase(const key_t_& key_) noexcept {
	const compare_t_ comp_;
	key_t_* end_ = m_end_;
	key_t_* const pos_ = hxlower_bound(this->data(), end_, key_, comp_);
	hxif_constexpr(!multi_t_) {
		if(pos_ >= end_ || comp_(key_, *pos_)) { return 0; }
		this->erase(pos_);
		return 1;
	}
	else {
		key_t_* const hi_ptr_ = hxupper_bound(pos_, end_, key_, comp_);
		const hxsize_t count_ = static_cast<hxsize_t>(hi_ptr_ - pos_);
		if(count_ == 0) { return 0; }

		key_t_* hxrestrict dst_ = pos_;
		for(key_t_* src_ = hi_ptr_; src_ != end_; ++dst_, ++src_) {
			*dst_ = hxmove(*src_);
		}
		end_ -= count_;
		for(key_t_* it_ = end_; it_ != m_end_; ++it_) {
			it_->key_t_::~key_t_();
		}
		m_end_ = end_;
		return count_;
	}
}

template<typename key_t_, typename compare_t_, bool multi_t_, hxsize_t capacity_>
hxinline hxattr_flatten auto hxflat_set<key_t_, compare_t_, multi_t_, capacity_>::erase(const key_t_* pos_) noexcept -> const key_t_* {
	hxassertmsg(pos_ != hxnull && static_cast<size_t>(pos_ - this->data()) < static_cast<size_t>(m_end_ - this->data()),
		"invalid_iterator");
	key_t_* const k_ = const_cast<key_t_*>(pos_);
	key_t_* const end_ = m_end_ - 1;
	for(key_t_* hxrestrict it_ = k_; it_ != end_; ++it_) {
		*it_ = hxmove(*(it_ + 1));
	}
	end_->key_t_::~key_t_();
	m_end_ = end_;
	return k_;
}

template<typename key_t_, typename compare_t_, bool multi_t_, hxsize_t capacity_>
hxinline hxattr_flatten auto hxflat_set<key_t_, compare_t_, multi_t_, capacity_>::find(const key_t_& key_) const
		-> const key_t_* {
	const compare_t_ comp_;
	const key_t_* const end_ = m_end_;
	const key_t_* const pos_ = hxlower_bound(this->data(), end_, key_, comp_);
	if(pos_ < end_ && !comp_(key_, *pos_)) {
		return pos_;
	}
	return hxnull;
}

template<typename key_t_, typename compare_t_, bool multi_t_, hxsize_t capacity_>
hxinline hxattr_flatten auto hxflat_set<key_t_, compare_t_, multi_t_, capacity_>::insert(const key_t_& key_) noexcept -> const key_t_* {
	const compare_t_ comp_;
	key_t_* const pos_ = hxlower_bound(this->data(), m_end_, key_, comp_);
	hxif_constexpr(!multi_t_) {
		if(pos_ < m_end_ && !comp_(key_, *pos_)) {
			return pos_;
		}
	}
	return this->insert_at_(pos_, key_);
}

template<typename key_t_, typename compare_t_, bool multi_t_, hxsize_t capacity_>
hxinline hxattr_flatten auto hxflat_set<key_t_, compare_t_, multi_t_, capacity_>::insert(key_t_&& key_) noexcept -> const key_t_* {
	const compare_t_ comp_;
	key_t_* const pos_ = hxlower_bound(this->data(), m_end_, key_, comp_);
	hxif_constexpr(!multi_t_) {
		if(pos_ < m_end_ && !comp_(key_, *pos_)) {
			return pos_;
		}
	}
	return this->insert_at_(pos_, hxmove(key_));
}

template<typename key_t_, typename compare_t_, bool multi_t_, hxsize_t capacity_>
hxinline hxattr_flatten auto hxflat_set<key_t_, compare_t_, multi_t_, capacity_>::lower_bound(const key_t_& key_) const
		-> const key_t_* {
	const compare_t_ comp_;
	const key_t_* const end_ = m_end_;
	return hxlower_bound(this->data(), end_, key_, comp_);
}

template<typename key_t_, typename compare_t_, bool multi_t_, hxsize_t capacity_>
hxinline hxattr_flatten void hxflat_set<key_t_, compare_t_, multi_t_, capacity_>::reserve(hxsize_t cap_,
		hxsystem_allocator_t allocator_, hxalignment_t alignment_) {
	this->reserve_storage(cap_, allocator_, alignment_);
	if(m_end_ == hxnull) {
		m_end_ = this->data();
	}
}

template<typename key_t_, typename compare_t_, bool multi_t_, hxsize_t capacity_>
hxinline hxattr_flatten void hxflat_set<key_t_, compare_t_, multi_t_, capacity_>::swap(
		hxflat_set& x_) noexcept {
	static_assert(capacity_ == hxallocator_dynamic_capacity,
		"hxallocator_dynamic_capacity required for hxflat_set::swap");
	hxswap_memcpy(*this, x_);
}

template<typename key_t_, typename compare_t_, bool multi_t_, hxsize_t capacity_>
hxinline hxattr_flatten auto hxflat_set<key_t_, compare_t_, multi_t_, capacity_>::upper_bound(const key_t_& key_) const
		-> const key_t_* {
	const compare_t_ comp_;
	const key_t_* const end_ = m_end_;
	return hxupper_bound(this->data(), end_, key_, comp_);
}

template<typename key_t_, typename compare_t_, bool multi_t_, hxsize_t capacity_>
template<typename key_u_>
hxinline hxattr_flatten auto hxflat_set<key_t_, compare_t_, multi_t_, capacity_>::insert_at_(
		key_t_* pos_, key_u_&& key_) noexcept -> const key_t_* {
	key_t_* const end_ = m_end_;
	hxassert_hard(end_ != this->data() + this->capacity(), "hxflat_set full");
	if(end_ != pos_) {
		::new(end_) key_t_(hxmove(*(end_ - 1)));
		for(key_t_* it_ = end_ - 1; it_ != pos_; --it_) {
			*it_ = hxmove(*(it_ - 1));
		}
		*pos_ = hxforward<key_u_>(key_);
	}
	else {
		::new(pos_) key_t_(hxforward<key_u_>(key_));
	}
	m_end_ = end_ + 1;
	return pos_;
}

template<typename key_t_, typename compare_t_, bool multi_t_, hxsize_t capacity_>
template<hxsize_t capacity_x_>
hxinline hxattr_flatten bool hxflat_set<key_t_, compare_t_, multi_t_, capacity_>::equal(
		const hxflat_set<key_t_, compare_t_, multi_t_, capacity_x_>& x_) const {
	const hxsize_t size_ = static_cast<hxsize_t>(m_end_ - this->data());
	if(size_ != static_cast<hxsize_t>(x_.m_end_ - x_.data())) { return false; }
	const key_t_* k0_ = this->data();
	const key_t_* k1_ = x_.data();
	for(hxsize_t i_ = 0; i_ < size_; ++i_) {
		if(!hxkey_equal(k0_[i_], k1_[i_])) { return false; }
	}
	return true;
}

template<typename key_t_, typename compare_t_, bool multi_t_, hxsize_t capacity_>
template<hxsize_t capacity_x_>
hxinline hxattr_flatten bool hxflat_set<key_t_, compare_t_, multi_t_, capacity_>::less(
		const hxflat_set<key_t_, compare_t_, multi_t_, capacity_x_>& x_) const {
	const hxsize_t size0_ = static_cast<hxsize_t>(m_end_ - this->data());
	const hxsize_t size1_ = static_cast<hxsize_t>(x_.m_end_ - x_.data());
	const hxsize_t size_ = hxmin(size0_, size1_);
	const key_t_* k0_ = this->data();
	const key_t_* k1_ = x_.data();
	for(hxsize_t i_ = 0; i_ < size_; ++i_) {
		if(!hxkey_equal(k0_[i_], k1_[i_])) { return hxkey_less(k0_[i_], k1_[i_]); }
	}
	return size0_ < size1_;
}

HX_END_INL_
#endif // HX_DOXYGEN_PARSER
