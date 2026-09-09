#pragma once
// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

// Iterators are not hardened, as per the standard. That causes NOLINT.

#ifndef LIBHATCHET_VER
#error Internal. Do not include this file directly.
#endif

#ifndef HX_DOXYGEN_PARSER
HX_INL_BEGIN_

template<hxflat_map_concept_ key_t_, hxflat_map_concept_ mapped_t_, typename compare_t_, hxsize_t capacity_, int traits_>
hxinline auto hxflat_map<key_t_, mapped_t_, compare_t_, capacity_, traits_>::const_iterator::operator+=(
		hxsize_t n_) -> const_iterator& {
	m_index_ += n_;
	hxassertf(m_map_ != hxnull, "bad_iter");
	hxassertf(static_cast<size_t>(m_index_) <= static_cast<size_t>(m_map_->m_size_), // NOLINT(clang-analyzer-core.NullDereference)
		"bad_iter %zd size %zd", m_index_, m_map_->m_size_);
	return *this;
}

template<hxflat_map_concept_ key_t_, hxflat_map_concept_ mapped_t_, typename compare_t_, hxsize_t capacity_, int traits_>
hxinline hxsize_t hxflat_map<key_t_, mapped_t_, compare_t_, capacity_, traits_>::const_iterator::operator-(
		const const_iterator& x_) const {
	hxassertf(m_map_ != hxnull && m_map_ == x_.m_map_, "bad_iter");
	return m_index_ - x_.m_index_;
}

template<hxflat_map_concept_ key_t_, hxflat_map_concept_ mapped_t_, typename compare_t_, hxsize_t capacity_, int traits_>
hxinline bool hxflat_map<key_t_, mapped_t_, compare_t_, capacity_, traits_>::const_iterator::operator==(
		const const_iterator& x_) const {
	hxassertf(m_map_ != hxnull && m_map_ == x_.m_map_, "bad_iter");
	return m_index_ == x_.m_index_;
}

template<hxflat_map_concept_ key_t_, hxflat_map_concept_ mapped_t_, typename compare_t_, hxsize_t capacity_, int traits_>
hxinline bool hxflat_map<key_t_, mapped_t_, compare_t_, capacity_, traits_>::const_iterator::operator<(
		const const_iterator& x_) const {
	hxassertf(m_map_ != hxnull && m_map_ == x_.m_map_, "bad_iter");
	return m_index_ < x_.m_index_;
}

#if HX_CPLUSPLUS < 202002L
template<hxflat_map_concept_ key_t_, hxflat_map_concept_ mapped_t_, typename compare_t_, hxsize_t capacity_, int traits_>
hxinline bool hxflat_map<key_t_, mapped_t_, compare_t_, capacity_, traits_>::const_iterator::operator!=(
		const const_iterator& x_) const {
	hxassertf(m_map_ != hxnull && m_map_ == x_.m_map_, "bad_iter");
	return m_index_ != x_.m_index_;
}
#endif

template<hxflat_map_concept_ key_t_, hxflat_map_concept_ mapped_t_, typename compare_t_, hxsize_t capacity_, int traits_>
hxinline hxattr_flatten const key_t_& hxflat_map<key_t_, mapped_t_, compare_t_, capacity_, traits_>::const_iterator::key(void) const {
	hxassertf(m_map_ != hxnull, "bad_iter");
	hxassertf(static_cast<size_t>(m_index_) < static_cast<size_t>(m_map_->m_size_), // NOLINT(clang-analyzer-core.NullDereference)
		"bad_iter %zd size %zd", m_index_, m_map_->m_size_);
	return m_map_->m_keys_.data()[m_index_];
}

