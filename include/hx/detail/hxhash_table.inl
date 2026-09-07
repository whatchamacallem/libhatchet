#pragma once
// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

#ifndef LIBHATCHET_VER
#error Internal. Do not include this file directly.
#endif

#ifndef HX_DOXYGEN_PARSER
HX_INL_BEGIN_

/// \cond HIDDEN
// Internal. Type-erasing trampolines.
template<typename node_t_>
hxinline bool hxhash_equal_trampoline_(const hxhash_node_base* node_, const void* key_) {
	return hxkey_equal(static_cast<const node_t_*>(node_)->hash_key(),
		*static_cast<const typename node_t_::key_t*>(key_));
}

template<typename node_t_, typename deleter_t_>
hxinline void hxhash_delete_trampoline_(hxhash_node_base* node_, void* context_) {
	(*static_cast<deleter_t_*>(context_))(static_cast<node_t_*>(node_));
}
/// \endcond

template<typename key_t_, typename value_t_>
hxinline bool hxhash_table_map_node<key_t_, value_t_>::operator==(const hxhash_table_map_node& x_) const {
	return hxhash_table_set_node<key_t_>::operator==(x_) && hxkey_equal(this->m_value_, x_.m_value_);
}

template<typename key_t_, typename value_t_>
hxinline bool hxhash_table_map_node<key_t_, value_t_>::operator<(const hxhash_table_map_node& x_) const {
	if(!hxkey_equal(this->hash_key(), x_.hash_key())) {
		return hxhash_table_set_node<key_t_>::operator<(x_);
	}
	return hxkey_less(this->m_value_, x_.m_value_);
}

template<hxhash_table_concept_ node_t_, typename deleter_t_, bool multi_t_, uint32_t table_size_bits_>
hxinline const node_t_&
hxhash_table<node_t_, deleter_t_, multi_t_, table_size_bits_>::const_iterator::operator*(void) const {
	hxassertf(m_current_node_ != hxnull, "bad_iter");
	return *m_current_node_;
}

template<hxhash_table_concept_ node_t_, typename deleter_t_, bool multi_t_, uint32_t table_size_bits_>
hxinline const node_t_*
hxhash_table<node_t_, deleter_t_, multi_t_, table_size_bits_>::const_iterator::operator->(void) const {
	hxassertf(m_current_node_ != hxnull, "bad_iter");
	return m_current_node_;
}

template<hxhash_table_concept_ node_t_, typename deleter_t_, bool multi_t_, uint32_t table_size_bits_>
hxinline node_t_&
hxhash_table<node_t_, deleter_t_, multi_t_, table_size_bits_>::iterator::operator*(void) const {
	hxassertf(this->m_current_node_ != hxnull, "bad_iter");
	return *this->m_current_node_;
}

template<hxhash_table_concept_ node_t_, typename deleter_t_, bool multi_t_, uint32_t table_size_bits_>
hxinline node_t_*
hxhash_table<node_t_, deleter_t_, multi_t_, table_size_bits_>::iterator::operator->(void) const {
	hxassertf(this->m_current_node_ != hxnull, "bad_iter");
	return this->m_current_node_;
}

template<hxhash_table_concept_ node_t_, typename deleter_t_, bool multi_t_, uint32_t table_size_bits_>
hxinline hxattr_flatten hxhash_table<node_t_, deleter_t_, multi_t_, table_size_bits_>::const_iterator::const_iterator(const hxhash_table* table_)
	: m_next_bucket_(const_cast<hxhash_table*>(table_)->m_table_.data())
	, m_bucket_end_(m_next_bucket_ + table_->m_table_.capacity())
	, m_current_node_(hxnull)
{
	this->next_bucket_();
}

template<hxhash_table_concept_ node_t_, typename deleter_t_, bool multi_t_, uint32_t table_size_bits_>
hxinline hxattr_flatten hxhash_table<node_t_, deleter_t_, multi_t_, table_size_bits_>::const_iterator::const_iterator(
		const hxhash_table* table_, node_t_* node_)
	: m_next_bucket_(const_cast<hxhash_table*>(table_)->get_bucket_head_(node_->hash_value()) + 1)
	, m_bucket_end_(const_cast<hxhash_table*>(table_)->m_table_.data() + table_->m_table_.capacity())
	, m_current_node_(node_)
{
}

