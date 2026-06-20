#pragma once
// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

#ifndef LIBHATCHET_VER
#error Internal. Do not include this file directly.
#endif

#ifndef HX_DOXYGEN_PARSER
HX_BEGIN_INL_

template<hxhash_table_concept_ node_t_, typename deleter_t_, bool multi_t_, hxhash_t table_size_bits_>
inline const node_t_&
hxhash_table<node_t_, deleter_t_, multi_t_, table_size_bits_>::const_iterator::operator*(void) const {
	hxassertmsg(m_current_node_ != hxnull, "invalid_iterator");
	return *m_current_node_;
}

template<hxhash_table_concept_ node_t_, typename deleter_t_, bool multi_t_, hxhash_t table_size_bits_>
inline const node_t_*
hxhash_table<node_t_, deleter_t_, multi_t_, table_size_bits_>::const_iterator::operator->(void) const {
	hxassertmsg(m_current_node_ != hxnull, "invalid_iterator");
	return m_current_node_;
}

template<hxhash_table_concept_ node_t_, typename deleter_t_, bool multi_t_, hxhash_t table_size_bits_>
inline node_t_&
hxhash_table<node_t_, deleter_t_, multi_t_, table_size_bits_>::iterator::operator*(void) const {
	hxassertmsg(this->m_current_node_ != hxnull, "invalid_iterator");
	return *this->m_current_node_;
}

template<hxhash_table_concept_ node_t_, typename deleter_t_, bool multi_t_, hxhash_t table_size_bits_>
inline node_t_*
hxhash_table<node_t_, deleter_t_, multi_t_, table_size_bits_>::iterator::operator->(void) const {
	hxassertmsg(this->m_current_node_ != hxnull, "invalid_iterator");
	return this->m_current_node_;
}

template<hxhash_table_concept_ node_t_, typename deleter_t_, bool multi_t_, hxhash_t table_size_bits_>
inline hxhash_table<node_t_, deleter_t_, multi_t_, table_size_bits_>::const_iterator::const_iterator(const hxhash_table* table_)
	: m_hash_table_(const_cast<hxhash_table*>(table_))
	, m_next_index_(0)
	, m_current_node_(hxnull)
{
	next_bucket();
}

template<hxhash_table_concept_ node_t_, typename deleter_t_, bool multi_t_, hxhash_t table_size_bits_>
inline hxhash_table<node_t_, deleter_t_, multi_t_, table_size_bits_>::const_iterator::const_iterator(
		const hxhash_table* table_, node_t_* node_)
	: m_hash_table_(const_cast<hxhash_table*>(table_))
	, m_next_index_(static_cast<hxsize_t>(node_->hash_value() >> (hxhash_bits - table_->m_table_.get_table_size_bits())) + 1)
	, m_current_node_(node_)
{
}

template<hxhash_table_concept_ node_t_, typename deleter_t_, bool multi_t_, hxhash_t table_size_bits_>
inline auto hxhash_table<node_t_, deleter_t_, multi_t_, table_size_bits_>::const_iterator::operator++(void)
		-> const_iterator& {
	hxassertmsg(m_current_node_, "invalid_iterator"); // !end
	m_current_node_ = m_current_node_->hash_next();
	if(m_current_node_ == hxnull) {
		this->next_bucket();
	}
	return *this;
}

template<hxhash_table_concept_ node_t_, typename deleter_t_, bool multi_t_, hxhash_t table_size_bits_>
inline bool
hxhash_table<node_t_, deleter_t_, multi_t_, table_size_bits_>::const_iterator::operator==(
	const const_iterator& x_) const {
	return m_current_node_ == x_.m_current_node_;
}

#if HX_CPLUSPLUS < 202002L // C++20 defaults != from ==.
template<hxhash_table_concept_ node_t_, typename deleter_t_, bool multi_t_, hxhash_t table_size_bits_>
inline bool
hxhash_table<node_t_, deleter_t_, multi_t_, table_size_bits_>::const_iterator::operator!=(
	const const_iterator& x_) const {
	return m_current_node_ != x_.m_current_node_;
}
#endif

template<hxhash_table_concept_ node_t_, typename deleter_t_, bool multi_t_, hxhash_t table_size_bits_>
inline void
hxhash_table<node_t_, deleter_t_, multi_t_, table_size_bits_>::const_iterator::next_bucket(void) {
	hxassertmsg(m_hash_table_ != hxnull, "invalid_iterator");
	hxassertmsg(m_current_node_ == hxnull, "invalid_iterator");
	while(m_next_index_ < m_hash_table_->m_table_.capacity()) {
		if(node_t_* n_ = m_hash_table_->m_table_.data()[m_next_index_++]) {
			m_current_node_ = n_;
			return;
		}
	}
}