template<hxflat_map_concept_ key_t_, hxflat_map_concept_ mapped_t_, typename compare_t_, hxsize_t capacity_, int traits_>
hxinline hxattr_flatten const mapped_t_& hxflat_map<key_t_, mapped_t_, compare_t_, capacity_, traits_>::const_iterator::value(void) const {
	hxassertf(m_map_ != hxnull, "bad_iter");
	hxassertf(static_cast<size_t>(m_index_) < static_cast<size_t>(m_map_->m_size_), // NOLINT(clang-analyzer-core.NullDereference)
		"bad_iter %zd size %zd", m_index_, m_map_->m_size_);
	return m_map_->m_values_.data()[m_index_];
}

template<hxflat_map_concept_ key_t_, hxflat_map_concept_ mapped_t_, typename compare_t_, hxsize_t capacity_, int traits_>
hxinline hxattr_flatten mapped_t_& hxflat_map<key_t_, mapped_t_, compare_t_, capacity_, traits_>::iterator::value(void) const {
	hxassertf(this->m_map_ != hxnull, "bad_iter");
	hxassertf(static_cast<size_t>(this->m_index_) < static_cast<size_t>(this->m_map_->m_size_),
		"bad_iter %zd size %zd", this->m_index_, this->m_map_->m_size_);
	return const_cast<hxflat_map*>(this->m_map_)->m_values_.data()[this->m_index_];
}

template<hxflat_map_concept_ key_t_, hxflat_map_concept_ mapped_t_, typename compare_t_, hxsize_t capacity_, int traits_>
hxinline hxflat_map<key_t_, mapped_t_, compare_t_, capacity_, traits_>::hxflat_map(void)
	: m_size_(0) {
}

template<hxflat_map_concept_ key_t_, hxflat_map_concept_ mapped_t_, typename compare_t_, hxsize_t capacity_, int traits_>
hxinline hxattr_flatten hxflat_map<key_t_, mapped_t_, compare_t_, capacity_, traits_>::hxflat_map(const hxflat_map& x_) noexcept
	: m_size_(0) {
	this->operator=(x_);
}

template<hxflat_map_concept_ key_t_, hxflat_map_concept_ mapped_t_, typename compare_t_, hxsize_t capacity_, int traits_>
hxinline hxattr_flatten hxflat_map<key_t_, mapped_t_, compare_t_, capacity_, traits_>::hxflat_map(
		std::initializer_list<pair_t_> x_) noexcept : m_size_(0) {
	hxif_constexpr(capacity_ == hxallocator_dynamic_capacity) {
		this->reserve(static_cast<hxsize_t>(x_.size()));
	}
	const pair_t_* hxrestrict src_ = x_.begin();
	for(const pair_t_*const end_ = x_.end(); src_ != end_; ++src_) {
		this->insert(src_->key_, src_->mapped_);
	}
}

template<hxflat_map_concept_ key_t_, hxflat_map_concept_ mapped_t_, typename compare_t_, hxsize_t capacity_, int traits_>
hxinline hxflat_map<key_t_, mapped_t_, compare_t_, capacity_, traits_>::hxflat_map(hxflat_map&& x_) noexcept {
	static_assert(capacity_ == hxallocator_dynamic_capacity,
		"hxallocator_dynamic_capacity required for temporaries");
	::memcpy(static_cast<void*>(this), &x_, sizeof x_); // NOLINT(bugprone-undefined-memory-manipulation)
	::memset(static_cast<void*>(&x_), 0x00, sizeof x_);
}

template<hxflat_map_concept_ key_t_, hxflat_map_concept_ mapped_t_, typename compare_t_, hxsize_t capacity_, int traits_>
hxinline hxattr_flatten hxflat_map<key_t_, mapped_t_, compare_t_, capacity_, traits_>::~hxflat_map(void) noexcept {
	this->clear();
}

