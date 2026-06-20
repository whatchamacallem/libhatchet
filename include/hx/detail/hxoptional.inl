#pragma once
// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

template<typename T_>
hxoptional<T_>::hxoptional(const hxoptional& other_) : m_engaged_(false) {
	if (other_.m_engaged_) {
		::new(static_cast<void*>(&m_storage_)) T_(*other_.as_ptr_());
		m_engaged_ = true;
	}
}

template<typename T_>
hxoptional<T_>::hxoptional(hxoptional&& other_) : m_engaged_(false) {
	if (other_.m_engaged_) {
		::new(static_cast<void*>(&m_storage_)) T_(hxmove(*other_.as_ptr_()));
		m_engaged_ = true;
		other_.reset();
	}
}

template<typename T_>
template<typename U_, hxenable_if_t<!hxis_same<hxremove_cvref_t<U_>, hxoptional<T_>>::value, bool>>
hxoptional<T_>::hxoptional(U_&& value_) : m_engaged_(false) {
	::new(static_cast<void*>(&m_storage_)) T_(hxforward<U_>(value_));
	m_engaged_ = true;
}

template<typename T_>
hxoptional<T_>& hxoptional<T_>::operator=(const hxoptional& other_) {
	if (this == &other_) {
		return *this;
	}
	reset();
	if (other_.m_engaged_) {
		::new(static_cast<void*>(&m_storage_)) T_(*other_.as_ptr_());
		m_engaged_ = true;
	}
	return *this;
}

template<typename T_>
hxoptional<T_>& hxoptional<T_>::operator=(hxoptional&& other_) {
	hxassertmsg(this != &other_, "self_assignment");
	reset();
	if (other_.m_engaged_) {
		::new(static_cast<void*>(&m_storage_)) T_(hxmove(*other_.as_ptr_()));
		m_engaged_ = true;
		other_.reset();
	}
	return *this;
}

template<typename T_>
template<typename U_>
hxoptional<T_>& hxoptional<T_>::operator=(U_&& value_) {
	if (m_engaged_) {
		*as_ptr_() = hxforward<U_>(value_);
	} else {
		::new(static_cast<void*>(&m_storage_)) T_(hxforward<U_>(value_));
		m_engaged_ = true;
	}
	return *this;
}

template<typename T_>
bool hxoptional<T_>::operator==(const hxoptional& rhs_) const {
	if (m_engaged_ != rhs_.m_engaged_) {
		return false;
	}
	return !m_engaged_ || (*as_ptr_() == *rhs_.as_ptr_());
}

template<typename T_>
bool hxoptional<T_>::operator<(const hxoptional& rhs_) const {
	if (!rhs_.m_engaged_) {
		return false;
	}
	if (!m_engaged_) {
		return true;
	}
	return *as_ptr_() < *rhs_.as_ptr_();
}

template<typename T_>
template<typename... args_t_>
T_& hxoptional<T_>::emplace(args_t_&&... args_) {
	reset();
	::new(static_cast<void*>(&m_storage_)) T_(hxforward<args_t_>(args_)...);
	m_engaged_ = true;
	return *as_ptr_();
}

template<typename T_>
void hxoptional<T_>::reset(void) {
	if (m_engaged_) {
		as_ptr_()->~T_();
		m_engaged_ = false;
	}
}

template<typename T_>
void hxoptional<T_>::swap(hxoptional& other_) {
	if (m_engaged_ && other_.m_engaged_) {
		hxswap(*as_ptr_(), *other_.as_ptr_());
	} else if (m_engaged_) {
		::new(static_cast<void*>(&other_.m_storage_)) T_(hxmove(*as_ptr_()));
		other_.m_engaged_ = true;
		reset();
	} else if (other_.m_engaged_) {
		::new(static_cast<void*>(&m_storage_)) T_(hxmove(*other_.as_ptr_()));
		m_engaged_ = true;
		other_.reset();
	}
}
