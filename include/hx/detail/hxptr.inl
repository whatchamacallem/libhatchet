#pragma once
// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

template<typename T_, typename deleter_t_>
hxconstexpr hxptr<T_, deleter_t_>::hxptr(hxptr&& other_) : m_ptr_(other_.m_ptr_) {
	other_.m_ptr_ = hxnull;
}

template<typename T_, typename deleter_t_>
hxconstexpr hxptr<T_, deleter_t_>::~hxptr(void) {
	if(m_ptr_) {
		deleter_t_()(m_ptr_);
	}
}

template<typename T_, typename deleter_t_>
hxconstexpr hxptr<T_, deleter_t_>& hxptr<T_, deleter_t_>::operator=(hxptr&& other_) {
	hxassertmsg(this != &other_, "self_assignment");
	if(m_ptr_) {
		deleter_t_()(m_ptr_);
	}
	m_ptr_ = other_.m_ptr_;
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
hxconstexpr T_& hxptr<T_, deleter_t_>::operator[](size_t n_) const {
	hxassertmsg(m_ptr_ != hxnull, "null_ptr");
	return m_ptr_[n_];
}

template<typename T_, typename deleter_t_>
hxconstexpr hxptr<T_, deleter_t_>::operator bool(void) const { return m_ptr_ != hxnull; }

template<typename T_, typename deleter_t_>
hxconstexpr bool hxptr<T_, deleter_t_>::operator==(const hxptr& other_) const {
	return m_ptr_ == other_.m_ptr_;
}

template<typename T_, typename deleter_t_>
hxconstexpr bool hxptr<T_, deleter_t_>::operator!=(const hxptr& other_) const {
	return m_ptr_ != other_.m_ptr_;
}

template<typename T_, typename deleter_t_>
hxconstexpr bool hxptr<T_, deleter_t_>::operator==(hxnullptr_t) const {
	return m_ptr_ == hxnull;
}

template<typename T_, typename deleter_t_>
hxconstexpr bool hxptr<T_, deleter_t_>::operator!=(hxnullptr_t) const {
	return m_ptr_ != hxnull;
}

template<typename T_, typename deleter_t_>
hxconstexpr T_* hxptr<T_, deleter_t_>::release(void) {
	T_* ptr_ = m_ptr_;
	m_ptr_ = hxnull;
	return ptr_;
}

template<typename T_, typename deleter_t_>
hxconstexpr void hxptr<T_, deleter_t_>::reset(T_* ptr_) {
	hxassertmsg(ptr_ != m_ptr_ || m_ptr_ == hxnull, "self_reset");
	if(m_ptr_) {
		deleter_t_()(m_ptr_);
	}
	m_ptr_ = ptr_;
}

template<typename T_, typename deleter_t_>
hxconstexpr void hxptr<T_, deleter_t_>::swap(hxptr& other_) {
	T_* tmp_ = m_ptr_;
	m_ptr_ = other_.m_ptr_;
	other_.m_ptr_ = tmp_;
}
