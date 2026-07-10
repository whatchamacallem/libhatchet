#pragma once
// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

#ifndef LIBHATCHET_VER
#error Internal. Do not include this file directly.
#endif

#ifndef HX_DOXYGEN_PARSER
HX_BEGIN_INL_

template<typename T_, typename deleter_t_, uint32_t table_size_bits_>
inline hxhandle_table<T_, deleter_t_, table_size_bits_>::hxhandle_table(deleter_t_ deleter_)
		: deleter_t_(hxmove(deleter_)), m_size_(0), m_free_head_(hxnull) {
	hxif_constexpr(table_size_bits_ != hxallocator_dynamic_capacity) {
		this->build_free_list_();
	}
}

template<typename T_, typename deleter_t_, uint32_t table_size_bits_>
inline hxhandle_table<T_, deleter_t_, table_size_bits_>::~hxhandle_table(void) {
	deleter_t_& deleter_ = static_cast<deleter_t_&>(*this);
	if(m_size_ != 0 && deleter_) {
		const size_t begin_ = reinterpret_cast<size_t>(m_table_.data());
		const size_t bytes_ = static_cast<size_t>(m_table_.capacity()) * sizeof(hxhandle_slot_);
		const hxsize_t size_ = m_table_.capacity();
		hxsize_t count_ = 0;
		for(hxhandle_slot_* hxrestrict it_ = m_table_.data(); count_ != size_; ++it_,++count_) {
			if((reinterpret_cast<size_t>(it_->next_) - begin_) >= bytes_) {
				deleter_(it_->ptr_);
			}
		}
	}
}

template<typename T_, typename deleter_t_, uint32_t table_size_bits_>
template<typename deleter_u_>
inline void hxhandle_table<T_, deleter_t_, table_size_bits_>::clear(deleter_u_&& deleter_) noexcept {
	if(m_size_ != 0) {
		const uint32_t increment_ = static_cast<uint32_t>(m_table_.capacity()) << 1;
		const size_t begin_ = reinterpret_cast<size_t>(m_table_.data());
		const size_t bytes_ = static_cast<size_t>(m_table_.capacity()) * sizeof(hxhandle_slot_);
		const hxsize_t size_ = m_table_.capacity();
		hxsize_t count_ = 0;
		for(hxhandle_slot_* hxrestrict it_ = m_table_.data(); count_ != size_; ++it_,++count_) {
			if((reinterpret_cast<size_t>(it_->next_) - begin_) >= bytes_) {
				if(deleter_) {
					deleter_(it_->ptr_);
				}
				it_->key_ += increment_;
			}
			it_->next_ = it_ + 1;
		}
		(m_table_.data() + size_ - 1)->next_ = hxnull;
		m_free_head_ = m_table_.data();
		m_size_ = 0;
	}
}

template<typename T_, typename deleter_t_, uint32_t table_size_bits_>
template<typename deleter_u_>
inline bool hxhandle_table<T_, deleter_t_, table_size_bits_>::erase(
		hxhandle_t handle_, deleter_u_&& deleter_) noexcept {
	hxassertmsg(m_table_.capacity() != 0, "table_unallocated");
	const uint32_t mask_ = m_table_.get_mask_();
	hxhandle_slot_* const slot_ = m_table_.data() + (static_cast<uint32_t>(handle_) & mask_);
	if(slot_->key_ != handle_) {
		return false;
	}
	if(deleter_) {
		deleter_(slot_->ptr_);
	}
	slot_->key_ = handle_ + ((mask_ + 1u) << 1);
	slot_->next_ = m_free_head_;
	m_free_head_ = slot_;
	--m_size_;
	return true;
}

template<typename T_, typename deleter_t_, uint32_t table_size_bits_>
inline hxptr<T_, deleter_t_>
hxhandle_table<T_, deleter_t_, table_size_bits_>::extract(hxhandle_t handle_) noexcept {
	hxassertmsg(m_table_.capacity() != 0, "table_unallocated");
	const uint32_t mask_ = m_table_.get_mask_();
	hxhandle_slot_* const slot_ = m_table_.data() + (static_cast<uint32_t>(handle_) & mask_);
	if(slot_->key_ != handle_) {
		return hxptr<T_, deleter_t_>(hxnull, static_cast<deleter_t_&>(*this));
	}
	T_* const ptr_ = slot_->ptr_;
	slot_->key_ = handle_ + ((mask_ + 1u) << 1);
	slot_->next_ = m_free_head_;
	m_free_head_ = slot_;
	--m_size_;
	return hxptr<T_, deleter_t_>(ptr_, static_cast<deleter_t_&>(*this));
}

