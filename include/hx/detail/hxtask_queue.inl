#pragma once
// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

#ifndef LIBHATCHET_VER
#error Internal. Do not include this file directly.
#endif

HX_BEGIN_INL_

template<typename callable_t_>
bool hxtask_queue::all_of(callable_t_&& fn_) const {
	hxtask_queue_lock_;
	return m_tasks_.all_of(hxforward<callable_t_>(fn_));
}

template<typename callable_t_>
bool hxtask_queue::any_of(callable_t_&& fn_) const {
	hxtask_queue_lock_;
	return m_tasks_.any_of(hxforward<callable_t_>(fn_));
}

inline bool hxtask_queue::cancel(hxtask* task_) {
	size_t erased_ = 0;
	{
		hxtask_queue_lock_;
		erased_ = m_tasks_.erase_if([task_](const record_t& r_) { return r_.task == task_; });
		if(erased_ != 0u) {
			hxdetail_::hxmake_heap_(m_tasks_.begin(), m_tasks_.end(), hxkey_less_t<record_t>{});
		}
	}
	if(erased_ != 0u) {
		task_->on_cancel(this);
		return true;
	}
	return false;
}

inline void hxtask_queue::clear(void) {
	hxtask_queue_lock_;
	m_tasks_.clear();
}

inline bool hxtask_queue::empty(void) const {
	hxtask_queue_lock_;
	return m_tasks_.empty();
}

template<typename callable_t_>
size_t hxtask_queue::erase_if(callable_t_&& fn_) {
	hxtask_queue_lock_;
	const size_t erased_ = m_tasks_.erase_if(hxforward<callable_t_>(fn_));
	if(erased_ != 0u) {
		// Restore the heap property all at once. Allows erase_if to modify
		// priority at the same time.
		hxdetail_::hxmake_heap_(m_tasks_.begin(), m_tasks_.end(), hxkey_less_t<record_t>{});
	}
	return erased_;
}

template<typename callable_t_>
void hxtask_queue::for_each(callable_t_&& fn_) const {
	hxtask_queue_lock_;
	m_tasks_.for_each(hxforward<callable_t_>(fn_));
}

template<typename callable_t_>
void hxtask_queue::for_each(callable_t_&& fn_) {
	hxtask_queue_lock_;
	m_tasks_.for_each(hxforward<callable_t_>(fn_));

	hxdetail_::hxmake_heap_(m_tasks_.begin(), m_tasks_.end(), hxkey_less_t<record_t>{});
}

inline bool hxtask_queue::full(void) const {
	hxtask_queue_lock_;
	return m_tasks_.full();
}

inline size_t hxtask_queue::max_size(void) const {
	// Capacity is fixed at construction, no lock needed.
	return m_tasks_.max_size();
}

inline size_t hxtask_queue::size(void) const {
	hxtask_queue_lock_;
	return m_tasks_.size();
}

HX_END_INL_