#if HX_CPLUSPLUS >= 202302L
template<hxflat_map_concept_ key_t_, hxflat_map_concept_ mapped_t_, typename compare_t_, hxsize_t capacity_, int traits_>
template<typename self_t_, typename callable_t_>
hxinline hxattr_flatten auto hxflat_map<key_t_, mapped_t_, compare_t_, capacity_, traits_>::and_then(
		this self_t_&& self_, const key_t_& key_, callable_t_&& callable_)
		-> hxremove_cvref_t<decltype(
			hxforward<callable_t_>(callable_)(
				hxforward_like<self_t_>(hxdeclval<mapped_t_&>())))> {
	const auto found_ = self_.find(key_);
	if(found_ != self_.cend()) {
		return hxforward<callable_t_>(callable_)(
			hxforward_like<self_t_>(found_.value()));
	}
	return hxnil;
}

template<hxflat_map_concept_ key_t_, hxflat_map_concept_ mapped_t_, typename compare_t_, hxsize_t capacity_, int traits_>
template<typename self_t_, typename callable_t_>
hxinline hxattr_flatten auto hxflat_map<key_t_, mapped_t_, compare_t_, capacity_, traits_>::and_then(
		this self_t_&& self_, const_iterator it_, callable_t_&& callable_)
		-> hxremove_cvref_t<decltype(
			hxforward<callable_t_>(callable_)(hxforward_like<self_t_>(
				hxdeclval<const mapped_t_&>())))> {
	if(it_ != self_.cend()) {
		const mapped_t_& value_ = it_.value();
		return hxforward<callable_t_>(callable_)(hxforward_like<self_t_>(value_));
	}
	return hxnil;
}
#endif // HX_CPLUSPLUS >= 202302L

template<hxflat_map_concept_ key_t_, hxflat_map_concept_ mapped_t_, typename compare_t_, hxsize_t capacity_, int traits_>
hxinline hxattr_flatten void hxflat_map<key_t_, mapped_t_, compare_t_, capacity_, traits_>::operator=(
		const hxflat_map& x_) noexcept {
	hxassertf(static_cast<const void*>(this) != static_cast<const void*>(&x_), "bad_ref");
	this->clear();
	const hxsize_t size_ = x_.m_size_;
	const key_t_* hxrestrict ks_ = x_.m_keys_.data();
	const mapped_t_* hxrestrict vs_ = x_.m_values_.data();
	key_t_* hxrestrict kd_ = m_keys_.data();
	mapped_t_* hxrestrict vd_ = m_values_.data();
	hxassert_hard(size_ <= m_keys_.capacity(), "hxflat_map capacity exceeded %zd", m_keys_.capacity());
	for(hxsize_t i_ = 0; i_ < size_; ++i_) {
		::new(kd_ + i_) key_t_(ks_[i_]);
		::new(vd_ + i_) mapped_t_(vs_[i_]);
	}
	m_size_ = size_;
}

template<hxflat_map_concept_ key_t_, hxflat_map_concept_ mapped_t_, typename compare_t_, hxsize_t capacity_, int traits_>
template<hxsize_t capacity_x_>
hxinline hxattr_flatten void hxflat_map<key_t_, mapped_t_, compare_t_, capacity_, traits_>::operator=(
		const hxflat_map<key_t_, mapped_t_, compare_t_, capacity_x_, traits_>& x_) noexcept {
	hxassertf(static_cast<const void*>(this) != static_cast<const void*>(&x_), "bad_ref");
	this->clear();
	const hxsize_t size_ = x_.m_size_;
	const key_t_* hxrestrict ks_ = x_.m_keys_.data();
	const mapped_t_* hxrestrict vs_ = x_.m_values_.data();
	key_t_* hxrestrict kd_ = m_keys_.data();
	mapped_t_* hxrestrict vd_ = m_values_.data();
	hxassert_hard(size_ <= m_keys_.capacity(), "hxflat_map capacity exceeded %zd", m_keys_.capacity());
	for(hxsize_t i_ = 0; i_ < size_; ++i_) {
		::new(kd_ + i_) key_t_(ks_[i_]);
		::new(vd_ + i_) mapped_t_(vs_[i_]);
	}
	m_size_ = size_;
}

