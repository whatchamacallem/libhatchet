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
hxoptional<T_>::hxoptional(const hxoptional& other_) noexcept : m_engaged_(false) {
	if (other_.m_engaged_) {
		::new(static_cast<void*>(&m_storage_)) T_(*reinterpret_cast<const T_*>(&other_.m_storage_));
		m_engaged_ = true;
	}
}

template<typename T_>
hxoptional<T_>::hxoptional(hxoptional&& other_) noexcept : m_engaged_(false) {
	if (other_.m_engaged_) {
		::new(static_cast<void*>(&m_storage_)) T_(hxmove(*reinterpret_cast<T_*>(&other_.m_storage_)));
		m_engaged_ = true;
		other_.reset();
	}
}

template<typename T_>
template<typename U_>
hxoptional<T_>::hxoptional(const hxoptional<U_>& other_) noexcept : m_engaged_(false) {
	if (other_.has_value()) {
		::new(static_cast<void*>(&m_storage_)) T_(*other_);
		m_engaged_ = true;
	}
}

template<typename T_>
template<typename U_>
hxoptional<T_>::hxoptional(hxoptional<U_>&& other_) noexcept : m_engaged_(false) {
	if (other_.has_value()) {
		::new(static_cast<void*>(&m_storage_)) T_(hxmove(*other_));
		m_engaged_ = true;
		other_.reset();
	}
}

template<typename T_>
template<typename U_, hxenable_if_t<
	!hxis_same<hxremove_cvref_t<U_>, hxoptional<T_>>::value &&
	!hxdetail_::hxis_hxoptional_<hxremove_cvref_t<U_>>::value, bool>>
hxoptional<T_>::hxoptional(U_&& value_) noexcept : m_engaged_(false) {
	::new(static_cast<void*>(&m_storage_)) T_(hxforward<U_>(value_));
	m_engaged_ = true;
}

template<typename T_>
hxoptional<T_>& hxoptional<T_>::operator=(const hxoptional& other_) noexcept {
	hxassertmsg(this != &other_, "self_assignment");
	reset();
	if (other_.m_engaged_) {
		::new(static_cast<void*>(&m_storage_)) T_(*reinterpret_cast<const T_*>(&other_.m_storage_));
		m_engaged_ = true;
	}
	return *this;
}

template<typename T_>
hxoptional<T_>& hxoptional<T_>::operator=(hxoptional&& other_) noexcept {
	hxassertmsg(this != &other_, "self_assignment");
	reset();
	if (other_.m_engaged_) {
		::new(static_cast<void*>(&m_storage_)) T_(hxmove(*reinterpret_cast<T_*>(&other_.m_storage_)));
		m_engaged_ = true;
		other_.reset();
	}
	return *this;
}

template<typename T_>
template<typename U_, hxenable_if_t<!hxis_same<hxremove_cvref_t<U_>, hxoptional<T_>>::value, bool>>
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
	const T_& l_ = *reinterpret_cast<const T_*>(&m_storage_);
	const T_& r_ = *reinterpret_cast<const T_*>(&rhs_.m_storage_);
	return !m_engaged_ || (l_ == r_);
}

template<typename T_>
bool hxoptional<T_>::operator==(const T_& value_) const {
	return m_engaged_ && (*reinterpret_cast<const T_*>(&m_storage_) == value_);
}

#if HX_CPLUSPLUS < 202002L // C++20 defaults != from ==.
template<typename T_>
bool hxoptional<T_>::operator!=(const T_& value_) const {
	return !m_engaged_ || (*reinterpret_cast<const T_*>(&m_storage_) != value_);
}
#endif

template<typename T_>
template<typename... args_t_>
T_& hxoptional<T_>::emplace(args_t_&&... args_) noexcept {
	reset();
	::new(static_cast<void*>(&m_storage_)) T_(hxforward<args_t_>(args_)...);
	m_engaged_ = true;
	return *reinterpret_cast<T_*>(&m_storage_);
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
	if (m_engaged_ && other_.m_engaged_) {
		hxswap(*reinterpret_cast<T_*>(&m_storage_), *reinterpret_cast<T_*>(&other_.m_storage_));
	} else if (m_engaged_) {
		::new(static_cast<void*>(&other_.m_storage_)) T_(hxmove(*reinterpret_cast<T_*>(&m_storage_)));
		other_.m_engaged_ = true;
		reset();
	} else if (other_.m_engaged_) {
		::new(static_cast<void*>(&m_storage_)) T_(hxmove(*reinterpret_cast<T_*>(&other_.m_storage_)));
		m_engaged_ = true;
		other_.reset();
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
T_ hxoptional<T_>::value_or(const T_& default_value_) const {
	return m_engaged_ ? *reinterpret_cast<const T_*>(&m_storage_) : default_value_;
}

HX_END_INL_
#endif // HX_DOXYGEN_PARSER
