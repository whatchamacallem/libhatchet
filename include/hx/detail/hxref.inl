#pragma once
// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

#ifndef LIBHATCHET_VER
#error Internal. Do not include this file directly.
#endif

#ifndef HX_DOXYGEN_PARSER
HX_INL_BEGIN_

template<typename T_>
template<typename U_, hxenable_if_t<
	!hxis_hxref_<hxremove_cvref_t<U_>>::value &&
	!hxis_hxexpected_<hxremove_cvref_t<U_>>::value &&
	hxbinds_directly<T_, U_>(), bool>>
hxinline hxattr_flatten hxref<T_>::hxref(U_& value_) : m_value_(&static_cast<T_&>(value_)) { }

template<typename T_>
template<typename E_>
hxinline hxattr_flatten hxref<T_>::hxref(hxexpected<T_, E_>& x_)
		: m_value_(x_.has_value() ? &*x_ : hxnull) { }

template<typename T_>
hxinline hxattr_flatten T_& hxref<T_>::operator*(void) const {
	hxassertf(m_value_ != hxnull, "bad_value");
	return *m_value_;
}

template<typename T_>
hxinline hxattr_flatten T_* hxref<T_>::operator->(void) const {
	hxassertf(m_value_ != hxnull, "bad_value");
	return m_value_;
}

template<typename T_>
template<typename U_, hxenable_if_t<
	!hxis_hxref_<hxremove_cvref_t<U_>>::value &&
	!hxis_hxexpected_<hxremove_cvref_t<U_>>::value &&
	hxbinds_directly<T_, U_>(), bool>>
hxinline hxattr_flatten hxref<T_>& hxref<T_>::operator=(U_& value_) {
	m_value_ = &static_cast<T_&>(value_);
	return *this;
}

template<typename T_>
hxinline hxattr_flatten bool hxref<T_>::operator==(const T_& value_) const {
	return m_value_ != hxnull && (*m_value_ == value_);
}

template<typename T_>
template<typename self_t_, typename callable_t_>
hxinline hxattr_flatten auto hxref<T_>::and_then(
		this self_t_&& self_, callable_t_&& callable_)
		-> hxremove_cvref_t<decltype(hxforward<callable_t_>(callable_)(hxdeclval<T_&>()))> {
	if(self_.m_value_ != hxnull) {
		return hxforward<callable_t_>(callable_)(*self_.m_value_);
	}
	return hxnil;
}

template<typename T_>
hxinline hxattr_flatten bool hxref<T_>::equal(const hxref& x_) const {
	if (m_value_ == hxnull || x_.m_value_ == hxnull) {
		return m_value_ == x_.m_value_;
	}
	return *m_value_ == *x_.m_value_;
}

template<typename T_>
hxinline hxattr_flatten hxhash_t hxref<T_>::hash(void) const {
	return this->has_value() ? hxkey_hash(*m_value_) : hxhash_t{31u};
}

template<typename T_>
template<typename self_t_, typename callable_t_>
hxinline hxattr_flatten hxref<T_> hxref<T_>::or_else(
		this self_t_&& self_, callable_t_&& callable_) {
	if(self_.m_value_ != hxnull) {
		return self_;
	}
	return hxforward<callable_t_>(callable_)();
}

template<typename T_>
hxinline hxattr_flatten void hxref<T_>::swap(hxref& x_) noexcept {
	hxswap(m_value_, x_.m_value_);
}

template<typename T_>
hxinline hxattr_flatten T_& hxref<T_>::value(void) const {
	hxassertf(m_value_ != hxnull, "bad_value");
	return *m_value_;
}

template<typename T_>
template<typename self_t_, typename... args_t_>
hxinline hxattr_flatten hxremove_cv_t<T_> hxref<T_>::value_or(
		this self_t_&& self_, args_t_&&... args_) {
	if(self_.m_value_ != hxnull) {
		return *self_.m_value_;
	}
	return hxremove_cv_t<T_>(hxforward<args_t_>(args_)...);
}

HX_INL_END_
#endif // HX_DOXYGEN_PARSER
