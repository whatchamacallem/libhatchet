#pragma once
// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

#ifndef LIBHATCHET_VER
#error Internal. Do not include this file directly.
#endif

// Slot 0 is a sentinel with handle 1, so handle 0 never resolves. A handle is
// |generation:(64-index_bits)|index:(masked by capacity-1)|. Generations begin
// at 1 and releasing a handle adds capacity, wrapping the generation through
// all 64 bits. A broken handle is already valid for reuse. Values stay dense:
// m_index_ holds each allocated slot's value index and erase moves the last
// value into the hole, finding the slot owning it with m_backref_, a parallel
// array to the values kept in slot_t_ for packing. The m_backref_ entries past
// the live values hold the free slots as a stack, so reuse is LIFO, and a free
// slot's m_index_ holds its stack position. m_backref_ and m_index_ stay mutual
// inverses, so inserting value i pops its slot from m_backref_[i] with the
// mapping already in place, only a hole-filling erase rewrites it and clear
// reuses the live backrefs in place. m_size_ counts live values, m_mask_ is
// capacity - 1 and the sentinel makes value capacity one less than slots.

#ifndef HX_DOXYGEN_PARSER
HX_BEGIN_INL_

template<typename T_, uint32_t table_size_bits_>
inline hxattr_flatten hxhandle_map<T_, table_size_bits_>::hxhandle_map(void)
		: m_size_(0u) {
	hxif_constexpr(table_size_bits_ != hxallocator_dynamic_capacity) {
		this->build_free_list_();
	}
	else {
		m_mask_.set_mask_(0u);
	}
}

template<typename T_, uint32_t table_size_bits_>
inline hxattr_flatten hxhandle_map<T_, table_size_bits_>::~hxhandle_map(void) {
	T_* hxrestrict it_ = m_values_.data();
	for(const T_* const end_ = it_ + m_size_; it_ != end_; ++it_) {
		it_->T_::~T_();
	}
}

template<typename T_, uint32_t table_size_bits_>
inline hxattr_flatten void hxhandle_map<T_, table_size_bits_>::clear(void) noexcept {
	const uint32_t size_ = m_size_;
	if(size_ == 0u) { return; }
	const uint32_t increment_ = static_cast<uint32_t>(m_slots_.capacity());
	slot_t_* const hxrestrict slots_ = m_slots_.data();
	const slot_t_* backref_ = slots_;
	T_* hxrestrict values_ = m_values_.data();
	for(const slot_t_* const end_ = slots_ + size_; backref_ != end_; ++backref_, ++values_) {
		slots_[backref_->m_backref_].m_handle_ += increment_;
		values_->T_::~T_();
	}
	m_size_ = 0u;
}

template<typename T_, uint32_t table_size_bits_>
inline hxattr_flatten bool hxhandle_map<T_, table_size_bits_>::erase(hxhandle_t handle_) noexcept {
	hxassertmsg(m_slots_.capacity() != 0, "table_unallocated");
	const uint32_t mask_ = m_mask_.get_mask_();
	const uint32_t index_ = static_cast<uint32_t>(handle_) & mask_;
	slot_t_* const hxrestrict slots_ = m_slots_.data();
	slot_t_* const slot_ = slots_ + index_;
	if(slot_->m_handle_ != handle_) {
		return false;
	}
	const uint32_t value_ = slot_->m_index_;
	const uint32_t last_ = m_size_ - 1u;
	T_* const hxrestrict values_ = m_values_.data();
	if(value_ != last_) {
		const uint32_t owner_ = slots_[last_].m_backref_;
		slots_[last_].m_backref_ = index_;
		slots_[value_].m_backref_ = owner_;
		slots_[owner_].m_index_ = value_;
		slot_->m_index_ = last_;
		values_[value_] = hxmove(values_[last_]);
	}
	slot_->m_handle_ = handle_ + (mask_ + 1u);
	m_size_ = last_;
	values_[last_].T_::~T_();
	return true;
}

