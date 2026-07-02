#pragma once
// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

#ifndef LIBHATCHET_VER
#error Internal. Do not include this file directly.
#endif

#ifndef HX_DOXYGEN_PARSER
HX_BEGIN_INL_

template<hxfree_list_concept_ T_, hxsize_t capacity_>
hxfree_list<T_, capacity_>::hxfree_list(void) noexcept {
	hxif_constexpr(capacity_ > 0) {
		this->enqueue_all_(capacity_);
	}
	else {
		m_free_head_ = hxnull;
		m_size_ = 0;
	}
}

template<hxfree_list_concept_ T_, hxsize_t capacity_>
hxfree_list<T_, capacity_>::hxfree_list(hxfree_list&& x_) noexcept {
	static_assert(capacity_ == hxallocator_dynamic_capacity,
		"hxallocator_dynamic_capacity required for temporaries.");
	::memcpy(static_cast<void*>(this), &x_, sizeof x_); // NOLINT(bugprone-undefined-memory-manipulation)
	::memset(static_cast<void*>(&x_), 0x00, sizeof x_);
}

template<hxfree_list_concept_ T_, hxsize_t capacity_>
hxfree_list<T_, capacity_>::~hxfree_list(void) noexcept {
	hxassertmsg(m_size_ == this->capacity(), "hxfree_list destroyed with unreleased slots");
}

template<hxfree_list_concept_ T_, hxsize_t capacity_>
template<typename... args_t_>
hxptr<T_, typename hxfree_list<T_, capacity_>::deleter_t>
		hxfree_list<T_, capacity_>::allocate(args_t_&&... args_) noexcept {
	hxassert_hard(m_free_head_ != hxnull, "hxfree_list is empty");
	slot_* const s_ = m_free_head_;
	m_free_head_ = s_->next_;
	--m_size_;
	T_* const p_ = ::new(static_cast<void*>(&s_->value_)) T_(hxforward<args_t_>(args_)...);
	return hxptr<T_, deleter_t>(p_, deleter_t(*this));
}

template<hxfree_list_concept_ T_, hxsize_t capacity_>
hxsize_t hxfree_list<T_, capacity_>::capacity(void) const {
	return hxallocator<T_, capacity_>::capacity();
}

template<hxfree_list_concept_ T_, hxsize_t capacity_>
bool hxfree_list<T_, capacity_>::is_allocator(const T_* p_) const noexcept {
	// A single unsigned range check that is also false for null.
	const uintptr_t offset_ = reinterpret_cast<uintptr_t>(p_)
		- reinterpret_cast<uintptr_t>(this->data());
	return offset_ < static_cast<uintptr_t>(this->capacity()) * sizeof(T_);
}

template<hxfree_list_concept_ T_, hxsize_t capacity_>
template<typename ptr_deleter_t_>
bool hxfree_list<T_, capacity_>::is_allocator(const hxptr<T_, ptr_deleter_t_>& ptr_) const noexcept {
	return this->is_allocator(ptr_.get());
}

template<hxfree_list_concept_ T_, hxsize_t capacity_>
void hxfree_list<T_, capacity_>::release(T_* p_) noexcept {
	hxassertmsg(this->is_allocator(p_), "invalid_pointer");
	p_->T_::~T_();
	slot_* const s_ = reinterpret_cast<slot_*>(p_);
#if (HX_HARDENING_MODE) == HX_HARDENING_MODE_DEBUG
	::memset(static_cast<void*>(s_), 0xcd, sizeof(slot_));
#endif
	s_->next_ = m_free_head_;
	m_free_head_ = s_;
	++m_size_;
}

template<hxfree_list_concept_ T_, hxsize_t capacity_>
template<typename ptr_deleter_t_>
void hxfree_list<T_, capacity_>::release(hxptr<T_, ptr_deleter_t_>&& ptr_) noexcept {
	this->release(ptr_.release());
}

template<hxfree_list_concept_ T_, hxsize_t capacity_>
void hxfree_list<T_, capacity_>::reserve(hxsize_t size_,
		hxsystem_allocator_t allocator_, hxalignment_t alignment_) noexcept {
	hxassert_hard(this->capacity() == 0, "reserve can not resize");
	this->reserve_storage(size_, allocator_, alignment_);
	this->enqueue_all_(size_);
}

template<hxfree_list_concept_ T_, hxsize_t capacity_>
template<typename... args_t_>
hxptr<T_, typename hxfree_list<T_, capacity_>::deleter_t>
		hxfree_list<T_, capacity_>::try_allocate(args_t_&&... args_) noexcept {
	if(m_free_head_ == hxnull) {
		return hxptr<T_, deleter_t>(hxnull, deleter_t(*this));
	}
	slot_* const s_ = m_free_head_;
	m_free_head_ = s_->next_;
	--m_size_;
	T_* const p_ = ::new(static_cast<void*>(&s_->value_)) T_(hxforward<args_t_>(args_)...);
	return hxptr<T_, deleter_t>(p_, deleter_t(*this));
}

template<hxfree_list_concept_ T_, hxsize_t capacity_>
void hxfree_list<T_, capacity_>::enqueue_all_(hxsize_t count_) noexcept {
	slot_* const slots_ = reinterpret_cast<slot_*>(this->data());
	slot_* head_ = hxnull;
	for(slot_* hxrestrict it_ = slots_ + count_; it_ != slots_;) {
		--it_;
		it_->next_ = head_;
		head_ = it_;
	}
	m_free_head_ = head_;
	m_size_ = count_;
}

HX_END_INL_
#endif // HX_DOXYGEN_PARSER
