#pragma once
// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

/// \file hx/hxoptional.hpp An optional value type.

#include "libhatchet.h"
#include "hxutility.h"

// ----------------------------------------------------------------------------
// hxnullopt_t / hxnullopt

/// `hxnullopt_t` - A tag type used to construct a disengaged `hxoptional`.
struct hxnullopt_t {
	/// Explicit constructor prevents implicit construction from `{}`.
	explicit constexpr hxnullopt_t(int) { }
};

/// `hxnullopt` - A sentinel value of type `hxnullopt_t` representing a
/// disengaged `hxoptional`. Implements `std::nullopt`.
#define hxnullopt hxnullopt_t(0)

// ----------------------------------------------------------------------------
// hxoptional

/// `hxoptional<T>` - Holds either a value of type `T` or nothing. Implements
/// `std::optional`. The value is stored in aligned internal storage without
/// dynamic allocation. A disengaged optional compares equal to `hxnullopt`.
/// - `T` : The contained value type. Must not be a reference or array type.
template<typename T_>
class hxoptional {
public:
	using value_type = T_;

	/// Constructs a disengaged `hxoptional`.
	constexpr hxoptional(void) : m_engaged_(false) { }

	/// Constructs a disengaged `hxoptional` from `hxnullopt`.
	/// - `nullopt` : The disengaged sentinel.
	constexpr hxoptional(hxnullopt_t) : m_engaged_(false) { }

	/// Copy constructor. Copies the engaged state and value from `other`.
	/// - `other` : The `hxoptional` to copy from.
	hxoptional(const hxoptional& other_);

	/// Move constructor. Moves the engaged state and value from `other`.
	/// `other` is left disengaged.
	/// - `other` : The `hxoptional` to move from.
	hxoptional(hxoptional&& other_);

	/// Constructs an engaged `hxoptional` by forwarding `value` into storage.
	/// - `value` : The value to construct from. Must be convertible to `T`.
	template<typename U_=T_, hxenable_if_t<!hxis_same<hxremove_cvref_t<U_>, hxoptional>::value, bool> = true>
	hxoptional(U_&& value_);

	/// Destroys the contained value if engaged.
	~hxoptional(void) { reset(); }

	/// Returns a reference to the contained value. The optional must be engaged.
	hxattr_nodiscard T_& operator*(void) {
		hxassertmsg(m_engaged_, "disengaged"); return *as_ptr_();
	}

	/// Returns a const reference to the contained value. The optional must be
	/// engaged.
	hxattr_nodiscard const T_& operator*(void) const {
		hxassertmsg(m_engaged_, "disengaged"); return *as_ptr_();
	}

	/// Returns a pointer to the contained value. The optional must be engaged.
	hxattr_nodiscard T_* operator->(void) {
		hxassertmsg(m_engaged_, "disengaged"); return as_ptr_();
	}

	/// Returns a const pointer to the contained value. The optional must be
	/// engaged.
	hxattr_nodiscard const T_* operator->(void) const {
		hxassertmsg(m_engaged_, "disengaged"); return as_ptr_();
	}

	/// Returns `true` if the optional contains a value.
	hxattr_nodiscard constexpr operator bool(void) const { return m_engaged_; }

	/// Copy assignment. Destroys any current value, then copies from `other`.
	/// - `other` : The `hxoptional` to copy from.
	hxoptional& operator=(const hxoptional& other_);

	/// Move assignment. Destroys any current value, then moves from `other`.
	/// `other` is left disengaged.
	/// - `other` : The `hxoptional` to move from.
	hxoptional& operator=(hxoptional&& other_);

	/// Disengages the optional, destroying any contained value.
	/// - `nullopt` : The disengaged sentinel.
	hxoptional& operator=(hxnullopt_t) { reset(); return *this; }

	/// Assigns `value` by forwarding, engaging the optional.
	/// - `value` : The value to assign from. Must be convertible to `T`.
	template<typename U_=T_>
	hxoptional& operator=(U_&& value_);

