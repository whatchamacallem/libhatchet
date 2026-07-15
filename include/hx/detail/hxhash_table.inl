#pragma once
// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

#ifndef LIBHATCHET_VER
#error Internal. Do not include this file directly.
#endif

#ifndef HX_DOXYGEN_PARSER
HX_BEGIN_INL_

template<hxhash_table_concept_ node_t_, typename deleter_t_, bool multi_t_, uint32_t table_size_bits_>
hxinline const node_t_&
hxhash_table<node_t_, deleter_t_, multi_t_, table_size_bits_>::const_iterator::operator*(void) const {
	hxassertmsg(m_current_node_ != hxnull, "invalid_iterator");
	return *m_current_node_;
}

template<hxhash_table_concept_ node_t_, typename deleter_t_, bool multi_t_, uint32_t table_size_bits_>
hxinline const node_t_*
hxhash_table<node_t_, deleter_t_, multi_t_, table_size_bits_>::const_iterator::operator->(void) const {
	hxassertmsg(m_current_node_ != hxnull, "invalid_iterator");
	return m_current_node_;
}

template<hxhash_table_concept_ node_t_, typename deleter_t_, bool multi_t_, uint32_t table_size_bits_>
hxinline node_t_&
hxhash_table<node_t_, deleter_t_, multi_t_, table_size_bits_>::iterator::operator*(void) const {
	hxassertmsg(this->m_current_node_ != hxnull, "invalid_iterator");
	return *this->m_current_node_;
}

template<hxhash_table_concept_ node_t_, typename deleter_t_, bool multi_t_, uint32_t table_size_bits_>
hxinline node_t_*
hxhash_table<node_t_, deleter_t_, multi_t_, table_size_bits_>::iterator::operator->(void) const {
	hxassertmsg(this->m_current_node_ != hxnull, "invalid_iterator");
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
	hxassertmsg(m_current_node_, "invalid_iterator"); // !end
	m_current_node_ = m_current_node_->hash_next();
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
	hxassertmsg(m_current_node_ == hxnull, "internal_error");
	node_t_** hxrestrict it_ = m_next_bucket_;
	while(it_ != m_bucket_end_) {
		node_t_* const node_ = *it_++;
		if(node_ != hxnull) {
			m_current_node_ = node_;
			m_next_bucket_ = it_;
			break;
		}
	}
}

// hxhash_table out-of-line implementations