template<typename T_, typename deleter_t_, uint32_t table_size_bits_>
inline T_* hxhandle_table<T_, deleter_t_, table_size_bits_>::get(hxhandle_t handle_) noexcept {
	hxassertmsg(m_table_.capacity() != 0, "table_unallocated");
	const hxhandle_slot_* const slot_ = m_table_.data()
		+ (static_cast<uint32_t>(handle_) & m_table_.get_mask_());
	return (slot_->key_ == handle_) ? slot_->ptr_ : hxnull;
}

template<typename T_, typename deleter_t_, uint32_t table_size_bits_>
inline const T_*
hxhandle_table<T_, deleter_t_, table_size_bits_>::get(hxhandle_t handle_) const noexcept {
	hxassertmsg(m_table_.capacity() != 0, "table_unallocated");
	const hxhandle_slot_* const slot_ = m_table_.data()
		+ (static_cast<uint32_t>(handle_) & m_table_.get_mask_());
	return (slot_->key_ == handle_) ? slot_->ptr_ : hxnull;
}

template<typename T_, typename deleter_t_, uint32_t table_size_bits_>
inline const deleter_t_&
hxhandle_table<T_, deleter_t_, table_size_bits_>::deleter(void) const {
	return static_cast<const deleter_t_&>(*this);
}

template<typename T_, typename deleter_t_, uint32_t table_size_bits_>
inline deleter_t_& hxhandle_table<T_, deleter_t_, table_size_bits_>::deleter(void) {
	return static_cast<deleter_t_&>(*this);
}

template<typename T_, typename deleter_t_, uint32_t table_size_bits_>
inline hxhandle_t hxhandle_table<T_, deleter_t_, table_size_bits_>::insert(T_* ptr_) noexcept {
	hxassertmsg(m_table_.capacity() != 0, "table_unallocated");
	hxassert_hard(m_free_head_ != hxnull, "table_full");
	hxhandle_slot_* const slot_ = m_free_head_;
	m_free_head_ = slot_->next_;
	slot_->ptr_ = ptr_;
	++m_size_;
	return slot_->key_;
}

template<typename T_, typename deleter_t_, uint32_t table_size_bits_>
template<typename deleter_u_>
inline hxhandle_t hxhandle_table<T_, deleter_t_, table_size_bits_>::insert(
		hxptr<T_, deleter_u_>&& ptr_) noexcept {
	return this->insert(ptr_.release());
}

template<typename T_, typename deleter_t_, uint32_t table_size_bits_>
inline hxptr<T_, deleter_t_>
hxhandle_table<T_, deleter_t_, table_size_bits_>::replace(hxhandle_t handle_, T_* ptr_) noexcept {
	hxassertmsg(ptr_ != hxnull, "null_pointer");
	hxassertmsg(m_table_.capacity() != 0, "table_unallocated");
	hxhandle_slot_* const slot_ = m_table_.data()
		+ (static_cast<uint32_t>(handle_) & m_table_.get_mask_());
	if(slot_->key_ != handle_) {
		return hxptr<T_, deleter_t_>(hxnull, static_cast<deleter_t_&>(*this));
	}
	T_* const previous_ = slot_->ptr_;
	slot_->ptr_ = ptr_;
	return hxptr<T_, deleter_t_>(previous_, static_cast<deleter_t_&>(*this));
}

template<typename T_, typename deleter_t_, uint32_t table_size_bits_>
inline void hxhandle_table<T_, deleter_t_, table_size_bits_>::set_table_size_bits(uint32_t bits_) {
	static_assert(table_size_bits_ == hxallocator_dynamic_capacity,
		"set_table_size_bits requires dynamic capacity");
	m_table_.set_table_size_bits_(bits_, (1u << bits_) - 1u);
	this->build_free_list_();
}

template<typename T_, typename deleter_t_, uint32_t table_size_bits_>
inline void hxhandle_table<T_, deleter_t_, table_size_bits_>::build_free_list_(void) {
	const uint32_t capacity_ = static_cast<uint32_t>(m_table_.capacity());
	hxhandle_slot_* hxrestrict slot_ = m_table_.data();
	for(uint32_t index_ = 0u; index_ < capacity_; ++index_, ++slot_) {
		slot_->key_ = index_ | capacity_;
		slot_->next_ = slot_ + 1;
	}
	(slot_ - 1)->next_ = hxnull;
	m_free_head_ = m_table_.data();
}

HX_END_INL_
#endif // HX_DOXYGEN_PARSER
