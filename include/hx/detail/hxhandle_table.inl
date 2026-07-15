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
inline hxattr_flatten hxhandle_table<T_, deleter_t_, table_size_bits_>::hxhandle_table(deleter_t_ deleter_)
		: deleter_t_(hxmove(deleter_)), m_size_(0), m_free_head_(hxnull) {
	hxif_constexpr(table_size_bits_ != hxallocator_dynamic_capacity) {
		this->build_free_list_();
	}
}

template<typename T_, typename deleter_t_, uint32_t table_size_bits_>
inline hxattr_flatten hxhandle_table<T_, deleter_t_, table_size_bits_>::~hxhandle_table(void) {
	deleter_t_& deleter_ = static_cast<deleter_t_&>(*this);
	if(m_size_ != 0 && deleter_) {
		const size_t begin_ = reinterpret_cast<size_t>(m_table_.data());
		const size_t bytes_ = static_cast<size_t>(m_table_.capacity()) * sizeof(slot_t_);
		const hxsize_t size_ = m_table_.capacity();
		hxsize_t count_ = 1;
		for(slot_t_* hxrestrict it_ = m_table_.data() + 1; count_ != size_; ++it_,++count_) {
			if((reinterpret_cast<size_t>(it_->m_next_) - begin_) >= bytes_) {
				deleter_(it_->m_ptr_);
			}
		}
	}
}

template<typename T_, typename deleter_t_, uint32_t table_size_bits_>
template<typename deleter_u_>
inline hxattr_flatten void hxhandle_table<T_, deleter_t_, table_size_bits_>::clear(
		deleter_u_&& deleter_) noexcept {
	if(m_size_ != 0) {
		const uint32_t increment_ = static_cast<uint32_t>(m_table_.capacity());
		const size_t begin_ = reinterpret_cast<size_t>(m_table_.data());
		const size_t bytes_ = static_cast<size_t>(m_table_.capacity()) * sizeof(slot_t_);
		const hxsize_t size_ = m_table_.capacity();
		hxsize_t count_ = 1;
		for(slot_t_* hxrestrict it_ = m_table_.data() + 1; count_ != size_; ++it_,++count_) {
			if((reinterpret_cast<size_t>(it_->m_next_) - begin_) >= bytes_) {
				if(deleter_) {
					deleter_(it_->m_ptr_);
				}
				it_->m_key_ += increment_;
			}
			it_->m_next_ = it_ + 1;
		}
		(m_table_.data() + size_ - 1)->m_next_ = m_table_.data();
		m_free_head_ = m_table_.data() + 1;
		m_size_ = 0;
	}
}

template<typename T_, typename deleter_t_, uint32_t table_size_bits_>
template<typename deleter_u_>
hxinline hxattr_flatten bool hxhandle_table<T_, deleter_t_, table_size_bits_>::erase(
		hxhandle_t handle_, deleter_u_&& deleter_) noexcept {
	hxassertmsg(m_table_.capacity() != 0, "table_unallocated");
	const uint32_t mask_ = m_table_.get_mask_();
	slot_t_* const slot_ = m_table_.data() + (static_cast<uint32_t>(handle_) & mask_);
	if(slot_->m_key_ != handle_) {
		return false;
	}
	if(deleter_) {
		deleter_(slot_->m_ptr_);
	}
	slot_->m_key_ = handle_ + (mask_ + 1u);
	slot_->m_next_ = m_free_head_;
	m_free_head_ = slot_;
	--m_size_;
	return true;
}

template<typename T_, typename deleter_t_, uint32_t table_size_bits_>
template<typename callable_t_>
hxinline hxattr_flatten hxsize_t hxhandle_table<T_, deleter_t_, table_size_bits_>::erase_if(
		callable_t_&& callable_) noexcept {
	deleter_t_& deleter_ = static_cast<deleter_t_&>(*this);
	const uint32_t mask_ = m_table_.get_mask_();
	const size_t begin_ = reinterpret_cast<size_t>(m_table_.data());
	const size_t bytes_ = static_cast<size_t>(m_table_.capacity()) * sizeof(slot_t_);
	const hxsize_t size_ = m_table_.capacity();
	hxsize_t erased_ = 0;
	hxsize_t count_ = 1;
	for(slot_t_* hxrestrict it_ = m_table_.data() + 1; count_ != size_; ++it_,++count_) {
		if((reinterpret_cast<size_t>(it_->m_next_) - begin_) >= bytes_
				&& hxforward<callable_t_>(callable_)(*it_->m_ptr_)) {
			if(deleter_) {
				deleter_(it_->m_ptr_);
			}
			it_->m_key_ += (mask_ + 1u);
			it_->m_next_ = m_free_head_;
			m_free_head_ = it_;
			--m_size_;
			++erased_;
		}
	}
	return erased_;
}

// Fixes gcc + optimizer + sanitizer -Wmaybe-uninitialized bug.
#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#endif

