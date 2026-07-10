#pragma once
// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

#ifndef LIBHATCHET_VER
#error Internal. Do not include this file directly.
#endif

#ifndef HX_DOXYGEN_PARSER
HX_BEGIN_INL_

// Fixes a gcc + optimizer -Wmaybe-uninitialized bug. Every method here does
// placement-new into m_storage_ behind an m_engaged_/has_value() check, and
// gcc's inliner intermittently loses track of that guard.
#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#endif

template<typename T_>
hxoptional<T_>::hxoptional(const hxoptional& other_) noexcept : m_engaged_(other_.m_engaged_) {
	if (m_engaged_) {
		::new(static_cast<void*>(&m_storage_)) T_(*reinterpret_cast<const T_*>(&other_.m_storage_));
	}
}

template<typename T_>
hxoptional<T_>::hxoptional(hxoptional&& other_) noexcept : m_engaged_(other_.m_engaged_) {
	if (m_engaged_) {
		T_* const src_ = reinterpret_cast<T_*>(&other_.m_storage_);
		::new(static_cast<void*>(&m_storage_)) T_(hxmove(*src_));
		src_->T_::~T_();
		other_.m_engaged_ = false;
	}
}

template<typename T_>
template<typename U_>
hxoptional<T_>::hxoptional(const hxoptional<U_>& other_) noexcept
		: m_engaged_(other_.has_value()) {
	if (m_engaged_) {
		::new(static_cast<void*>(&m_storage_)) T_(*other_);
	}
}

template<typename T_>
template<typename U_>
hxoptional<T_>::hxoptional(hxoptional<U_>&& other_) noexcept
		: m_engaged_(other_.has_value()) {
	if (m_engaged_) {
		::new(static_cast<void*>(&m_storage_)) T_(hxmove(*other_));
		other_.reset();
	}
}

template<typename T_>
template<typename U_, hxenable_if_t<
	!hxdetail_::hxis_hxoptional_<hxremove_cvref_t<U_>>::value, bool>>
hxoptional<T_>::hxoptional(U_&& value_) noexcept : m_engaged_(true) {
	::new(static_cast<void*>(&m_storage_)) T_(hxforward<U_>(value_));
}

template<typename T_>
hxoptional<T_>& hxoptional<T_>::operator=(const hxoptional& other_) noexcept {
	hxassertmsg(this != &other_, "self_assignment");
	if (other_.m_engaged_) {
		const T_& src_ = *reinterpret_cast<const T_*>(&other_.m_storage_);
		if (m_engaged_) {
			*reinterpret_cast<T_*>(&m_storage_) = src_;
		} else {
			::new(static_cast<void*>(&m_storage_)) T_(src_);
			m_engaged_ = true;
		}
	} else {
		this->reset();
	}
	return *this;
}

template<typename T_>
hxoptional<T_>& hxoptional<T_>::operator=(hxoptional&& other_) noexcept {
	hxassertmsg(this != &other_, "self_assignment");
	if (other_.m_engaged_) {
		T_* const src_ = reinterpret_cast<T_*>(&other_.m_storage_);
		if (m_engaged_) {
			*reinterpret_cast<T_*>(&m_storage_) = hxmove(*src_);
		} else {
			::new(static_cast<void*>(&m_storage_)) T_(hxmove(*src_));
			m_engaged_ = true;
		}
		src_->T_::~T_();
		other_.m_engaged_ = false;
	} else {
		this->reset();
	}
	return *this;
}

template<typename T_>
template<typename U_, hxenable_if_t<
	!hxdetail_::hxis_hxoptional_<hxremove_cvref_t<U_>>::value, bool>>
hxoptional<T_>& hxoptional<T_>::operator=(U_&& value_) noexcept {
	if (m_engaged_) {
		*reinterpret_cast<T_*>(&m_storage_) = hxforward<U_>(value_);
	} else {
		::new(static_cast<void*>(&m_storage_)) T_(hxforward<U_>(value_));
		m_engaged_ = true;
	}
	return *this;
}

template<typename T_>
template<typename U_>
hxoptional<T_>& hxoptional<T_>::operator=(const hxoptional<U_>& other_) noexcept {
	if (other_.has_value()) {
		if (m_engaged_) {
			*reinterpret_cast<T_*>(&m_storage_) = *other_;
		} else {
			::new(static_cast<void*>(&m_storage_)) T_(*other_);
			m_engaged_ = true;
		}
	} else {
		this->reset();
	}
	return *this;
}

template<typename T_>
template<typename U_>
hxoptional<T_>& hxoptional<T_>::operator=(hxoptional<U_>&& other_) noexcept {
	if (other_.has_value()) {
		if (m_engaged_) {
			*reinterpret_cast<T_*>(&m_storage_) = hxmove(*other_);
		} else {
			::new(static_cast<void*>(&m_storage_)) T_(hxmove(*other_));
			m_engaged_ = true;
		}
		other_.reset();
	} else {
		this->reset();
	}
	return *this;
}

