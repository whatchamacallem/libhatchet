#pragma once
// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

#ifndef LIBHATCHET_VER
#error Internal. Do not include this file directly.
#endif

#ifndef HX_DOXYGEN_PARSER
HX_BEGIN_INL_

template<typename callable_t_>
bool hxtask_queue::all_of(callable_t_&& callable_) const noexcept {
	hxtask_queue_lock_;
	return m_tasks_.all_of(hxforward<callable_t_>(callable_));
}

template<typename callable_t_>
bool hxtask_queue::any_of(callable_t_&& callable_) const noexcept {
	hxtask_queue_lock_;
	return m_tasks_.any_of(hxforward<callable_t_>(callable_));
}

inline bool hxtask_queue::empty(void) const {
	hxtask_queue_lock_;
	return m_tasks_.empty();
}

template<typename callable_t_>
hxsize_t hxtask_queue::erase_if(callable_t_&& callable_) noexcept {
	hxtask_queue_lock_;
	const hxsize_t erased_ = m_tasks_.erase_if_unordered(hxforward<callable_t_>(callable_));
	hxdetail_::hxmake_heap_(m_tasks_.begin(), m_tasks_.end(), hxkey_less_t<record_t>{});
#if HX_USE_THREADS
	if(erased_ != 0 && m_tasks_.empty()) {
		m_cond_var_completion_.notify_all();
	}
#endif
	return erased_;
}

template<typename callable_t_>
void hxtask_queue::for_each(callable_t_&& callable_) const noexcept {
	hxtask_queue_lock_;
	m_tasks_.for_each(hxforward<callable_t_>(callable_));
}

template<typename callable_t_>
void hxtask_queue::for_each(callable_t_&& callable_) noexcept {
	hxtask_queue_lock_;
	m_tasks_.for_each(hxforward<callable_t_>(callable_));
	hxdetail_::hxmake_heap_(m_tasks_.begin(), m_tasks_.end(), hxkey_less_t<record_t>{});
}

inline bool hxtask_queue::full(void) const {
	hxtask_queue_lock_;
	return m_tasks_.full();
}

inline hxsize_t hxtask_queue::max_size(void) const {
	return m_tasks_.max_size();
}

inline hxsize_t hxtask_queue::size(void) const {
	hxtask_queue_lock_;
	return m_tasks_.size();
}

HX_END_INL_
#endif // HX_DOXYGEN_PARSER