// hxhash_table out-of-line implementations

template<hxhash_table_concept_ node_t_, typename deleter_t_, bool multi_t_, hxhash_t table_size_bits_>
inline hxhash_table<node_t_, deleter_t_, multi_t_, table_size_bits_>::hxhash_table(deleter_t_ deleter_)
		: m_deleter_(deleter_) {
	m_size_ = 0;
	// Pre-requires keyword checks.
	static_assert(hxis_same<decltype(hxdeclval<const node_t_&>().hash_next()),
		node_t_*>::value,
		"node_t::hash_next must be: node_t* hash_next() const");
	static_assert(hxis_same<decltype(hxdeclval<node_t_&>().set_hash_next(
		static_cast<node_t_*>(hxnull))), void>::value,
		"node_t::set_hash_next must be: void set_hash_next(node_t*)");
	static_assert(hxis_same<decltype(hxdeclval<const node_t_&>().hash_key()),
		const typename node_t_::key_t&>::value,
		"node_t::hash_key must be: const key_t& hash_key() const");
	static_assert(hxis_same<decltype(hxdeclval<const node_t_&>().hash_value()),
		hxhash_t>::value,
		"node_t::hash_value must be: hxhash_t hash_value() const");
}

template<hxhash_table_concept_ node_t_, typename deleter_t_, bool multi_t_, hxhash_t table_size_bits_>
template<typename deleter_override_t_>
inline void hxhash_table<node_t_, deleter_t_, multi_t_, table_size_bits_>::clear(
		const deleter_override_t_& deleter_) noexcept {
	if(m_size_ != 0) {
		if(deleter_) {
			node_t_* const* const it_end_ = m_table_.data() + m_table_.capacity();
			for(node_t_** it_ = m_table_.data(); it_ != it_end_; ++it_) {
				node_t_* n_ = *it_;
				if(n_) {
					*it_ = hxnull;
					do {
						node_t_* const next_ = n_->hash_next();
						deleter_(n_);
						n_ = next_;
					} while(n_);
				}
			}
			m_size_ = 0;
		}
		else {
			this->release_all();
		}
	}
}

template<hxhash_table_concept_ node_t_, typename deleter_t_, bool multi_t_, hxhash_t table_size_bits_>
inline hxsize_t hxhash_table<node_t_, deleter_t_, multi_t_, table_size_bits_>::count(
		const typename node_t_::key_t& key_) const {
	hxsize_t total_ = 0;
	const hxhash_t hash_ = node_t_::hash_value(key_);
	for(const node_t_* n_ = *this->get_bucket_head_(hash_); n_; n_ = n_->hash_next()) {
		if(hxkey_equal(n_->hash_key(), key_)) {
			++total_;
		}
	}
	return total_;
}

template<hxhash_table_concept_ node_t_, typename deleter_t_, bool multi_t_, hxhash_t table_size_bits_>
template<hxsystem_allocator_t allocator_, hxalignment_t align_, bool multi_, class... args_t_>
inline auto hxhash_table<node_t_, deleter_t_, multi_t_, table_size_bits_>::emplace(args_t_&&... args_) noexcept
		-> hxenable_if_t<multi_, iterator> {
	node_t_* const ptr_ = hxnew<node_t_, allocator_, align_>(hxforward<args_t_>(args_)...);
	return this->insert(ptr_);
}

template<hxhash_table_concept_ node_t_, typename deleter_t_, bool multi_t_, hxhash_t table_size_bits_>
template<typename deleter_override_t_>
inline hxsize_t hxhash_table<node_t_, deleter_t_, multi_t_, table_size_bits_>::erase(
		const typename node_t_::key_t& key_, const deleter_override_t_& deleter_) noexcept {
	const hxhash_t hash_ = node_t_::hash_value(key_);
	node_t_** const head_ = this->get_bucket_head_(hash_);
	node_t_* n_ = *head_;
	if(n_ == hxnull) { return 0; }

	// Unlink matches at the head of the bucket so the loop below only unlinks
	// interior nodes. The bucket head is only written when it changes.
	hxsize_t count_ = 0;
	if(hxkey_equal(n_->hash_key(), key_)) {
		do {
			node_t_* const next_ = n_->hash_next();
			if(deleter_) {
				deleter_(n_);
			}
			++count_;
			n_ = next_;
		} while(n_ != hxnull && hxkey_equal(n_->hash_key(), key_));
		*head_ = n_;
	}

	if(n_ != hxnull) {
		node_t_* previous_ = n_;
		n_ = n_->hash_next();
		while(n_ != hxnull) {
			node_t_* const next_ = n_->hash_next();
			if(hxkey_equal(n_->hash_key(), key_)) {
				previous_->set_hash_next(next_);
				if(deleter_) {
					deleter_(n_);
				}
				++count_;
			}
			else {
				previous_ = n_;
			}
			n_ = next_;
		}
	}
	m_size_ -= count_;
	return count_;
}