template<hxhash_table_concept_ node_t_, typename deleter_t_, bool multi_t_, uint32_t table_size_bits_>
hxinline hxattr_flatten auto hxhash_table<node_t_, deleter_t_, multi_t_, table_size_bits_>::const_iterator::operator++(void)
		-> const_iterator& {
	hxassertf(m_current_node_, "bad_iter"); // !end
	m_current_node_ = static_cast<node_t_*>(m_current_node_->hash_next);
	if(m_current_node_ == hxnull) {
		this->next_bucket_();
	}
	return *this;
}

template<hxhash_table_concept_ node_t_, typename deleter_t_, bool multi_t_, uint32_t table_size_bits_>
hxinline bool hxhash_table<node_t_, deleter_t_, multi_t_, table_size_bits_>::const_iterator::operator==(
		const const_iterator& x_) const {
	return m_current_node_ == x_.m_current_node_;
}

template<hxhash_table_concept_ node_t_, typename deleter_t_, bool multi_t_, uint32_t table_size_bits_>
hxinline void hxhash_table<node_t_, deleter_t_, multi_t_, table_size_bits_>::const_iterator::next_bucket_(void) {
	hxassertf(m_current_node_ == hxnull, "sys_err");
	hxhash_node_base** hxrestrict it_ = m_next_bucket_;
	while(it_ != m_bucket_end_) {
		node_t_* const node_ = static_cast<node_t_*>(*it_++);
		if(node_ != hxnull) {
			m_current_node_ = node_;
			m_next_bucket_ = it_;
			break;
		}
	}
}

// hxhash_table out-of-line implementations

template<hxhash_table_concept_ node_t_, typename deleter_t_, bool multi_t_, uint32_t table_size_bits_>
hxinline hxattr_flatten hxhash_table<node_t_, deleter_t_, multi_t_, table_size_bits_>::hxhash_table(deleter_t_ deleter_)
		: deleter_t_(hxmove(deleter_)), m_size_(0) {
	static_assert(hxis_same<decltype(hxdeclval<const node_t_&>().hash_key()),
		const typename node_t_::key_t&>(),
		"node_t::hash_key must be: const key_t& hash_key() const");
}

template<hxhash_table_concept_ node_t_, typename deleter_t_, bool multi_t_, uint32_t table_size_bits_>
template<hxrange_concept_ range_t_, hxenable_if_t<!hxis_lvalue_reference<range_t_>(), int> >
hxinline hxattr_flatten void hxhash_table<node_t_, deleter_t_, multi_t_, table_size_bits_>::add_range(
		range_t_&& range_) noexcept {
	hxrestrict_t<decltype(range_.begin())> it_(range_.begin());
	for(const auto end_ = range_.end(); it_ != end_; ++it_) {
		this->insert(&*it_);
	}
}

#if HX_CPLUSPLUS >= 202302L
template<hxhash_table_concept_ node_t_, typename deleter_t_, bool multi_t_, uint32_t table_size_bits_>
template<typename self_t_, typename callable_t_>
hxinline hxattr_flatten auto hxhash_table<node_t_, deleter_t_, multi_t_, table_size_bits_>::and_then(
		this self_t_&& self_, const key_t& key_, callable_t_&& callable_)
		-> hxremove_cvref_t<decltype(
			hxforward<callable_t_>(callable_)(
				hxforward_like<self_t_>(hxdeclval<node_t_&>())))> {
	const auto found_ = self_.find(key_);
	if(found_ != self_.end()) {
		return hxforward<callable_t_>(callable_)(
			hxforward_like<self_t_>(*found_));
	}
	return hxnil;
}