template<typename T_, uint32_t table_size_bits_>
template<typename callable_t_>
inline hxattr_flatten hxsize_t hxhandle_map<T_, table_size_bits_>::erase_if(
		callable_t_&& callable_) noexcept {
	hxassertmsg(m_slots_.capacity() != 0, "table_unallocated");
	const uint32_t mask_ = m_mask_.get_mask_();
	slot_t_* const hxrestrict slots_ = m_slots_.data();
	T_* const hxrestrict values_ = m_values_.data();
	uint32_t last_ = m_size_;
	hxsize_t erased_ = 0;
	for(uint32_t value_ = last_; value_-- != 0u;) {
		if(hxforward<callable_t_>(callable_)(values_[value_])) {
			--last_;
			const uint32_t owner_ = slots_[last_].m_backref_;
			const uint32_t index_ = slots_[value_].m_backref_;
			if(value_ != last_) {
				slots_[last_].m_backref_ = index_;
				slots_[value_].m_backref_ = owner_;
				slots_[owner_].m_index_ = value_;
				slots_[index_].m_index_ = last_;
				values_[value_] = hxmove(values_[last_]);
			}
			slots_[index_].m_handle_ += (mask_ + 1u);
			values_[last_].T_::~T_();
			++erased_;
		}
	}
	m_size_ = last_;
	return erased_;
}

template<typename T_, uint32_t table_size_bits_>
hxinline hxattr_flatten T_* hxhandle_map<T_, table_size_bits_>::get(hxhandle_t handle_) noexcept {
	hxassertmsg(m_slots_.capacity() != 0, "table_unallocated");
	const slot_t_* const slot_ = m_slots_.data() + (static_cast<uint32_t>(handle_) & m_mask_.get_mask_());
	return slot_->m_handle_ == handle_ ? m_values_.data() + slot_->m_index_ : hxnull;
}

template<typename T_, uint32_t table_size_bits_>
hxinline hxattr_flatten const T_*
hxhandle_map<T_, table_size_bits_>::get(hxhandle_t handle_) const noexcept {
	hxassertmsg(m_slots_.capacity() != 0, "table_unallocated");
	const slot_t_* const slot_ = m_slots_.data() + (static_cast<uint32_t>(handle_) & m_mask_.get_mask_());
	return slot_->m_handle_ == handle_ ? m_values_.data() + slot_->m_index_ : hxnull;
}

template<typename T_, uint32_t table_size_bits_>
hxinline hxattr_flatten hxhandle_t
hxhandle_map<T_, table_size_bits_>::handle_at(hxsize_t index_) const noexcept {
	hxassertmsg(static_cast<size_t>(index_) < static_cast<size_t>(m_size_), "invalid_index %zu", index_);
	const slot_t_* const slots_ = m_slots_.data();
	return slots_[slots_[static_cast<uint32_t>(index_)].m_backref_].m_handle_;
}

template<typename T_, uint32_t table_size_bits_>
template<typename... args_t_>
hxinline hxattr_flatten hxhandle_t hxhandle_map<T_, table_size_bits_>::insert(
		args_t_&&... args_) noexcept {
	hxassertmsg(m_slots_.capacity() != 0, "table_unallocated");
	const uint32_t value_ = m_size_;
	hxassert_hard(value_ != m_mask_.get_mask_(), "table_full");
	const slot_t_* const hxrestrict slots_ = m_slots_.data();
	const hxhandle_t handle_ = slots_[slots_[value_].m_backref_].m_handle_;
	m_size_ = value_ + 1u;
	::new(m_values_.data() + value_) T_(hxforward<args_t_>(args_)...);
	return handle_;
}

template<typename T_, uint32_t table_size_bits_>
inline hxattr_flatten void hxhandle_map<T_, table_size_bits_>::set_size_bits(uint32_t bits_) {
	static_assert(table_size_bits_ == hxallocator_dynamic_capacity,
		"set_size_bits requires dynamic capacity");
	hxassertmsg(bits_ - 1u < 30u, "invalid_bits");
	const hxsize_t capacity_ = static_cast<hxsize_t>(1) << bits_;
	m_mask_.set_mask_(static_cast<uint32_t>(capacity_) - 1u);
	m_slots_.reserve_storage(capacity_);
	m_values_.reserve_storage(capacity_ - 1);
	this->build_free_list_();
}

template<typename T_, uint32_t table_size_bits_>
inline hxattr_flatten void hxhandle_map<T_, table_size_bits_>::build_free_list_(void) {
	const uint32_t capacity_ = static_cast<uint32_t>(m_slots_.capacity());
	slot_t_* hxrestrict slot_ = m_slots_.data();
	// The sentinel handle has non-zero low bits so no handle resolves to it.
	slot_->m_handle_ = 1u;
	++slot_;
	for(uint32_t index_ = 1u; index_ != capacity_; ++index_, ++slot_) {
		slot_->m_handle_ = index_ | capacity_;
		slot_->m_index_ = index_ - 1u;
		// The last m_backref_ is unreachable.
		slot_[-1].m_backref_ = index_;
	}
	m_size_ = 0u;
}

HX_END_INL_
#endif // HX_DOXYGEN_PARSER