template<typename T_, typename deleter_t_, uint32_t table_size_bits_>
hxinline hxattr_flatten hxptr<T_, deleter_t_>
hxhandle_table<T_, deleter_t_, table_size_bits_>::extract(hxhandle_t handle_) noexcept {
	hxassertmsg(m_table_.capacity() != 0, "table_unallocated");
	const uint32_t mask_ = m_table_.get_mask_();
	slot_t_* const slot_ = m_table_.data() + (static_cast<uint32_t>(handle_) & mask_);
	if(slot_->m_key_ != handle_) {
		return hxptr<T_, deleter_t_>(hxnull, static_cast<deleter_t_&>(*this));
	}
	T_* const ptr_ = slot_->m_ptr_;
	slot_->m_key_ = handle_ + (mask_ + 1u);
	slot_->m_next_ = m_free_head_;
	m_free_head_ = slot_;
	--m_size_;
	return hxptr<T_, deleter_t_>(ptr_, static_cast<deleter_t_&>(*this));
}

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic pop
#endif

template<typename T_, typename deleter_t_, uint32_t table_size_bits_>
hxinline hxattr_flatten T_* hxhandle_table<T_, deleter_t_, table_size_bits_>::get(hxhandle_t handle_) noexcept {
	hxassertmsg(m_table_.capacity() != 0, "table_unallocated");
	const slot_t_* const slot_ = m_table_.data()
		+ (static_cast<uint32_t>(handle_) & m_table_.get_mask_());
	return (slot_->m_key_ == handle_) ? slot_->m_ptr_ : hxnull;
}

template<typename T_, typename deleter_t_, uint32_t table_size_bits_>
hxinline hxattr_flatten const T_*
hxhandle_table<T_, deleter_t_, table_size_bits_>::get(hxhandle_t handle_) const noexcept {
	hxassertmsg(m_table_.capacity() != 0, "table_unallocated");
	const slot_t_* const slot_ = m_table_.data()
		+ (static_cast<uint32_t>(handle_) & m_table_.get_mask_());
	return (slot_->m_key_ == handle_) ? slot_->m_ptr_ : hxnull;
}

template<typename T_, typename deleter_t_, uint32_t table_size_bits_>
hxinline hxattr_flatten const deleter_t_&
hxhandle_table<T_, deleter_t_, table_size_bits_>::deleter(void) const {
	return static_cast<const deleter_t_&>(*this);
}

template<typename T_, typename deleter_t_, uint32_t table_size_bits_>
hxinline hxattr_flatten deleter_t_& hxhandle_table<T_, deleter_t_, table_size_bits_>::deleter(void) {
	return static_cast<deleter_t_&>(*this);
}

template<typename T_, typename deleter_t_, uint32_t table_size_bits_>
hxinline hxattr_flatten hxhandle_t hxhandle_table<T_, deleter_t_, table_size_bits_>::insert(T_* ptr_) noexcept {
	hxassertmsg(m_table_.capacity() != 0, "table_unallocated");
	hxassert_hard(m_free_head_ != m_table_.data(), "table_full");
	if(ptr_ == hxnull) {
		return hxnull_handle;
	}
	slot_t_* const slot_ = m_free_head_;
	m_free_head_ = slot_->m_next_;
	slot_->m_ptr_ = ptr_;
	++m_size_;
	return slot_->m_key_;
}

template<typename T_, typename deleter_t_, uint32_t table_size_bits_>
template<typename deleter_u_>
hxinline hxattr_flatten hxhandle_t hxhandle_table<T_, deleter_t_, table_size_bits_>::insert(
		hxptr<T_, deleter_u_>&& ptr_) noexcept {
	return this->insert(ptr_.release());
}

template<typename T_, typename deleter_t_, uint32_t table_size_bits_>
hxinline hxattr_flatten void hxhandle_table<T_, deleter_t_, table_size_bits_>::set_size_bits(uint32_t bits_) {
	static_assert(table_size_bits_ == hxallocator_dynamic_capacity,
		"set_size_bits requires dynamic capacity");
	m_table_.set_size_bits_(bits_, (1u << bits_) - 1u);
	this->build_free_list_();
}

template<typename T_, typename deleter_t_, uint32_t table_size_bits_>
inline hxattr_flatten void hxhandle_table<T_, deleter_t_, table_size_bits_>::build_free_list_(void) {
	const uint32_t capacity_ = static_cast<uint32_t>(m_table_.capacity());
	slot_t_* const hxrestrict data_ = m_table_.data();
	data_->m_key_ = capacity_;
	slot_t_* hxrestrict slot_ = data_ + 1;
	for(uint32_t index_ = 1u; index_ < capacity_; ++index_, ++slot_) {
		slot_->m_key_ = index_;
		slot_->m_next_ = slot_ + 1;
	}
	(slot_ - 1)->m_next_ = data_;
	m_free_head_ = data_ + 1;
}

HX_END_INL_
#endif // HX_DOXYGEN_PARSER