template<hxhash_table_concept_ node_t_, typename deleter_t_, bool multi_t_, uint32_t table_size_bits_>
template<typename self_t_, typename callable_t_>
hxinline hxattr_flatten auto hxhash_table<node_t_, deleter_t_, multi_t_, table_size_bits_>::and_then(
		this self_t_&& self_, const_iterator it_, callable_t_&& callable_)
		-> hxremove_cvref_t<decltype(
			hxforward<callable_t_>(callable_)(hxforward_like<self_t_>(
				hxdeclval<const node_t_&>())))> {
	if(it_ != self_.end()) {
		const node_t_& node_ = *it_;
		return hxforward<callable_t_>(callable_)(hxforward_like<self_t_>(node_));
	}
	return hxnil;
}
#endif // HX_CPLUSPLUS >= 202302L

template<hxhash_table_concept_ node_t_, typename deleter_t_, bool multi_t_, uint32_t table_size_bits_>
template<typename deleter_u_>
hxinline hxattr_flatten void hxhash_table<node_t_, deleter_t_, multi_t_, table_size_bits_>::clear(
		deleter_u_&& deleter_) noexcept {
	if(m_size_ != 0) {
		if(deleter_) {
			this->table_clear_(m_table_.data(), m_table_.capacity(),
				&hxhash_delete_trampoline_<node_t_, hxremove_reference_t<deleter_u_>>, &deleter_);
			m_size_ = 0;
		}
		else {
			this->release_all();
		}
	}
}

template<hxhash_table_concept_ node_t_, typename deleter_t_, bool multi_t_, uint32_t table_size_bits_>
hxinline hxattr_flatten hxsize_t hxhash_table<node_t_, deleter_t_, multi_t_, table_size_bits_>::count(
		const typename node_t_::key_t& key_) const {
	const hxhash_t hash_ = node_t_::hash_value(key_);
	hxif_constexpr(!multi_t_) {
		for(const node_t_* node_ = static_cast<const node_t_*>(*this->get_bucket_head_(hash_)); node_;
				node_ = static_cast<const node_t_*>(node_->hash_next)) {
			if(hxkey_equal(node_->hash_key(), key_)) {
				return 1;
			}
		}
		return 0;
	}
	else {
		const hxhash_node_base* const head_ = *this->get_bucket_head_(hash_);
		return this->chain_count_multi_(head_, &key_, &hxhash_equal_trampoline_<node_t_>);
	}
}

template<hxhash_table_concept_ node_t_, typename deleter_t_, bool multi_t_, uint32_t table_size_bits_>
template<hxsystem_allocator_t allocator_, hxalignment_t alignment_, bool multi_, class... args_t_>
hxinline hxattr_flatten auto hxhash_table<node_t_, deleter_t_, multi_t_, table_size_bits_>::emplace(args_t_&&... args_) noexcept
		-> hxenable_if_t<multi_, iterator> {
	node_t_* const ptr_ = hxnew<node_t_, allocator_, alignment_>(hxforward<args_t_>(args_)...);
	return this->insert(ptr_);
}

template<hxhash_table_concept_ node_t_, typename deleter_t_, bool multi_t_, uint32_t table_size_bits_>
template<typename deleter_u_>
hxinline hxattr_flatten hxsize_t hxhash_table<node_t_, deleter_t_, multi_t_, table_size_bits_>::erase(
		const typename node_t_::key_t& key_, deleter_u_&& deleter_) noexcept {
	const hxhash_t hash_ = node_t_::hash_value(key_);
	hxhash_node_base** const head_ = this->get_bucket_head_(hash_);
	hxif_constexpr(!multi_t_) {
		hxhash_node_base* const node_ = this->chain_extract_(head_, &key_, &hxhash_equal_trampoline_<node_t_>);
		if(node_ == hxnull) {
			return 0;
		}
		if(deleter_) {
			deleter_(static_cast<node_t_*>(node_));
		}
		--m_size_;
		return 1;
	}
	else {
		const hxhash_deleter_fn_ deleter_fn_ =
			deleter_ ? &hxhash_delete_trampoline_<node_t_, hxremove_reference_t<deleter_u_>> : hxnull;
		const hxsize_t erased_ = this->chain_erase_multi_(head_, &key_,
			&hxhash_equal_trampoline_<node_t_>, deleter_fn_, &deleter_);
		m_size_ -= erased_;
		return erased_;
	}
}