template<hxhash_table_concept_ node_t_, typename deleter_t_, bool multi_t_, hxhash_t table_size_bits_>
inline hxptr<node_t_, deleter_t_>
hxhash_table<node_t_, deleter_t_, multi_t_, table_size_bits_>::extract(
		const typename node_t_::key_t& key_) noexcept {
	const hxhash_t hash_ = node_t_::hash_value(key_);
	node_t_** const head_ = this->get_bucket_head_(hash_);
	node_t_* previous_ = hxnull;
	for(node_t_* n_ = *head_; n_ != hxnull; n_ = n_->hash_next()) {
		if(hxkey_equal(n_->hash_key(), key_)) {
			if(previous_ != hxnull) {
				previous_->set_hash_next(n_->hash_next());
			}
			else {
				*head_ = n_->hash_next();
			}
			--m_size_;
			return hxptr<node_t_, deleter_t_>(n_, m_deleter_);
		}
		previous_ = n_;
	}
	return hxptr<node_t_, deleter_t_>(hxnull, m_deleter_);
}

template<hxhash_table_concept_ node_t_, typename deleter_t_, bool multi_t_, hxhash_t table_size_bits_>
inline node_t_* hxhash_table<node_t_, deleter_t_, multi_t_, table_size_bits_>::find(
		const typename node_t_::key_t& key_, const node_t_* previous_) {
	const hxhash_t hash_ = node_t_::hash_value(key_);
	if(previous_ == hxnull) {
		for(node_t_* n_ = *this->get_bucket_head_(hash_); n_; n_ = n_->hash_next()) {
			if(hxkey_equal(n_->hash_key(), key_)) {
				return n_;
			}
		}
	}
	else {
		hxassertmsg(hxkey_equal(key_, previous_->hash_key()), "previous_mismatch");
		hxassertmsg(hash_ == previous_->hash_value(), "previous_mismatch");
		for(node_t_* n_ = previous_->hash_next(); n_; n_ = n_->hash_next()) {
			if(hxkey_equal(n_->hash_key(), key_)) {
				return n_;
			}
		}
	}
	return hxnull;
}

template<hxhash_table_concept_ node_t_, typename deleter_t_, bool multi_t_, hxhash_t table_size_bits_>
inline const node_t_*
hxhash_table<node_t_, deleter_t_, multi_t_, table_size_bits_>::find(
		const typename node_t_::key_t& key_, const node_t_* previous_) const {
	// This code calls the non-const version for brevity.
	return const_cast<hxhash_table*>(this)->find(key_, previous_);
}

template<hxhash_table_concept_ node_t_, typename deleter_t_, bool multi_t_, hxhash_t table_size_bits_>
inline typename hxhash_table<node_t_, deleter_t_, multi_t_, table_size_bits_>::iterator
hxhash_table<node_t_, deleter_t_, multi_t_, table_size_bits_>::insert(node_t_* ptr_) noexcept {
	hxassertmsg(ptr_ != hxnull, "null_node");
	hxassertmsg(this->find(ptr_->hash_key()) != ptr_, "container_reinsert");
	node_t_** pos_ = this->get_bucket_head_(ptr_->hash_value());
	if(!multi_t_) {
		for(node_t_* n_ = *pos_; n_; n_ = n_->hash_next()) {
			if(hxkey_equal(n_->hash_key(), ptr_->hash_key())) {
				if(m_deleter_) { m_deleter_(ptr_); }
				return iterator(this, n_);
			}
		}
	}
	ptr_->set_hash_next(*pos_);
	*pos_ = ptr_;
	++m_size_;
	return iterator(this, ptr_);
}