template<hxflat_map_concept_ key_t_, hxflat_map_concept_ mapped_t_, typename compare_t_, hxsize_t capacity_, int traits_>
hxinline hxattr_flatten void hxflat_map<key_t_, mapped_t_, compare_t_, capacity_, traits_>::operator=(
		hxflat_map&& x_) noexcept {
	hxassertf(static_cast<const void*>(this) != static_cast<const void*>(&x_), "bad_ref");
	this->swap(x_);
}

template<hxflat_map_concept_ key_t_, hxflat_map_concept_ mapped_t_, typename compare_t_, hxsize_t capacity_, int traits_>
hxinline hxattr_flatten auto hxflat_map<key_t_, mapped_t_, compare_t_, capacity_, traits_>::operator[](
		hxsize_t index_) const -> const_iterator {
	hxassert_hard(static_cast<size_t>(index_) < static_cast<size_t>(m_size_), "bad_index %zd size %zd", index_, m_size_);
	return const_iterator(this, index_);
}

template<hxflat_map_concept_ key_t_, hxflat_map_concept_ mapped_t_, typename compare_t_, hxsize_t capacity_, int traits_>
hxinline hxattr_flatten auto hxflat_map<key_t_, mapped_t_, compare_t_, capacity_, traits_>::operator[](
		hxsize_t index_) -> iterator {
	hxassert_hard(static_cast<size_t>(index_) < static_cast<size_t>(m_size_), "bad_index %zd size %zd", index_, m_size_);
	return iterator(this, index_);
}

template<hxflat_map_concept_ key_t_, hxflat_map_concept_ mapped_t_, typename compare_t_, hxsize_t capacity_, int traits_>
hxinline hxattr_flatten void hxflat_map<key_t_, mapped_t_, compare_t_, capacity_, traits_>::clear(void) noexcept {
	key_t_* hxrestrict k_ = m_keys_.data();
	mapped_t_* hxrestrict v_ = m_values_.data();
	const hxsize_t n_ = m_size_;
	m_size_ = 0;
	for(hxsize_t i_ = 0; i_ < n_; ++i_) {
		k_[i_].key_t_::~key_t_();
		v_[i_].mapped_t_::~mapped_t_();
	}
}

template<hxflat_map_concept_ key_t_, hxflat_map_concept_ mapped_t_, typename compare_t_, hxsize_t capacity_, int traits_>
hxinline hxattr_flatten hxsize_t hxflat_map<key_t_, mapped_t_, compare_t_, capacity_, traits_>::count(const key_t_& key_) const {
	const compare_t comp_;
	const key_t_* const keys_ = m_keys_.data();
	const key_t_* const end_ = keys_ + m_size_;
	hxif_constexpr(!(traits_ & hxtrait_multi)) {
		return hxlower_bound_search_<hxrange<const key_t_*>, key_t_, compare_t, traits_>(
			hxmake_range(keys_, end_), key_, comp_).b ? 1 : 0;
	}
	else {
		const key_t_* const it_ = hxlower_bound_search_<hxrange<const key_t_*>, key_t_, compare_t, traits_>(
			hxmake_range(keys_, end_), key_, comp_).a;
		return hxupper_bound_<hxrange<const key_t_*>, key_t_, compare_t, traits_>(
			hxmake_range(it_, end_), key_, comp_) - it_;
	}
}