template<hxhash_table_concept_ node_t_, typename deleter_t_, bool multi_t_, uint32_t table_size_bits_>
hxinline hxattr_flatten hxsize_t hxhash_table<node_t_, deleter_t_, multi_t_, table_size_bits_>::erase(const typename node_t_::key_t& key_) noexcept {
	return this->erase(key_, static_cast<deleter_t_&>(*this));
}

template<hxhash_table_concept_ node_t_, typename deleter_t_, bool multi_t_, uint32_t table_size_bits_>
hxinline hxattr_flatten typename hxhash_table<node_t_, deleter_t_, multi_t_, table_size_bits_>::iterator
hxhash_table<node_t_, deleter_t_, multi_t_, table_size_bits_>::erase(const const_iterator& it_) noexcept {
	node_t_* const node_ = it_.m_current_node_;
	hxassertf(node_ != hxnull, "bad_iter");
	iterator next_(this, node_);
	++next_;
	this->chain_erase_at_(this->get_bucket_head_(node_->hash_value()), node_);
	--m_size_;
	deleter_t_& deleter_ = static_cast<deleter_t_&>(*this);
	if(deleter_) {
		deleter_(node_);
	}
	return next_;
}

template<hxhash_table_concept_ node_t_, typename deleter_t_, bool multi_t_, uint32_t table_size_bits_>
hxattr_flatten hxptr<node_t_, deleter_t_>
hxhash_table<node_t_, deleter_t_, multi_t_, table_size_bits_>::extract(
		const const_iterator& it_) noexcept {
	node_t_* const node_ = it_.m_current_node_;
	hxassertf(node_ != hxnull, "bad_iter");
	this->chain_erase_at_(this->get_bucket_head_(node_->hash_value()), node_);
	--m_size_;
	return hxptr<node_t_, deleter_t_>(node_, static_cast<deleter_t_&>(*this));
}

template<hxhash_table_concept_ node_t_, typename deleter_t_, bool multi_t_, uint32_t table_size_bits_>
hxattr_flatten hxptr<node_t_, deleter_t_>
hxhash_table<node_t_, deleter_t_, multi_t_, table_size_bits_>::extract(
		const typename node_t_::key_t& key_) noexcept {
	const hxhash_t hash_ = node_t_::hash_value(key_);
	node_t_* const node_ = static_cast<node_t_*>(this->chain_extract_(
		this->get_bucket_head_(hash_), &key_, &hxhash_equal_trampoline_<node_t_>));
	m_size_ -= node_ != hxnull;
	return hxptr<node_t_, deleter_t_>(node_, static_cast<deleter_t_&>(*this));
}

template<hxhash_table_concept_ node_t_, typename deleter_t_, bool multi_t_, uint32_t table_size_bits_>
hxinline hxattr_flatten auto hxhash_table<node_t_, deleter_t_, multi_t_, table_size_bits_>::find(
		const typename node_t_::key_t& key_, const const_iterator& previous_) const -> const_iterator {
	const hxhash_t hash_ = node_t_::hash_value(key_);
	const node_t_* const previous_node_ = previous_.m_current_node_;
	if(previous_node_ == hxnull) {
		for(const node_t_* node_ = static_cast<const node_t_*>(*this->get_bucket_head_(hash_)); node_;
				node_ = static_cast<const node_t_*>(node_->hash_next)) {
			if(hxkey_equal(node_->hash_key(), key_)) {
				return const_iterator(this, const_cast<node_t_*>(node_));
			}
		}
	}
	else hxif_constexpr(multi_t_) {
		hxassertf(hxkey_equal(key_, previous_node_->hash_key()), "history_mismatch");
		hxassertf(hash_ == previous_node_->hash_value(), "history_mismatch");
		for(const node_t_* node_ = static_cast<const node_t_*>(previous_node_->hash_next); node_;
				node_ = static_cast<const node_t_*>(node_->hash_next)) {
			if(hxkey_equal(node_->hash_key(), key_)) {
				return const_iterator(this, const_cast<node_t_*>(node_));
			}
		}
	}
	return this->end();
}

