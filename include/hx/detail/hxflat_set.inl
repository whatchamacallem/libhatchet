#pragma once
// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

#ifndef LIBHATCHET_VER
#error Internal. Do not include this file directly.
#endif

#ifndef HX_DOXYGEN_PARSER
HX_INL_BEGIN_

template<hxflat_set_concept_ key_t_, typename compare_t_, hxsize_t capacity_, int traits_>
hxinline hxattr_flatten hxflat_set<key_t_, compare_t_, capacity_, traits_>::hxflat_set(void)
	: m_end_(this->data()) {
}

template<hxflat_set_concept_ key_t_, typename compare_t_, hxsize_t capacity_, int traits_>
hxinline hxattr_flatten hxflat_set<key_t_, compare_t_, capacity_, traits_>::hxflat_set(const hxflat_set& x_) noexcept
	: m_end_(this->data()) {
	this->operator=(x_);
}

template<hxflat_set_concept_ key_t_, typename compare_t_, hxsize_t capacity_, int traits_>
hxinline hxattr_flatten hxflat_set<key_t_, compare_t_, capacity_, traits_>::hxflat_set(
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

template<hxflat_set_concept_ key_t_, typename compare_t_, hxsize_t capacity_, int traits_>
hxinline hxflat_set<key_t_, compare_t_, capacity_, traits_>::hxflat_set(hxflat_set&& x_) noexcept {
	static_assert(capacity_ == hxallocator_dynamic_capacity,
		"hxallocator_dynamic_capacity required for temporaries");
	::memcpy(static_cast<void*>(this), &x_, sizeof x_); // NOLINT(bugprone-undefined-memory-manipulation)
	::memset(static_cast<void*>(&x_), 0x00, sizeof x_);
}

template<hxflat_set_concept_ key_t_, typename compare_t_, hxsize_t capacity_, int traits_>
hxinline hxattr_flatten hxflat_set<key_t_, compare_t_, capacity_, traits_>::~hxflat_set(void) noexcept {
	this->clear();
}

#if HX_CPLUSPLUS >= 202302L
template<hxflat_set_concept_ key_t_, typename compare_t_, hxsize_t capacity_, int traits_>
template<typename self_t_, typename callable_t_>
hxinline hxattr_flatten auto hxflat_set<key_t_, compare_t_, capacity_, traits_>::and_then(
		this self_t_&& self_, const key_t_& key_, callable_t_&& callable_)
		-> hxremove_cvref_t<decltype(hxforward<callable_t_>(callable_)(
			hxforward_like<self_t_>(hxdeclval<const key_t_&>())))> {
	const key_t_* const found_ = self_.find(key_);
	return hxforward<self_t_>(self_).and_then(found_, hxforward<callable_t_>(callable_));
}

template<hxflat_set_concept_ key_t_, typename compare_t_, hxsize_t capacity_, int traits_>
template<typename self_t_, typename callable_t_>
hxinline hxattr_flatten auto hxflat_set<key_t_, compare_t_, capacity_, traits_>::and_then(
		this self_t_&& self_, const key_t_* it_, callable_t_&& callable_)
		-> hxremove_cvref_t<decltype(hxforward<callable_t_>(callable_)(
			hxforward_like<self_t_>(hxdeclval<const key_t_&>())))> {
	if(it_ != self_.m_end_) {
		const key_t_& value_ = *it_;
		return hxforward<callable_t_>(callable_)(hxforward_like<self_t_>(value_));
	}
	return hxnil;
}
#endif // HX_CPLUSPLUS >= 202302L

template<hxflat_set_concept_ key_t_, typename compare_t_, hxsize_t capacity_, int traits_>
hxinline hxattr_flatten void hxflat_set<key_t_, compare_t_, capacity_, traits_>::operator=(
		const hxflat_set& x_) noexcept {
	hxassertf(static_cast<const void*>(this) != static_cast<const void*>(&x_), "bad_ref");
	this->clear();
	const key_t_* hxrestrict src_ = x_.data();
	const key_t_* const src_end_ = x_.m_end_;
	key_t_* hxrestrict dst_ = this->data();
	hxassert_hard(src_end_ - src_ <= this->data() + this->capacity() - dst_,
		"hxflat_set capacity exceeded %zd", this->capacity());
	for(; src_ != src_end_; ++src_, ++dst_) {
		::new(dst_) key_t_(*src_);
	}
	m_end_ = dst_;
}

template<hxflat_set_concept_ key_t_, typename compare_t_, hxsize_t capacity_, int traits_>
template<hxsize_t capacity_x_>
hxinline hxattr_flatten void hxflat_set<key_t_, compare_t_, capacity_, traits_>::operator=(
		const hxflat_set<key_t_, compare_t_, capacity_x_, traits_>& x_) noexcept {
	hxassertf(static_cast<const void*>(this) != static_cast<const void*>(&x_), "bad_ref");
	this->clear();
	const key_t_* hxrestrict src_ = x_.data();
	const key_t_* const src_end_ = x_.m_end_;
	key_t_* hxrestrict dst_ = this->data();
	hxassert_hard(src_end_ - src_ <= this->data() + this->capacity() - dst_,
		"hxflat_set capacity exceeded %zd", this->capacity());
	for(; src_ != src_end_; ++src_, ++dst_) {
		::new(dst_) key_t_(*src_);
	}
	m_end_ = dst_;
}

template<hxflat_set_concept_ key_t_, typename compare_t_, hxsize_t capacity_, int traits_>
hxinline hxattr_flatten void hxflat_set<key_t_, compare_t_, capacity_, traits_>::operator=(
		hxflat_set&& x_) noexcept {
	hxassertf(static_cast<const void*>(this) != static_cast<const void*>(&x_), "bad_ref");
	this->swap(x_);
}

template<hxflat_set_concept_ key_t_, typename compare_t_, hxsize_t capacity_, int traits_>
hxinline hxattr_flatten auto hxflat_set<key_t_, compare_t_, capacity_, traits_>::operator[](
		hxsize_t index_) const -> const key_t_* {
	const key_t_* const ptr_ = this->data() + index_;
	hxassert_hard(ptr_ < m_end_, "bad_index %zd", index_);
	return ptr_;
}

template<hxflat_set_concept_ key_t_, typename compare_t_, hxsize_t capacity_, int traits_>
template<hxsize_t capacity_x_>
hxinline hxattr_flatten bool hxflat_set<key_t_, compare_t_, capacity_, traits_>::operator==(
		const hxflat_set<key_t_, compare_t_, capacity_x_, traits_>& x_) const {
	return hxequal_range(*this, x_);
}

template<hxflat_set_concept_ key_t_, typename compare_t_, hxsize_t capacity_, int traits_>
template<hxsize_t capacity_x_>
hxinline hxattr_flatten bool hxflat_set<key_t_, compare_t_, capacity_, traits_>::operator<(
		const hxflat_set<key_t_, compare_t_, capacity_x_, traits_>& x_) const {
	return hxless_range(*this, x_);
}

template<hxflat_set_concept_ key_t_, typename compare_t_, hxsize_t capacity_, int traits_>
template<hxrange_concept_ range_t_>
hxinline hxattr_flatten void hxflat_set<key_t_, compare_t_, capacity_, traits_>::add_range(range_t_&& range_) noexcept {
	hxrestrict_t<decltype(range_.begin())> it_(range_.begin());
	for(const auto end_ = range_.end(); it_ != end_; ++it_) {
		this->insert(hxforward_like<range_t_>(*it_));
	}
}

template<hxflat_set_concept_ key_t_, typename compare_t_, hxsize_t capacity_, int traits_>
hxinline hxattr_flatten hxsize_t hxflat_set<key_t_, compare_t_, capacity_, traits_>::capacity(void) const {
	return hxallocator<key_t_, capacity_>::capacity();
}

template<hxflat_set_concept_ key_t_, typename compare_t_, hxsize_t capacity_, int traits_>
hxinline hxattr_flatten void hxflat_set<key_t_, compare_t_, capacity_, traits_>::clear(void) noexcept {
	key_t_* it_ = this->data();
	const key_t_* const end_ = m_end_;
	m_end_ = this->data();
	for(; it_ != end_; ++it_) {
		it_->key_t_::~key_t_();
	}
}

template<hxflat_set_concept_ key_t_, typename compare_t_, hxsize_t capacity_, int traits_>
hxinline hxattr_flatten hxsize_t hxflat_set<key_t_, compare_t_, capacity_, traits_>::count(const key_t_& key_) const {
	const compare_t comp_;
	const key_t_* const end_ = m_end_;
	hxif_constexpr(!(traits_ & hxtrait_multi)) {
		return hxlower_bound_search_<const hxflat_set&, key_t_, compare_t, traits_>(*this, key_, comp_).b ? 1 : 0;
	}
	else {
		const key_t_* const it_ = hxlower_bound_search_<const hxflat_set&, key_t_, compare_t, traits_>(
			*this, key_, comp_).a;
		return hxupper_bound_<hxrange<const key_t_*>, key_t_, compare_t, traits_>(
			hxmake_range(it_, end_), key_, comp_) - it_;
	}
}

template<hxflat_set_concept_ key_t_, typename compare_t_, hxsize_t capacity_, int traits_>
template<typename... args_t_>
hxinline hxattr_flatten auto hxflat_set<key_t_, compare_t_, capacity_, traits_>::emplace(
		args_t_&&... args_) noexcept -> const key_t_* {
	key_t_ key_(hxforward<args_t_>(args_)...);
	return this->insert(hxmove(key_));
}

template<hxflat_set_concept_ key_t_, typename compare_t_, hxsize_t capacity_, int traits_>
hxattr_flatten hxsize_t hxflat_set<key_t_, compare_t_, capacity_, traits_>::erase(const key_t_& key_) noexcept {
	const compare_t comp_;
	key_t_* end_ = m_end_;
	hxif_constexpr(!(traits_ & hxtrait_multi)) {
		const auto lo_ = hxlower_bound_search_<hxrange<key_t_*>, key_t_, compare_t, traits_>(
			hxmake_range(this->data(), end_), key_, comp_);
		if(!lo_.b) { return 0; }
		this->erase(lo_.a);
		return 1;
	}
	else {
		key_t_* const it_ = hxlower_bound_search_<hxrange<key_t_*>, key_t_, compare_t, traits_>(
			hxmake_range(this->data(), end_), key_, comp_).a;
		key_t_* const hi_ptr_ = hxupper_bound_<hxrange<key_t_*>, key_t_, compare_t, traits_>(
			hxmake_range(it_, end_), key_, comp_);
		const hxsize_t count_ = static_cast<hxsize_t>(hi_ptr_ - it_);
		if(count_ == 0) { return 0; }

		key_t_* hxrestrict dst_ = it_;
		for(key_t_* src_ = hi_ptr_; src_ != end_; ++dst_, ++src_) {
			*dst_ = hxmove(*src_);
		}
		end_ -= count_;
		for(key_t_* pos_ = end_; pos_ != m_end_; ++pos_) {
			pos_->key_t_::~key_t_();
		}
		m_end_ = end_;
		return count_;
	}
}

template<hxflat_set_concept_ key_t_, typename compare_t_, hxsize_t capacity_, int traits_>
hxinline hxattr_flatten auto hxflat_set<key_t_, compare_t_, capacity_, traits_>::erase(const key_t_* it_) noexcept -> const key_t_* {
	hxassertf(it_ != hxnull && static_cast<size_t>(it_ - this->data()) < static_cast<size_t>(m_end_ - this->data()),
		"bad_iter");
	key_t_* const k_ = const_cast<key_t_*>(it_);
	key_t_* const end_ = m_end_ - 1;
	for(key_t_* hxrestrict pos_ = k_; pos_ != end_; ++pos_) {
		*pos_ = hxmove(*(pos_ + 1));
	}
	end_->key_t_::~key_t_();
	m_end_ = end_;
	return k_;
}

template<hxflat_set_concept_ key_t_, typename compare_t_, hxsize_t capacity_, int traits_>
hxinline hxattr_flatten auto hxflat_set<key_t_, compare_t_, capacity_, traits_>::find(const key_t_& key_) const
		-> const key_t_* {
	const compare_t comp_;
	const auto lo_ = hxlower_bound_search_<const hxflat_set&, key_t_, compare_t, traits_>(*this, key_, comp_);
	return lo_.b ? lo_.a : this->end();
}

template<hxflat_set_concept_ key_t_, typename compare_t_, hxsize_t capacity_, int traits_>
hxinline hxattr_flatten bool hxflat_set<key_t_, compare_t_, capacity_, traits_>::has_value(
		const key_t_& key_) const {
	return this->find(key_) != this->end();
}

template<hxflat_set_concept_ key_t_, typename compare_t_, hxsize_t capacity_, int traits_>
hxinline hxattr_flatten auto hxflat_set<key_t_, compare_t_, capacity_, traits_>::insert(const key_t_& key_) noexcept -> const key_t_* {
	const compare_t comp_;
	hxif_constexpr(!(traits_ & hxtrait_multi)) {
		const auto lo_ = hxlower_bound_search_<hxrange<key_t_*>, key_t_, compare_t, traits_>(
			hxmake_range(this->data(), m_end_), key_, comp_);
		if(lo_.b) { return lo_.a; }
		return this->insert_at_(lo_.a, key_);
	}
	else {
		key_t_* const it_ = hxlower_bound_search_<hxrange<key_t_*>, key_t_, compare_t, traits_>(
			hxmake_range(this->data(), m_end_), key_, comp_).a;
		return this->insert_at_(it_, key_);
	}
}

template<hxflat_set_concept_ key_t_, typename compare_t_, hxsize_t capacity_, int traits_>
hxinline hxattr_flatten auto hxflat_set<key_t_, compare_t_, capacity_, traits_>::insert(key_t_&& key_) noexcept -> const key_t_* {
	const compare_t comp_;
	hxif_constexpr(!(traits_ & hxtrait_multi)) {
		const auto lo_ = hxlower_bound_search_<hxrange<key_t_*>, key_t_, compare_t, traits_>(
			hxmake_range(this->data(), m_end_), key_, comp_);
		if(lo_.b) { return lo_.a; }
		return this->insert_at_(lo_.a, hxmove(key_));
	}
	else {
		key_t_* const it_ = hxlower_bound_search_<hxrange<key_t_*>, key_t_, compare_t, traits_>(
			hxmake_range(this->data(), m_end_), key_, comp_).a;
		return this->insert_at_(it_, hxmove(key_));
	}
}

template<hxflat_set_concept_ key_t_, typename compare_t_, hxsize_t capacity_, int traits_>
hxinline hxattr_flatten const key_t_*
hxflat_set<key_t_, compare_t_, capacity_, traits_>::keys(void) const noexcept {
	return this->data();
}

template<hxflat_set_concept_ key_t_, typename compare_t_, hxsize_t capacity_, int traits_>
hxinline hxattr_flatten auto hxflat_set<key_t_, compare_t_, capacity_, traits_>::lower_bound(const key_t_& key_) const
		-> const key_t_* {
	const compare_t comp_;
	return hxlower_bound_search_<const hxflat_set&, key_t_, compare_t, traits_>(*this, key_, comp_).a;
}

#if HX_CPLUSPLUS >= 202302L
template<hxflat_set_concept_ key_t_, typename compare_t_, hxsize_t capacity_, int traits_>
template<typename self_t_, typename callable_t_>
hxinline hxattr_flatten auto hxflat_set<key_t_, compare_t_, capacity_, traits_>::or_else(
		this self_t_&& self_, const key_t_& key_, callable_t_&& callable_)
		-> decltype(self_.end()) {
	auto const found_ = self_.find(key_);
	return self_.or_else(found_, hxforward<callable_t_>(callable_));
}

template<hxflat_set_concept_ key_t_, typename compare_t_, hxsize_t capacity_, int traits_>
template<typename self_t_, typename callable_t_>
hxinline hxattr_flatten auto hxflat_set<key_t_, compare_t_, capacity_, traits_>::or_else(
		this self_t_&& self_, const_iterator it_, callable_t_&& callable_)
		-> decltype(self_.end()) {
	if(it_ != self_.m_end_) {
		return decltype(self_.end())(it_);
	}
	return hxforward<callable_t_>(callable_)();
}
#endif // HX_CPLUSPLUS >= 202302L

template<hxflat_set_concept_ key_t_, typename compare_t_, hxsize_t capacity_, int traits_>
hxinline hxattr_flatten void hxflat_set<key_t_, compare_t_, capacity_, traits_>::reserve(hxsize_t cap_,
		hxsystem_allocator_t allocator_, hxalignment_t alignment_) {
	this->reserve_storage(cap_, allocator_, alignment_);
	if(m_end_ == hxnull) {
		m_end_ = this->data();
	}
}

template<hxflat_set_concept_ key_t_, typename compare_t_, hxsize_t capacity_, int traits_>
hxinline hxattr_flatten void hxflat_set<key_t_, compare_t_, capacity_, traits_>::swap(
		hxflat_set& x_) noexcept {
	static_assert(capacity_ == hxallocator_dynamic_capacity,
		"hxallocator_dynamic_capacity required for hxflat_set::swap");
	hxswap_memcpy(*this, x_);
}

template<hxflat_set_concept_ key_t_, typename compare_t_, hxsize_t capacity_, int traits_>
hxinline hxattr_flatten auto hxflat_set<key_t_, compare_t_, capacity_, traits_>::upper_bound(const key_t_& key_) const
		-> const key_t_* {
	const compare_t comp_;
	return hxupper_bound_<const hxflat_set&, key_t_, compare_t, traits_>(*this, key_, comp_);
}

#if HX_CPLUSPLUS >= 202302L
template<hxflat_set_concept_ key_t_, typename compare_t_, hxsize_t capacity_, int traits_>
template<typename self_t_, typename... args_t_>
hxinline hxattr_flatten key_t_ hxflat_set<key_t_, compare_t_, capacity_, traits_>::value_or(
		this self_t_&& self_, const key_t_& key_, args_t_&&... args_) {
	const key_t_* const found_ = self_.find(key_);
	return hxforward<self_t_>(self_).value_or(found_, hxforward<args_t_>(args_)...);
}

template<hxflat_set_concept_ key_t_, typename compare_t_, hxsize_t capacity_, int traits_>
template<typename self_t_, typename... args_t_>
hxinline hxattr_flatten key_t_ hxflat_set<key_t_, compare_t_, capacity_, traits_>::value_or(
		this self_t_&& self_, const key_t_* it_, args_t_&&... args_) {
	if(it_ != self_.m_end_) {
		const key_t_& value_ = *it_;
		return static_cast<key_t_>(hxforward_like<self_t_>(value_));
	}
	return key_t_(hxforward<args_t_>(args_)...);
}
#endif // HX_CPLUSPLUS >= 202302L

template<hxflat_set_concept_ key_t_, typename compare_t_, hxsize_t capacity_, int traits_>
template<typename key_u_>
hxinline hxattr_flatten auto hxflat_set<key_t_, compare_t_, capacity_, traits_>::insert_at_(
		key_t_* it_, key_u_&& key_) noexcept -> const key_t_* {
	key_t_* const end_ = m_end_;
	hxassert_hard(end_ != this->data() + this->capacity(), "hxflat_set full %zd", this->capacity());
	if(end_ != it_) {
		::new(end_) key_t_(hxmove(*(end_ - 1)));
		for(key_t_* pos_ = end_ - 1; pos_ != it_; --pos_) {
			*pos_ = hxmove(*(pos_ - 1));
		}
		*it_ = hxforward<key_u_>(key_);
	}
	else {
		::new(it_) key_t_(hxforward<key_u_>(key_));
	}
	m_end_ = end_ + 1;
	return it_;
}

HX_INL_END_
#endif // HX_DOXYGEN_PARSER
