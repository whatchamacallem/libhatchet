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
	: deleter_t_(hxmove(deleter_)), m_ptr_(ptr_) { }

template<typename T_, typename deleter_t_>
hxconstexpr hxptr<T_, deleter_t_>::hxptr(hxptr&& other_) noexcept
		: deleter_t_(hxmove(static_cast<deleter_t_&>(other_))), m_ptr_(other_.m_ptr_) {
	other_.m_ptr_ = hxnull;
}

template<typename T_, typename deleter_t_>
hxconstexpr hxptr<T_, deleter_t_>::~hxptr(void) noexcept {
	if(m_ptr_ != hxnull) {
		static_cast<deleter_t_&>(*this)(m_ptr_);
	}
}

template<typename T_, typename deleter_t_>
hxconstexpr hxptr<T_, deleter_t_>& hxptr<T_, deleter_t_>::operator=(hxptr&& other_) noexcept {
	hxassertmsg(this != &other_, "self_assignment");
	if(m_ptr_ != hxnull) {
		static_cast<deleter_t_&>(*this)(m_ptr_);
	}
	m_ptr_ = other_.m_ptr_;
	static_cast<deleter_t_&>(*this) = hxmove(other_.deleter());
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

template<typename T_, typename deleter_t_>
hxconstexpr bool hxptr<T_, deleter_t_>::operator==(hxnullptr_t) const {
	return m_ptr_ == hxnull;
}

#if HX_CPLUSPLUS < 202002L // C++20 defaults != from ==.
template<typename T_, typename deleter_t_>
hxconstexpr bool hxptr<T_, deleter_t_>::operator!=(const hxptr& other_) const {
	return !(*this == other_);
}

template<typename T_, typename deleter_t_>
hxconstexpr bool hxptr<T_, deleter_t_>::operator!=(hxnullptr_t) const {
	return !(*this == hxnullptr);
}
#endif

template<typename T_, typename deleter_t_>
template<typename function_t_>
hxconstexpr auto hxptr<T_, deleter_t_>::and_then(function_t_&& callable_) const
		-> hxremove_cvref_t<decltype(hxforward<function_t_>(callable_)(hxdeclval<T_&>()))> {
	using return_t_ = hxremove_cvref_t<decltype(hxforward<function_t_>(callable_)(hxdeclval<T_&>()))>;
	if(m_ptr_ != hxnull) {
		return hxforward<function_t_>(callable_)(*m_ptr_);
	}
	return return_t_();
}

template<typename T_, typename deleter_t_>
hxconstexpr const deleter_t_& hxptr<T_, deleter_t_>::deleter(void) const {
	return static_cast<const deleter_t_&>(*this);
}

template<typename T_, typename deleter_t_>
hxconstexpr deleter_t_& hxptr<T_, deleter_t_>::deleter(void) {
	return static_cast<deleter_t_&>(*this);
}

template<typename T_, typename deleter_t_>
template<typename function_t_>
hxconstexpr hxptr<T_, deleter_t_> hxptr<T_, deleter_t_>::or_else(function_t_&& callable_) && {
	if(m_ptr_ != hxnull) {
		return hxmove(*this);
	}
	return hxforward<function_t_>(callable_)();
}

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
		static_cast<deleter_t_&>(*this)(m_ptr_);
	}
	m_ptr_ = ptr_;
}

template<typename T_, typename deleter_t_>
hxconstexpr void hxptr<T_, deleter_t_>::swap(hxptr& other_) noexcept {
	hxswap(m_ptr_, other_.m_ptr_);
	hxswap(static_cast<deleter_t_&>(*this), other_.deleter());
}

template<typename T_, typename deleter_t_>
template<typename U_>
hxconstexpr hxremove_cv_t<T_> hxptr<T_, deleter_t_>::value_or(U_&& default_value_) const {
	return m_ptr_ != hxnull ? *m_ptr_
		: static_cast<hxremove_cv_t<T_>>(hxforward<U_>(default_value_));
}

HX_END_INL_
#endif // HX_DOXYGEN_PARSER
