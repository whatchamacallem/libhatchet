#pragma once
// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

#ifndef LIBHATCHET_VER
#error Internal. Do not include this file directly.
#endif

#ifndef HX_DOXYGEN_PARSER
HX_INL_BEGIN_

template<typename T_, typename deleter_t_>
hxinline hxconstexpr hxptr<T_, deleter_t_>::hxptr(T_* ptr_, deleter_t_ deleter_) noexcept
	: deleter_t_(hxmove(deleter_)), m_ptr_(ptr_) { }

template<typename T_, typename deleter_t_>
hxinline hxconstexpr hxptr<T_, deleter_t_>::hxptr(hxptr&& x_) noexcept
		: deleter_t_(hxmove(static_cast<deleter_t_&>(x_))), m_ptr_(x_.m_ptr_) {
	x_.m_ptr_ = hxnull;
}

template<typename T_, typename deleter_t_>
hxinline hxconstexpr hxptr<T_, deleter_t_>::~hxptr(void) noexcept {
	if(m_ptr_ != hxnull) {
		static_cast<deleter_t_&>(*this)(m_ptr_);
	}
}

template<typename T_, typename deleter_t_>
hxinline hxconstexpr hxptr<T_, deleter_t_>& hxptr<T_, deleter_t_>::operator=(hxptr&& x_) noexcept {
	hxassertmsg(this != &x_, "self_copy");
	if(m_ptr_ != hxnull) {
		static_cast<deleter_t_&>(*this)(m_ptr_);
	}
	m_ptr_ = x_.m_ptr_;
	static_cast<deleter_t_&>(*this) = hxmove(x_.deleter());
	x_.m_ptr_ = hxnull;
	return *this;
}

template<typename T_, typename deleter_t_>
hxinline hxconstexpr T_& hxptr<T_, deleter_t_>::operator*(void) const {
	hxassertmsg(m_ptr_ != hxnull, "null_ptr");
	return *m_ptr_;
}

template<typename T_, typename deleter_t_>
hxinline hxconstexpr T_* hxptr<T_, deleter_t_>::operator->(void) const {
	hxassertmsg(m_ptr_ != hxnull, "null_ptr");
	return m_ptr_;
}

template<typename T_, typename deleter_t_>
hxinline hxconstexpr hxptr<T_, deleter_t_>::operator bool(void) const { return m_ptr_ != hxnull; }

template<typename T_, typename deleter_t_>
hxinline hxconstexpr bool hxptr<T_, deleter_t_>::operator==(hxnil_t) const {
	return m_ptr_ == hxnull;
}

#if HX_CPLUSPLUS < 202002L // C++20 defaults != from ==.
template<typename T_, typename deleter_t_>
hxinline hxconstexpr bool hxptr<T_, deleter_t_>::operator!=(const hxptr& x_) const {
	return !(*this == x_);
}

template<typename T_, typename deleter_t_>
hxinline hxconstexpr bool hxptr<T_, deleter_t_>::operator!=(hxnil_t) const {
	return !(*this == hxnil);
}
#endif

#if HX_CPLUSPLUS >= 202302L
template<typename T_, typename deleter_t_>
template<typename self_t_, typename callable_t_>
hxinline hxconstexpr auto hxptr<T_, deleter_t_>::and_then(
		this self_t_&& self_, callable_t_&& callable_)
		-> hxremove_cvref_t<decltype(hxforward<callable_t_>(callable_)(hxdeclval<T_&>()))> {
	if(self_.m_ptr_ != hxnull) {
		return hxforward<callable_t_>(callable_)(*self_.m_ptr_);
	}
	return hxnil;
}
#endif // HX_CPLUSPLUS >= 202302L

template<typename T_, typename deleter_t_>
hxinline hxconstexpr const deleter_t_& hxptr<T_, deleter_t_>::deleter(void) const {
	return static_cast<const deleter_t_&>(*this);
}

template<typename T_, typename deleter_t_>
hxinline hxconstexpr deleter_t_& hxptr<T_, deleter_t_>::deleter(void) {
	return static_cast<deleter_t_&>(*this);
}

template<typename T_, typename deleter_t_>
template<hxsystem_allocator_t allocator_, hxalignment_t alignment_, typename... args_t_>
hxinline hxattr_flatten T_& hxptr<T_, deleter_t_>::emplace(args_t_&&... args_) noexcept {
	this->reset(hxnew<T_, allocator_, alignment_>(hxforward<args_t_>(args_)...));
	return *m_ptr_;
}

template<typename T_, typename deleter_t_>
hxinline hxconstexpr bool hxptr<T_, deleter_t_>::equal(const hxptr& x_) const {
	return m_ptr_ == x_.m_ptr_;
}

template<typename T_, typename deleter_t_>
hxinline hxattr_flatten hxhash_t hxptr<T_, deleter_t_>::hash(void) const {
	return this->has_value() ? hxkey_hash(*m_ptr_) : hxhash_t{31u};
}

#if HX_CPLUSPLUS >= 202302L
template<typename T_, typename deleter_t_>
template<typename self_t_, typename callable_t_>
hxinline hxconstexpr hxptr<T_, deleter_t_> hxptr<T_, deleter_t_>::or_else(
		this self_t_&& self_, callable_t_&& callable_)
		requires(hxis_rvalue_reference<self_t_&&>() &&
			!hxis_const<hxremove_reference_t<self_t_>>()) {
	if(self_.m_ptr_ != hxnull) {
		return self_;
	}
	return hxforward<callable_t_>(callable_)();
}
#endif // HX_CPLUSPLUS >= 202302L

template<typename T_, typename deleter_t_>
hxinline hxconstexpr T_* hxptr<T_, deleter_t_>::release(void) {
	T_* const ptr_ = m_ptr_;
	m_ptr_ = hxnull;
	return ptr_;
}

template<typename T_, typename deleter_t_>
hxinline hxconstexpr void hxptr<T_, deleter_t_>::reset(T_* ptr_) noexcept {
	hxassertmsg(ptr_ != m_ptr_ || m_ptr_ == hxnull, "self_reset");
	if(m_ptr_ != hxnull) {
		static_cast<deleter_t_&>(*this)(m_ptr_);
	}
	m_ptr_ = ptr_;
}

template<typename T_, typename deleter_t_>
hxinline hxconstexpr void hxptr<T_, deleter_t_>::swap(hxptr& x_) noexcept {
	hxswap(m_ptr_, x_.m_ptr_);
	hxswap(static_cast<deleter_t_&>(*this), x_.deleter());
}

#if HX_CPLUSPLUS >= 202302L
template<typename T_, typename deleter_t_>
template<typename self_t_, typename... args_t_>
hxinline hxconstexpr hxremove_cv_t<T_> hxptr<T_, deleter_t_>::value_or(
		this self_t_&& self_, args_t_&&... args_) {
	if(self_.m_ptr_ != hxnull) {
		return static_cast<hxremove_cv_t<T_>>(*self_.m_ptr_);
	}
	return hxremove_cv_t<T_>(hxforward<args_t_>(args_)...);
}
#endif // HX_CPLUSPLUS >= 202302L

template<typename T_, typename deleter_t_>
hxinline hxconstexpr T_& hxptr<T_, deleter_t_>::value(void) const {
	hxassertmsg(m_ptr_ != hxnull, "null_ptr");
	return *m_ptr_;
}

HX_INL_END_
#endif // HX_DOXYGEN_PARSER
