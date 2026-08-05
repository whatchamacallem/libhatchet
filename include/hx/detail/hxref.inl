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
	!hxdetail_::hxis_hxoptional_<hxremove_cvref_t<U_>>::value, bool>>
hxinline hxattr_flatten hxref<T_>::hxref(U_& value_) : m_value_(&static_cast<T_&>(value_)) {
	static_assert(hxbinds_directly<T_, U_>::value,
		"U& must bind to T& without creating a temporary");
}

template<typename T_>
template<typename U_, hxenable_if_t<
	!hxis_same<U_, T_>::value, bool>>
hxinline hxattr_flatten hxref<T_>::hxref(const hxref<U_>& other_)
		: m_value_(other_.has_value() ? &static_cast<T_&>(*other_) : hxnull) {
	static_assert(hxbinds_directly<T_, U_>::value,
		"U& must bind to T& without creating a temporary");
}

template<typename T_>
template<typename U_>
hxinline hxattr_flatten hxref<T_>::hxref(hxoptional<U_>& other_)
		: m_value_(other_.has_value() ? &static_cast<T_&>(*other_) : hxnull) {
	static_assert(hxbinds_directly<T_, U_>::value,
		"U& must bind to T& without creating a temporary");
}

template<typename T_>
hxinline hxattr_flatten T_& hxref<T_>::operator*(void) const {
	hxassertmsg(m_value_ != hxnull, "optional_disengaged");
	return *m_value_;
}

template<typename T_>
hxinline hxattr_flatten T_* hxref<T_>::operator->(void) const {
	hxassertmsg(m_value_ != hxnull, "optional_disengaged");
	return m_value_;
}

template<typename T_>
template<typename U_, hxenable_if_t<
	!hxdetail_::hxis_hxref_<hxremove_cvref_t<U_>>::value &&
	!hxdetail_::hxis_hxoptional_<hxremove_cvref_t<U_>>::value, bool>>
hxinline hxattr_flatten hxref<T_>& hxref<T_>::operator=(U_& value_) {
	static_assert(hxbinds_directly<T_, U_>::value,
		"U& must bind to T& without creating a temporary");
	m_value_ = &static_cast<T_&>(value_);
	return *this;
}

template<typename T_>
hxinline hxattr_flatten bool hxref<T_>::operator==(const hxref& rhs_) const {
	if (m_value_ == hxnull || rhs_.m_value_ == hxnull) {
		return m_value_ == rhs_.m_value_;
	}
	return *m_value_ == *rhs_.m_value_;
}

template<typename T_>
hxinline hxattr_flatten bool hxref<T_>::operator==(const T_& value_) const {
	return m_value_ != hxnull && (*m_value_ == value_);
}

template<typename T_>
template<typename function_t_>
hxinline hxattr_flatten auto hxref<T_>::and_then(function_t_&& callable_) const
		-> hxremove_cvref_t<decltype(hxforward<function_t_>(callable_)(hxdeclval<T_&>()))> {
	if (m_value_ != hxnull) {
		return hxforward<function_t_>(callable_)(*m_value_);
	}
	return hxnullopt;
}

template<typename T_>
template<typename U_>
hxinline hxattr_flatten T_& hxref<T_>::emplace(U_& value_) {
	static_assert(hxbinds_directly<T_, U_>::value,
		"U& must bind to T& without creating a temporary");
	m_value_ = &static_cast<T_&>(value_);
	return *m_value_;
}

template<typename T_>
template<typename function_t_>
hxinline hxattr_flatten hxref<T_> hxref<T_>::or_else(function_t_&& callable_) const {
	if(m_value_ != hxnull) {
		return *this;
	}
	return hxforward<function_t_>(callable_)();
}

template<typename T_>
hxinline hxattr_flatten void hxref<T_>::swap(hxref& other_) noexcept {
	hxswap(m_value_, other_.m_value_);
}

template<typename T_>
hxinline hxattr_flatten T_& hxref<T_>::value(void) const {
	hxassertmsg(m_value_ != hxnull, "optional_disengaged");
	return *m_value_;
}

template<typename T_>
template<typename U_>
hxinline hxattr_flatten hxremove_cv_t<T_> hxref<T_>::value_or(U_&& default_value_) const {
	if(m_value_ != hxnull) {
		return *m_value_;
	}
	return static_cast<hxremove_cv_t<T_>>(hxforward<U_>(default_value_));
}

HX_END_INL_
#endif // HX_DOXYGEN_PARSER