template<hxflat_map_concept_ key_t_, hxflat_map_concept_ mapped_t_, typename compare_t_, hxsize_t capacity_, int traits_>
template<typename... args_t_>
hxinline hxattr_flatten auto hxflat_map<key_t_, mapped_t_, compare_t_, capacity_, traits_>::emplace(
		const key_t_& key_, args_t_&&... args_) noexcept -> iterator {
	const compare_t comp_;
	const key_t_* const keys_ = m_keys_.data();
	const key_t_* const end_ = keys_ + m_size_;
	hxif_constexpr(!(traits_ & hxtrait_multi)) {
		const auto lo_ = hxlower_bound_search_<hxrange<const key_t_*>, key_t_, compare_t, traits_>(
			hxmake_range(keys_, end_), key_, comp_);
		const hxsize_t index_ = lo_.a - keys_;
		if(lo_.b) {
			return iterator(this, index_);
		}
		mapped_t_ mapped_(hxforward<args_t_>(args_)...);
		return this->insert_at_(index_, key_, hxmove(mapped_));
	}
	else {
		const hxsize_t index_ = hxlower_bound_search_<hxrange<const key_t_*>, key_t_, compare_t, traits_>(
			hxmake_range(keys_, end_), key_, comp_).a - keys_;
		mapped_t_ mapped_(hxforward<args_t_>(args_)...);
		return this->insert_at_(index_, key_, hxmove(mapped_));
	}
}

template<hxflat_map_concept_ key_t_, hxflat_map_concept_ mapped_t_, typename compare_t_, hxsize_t capacity_, int traits_>
template<hxsize_t capacity_x_>
hxinline hxattr_flatten bool hxflat_map<key_t_, mapped_t_, compare_t_, capacity_, traits_>::operator==(
		const hxflat_map<key_t_, mapped_t_, compare_t_, capacity_x_, traits_>& x_) const {
	return hxequal_range(*this, x_);
}