template<typename T_>
T_& hxoptional<T_>::operator*(void) {
	hxassertmsg(m_engaged_, "optional_disengaged");
	return *reinterpret_cast<T_*>(&m_storage_);
}

template<typename T_>
const T_& hxoptional<T_>::operator*(void) const {
	hxassertmsg(m_engaged_, "optional_disengaged");
	return *reinterpret_cast<const T_*>(&m_storage_);
}

template<typename T_>
T_* hxoptional<T_>::operator->(void) {
	hxassertmsg(m_engaged_, "optional_disengaged");
	return reinterpret_cast<T_*>(&m_storage_);
}

template<typename T_>
const T_* hxoptional<T_>::operator->(void) const {
	hxassertmsg(m_engaged_, "optional_disengaged");
	return reinterpret_cast<const T_*>(&m_storage_);
}

template<typename T_>
bool hxoptional<T_>::operator==(const hxoptional& rhs_) const {
	if (m_engaged_ != rhs_.m_engaged_) {
		return false;
	}
	if (!m_engaged_) {
		return true;
	}
	const T_& l_ = *reinterpret_cast<const T_*>(&m_storage_);
	const T_& r_ = *reinterpret_cast<const T_*>(&rhs_.m_storage_);
	return l_ == r_;
}

template<typename T_>
bool hxoptional<T_>::operator==(const T_& value_) const {
	return m_engaged_ && (*reinterpret_cast<const T_*>(&m_storage_) == value_);
}

template<typename T_>
template<typename function_t_>
auto hxoptional<T_>::and_then(function_t_&& callable_)
		-> hxremove_cvref_t<decltype(hxforward<function_t_>(callable_)(hxdeclval<T_&>()))> {
	if (m_engaged_) {
		return hxforward<function_t_>(callable_)(*reinterpret_cast<T_*>(&m_storage_));
	}
	return hxnullopt;
}

template<typename T_>
template<typename function_t_>
auto hxoptional<T_>::and_then(function_t_&& callable_) const
		-> hxremove_cvref_t<decltype(hxforward<function_t_>(callable_)(hxdeclval<const T_&>()))> {
	if (m_engaged_) {
		return hxforward<function_t_>(callable_)(*reinterpret_cast<const T_*>(&m_storage_));
	}
	return hxnullopt;
}

template<typename T_>
template<typename... args_t_>
T_& hxoptional<T_>::emplace(args_t_&&... args_) noexcept {
	this->reset();
	T_* const p_ = ::new(static_cast<void*>(&m_storage_)) T_(hxforward<args_t_>(args_)...);
	m_engaged_ = true;
	return *p_;
}

template<typename T_>
template<typename function_t_>
hxoptional<T_> hxoptional<T_>::or_else(function_t_&& callable_) const {
	if (m_engaged_) {
		return *this;
	}
	return hxforward<function_t_>(callable_)();
}

template<typename T_>
void hxoptional<T_>::reset(void) noexcept {
	if (m_engaged_) {
		reinterpret_cast<T_*>(&m_storage_)->T_::~T_();
		m_engaged_ = false;
	}
}

template<typename T_>
void hxoptional<T_>::swap(hxoptional& other_) noexcept {
	T_* const l_ = reinterpret_cast<T_*>(&m_storage_);
	T_* const r_ = reinterpret_cast<T_*>(&other_.m_storage_);
	if (m_engaged_ && other_.m_engaged_) {
		hxswap(*l_, *r_);
	} else if (m_engaged_) {
		::new(static_cast<void*>(r_)) T_(hxmove(*l_));
		l_->T_::~T_();
		m_engaged_ = false;
		other_.m_engaged_ = true;
	} else if (other_.m_engaged_) {
		::new(static_cast<void*>(l_)) T_(hxmove(*r_));
		r_->T_::~T_();
		m_engaged_ = true;
		other_.m_engaged_ = false;
	}
}

template<typename T_>
T_& hxoptional<T_>::value(void) {
	hxassertmsg(m_engaged_, "optional_disengaged");
	return *reinterpret_cast<T_*>(&m_storage_);
}

template<typename T_>
const T_& hxoptional<T_>::value(void) const {
	hxassertmsg(m_engaged_, "optional_disengaged");
	return *reinterpret_cast<const T_*>(&m_storage_);
}

template<typename T_>
template<typename U_>
T_ hxoptional<T_>::value_or(U_&& default_value_) const {
	return m_engaged_ ? *reinterpret_cast<const T_*>(&m_storage_)
		: static_cast<T_>(hxforward<U_>(default_value_));
}

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic pop
#endif

HX_END_INL_
#endif // HX_DOXYGEN_PARSER