	/// Returns `true` if both optionals are disengaged or both are engaged with
	/// equal values.
	/// - `rhs` : Right-hand side optional.
	hxattr_nodiscard bool operator==(const hxoptional& rhs_) const;

	/// Returns `true` if this optional is disengaged.
	/// - `nullopt` : The disengaged sentinel.
	hxattr_nodiscard bool operator==(hxnullopt_t) const { return !m_engaged_; }

	/// Returns `true` if this optional is engaged and its value equals `value`.
	/// - `value` : The value to compare against.
	hxattr_nodiscard bool operator==(const T_& value_) const {
		return m_engaged_ && (*as_ptr_() == value_);
	}

	/// Returns `true` if the optionals are not equal.
	/// - `rhs` : Right-hand side optional.
	hxattr_nodiscard bool operator!=(const hxoptional& rhs_) const {
		return !(*this == rhs_);
	}

	/// Returns `true` if this optional is engaged.
	/// - `nullopt` : The disengaged sentinel.
	hxattr_nodiscard bool operator!=(hxnullopt_t) const { return m_engaged_; }

	/// Returns `true` if this optional is disengaged or its value does not
	/// equal `value`.
	/// - `value` : The value to compare against.
	hxattr_nodiscard bool operator!=(const T_& value_) const {
		return !m_engaged_ || (*as_ptr_() != value_);
	}

	/// Returns `true` if this is disengaged or this compares less than `rhs`.
	/// A disengaged optional is less than any engaged optional.
	/// - `rhs` : Right-hand side optional.
	hxattr_nodiscard bool operator<(const hxoptional& rhs_) const;

	/// Returns `true` if this compares less than or equal to `rhs`.
	/// - `rhs` : Right-hand side optional.
	hxattr_nodiscard bool operator<=(const hxoptional& rhs_) const {
		return !(rhs_ < *this);
	}

	/// Returns `true` if this compares greater than `rhs`.
	/// - `rhs` : Right-hand side optional.
	hxattr_nodiscard bool operator>(const hxoptional& rhs_) const {
		return rhs_ < *this;
	}

	/// Returns `true` if this compares greater than or equal to `rhs`.
	/// - `rhs` : Right-hand side optional.
	hxattr_nodiscard bool operator>=(const hxoptional& rhs_) const {
		return !(*this < rhs_);
	}

	/// Constructs the contained value in place from `args`, destroying any
	/// previous value.
	/// - `args` : Arguments forwarded to the constructor of `T`.
	template<typename... args_t_>
	T_& emplace(args_t_&&... args_);

	/// Returns `true` if the optional contains a value.
	hxattr_nodiscard constexpr bool has_value(void) const { return m_engaged_; }

	/// Destroys the contained value if engaged and leaves the optional
	/// disengaged.
	void reset(void);

	/// Exchanges the contents with `other`. Both engaged states and values are
	/// swapped.
	/// - `other` : The `hxoptional` to swap with.
	void swap(hxoptional& other_);

	/// Returns a reference to the contained value. The optional must be engaged.
	hxattr_nodiscard T_& value(void) {
		hxassertmsg(m_engaged_, "optional_disengaged"); return *as_ptr_();
	}

	/// Returns a const reference to the contained value. The optional must be
	/// engaged.
	hxattr_nodiscard const T_& value(void) const {
		hxassertmsg(m_engaged_, "optional_disengaged"); return *as_ptr_();
	}

	/// Returns the contained value if engaged, otherwise returns `default_value`.
	/// - `default_value` : The value to return when disengaged.
	hxattr_nodiscard T_ value_or(const T_& default_value_) const {
		return m_engaged_ ? *as_ptr_() : default_value_;
	}

private:
	hxattr_nodiscard T_* as_ptr_(void) {
		return reinterpret_cast<T_*>(&m_storage_);
	}

	hxattr_nodiscard const T_* as_ptr_(void) const {
		return reinterpret_cast<const T_*>(&m_storage_);
	}

	alignas(T_) unsigned char m_storage_[sizeof(T_)];
	bool m_engaged_;
};

// ----------------------------------------------------------------------------
// Out-of-line member implementations

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