template<hxhash_table_concept_ node_t_, typename deleter_t_, bool multi_t_, uint32_t table_size_bits_>
hxinline hxattr_flatten auto hxhash_table<node_t_, deleter_t_, multi_t_, table_size_bits_>::find(
		const typename node_t_::key_t& key_, const const_iterator& previous_) -> iterator {
	return iterator(static_cast<const hxhash_table*>(this)->find(key_, previous_));
}

template<hxhash_table_concept_ node_t_, typename deleter_t_, bool multi_t_, uint32_t table_size_bits_>
hxinline hxattr_flatten bool hxhash_table<node_t_, deleter_t_, multi_t_, table_size_bits_>::has_value(
		const key_t& key_) const {
	return this->find(key_) != this->end();
}

template<hxhash_table_concept_ node_t_, typename deleter_t_, bool multi_t_, uint32_t table_size_bits_>
hxinline hxattr_flatten const deleter_t_&
hxhash_table<node_t_, deleter_t_, multi_t_, table_size_bits_>::deleter(void) const {
	return static_cast<const deleter_t_&>(*this);
}

template<hxhash_table_concept_ node_t_, typename deleter_t_, bool multi_t_, uint32_t table_size_bits_>
hxinline hxattr_flatten deleter_t_& hxhash_table<node_t_, deleter_t_, multi_t_, table_size_bits_>::deleter(void) {
	return static_cast<deleter_t_&>(*this);
}

template<hxhash_table_concept_ node_t_, typename deleter_t_, bool multi_t_, uint32_t table_size_bits_>
hxattr_flatten typename hxhash_table<node_t_, deleter_t_, multi_t_, table_size_bits_>::iterator
hxhash_table<node_t_, deleter_t_, multi_t_, table_size_bits_>::insert(node_t_* ptr_) noexcept {
	hxassertf([&]{ const iterator found_ = this->find(ptr_->hash_key());
		return found_ == this->end() || &*found_ != ptr_; }(), "reinsert_err");
	hxhash_node_base** const head_ = this->get_bucket_head_(ptr_->hash_value());
	hxif_constexpr(!multi_t_) {
		for(node_t_* existing_ = static_cast<node_t_*>(*head_); existing_;
				existing_ = static_cast<node_t_*>(existing_->hash_next)) {
			if(hxkey_equal(existing_->hash_key(), ptr_->hash_key())) {
				deleter_t_& deleter_ = static_cast<deleter_t_&>(*this);
				if(deleter_) {
					deleter_(ptr_);
				}
				return iterator(this, existing_);
			}
		}
	}
	ptr_->hash_next = *head_;
	*head_ = ptr_;
	++m_size_;
	return iterator(this, ptr_);
}

template<hxhash_table_concept_ node_t_, typename deleter_t_, bool multi_t_, uint32_t table_size_bits_>
template<typename deleter_u_>
hxattr_flatten typename hxhash_table<node_t_, deleter_t_, multi_t_, table_size_bits_>::iterator
hxhash_table<node_t_, deleter_t_, multi_t_, table_size_bits_>::insert(hxptr<node_t_, deleter_u_>&& ptr_) noexcept {
	node_t_* const raw_ = ptr_.get();
	hxassertf([&]{ const iterator found_ = this->find(raw_->hash_key());
		return found_ == this->end() || &*found_ != raw_; }(), "reinsert_err");
	hxhash_node_base** const head_ = this->get_bucket_head_(raw_->hash_value());
	hxif_constexpr(!multi_t_) {
		for(node_t_* existing_ = static_cast<node_t_*>(*head_); existing_;
				existing_ = static_cast<node_t_*>(existing_->hash_next)) {
			if(hxkey_equal(existing_->hash_key(), raw_->hash_key())) {
				return iterator(this, existing_);
			}
		}
	}
	raw_->hash_next = *head_;
	*head_ = raw_;
	node_t_* const released_ = ptr_.release();
	hxassertf(released_ == raw_, "sys_err"); (void)released_;
	++m_size_;
	return iterator(this, raw_);
}

