#pragma once
// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

#ifndef LIBHATCHET_VER
#error Internal. Do not include this file directly.
#endif

#ifndef HX_DOXYGEN_PARSER
HX_BEGIN_INL_

template<typename T_>
template<typename U_, hxenable_if_t<
	!hxdetail_::hxis_hxref_<hxremove_cvref_t<U_>>::value &&
	!hxdetail_::hxis_hxoptional_<hxremove_cvref_t<U_>>::value &&
	hxis_lvalue_reference<U_&>::value, bool>>
hxref<T_>::hxref(U_& value_) : m_value_(&static_cast<T_&>(value_)) { }

template<typename T_>
template<typename U_, hxenable_if_t<
	!hxis_same<U_, T_>::value, bool>>
hxref<T_>::hxref(const hxref<U_>& other_)
		: m_value_(other_.has_value() ? &static_cast<T_&>(*other_) : hxnull) { }

template<typename T_>
template<typename U_>
hxref<T_>::hxref(hxoptional<U_>& other_)
		: m_value_(other_.has_value() ? &static_cast<T_&>(*other_) : hxnull) { }

template<typename T_>
T_& hxref<T_>::operator*(void) const {
	hxassertmsg(m_value_ != hxnull, "optional_disengaged");
	return *m_value_;
}

template<typename T_>
T_* hxref<T_>::operator->(void) const {
	hxassertmsg(m_value_ != hxnull, "optional_disengaged");
	return m_value_;
}

template<typename T_>
hxref<T_>& hxref<T_>::operator=(const hxref& other_) {
	hxassertmsg(this != &other_, "self_assignment");
	m_value_ = other_.m_value_;
	return *this;
}

template<typename T_>
template<typename U_, hxenable_if_t<
	!hxdetail_::hxis_hxref_<hxremove_cvref_t<U_>>::value &&
	!hxdetail_::hxis_hxoptional_<hxremove_cvref_t<U_>>::value &&
	hxis_lvalue_reference<U_&>::value, bool>>
hxref<T_>& hxref<T_>::operator=(U_& value_) {
	m_value_ = &static_cast<T_&>(value_);
	return *this;
}

template<typename T_>
bool hxref<T_>::operator==(const hxref& rhs_) const {
	if ((m_value_ == hxnull) != (rhs_.m_value_ == hxnull)) {
		return false;
	}
	return m_value_ == hxnull || (*m_value_ == *rhs_.m_value_);
}

template<typename T_>
bool hxref<T_>::operator==(const T_& value_) const {
	return m_value_ != hxnull && (*m_value_ == value_);
}

template<typename T_>
template<typename function_t_>
auto hxref<T_>::and_then(function_t_&& callable_) const
		-> hxremove_cvref_t<decltype(hxforward<function_t_>(callable_)(hxdeclval<T_&>()))> {
	if (m_value_ != hxnull) {
		return hxforward<function_t_>(callable_)(*m_value_);
	}
	return hxnullopt;
}

template<typename T_>
template<typename function_t_>
hxref<T_> hxref<T_>::or_else(function_t_&& callable_) const {
	return m_value_ != hxnull ? *this : hxforward<function_t_>(callable_)();
}

template<typename T_>
void hxref<T_>::swap(hxref& other_) noexcept {
	T_* const t_ = m_value_;
	m_value_ = other_.m_value_;
	other_.m_value_ = t_;
}

template<typename T_>
T_& hxref<T_>::value(void) const {
	hxassertmsg(m_value_ != hxnull, "optional_disengaged");
	return *m_value_;
}

template<typename T_>
template<typename U_>
hxremove_cv_t<T_> hxref<T_>::value_or(U_&& default_value_) const {
	return m_value_ != hxnull ? *m_value_
		: static_cast<hxremove_cv_t<T_>>(hxforward<U_>(default_value_));
}

HX_END_INL_
#endif // HX_DOXYGEN_PARSER