template<hxflat_map_concept_ key_t_, hxflat_map_concept_ mapped_t_, typename compare_t_, hxsize_t capacity_, int traits_>
hxattr_flatten hxsize_t hxflat_map<key_t_, mapped_t_, compare_t_, capacity_, traits_>::erase(const key_t_& key_) noexcept {
	const compare_t comp_;
	const hxsize_t size_ = m_size_;
	key_t_* hxrestrict k_ = m_keys_.data();
	key_t_* const end_ = k_ + size_;
	hxif_constexpr(!(traits_ & hxtrait_multi)) {
		const auto lo_ = hxlower_bound_search_<hxrange<key_t_*>, key_t_, compare_t, traits_>(
			hxmake_range(k_, end_), key_, comp_);
		if(!lo_.b) { return 0; }
		this->erase(iterator(this, lo_.a - k_));
		return 1;
	}
	else {
		const hxsize_t index_ = hxlower_bound_search_<hxrange<key_t_*>, key_t_, compare_t, traits_>(
			hxmake_range(k_, end_), key_, comp_).a - k_;
		mapped_t_* hxrestrict v_ = m_values_.data();
		const hxsize_t hi_ = hxupper_bound_<hxrange<key_t_*>, key_t_, compare_t, traits_>(
			hxmake_range(k_ + index_, end_), key_, comp_) - k_;
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

template<hxflat_map_concept_ key_t_, hxflat_map_concept_ mapped_t_, typename compare_t_, hxsize_t capacity_, int traits_>
hxattr_flatten auto hxflat_map<key_t_, mapped_t_, compare_t_, capacity_, traits_>::erase(const_iterator it_)
		noexcept -> iterator {
	const hxsize_t size_ = m_size_;
	hxassertf(it_.m_map_ == this && static_cast<size_t>(it_.m_index_) < static_cast<size_t>(size_),
		"bad_iter %zd size %zd", it_.m_index_, size_);
	const hxsize_t index_ = it_.m_index_;
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

template<hxflat_map_concept_ key_t_, hxflat_map_concept_ mapped_t_, typename compare_t_, hxsize_t capacity_, int traits_>
hxinline hxattr_flatten auto hxflat_map<key_t_, mapped_t_, compare_t_, capacity_, traits_>::find(
		const key_t_& key_) const -> const_iterator {
	const compare_t comp_;
	const key_t_* const keys_ = m_keys_.data();
	const key_t_* const end_ = keys_ + m_size_;
	const auto lo_ = hxlower_bound_search_<hxrange<const key_t_*>, key_t_, compare_t, traits_>(
		hxmake_range(keys_, end_), key_, comp_);
	if(lo_.b) {
		return const_iterator(this, static_cast<hxsize_t>(lo_.a - keys_));
	}
	return this->end();
}

template<hxflat_map_concept_ key_t_, hxflat_map_concept_ mapped_t_, typename compare_t_, hxsize_t capacity_, int traits_>
hxinline hxattr_flatten auto hxflat_map<key_t_, mapped_t_, compare_t_, capacity_, traits_>::find(
		const key_t_& key_) -> iterator {
	return iterator(const_cast<const hxflat_map*>(this)->find(key_));
}

template<hxflat_map_concept_ key_t_, hxflat_map_concept_ mapped_t_, typename compare_t_, hxsize_t capacity_, int traits_>
hxinline hxattr_flatten bool hxflat_map<key_t_, mapped_t_, compare_t_, capacity_, traits_>::has_value(
		const key_t_& key_) const {
	return this->find(key_) != this->end();
}

// Fixes gcc + optimizer + sanitizer -Wmaybe-uninitialized bug.
#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#endif

template<hxflat_map_concept_ key_t_, hxflat_map_concept_ mapped_t_, typename compare_t_, hxsize_t capacity_, int traits_>
hxinline hxattr_flatten auto hxflat_map<key_t_, mapped_t_, compare_t_, capacity_, traits_>::insert(
		const key_t_& key_, const mapped_t_& mapped_) noexcept -> iterator {
	const compare_t comp_;
	const key_t_* const keys_ = m_keys_.data();
	const key_t_* const end_ = keys_ + m_size_;
	hxif_constexpr(!(traits_ & hxtrait_multi)) {
		const auto lo_ = hxlower_bound_search_<hxrange<const key_t_*>, key_t_, compare_t, traits_>(
			hxmake_range(keys_, end_), key_, comp_);
		const hxsize_t index_ = lo_.a - keys_;
		if(lo_.b) {
			return iterator(this, index_);
		}
		return this->insert_at_(index_, key_, mapped_);
	}
	else {
		const hxsize_t index_ = hxlower_bound_search_<hxrange<const key_t_*>, key_t_, compare_t, traits_>(
			hxmake_range(keys_, end_), key_, comp_).a - keys_;
		return this->insert_at_(index_, key_, mapped_);
	}
}

template<hxflat_map_concept_ key_t_, hxflat_map_concept_ mapped_t_, typename compare_t_, hxsize_t capacity_, int traits_>
hxinline hxattr_flatten auto hxflat_map<key_t_, mapped_t_, compare_t_, capacity_, traits_>::insert(
		const key_t_& key_, mapped_t_&& mapped_) noexcept -> iterator {
	const compare_t comp_;
	const key_t_* const keys_ = m_keys_.data();
	const key_t_* const end_ = keys_ + m_size_;
	hxif_constexpr(!(traits_ & hxtrait_multi)) {
		const auto lo_ = hxlower_bound_search_<hxrange<const key_t_*>, key_t_, compare_t, traits_>(
			hxmake_range(keys_, end_), key_, comp_);
		const hxsize_t index_ = lo_.a - keys_;
		if(lo_.b) {
			return iterator(this, index_);
		}
		return this->insert_at_(index_, key_, hxmove(mapped_));
	}
	else {
		const hxsize_t index_ = hxlower_bound_search_<hxrange<const key_t_*>, key_t_, compare_t, traits_>(
			hxmake_range(keys_, end_), key_, comp_).a - keys_;
		return this->insert_at_(index_, key_, hxmove(mapped_));
	}
}

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic pop
#endif

template<hxflat_map_concept_ key_t_, hxflat_map_concept_ mapped_t_, typename compare_t_, hxsize_t capacity_, int traits_>
hxinline hxattr_flatten const key_t_*
hxflat_map<key_t_, mapped_t_, compare_t_, capacity_, traits_>::keys(void) const noexcept {
	return m_keys_.data();
}

template<hxflat_map_concept_ key_t_, hxflat_map_concept_ mapped_t_, typename compare_t_, hxsize_t capacity_, int traits_>
template<hxsize_t capacity_x_>
hxinline hxattr_flatten bool hxflat_map<key_t_, mapped_t_, compare_t_, capacity_, traits_>::operator<(
		const hxflat_map<key_t_, mapped_t_, compare_t_, capacity_x_, traits_>& x_) const {
	return hxless_range(*this, x_);
}

template<hxflat_map_concept_ key_t_, hxflat_map_concept_ mapped_t_, typename compare_t_, hxsize_t capacity_, int traits_>
hxinline hxattr_flatten auto hxflat_map<key_t_, mapped_t_, compare_t_, capacity_, traits_>::lower_bound(const key_t_& key_) const
		-> const_iterator {
	const compare_t comp_;
	const key_t_* const keys_ = m_keys_.data();
	return const_iterator(this, hxlower_bound_search_<hxrange<const key_t_*>, key_t_, compare_t, traits_>(
		hxmake_range(keys_, keys_ + m_size_), key_, comp_).a - keys_);
}

template<hxflat_map_concept_ key_t_, hxflat_map_concept_ mapped_t_, typename compare_t_, hxsize_t capacity_, int traits_>
hxinline hxattr_flatten auto hxflat_map<key_t_, mapped_t_, compare_t_, capacity_, traits_>::lower_bound(const key_t_& key_)
		-> iterator {
	return iterator(const_cast<const hxflat_map*>(this)->lower_bound(key_));
}

#if HX_CPLUSPLUS >= 202302L
template<hxflat_map_concept_ key_t_, hxflat_map_concept_ mapped_t_, typename compare_t_, hxsize_t capacity_, int traits_>
template<typename self_t_, typename callable_t_>
hxinline hxattr_flatten auto hxflat_map<key_t_, mapped_t_, compare_t_, capacity_, traits_>::or_else(
		this self_t_&& self_, const key_t_& key_, callable_t_&& callable_)
		-> decltype(self_.end()) {
	auto const found_ = self_.find(key_);
	if(found_ != self_.cend()) {
		return found_;
	}
	return hxforward<callable_t_>(callable_)();
}

template<hxflat_map_concept_ key_t_, hxflat_map_concept_ mapped_t_, typename compare_t_, hxsize_t capacity_, int traits_>
template<typename self_t_, typename callable_t_>
hxinline hxattr_flatten auto hxflat_map<key_t_, mapped_t_, compare_t_, capacity_, traits_>::or_else(
		this self_t_&& self_, const_iterator it_, callable_t_&& callable_)
		-> decltype(self_.end()) {
	if(it_ != self_.cend()) {
		return decltype(self_.end())(it_);
	}
	return hxforward<callable_t_>(callable_)();
}
#endif // HX_CPLUSPLUS >= 202302L

template<hxflat_map_concept_ key_t_, hxflat_map_concept_ mapped_t_, typename compare_t_, hxsize_t capacity_, int traits_>
hxinline hxattr_flatten void hxflat_map<key_t_, mapped_t_, compare_t_, capacity_, traits_>::reserve(hxsize_t cap_,
		hxsystem_allocator_t allocator_, hxalignment_t alignment_) noexcept {
	m_keys_.reserve_storage(cap_, allocator_, alignment_);
	m_values_.reserve_storage(cap_, allocator_, alignment_);
}

template<hxflat_map_concept_ key_t_, hxflat_map_concept_ mapped_t_, typename compare_t_, hxsize_t capacity_, int traits_>
hxinline hxattr_flatten void hxflat_map<key_t_, mapped_t_, compare_t_, capacity_, traits_>::swap(
		hxflat_map& x_) noexcept {
	static_assert(capacity_ == hxallocator_dynamic_capacity,
		"hxallocator_dynamic_capacity required for hxflat_map::swap");
	hxswap_memcpy(*this, x_);
}

template<hxflat_map_concept_ key_t_, hxflat_map_concept_ mapped_t_, typename compare_t_, hxsize_t capacity_, int traits_>
hxinline hxattr_flatten auto hxflat_map<key_t_, mapped_t_, compare_t_, capacity_, traits_>::upper_bound(const key_t_& key_) const
		-> const_iterator {
	const compare_t comp_;
	const key_t_* const keys_ = m_keys_.data();
	return const_iterator(this, hxupper_bound_<hxrange<const key_t_*>, key_t_, compare_t, traits_>(
		hxmake_range(keys_, keys_ + m_size_), key_, comp_) - keys_);
}

template<hxflat_map_concept_ key_t_, hxflat_map_concept_ mapped_t_, typename compare_t_, hxsize_t capacity_, int traits_>
hxinline hxattr_flatten auto hxflat_map<key_t_, mapped_t_, compare_t_, capacity_, traits_>::upper_bound(const key_t_& key_)
		-> iterator {
	return iterator(const_cast<const hxflat_map*>(this)->upper_bound(key_));
}

template<hxflat_map_concept_ key_t_, hxflat_map_concept_ mapped_t_, typename compare_t_, hxsize_t capacity_, int traits_>
hxinline hxattr_flatten const mapped_t_*
hxflat_map<key_t_, mapped_t_, compare_t_, capacity_, traits_>::values(void) const noexcept {
	return m_values_.data();
}

template<hxflat_map_concept_ key_t_, hxflat_map_concept_ mapped_t_, typename compare_t_, hxsize_t capacity_, int traits_>
hxinline hxattr_flatten mapped_t_*
hxflat_map<key_t_, mapped_t_, compare_t_, capacity_, traits_>::values(void) noexcept {
	return m_values_.data();
}

#if HX_CPLUSPLUS >= 202302L
template<hxflat_map_concept_ key_t_, hxflat_map_concept_ mapped_t_, typename compare_t_, hxsize_t capacity_, int traits_>
template<typename self_t_, typename... args_t_>
hxinline hxattr_flatten mapped_t_
hxflat_map<key_t_, mapped_t_, compare_t_, capacity_, traits_>::value_or(
		this self_t_&& self_, const key_t_& key_, args_t_&&... args_) {
	const auto found_ = self_.find(key_);
	if(found_ != self_.cend()) {
		return static_cast<mapped_t_>(hxforward_like<self_t_>(found_.value()));
	}
	return mapped_t_(hxforward<args_t_>(args_)...);
}

template<hxflat_map_concept_ key_t_, hxflat_map_concept_ mapped_t_, typename compare_t_, hxsize_t capacity_, int traits_>
template<typename self_t_, typename... args_t_>
hxinline hxattr_flatten mapped_t_
hxflat_map<key_t_, mapped_t_, compare_t_, capacity_, traits_>::value_or(
		this self_t_&& self_, const_iterator it_, args_t_&&... args_) {
	if(it_ != self_.cend()) {
		const mapped_t_& value_ = it_.value();
		return static_cast<mapped_t_>(hxforward_like<self_t_>(value_));
	}
	return mapped_t_(hxforward<args_t_>(args_)...);
}
#endif // HX_CPLUSPLUS >= 202302L

template<hxflat_map_concept_ key_t_, hxflat_map_concept_ mapped_t_, typename compare_t_, hxsize_t capacity_, int traits_>
template<typename mapped_u_>
hxattr_flatten auto hxflat_map<key_t_, mapped_t_, compare_t_, capacity_, traits_>::insert_at_(
		hxsize_t index_, const key_t_& key_, mapped_u_&& mapped_) noexcept -> iterator {
	const hxsize_t size_ = m_size_;
	hxassert_hard(size_ < m_keys_.capacity(), "hxflat_map full %zd", m_keys_.capacity());
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

HX_INL_END_
#endif // HX_DOXYGEN_PARSER
