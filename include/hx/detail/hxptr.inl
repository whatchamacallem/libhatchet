#pragma once
// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

#ifndef LIBHATCHET_VER
#error Internal. Do not include this file directly.
#endif

#ifndef HX_DOXYGEN_PARSER
HX_BEGIN_INL_

template<typename T_, typename deleter_t_>
hxconstexpr hxptr<T_, deleter_t_>::hxptr(T_* ptr_, deleter_t_ deleter_) noexcept
	: m_ptr_(ptr_), m_deleter_(deleter_) { }

template<typename T_, typename deleter_t_>
hxconstexpr hxptr<T_, deleter_t_>::hxptr(hxptr&& other_) noexcept
		: m_ptr_(other_.m_ptr_), m_deleter_(hxmove(other_.m_deleter_)) {
	other_.m_ptr_ = hxnull;
}

template<typename T_, typename deleter_t_>
hxconstexpr hxptr<T_, deleter_t_>::~hxptr(void) noexcept {
	if(m_ptr_ != hxnull) {
		m_deleter_(m_ptr_);
	}
}

template<typename T_, typename deleter_t_>
hxconstexpr hxptr<T_, deleter_t_>& hxptr<T_, deleter_t_>::operator=(hxptr&& other_) noexcept {
	hxassertmsg(this != &other_, "self_assignment");
	if(m_ptr_ != hxnull) {
		m_deleter_(m_ptr_);
	}
	m_ptr_ = other_.m_ptr_;
	m_deleter_ = hxmove(other_.m_deleter_);
	other_.m_ptr_ = hxnull;
	return *this;
}

template<typename T_, typename deleter_t_>
hxconstexpr T_& hxptr<T_, deleter_t_>::operator*(void) const {
	hxassertmsg(m_ptr_ != hxnull, "null_ptr");
	return *m_ptr_;
}

template<typename T_, typename deleter_t_>
hxconstexpr T_* hxptr<T_, deleter_t_>::operator->(void) const {
	hxassertmsg(m_ptr_ != hxnull, "null_ptr");
	return m_ptr_;
}

template<typename T_, typename deleter_t_>
hxconstexpr hxptr<T_, deleter_t_>::operator bool(void) const { return m_ptr_ != hxnull; }

template<typename T_, typename deleter_t_>
hxconstexpr bool hxptr<T_, deleter_t_>::operator==(const hxptr& other_) const {
	return m_ptr_ == other_.m_ptr_;
}

#if HX_CPLUSPLUS < 202002L // C++20 defaults != from ==.
template<typename T_, typename deleter_t_>
hxconstexpr bool hxptr<T_, deleter_t_>::operator!=(const hxptr& other_) const {
	return m_ptr_ != other_.m_ptr_;
}
#endif

template<typename T_, typename deleter_t_>
hxconstexpr bool hxptr<T_, deleter_t_>::operator==(hxnullptr_t) const {
	return m_ptr_ == hxnull;
}

#if HX_CPLUSPLUS < 202002L // C++20 defaults != from ==.
template<typename T_, typename deleter_t_>
hxconstexpr bool hxptr<T_, deleter_t_>::operator!=(hxnullptr_t) const {
	return m_ptr_ != hxnull;
}
#endif

template<typename T_, typename deleter_t_>
hxconstexpr T_* hxptr<T_, deleter_t_>::release(void) {
	T_* const ptr_ = m_ptr_;
	m_ptr_ = hxnull;
	return ptr_;
}

template<typename T_, typename deleter_t_>
hxconstexpr void hxptr<T_, deleter_t_>::reset(T_* ptr_) noexcept {
	hxassertmsg(ptr_ != m_ptr_ || m_ptr_ == hxnull, "self_reset");
	if(m_ptr_ != hxnull) {
		m_deleter_(m_ptr_);
	}
	m_ptr_ = ptr_;
}

template<typename T_, typename deleter_t_>
hxconstexpr void hxptr<T_, deleter_t_>::swap(hxptr& other_) noexcept {
	hxswap(m_ptr_, other_.m_ptr_);
	hxswap(m_deleter_, other_.m_deleter_);
}

HX_END_INL_
#endif // HX_DOXYGEN_PARSER