template<hxhash_table_concept_ node_t_, typename deleter_t_, bool multi_t_, uint32_t table_size_bits_>
hxinline hxattr_flatten float hxhash_table<node_t_, deleter_t_, multi_t_, table_size_bits_>::load_factor(void) const {
	return m_table_.capacity()
		? (static_cast<float>(m_size_) / static_cast<float>(m_table_.capacity()))
		: 0.0f;
}

template<hxhash_table_concept_ node_t_, typename deleter_t_, bool multi_t_, uint32_t table_size_bits_>
hxattr_flatten hxsize_t hxhash_table<node_t_, deleter_t_, multi_t_, table_size_bits_>::load_max(void) const {
	hxsize_t maximum_ = 0;
	const hxhash_node_base*const* const it_end_ = m_table_.data() + m_table_.capacity();
	for(const hxhash_node_base*const* hxrestrict it_ = m_table_.data(); it_ != it_end_; ++it_) {
		maximum_ = hxmax(maximum_, this->chain_length_(*it_));
	}
	return maximum_;
}

#if HX_CPLUSPLUS >= 202302L
template<hxhash_table_concept_ node_t_, typename deleter_t_, bool multi_t_, uint32_t table_size_bits_>
template<typename self_t_, typename callable_t_>
hxinline hxattr_flatten auto hxhash_table<node_t_, deleter_t_, multi_t_, table_size_bits_>::or_else(
		this self_t_&& self_, const key_t& key_, callable_t_&& callable_)
		-> decltype(self_.end()) {
	auto const found_ = self_.find(key_);
	if(found_ != self_.end()) {
		return found_;
	}
	return hxforward<callable_t_>(callable_)();
}

template<hxhash_table_concept_ node_t_, typename deleter_t_, bool multi_t_, uint32_t table_size_bits_>
template<typename self_t_, typename callable_t_>
hxinline hxattr_flatten auto hxhash_table<node_t_, deleter_t_, multi_t_, table_size_bits_>::or_else(
		this self_t_&& self_, const_iterator it_, callable_t_&& callable_)
		-> decltype(self_.end()) {
	if(it_ != self_.end()) {
		return it_;
	}
	return hxforward<callable_t_>(callable_)();
}
#endif // HX_CPLUSPLUS >= 202302L

template<hxhash_table_concept_ node_t_, typename deleter_t_, bool multi_t_, uint32_t table_size_bits_>
hxinline hxattr_flatten void hxhash_table<node_t_, deleter_t_, multi_t_, table_size_bits_>::release_all(void) {
	if(m_size_ != 0) {
		::memset(m_table_.data(), 0x00, sizeof(hxhash_node_base*) * static_cast<size_t>(m_table_.capacity()));
		m_size_ = 0;
	}
}

template<hxhash_table_concept_ node_t_, typename deleter_t_, bool multi_t_, uint32_t table_size_bits_>
hxinline hxattr_flatten hxsize_t hxhash_table<node_t_, deleter_t_, multi_t_, table_size_bits_>::release_key(
		const typename node_t_::key_t& key_) {
	return this->erase(key_, static_cast<void(*)(node_t_*)>(hxnull));
}

template<hxhash_table_concept_ node_t_, typename deleter_t_, bool multi_t_, uint32_t table_size_bits_>
hxattr_flatten hxptr<node_t_, deleter_t_>
hxhash_table<node_t_, deleter_t_, multi_t_, table_size_bits_>::replace(node_t_* ptr_) noexcept {
	hxassertf(ptr_ != hxnull, "null_node");
	node_t_* const replaced_ = static_cast<node_t_*>(this->chain_replace_(
		this->get_bucket_head_(ptr_->hash_value()), ptr_,
		&ptr_->hash_key(), &hxhash_equal_trampoline_<node_t_>));
	m_size_ += replaced_ == hxnull;
	return hxptr<node_t_, deleter_t_>(replaced_, static_cast<deleter_t_&>(*this));
}