template<hxhash_table_concept_ node_t_, typename deleter_t_, bool multi_t_, uint32_t table_size_bits_>
hxattr_flatten hxhash_table<node_t_, deleter_t_, multi_t_, table_size_bits_>::hxhash_table(deleter_t_ deleter_)
		: deleter_t_(hxmove(deleter_)), m_size_(0) {
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

template<hxhash_table_concept_ node_t_, typename deleter_t_, bool multi_t_, uint32_t table_size_bits_>
template<typename deleter_u_>
hxattr_flatten void hxhash_table<node_t_, deleter_t_, multi_t_, table_size_bits_>::clear(
		deleter_u_&& deleter_) noexcept {
	if(m_size_ != 0) {
		if(deleter_) {
			node_t_* const* const it_end_ = m_table_.data() + m_table_.capacity();
			for(node_t_** hxrestrict it_ = m_table_.data(); it_ != it_end_; ++it_) {
				node_t_* node_ = *it_;
				if(node_) {
					*it_ = hxnull;
					do {
						node_t_* const next_ = node_->hash_next();
						deleter_(node_);
						node_ = next_;
					} while(node_);
				}
			}
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
	hxsize_t total_ = 0; // NOLINT(misc-const-correctness)
	const hxhash_t hash_ = node_t_::hash_value(key_);
	for(const node_t_* node_ = *this->get_bucket_head_(hash_); node_; node_ = node_->hash_next()) {
		if(hxkey_equal(node_->hash_key(), key_)) {
			hxif_constexpr(!multi_t_) {
				return 1;
			}
			else {
				++total_;
			}
		}
	}
	return total_;
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
hxattr_flatten hxsize_t hxhash_table<node_t_, deleter_t_, multi_t_, table_size_bits_>::erase(
		const typename node_t_::key_t& key_, deleter_u_&& deleter_) noexcept {
	const hxhash_t hash_ = node_t_::hash_value(key_);
	node_t_** const head_ = this->get_bucket_head_(hash_);
	hxif_constexpr(!multi_t_) {
		node_t_* previous_ = hxnull;
		for(node_t_* node_ = *head_; node_ != hxnull; node_ = node_->hash_next()) {
			if(hxkey_equal(node_->hash_key(), key_)) {
				if(previous_ != hxnull) {
					previous_->set_hash_next(node_->hash_next());
				}
				else {
					*head_ = node_->hash_next();
				}
				if(deleter_) {
					deleter_(node_);
				}
				--m_size_;
				return 1;
			}
			previous_ = node_;
		}
		return 0;
	}
	else {
		node_t_* node_ = *head_;
		if(node_ == hxnull) {
			return 0;
		}
		hxsize_t count_ = 0;
		// The matching prefix pointed to by the head.
		if(hxkey_equal(node_->hash_key(), key_)) {
			do {
				node_t_* const next_ = node_->hash_next();
				if(deleter_) {
					deleter_(node_);
				}
				++count_;
				node_ = next_;
			} while(node_ != hxnull && hxkey_equal(node_->hash_key(), key_));
			*head_ = node_;
		}
		// Those following a non-matching node.
		if(node_ != hxnull) {
			node_t_* previous_ = node_;
			node_ = node_->hash_next();
			while(node_ != hxnull) {
				node_t_* const next_ = node_->hash_next();
				if(hxkey_equal(node_->hash_key(), key_)) {
					previous_->set_hash_next(next_);
					if(deleter_) {
						deleter_(node_);
					}
					++count_;
				}
				else {
					previous_ = node_;
				}
				node_ = next_;
			}
		}
		m_size_ -= count_;
		return count_;
	}
}

template<hxhash_table_concept_ node_t_, typename deleter_t_, bool multi_t_, uint32_t table_size_bits_>
hxinline hxattr_flatten hxsize_t hxhash_table<node_t_, deleter_t_, multi_t_, table_size_bits_>::erase(const typename node_t_::key_t& key_) noexcept {
	return this->erase(key_, static_cast<deleter_t_&>(*this));
}

template<hxhash_table_concept_ node_t_, typename deleter_t_, bool multi_t_, uint32_t table_size_bits_>
hxattr_flatten typename hxhash_table<node_t_, deleter_t_, multi_t_, table_size_bits_>::iterator
hxhash_table<node_t_, deleter_t_, multi_t_, table_size_bits_>::erase(const const_iterator& it_) noexcept {
	node_t_* const node_ = it_.m_current_node_;
	hxassertmsg(node_ != hxnull, "invalid_iterator");
	iterator next_(this, node_);
	++next_;
	node_t_** const head_ = this->get_bucket_head_(node_->hash_value());
	if(*head_ == node_) {
		*head_ = node_->hash_next();
	}
	else {
		node_t_* previous_ = *head_;
		while(previous_->hash_next() != node_) {
			previous_ = previous_->hash_next();
		}
		previous_->set_hash_next(node_->hash_next());
	}
	--m_size_;
	return next_;
}

template<hxhash_table_concept_ node_t_, typename deleter_t_, bool multi_t_, uint32_t table_size_bits_>
hxattr_flatten hxptr<node_t_, deleter_t_>
hxhash_table<node_t_, deleter_t_, multi_t_, table_size_bits_>::extract(
		const typename node_t_::key_t& key_) noexcept {
	const hxhash_t hash_ = node_t_::hash_value(key_);
	node_t_** const head_ = this->get_bucket_head_(hash_);
	node_t_* previous_ = hxnull;
	for(node_t_* node_ = *head_; node_ != hxnull; node_ = node_->hash_next()) {
		if(hxkey_equal(node_->hash_key(), key_)) {
			if(previous_ != hxnull) {
				previous_->set_hash_next(node_->hash_next());
			}
			else {
				*head_ = node_->hash_next();
			}
			--m_size_;
			return hxptr<node_t_, deleter_t_>(node_, static_cast<deleter_t_&>(*this));
		}
		previous_ = node_;
	}
	return hxptr<node_t_, deleter_t_>(hxnull, static_cast<deleter_t_&>(*this));
}

template<hxhash_table_concept_ node_t_, typename deleter_t_, bool multi_t_, uint32_t table_size_bits_>
hxattr_flatten node_t_* hxhash_table<node_t_, deleter_t_, multi_t_, table_size_bits_>::find(
		const typename node_t_::key_t& key_, const node_t_* previous_) {
	const hxhash_t hash_ = node_t_::hash_value(key_);
	if(previous_ == hxnull) {
		for(node_t_* node_ = *this->get_bucket_head_(hash_); node_; node_ = node_->hash_next()) {
			if(hxkey_equal(node_->hash_key(), key_)) {
				return node_;
			}
		}
	}
	else hxif_constexpr(multi_t_) {
		hxassertmsg(hxkey_equal(key_, previous_->hash_key()), "previous_mismatch");
		hxassertmsg(hash_ == previous_->hash_value(), "previous_mismatch");
		for(node_t_* node_ = previous_->hash_next(); node_; node_ = node_->hash_next()) {
			if(hxkey_equal(node_->hash_key(), key_)) {
				return node_;
			}
		}
	}
	return hxnull;
}

template<hxhash_table_concept_ node_t_, typename deleter_t_, bool multi_t_, uint32_t table_size_bits_>
hxinline hxattr_flatten const node_t_*
hxhash_table<node_t_, deleter_t_, multi_t_, table_size_bits_>::find(
		const typename node_t_::key_t& key_, const node_t_* previous_) const {
	// This code calls the non-const version for brevity.
	return const_cast<hxhash_table*>(this)->find(key_, previous_);
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
	hxassertmsg(ptr_ != hxnull, "null_node");
	hxassertmsg(this->find(ptr_->hash_key()) != ptr_, "container_reinsert");
	node_t_** const pos_ = this->get_bucket_head_(ptr_->hash_value());
	hxif_constexpr(!multi_t_) {
		const key_t& key_ = ptr_->hash_key();
		for(node_t_* node_ = *pos_; node_; node_ = node_->hash_next()) {
			if(hxkey_equal(node_->hash_key(), key_)) {
				if(static_cast<deleter_t_&>(*this)) {
					static_cast<deleter_t_&>(*this)(ptr_);
				}
				return iterator(this, node_);
			}
		}
	}
	ptr_->set_hash_next(*pos_);
	*pos_ = ptr_;
	++m_size_;
	return iterator(this, ptr_);
}

template<hxhash_table_concept_ node_t_, typename deleter_t_, bool multi_t_, uint32_t table_size_bits_>
template<typename deleter_u_>
hxattr_flatten typename hxhash_table<node_t_, deleter_t_, multi_t_, table_size_bits_>::iterator
hxhash_table<node_t_, deleter_t_, multi_t_, table_size_bits_>::insert(hxptr<node_t_, deleter_u_>&& ptr_) noexcept {
	hxassertmsg(ptr_.get() != hxnull, "null_node");
	node_t_* const raw_ = ptr_.get();
	hxassertmsg(this->find(raw_->hash_key()) != raw_, "container_reinsert");
	node_t_** const pos_ = this->get_bucket_head_(raw_->hash_value());
	hxif_constexpr(!multi_t_) {
		const key_t& key_ = raw_->hash_key();
		for(node_t_* node_ = *pos_; node_; node_ = node_->hash_next()) {
			if(hxkey_equal(node_->hash_key(), key_)) {
				return iterator(this, node_);
			}
		}
	}
	raw_->set_hash_next(*pos_);
	*pos_ = ptr_.release();
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
	const node_t_*const* const it_end_ = m_table_.data() + m_table_.capacity();
	for(const node_t_*const* hxrestrict it_ = m_table_.data(); it_ != it_end_; ++it_) {
		hxsize_t count_ = 0;
		for(const node_t_* node_ = *it_; node_; node_ = node_->hash_next()) {
			++count_;
		}
		maximum_ = hxmax(maximum_, count_);
	}
	return maximum_;
}

template<hxhash_table_concept_ node_t_, typename deleter_t_, bool multi_t_, uint32_t table_size_bits_>
hxinline hxattr_flatten void hxhash_table<node_t_, deleter_t_, multi_t_, table_size_bits_>::release_all(void) {
	if(m_size_ != 0) {
		::memset(m_table_.data(), 0x00, sizeof(node_t_*) * static_cast<size_t>(m_table_.capacity()));
		m_size_ = 0;
	}
}

template<hxhash_table_concept_ node_t_, typename deleter_t_, bool multi_t_, uint32_t table_size_bits_>
hxinline hxattr_flatten hxsize_t hxhash_table<node_t_, deleter_t_, multi_t_, table_size_bits_>::release_key(
		const typename node_t_::key_t& key_) {
	// Pass a null pointer for the deleter. Just to show off.
	return this->erase(key_, static_cast<void(*)(node_t_*)>(hxnull));
}

template<hxhash_table_concept_ node_t_, typename deleter_t_, bool multi_t_, uint32_t table_size_bits_>
hxattr_flatten hxptr<node_t_, deleter_t_>
hxhash_table<node_t_, deleter_t_, multi_t_, table_size_bits_>::replace(node_t_* ptr_) noexcept {
	hxassertmsg(ptr_ != hxnull, "null_node");
	const key_t& key_ = ptr_->hash_key();
	node_t_** const head_ = this->get_bucket_head_(ptr_->hash_value());
	node_t_* previous_ = hxnull;
	for(node_t_* node_ = *head_; node_ != hxnull; node_ = node_->hash_next()) {
		if(hxkey_equal(node_->hash_key(), key_)) {
			ptr_->set_hash_next(node_->hash_next());
			if(previous_ != hxnull) {
				previous_->set_hash_next(ptr_);
			}
			else {
				*head_ = ptr_;
			}
			return hxptr<node_t_, deleter_t_>(node_, static_cast<deleter_t_&>(*this));
		}
		previous_ = node_;
	}
	ptr_->set_hash_next(*head_);
	*head_ = ptr_;
	++m_size_;
	return hxptr<node_t_, deleter_t_>(hxnull, static_cast<deleter_t_&>(*this));
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
	node_t_* const existing_ = this->find(key_);
	if(existing_ != hxnull) {
		return iterator(this, existing_);
	}
	node_t_* const ptr_ = hxnew<node_t_, allocator_, alignment_>(hxforward<args_t_>(args_)...);
	return this->insert(ptr_);
}

template<hxhash_table_concept_ node_t_, typename deleter_t_, bool multi_t_, uint32_t table_size_bits_>
hxinline hxattr_flatten node_t_** hxhash_table<node_t_, deleter_t_, multi_t_, table_size_bits_>::get_bucket_head_(hxhash_t hash_) {
	const uint32_t shift_ = m_table_.get_hash_shift_();
	const hxhash_t index_ = hash_ >> shift_;
	hxassertmsg(static_cast<hxsize_t>(index_) < m_table_.capacity(), "internal_error");
	return m_table_.data() + index_;
}

template<hxhash_table_concept_ node_t_, typename deleter_t_, bool multi_t_, uint32_t table_size_bits_>
hxinline hxattr_flatten const node_t_*const* hxhash_table<node_t_, deleter_t_, multi_t_, table_size_bits_>::get_bucket_head_(hxhash_t hash_) const {
	const uint32_t shift_ = m_table_.get_hash_shift_();
	const hxhash_t index_ = hash_ >> shift_;
	hxassertmsg(static_cast<hxsize_t>(index_) < m_table_.capacity(), "internal_error");
	return m_table_.data() + index_;
}

HX_END_INL_
#endif // HX_DOXYGEN_PARSER