template<hxhash_table_concept_ node_t_, typename deleter_t_, bool multi_t_, hxhash_t table_size_bits_>
template<typename ptr_deleter_t_>
inline typename hxhash_table<node_t_, deleter_t_, multi_t_, table_size_bits_>::iterator
hxhash_table<node_t_, deleter_t_, multi_t_, table_size_bits_>::insert(hxptr<node_t_, ptr_deleter_t_>&& ptr_) noexcept {
	hxassertmsg(ptr_.get() != hxnull, "null_node");
	node_t_* const raw_ = ptr_.get();
	hxassertmsg(this->find(raw_->hash_key()) != raw_, "container_reinsert");
	node_t_** pos_ = this->get_bucket_head_(raw_->hash_value());
	if(!multi_t_) {
		for(node_t_* n_ = *pos_; n_; n_ = n_->hash_next()) {
			if(hxkey_equal(n_->hash_key(), raw_->hash_key())) {
				return iterator(this, n_);
			}
		}
	}
	raw_->set_hash_next(*pos_);
	*pos_ = ptr_.release();
	++m_size_;
	return iterator(this, raw_);
}

template<hxhash_table_concept_ node_t_, typename deleter_t_, bool multi_t_, hxhash_t table_size_bits_>
inline float hxhash_table<node_t_, deleter_t_, multi_t_, table_size_bits_>::load_factor(void) const {
	return m_table_.capacity()
		? (static_cast<float>(m_size_) / static_cast<float>(m_table_.capacity()))
		: 0.0f;
}

template<hxhash_table_concept_ node_t_, typename deleter_t_, bool multi_t_, hxhash_t table_size_bits_>
inline hxsize_t hxhash_table<node_t_, deleter_t_, multi_t_, table_size_bits_>::load_max(void) const {
	hxsize_t maximum_ = 0;
	const node_t_*const* it_end_ = m_table_.data() + m_table_.capacity();
	for(const node_t_*const* it_ = m_table_.data(); it_ != it_end_; ++it_) {
		hxsize_t count_ = 0;
		for(const node_t_* n_ = *it_; n_; n_ = n_->hash_next()) {
			++count_;
		}
		maximum_ = hxmax(maximum_, count_);
	}
	return maximum_;
}

template<hxhash_table_concept_ node_t_, typename deleter_t_, bool multi_t_, hxhash_t table_size_bits_>
inline void hxhash_table<node_t_, deleter_t_, multi_t_, table_size_bits_>::release_all(void) {
	if(m_size_ != 0) {
		::memset(m_table_.data(), 0x00, sizeof(node_t_*) * static_cast<size_t>(m_table_.capacity()));
		m_size_ = 0;
	}
}

template<hxhash_table_concept_ node_t_, typename deleter_t_, bool multi_t_, hxhash_t table_size_bits_>
inline hxsize_t hxhash_table<node_t_, deleter_t_, multi_t_, table_size_bits_>::release_key(
		const typename node_t_::key_t& key_) {
	// Pass a null pointer for the deleter. Just to show off.
	return this->erase(key_, static_cast<void(*)(node_t_*)>(hxnull));
}

template<hxhash_table_concept_ node_t_, typename deleter_t_, bool multi_t_, hxhash_t table_size_bits_>
template<hxsystem_allocator_t allocator_, hxalignment_t align_, bool multi_, class... args_t_>
inline auto hxhash_table<node_t_, deleter_t_, multi_t_, table_size_bits_>::try_emplace(
		const typename node_t_::key_t& key_, args_t_&&... args_) noexcept -> hxenable_if_t<!multi_, iterator> {
	node_t_* const existing_ = this->find(key_);
	if(existing_ != hxnull) {
		return iterator(this, existing_);
	}
	node_t_* const ptr_ = hxnew<node_t_, allocator_, align_>(hxforward<args_t_>(args_)...);
	return this->insert(ptr_);
}

template<hxhash_table_concept_ node_t_, typename deleter_t_, bool multi_t_, hxhash_t table_size_bits_>
inline node_t_** hxhash_table<node_t_, deleter_t_, multi_t_, table_size_bits_>::get_bucket_head_(hxhash_t hash_) {
	const hxhash_t index_ = hash_ >> (hxhash_bits - m_table_.get_table_size_bits());
	hxassertmsg(static_cast<hxsize_t>(index_) < m_table_.capacity(), "internal_error");
	return m_table_.data() + index_;
}

template<hxhash_table_concept_ node_t_, typename deleter_t_, bool multi_t_, hxhash_t table_size_bits_>
inline const node_t_*const* hxhash_table<node_t_, deleter_t_, multi_t_, table_size_bits_>::get_bucket_head_(hxhash_t hash_) const {
	const hxhash_t index_ = hash_ >> (hxhash_bits - m_table_.get_table_size_bits());
	hxassertmsg(static_cast<hxsize_t>(index_) < m_table_.capacity(), "internal_error");
	return m_table_.data() + index_;
}

HX_END_INL_
#endif // HX_DOXYGEN_PARSER