template<hxhash_table_concept_ node_t_, typename deleter_t_, bool multi_t_, uint32_t table_size_bits_>
hxinline hxattr_flatten void hxhash_table<node_t_, deleter_t_, multi_t_, table_size_bits_>::set_size_bits(hxhash_t bits_) {
	static_assert(table_size_bits_ == hxallocator_dynamic_capacity,
		"set_size_bits requires dynamic capacity");
	m_table_.set_size_bits_(bits_, hxhash_bits - bits_);
}

template<hxhash_table_concept_ node_t_, typename deleter_t_, bool multi_t_, uint32_t table_size_bits_>
template<hxsystem_allocator_t allocator_, hxalignment_t alignment_, bool multi_, class... args_t_>
hxinline hxattr_flatten auto hxhash_table<node_t_, deleter_t_, multi_t_, table_size_bits_>::try_emplace(
		const typename node_t_::key_t& key_, args_t_&&... args_) noexcept -> hxenable_if_t<!multi_, iterator> {
	const iterator existing_ = this->find(key_);
	if(existing_ != this->end()) {
		return existing_;
	}
	node_t_* const ptr_ = hxnew<node_t_, allocator_, alignment_>(hxforward<args_t_>(args_)...);
	return this->insert(ptr_);
}

#if HX_CPLUSPLUS >= 202302L
template<hxhash_table_concept_ node_t_, typename deleter_t_, bool multi_t_, uint32_t table_size_bits_>
template<typename self_t_>
hxinline hxattr_flatten auto hxhash_table<node_t_, deleter_t_, multi_t_, table_size_bits_>::value_or(
		this self_t_&& self_, const key_t& key_, const node_t_* default_value_)
		-> decltype(self_.end()) {
	auto const found_ = self_.find(key_);
	if(found_ != self_.end()) {
		return found_;
	}
	return decltype(self_.end())(&self_, const_cast<node_t_*>(default_value_));
}

template<hxhash_table_concept_ node_t_, typename deleter_t_, bool multi_t_, uint32_t table_size_bits_>
template<typename self_t_>
hxinline hxattr_flatten auto hxhash_table<node_t_, deleter_t_, multi_t_, table_size_bits_>::value_or(
		this self_t_&& self_, const_iterator it_, const node_t_* default_value_)
		-> decltype(self_.end()) {
	if(it_ != self_.end()) {
		return it_;
	}
	return decltype(self_.end())(&self_, const_cast<node_t_*>(default_value_));
}
#endif // HX_CPLUSPLUS >= 202302L

template<hxhash_table_concept_ node_t_, typename deleter_t_, bool multi_t_, uint32_t table_size_bits_>
hxinline hxattr_flatten hxhash_node_base** hxhash_table<node_t_, deleter_t_, multi_t_, table_size_bits_>::get_bucket_head_(hxhash_t hash_) {
	const uint32_t shift_ = m_table_.get_hash_shift_();
	const hxhash_t index_ = hash_ >> shift_;
	hxassertf(static_cast<hxsize_t>(index_) < m_table_.capacity(), "sys_err");
	return m_table_.data() + index_;
}

template<hxhash_table_concept_ node_t_, typename deleter_t_, bool multi_t_, uint32_t table_size_bits_>
hxinline hxattr_flatten const hxhash_node_base*const* hxhash_table<node_t_, deleter_t_, multi_t_, table_size_bits_>::get_bucket_head_(hxhash_t hash_) const {
	const uint32_t shift_ = m_table_.get_hash_shift_();
	const hxhash_t index_ = hash_ >> shift_;
	hxassertf(static_cast<hxsize_t>(index_) < m_table_.capacity(), "sys_err");
	return m_table_.data() + index_;
}

HX_INL_END_
#endif // HX_DOXYGEN_PARSER
