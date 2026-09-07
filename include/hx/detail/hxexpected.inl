#pragma once
// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

#ifndef LIBHATCHET_VER
#error Internal. Do not include this file directly.
#endif

#ifndef HX_DOXYGEN_PARSER
HX_INL_BEGIN_

// Fixes gcc + optimizer + sanitizer -Wmaybe-uninitialized bug.
#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#endif

template<hxexpected_concept_ T_, hxunexpected_concept_ E_>
hxinline hxattr_flatten hxexpected<T_, E_>::hxexpected(const hxexpected& x_) noexcept
		: m_error_(x_.m_error_) {
	if(this->has_value()) {
		::new(static_cast<void*>(this->data())) T_(*x_.data());
	}
}

template<hxexpected_concept_ T_, hxunexpected_concept_ E_>
hxinline hxattr_flatten hxexpected<T_, E_>::hxexpected(hxexpected&& x_) noexcept
		: m_error_(hxmove(x_.m_error_)) {
	if(this->has_value()) {
		::new(static_cast<void*>(this->data())) T_(hxmove(*x_.data()));
		x_.reset();
	}
}

template<hxexpected_concept_ T_, hxunexpected_concept_ E_>
template<typename error_t_, typename... args_t_, hxenable_if_t<
	!hxis_hxexpected_<hxremove_cvref_t<error_t_>>::value, bool>>
hxinline hxattr_flatten hxexpected<T_, E_>::hxexpected(error_t_&& error_, args_t_&&... args_) noexcept
		: m_error_(hxforward<error_t_>(error_)) {
	if(this->has_value()) {
		::new(static_cast<void*>(this->data())) T_(hxforward<args_t_>(args_)...);
	}
}

template<hxexpected_concept_ T_, hxunexpected_concept_ E_>
hxinline hxattr_flatten hxexpected<T_, E_>::~hxexpected(void) {
	if(this->has_value()) {
		this->data()->T_::~T_();
	}
}

template<hxexpected_concept_ T_, hxunexpected_concept_ E_>
hxinline hxattr_flatten const T_& hxexpected<T_, E_>::operator*(void) const {
	hxassertf(this->has_value(), "bad_value");
	return *this->data();
}

template<hxexpected_concept_ T_, hxunexpected_concept_ E_>
hxinline hxattr_flatten T_& hxexpected<T_, E_>::operator*(void) {
	hxassertf(this->has_value(), "bad_value");
	return *this->data();
}

template<hxexpected_concept_ T_, hxunexpected_concept_ E_>
hxinline hxattr_flatten const T_* hxexpected<T_, E_>::operator->(void) const {
	hxassertf(this->has_value(), "bad_value");
	return this->data();
}

template<hxexpected_concept_ T_, hxunexpected_concept_ E_>
hxinline hxattr_flatten T_* hxexpected<T_, E_>::operator->(void) {
	hxassertf(this->has_value(), "bad_value");
	return this->data();
}

template<hxexpected_concept_ T_, hxunexpected_concept_ E_>
hxinline hxattr_flatten hxexpected<T_, E_>&
hxexpected<T_, E_>::operator=(const hxexpected& x_) noexcept {
	hxassertf(this != &x_, "self_copy");
	const bool has_value_ = this->has_value();
	const bool other_has_value_ = x_.has_value();
	if(other_has_value_) {
		if(has_value_) {
			*this->data() = *x_.data();
		} else {
			::new(static_cast<void*>(this->data())) T_(*x_.data());
		}
	} else if(has_value_) {
		this->data()->T_::~T_();
	}
	m_error_ = x_.m_error_;
	return *this;
}

template<hxexpected_concept_ T_, hxunexpected_concept_ E_>
hxinline hxattr_flatten hxexpected<T_, E_>&
hxexpected<T_, E_>::operator=(hxexpected&& x_) noexcept {
	hxassertf(this != &x_, "self_copy");
	const bool has_value_ = this->has_value();
	const bool other_has_value_ = x_.has_value();
	if(other_has_value_) {
		if(has_value_) {
			*this->data() = hxmove(*x_.data());
		} else {
			::new(static_cast<void*>(this->data())) T_(hxmove(*x_.data()));
		}
	} else if(has_value_) {
		this->data()->T_::~T_();
	}
	m_error_ = hxmove(x_.m_error_);
	if(other_has_value_) {
		x_.reset();
	}
	return *this;
}

template<hxexpected_concept_ T_, hxunexpected_concept_ E_>
template<typename U_, hxenable_if_t<
	!hxis_hxexpected_<hxremove_cvref_t<U_>>::value &&
	!hxis_same<hxremove_cvref_t<U_>, E_>(), bool>>
hxinline hxattr_flatten hxexpected<T_, E_>&
hxexpected<T_, E_>::operator=(U_&& value_) noexcept {
	if(this->has_value()) {
		*this->data() = hxforward<U_>(value_);
	} else {
		::new(static_cast<void*>(this->data())) T_(hxforward<U_>(value_));
	}
	m_error_ = E_(false);
	return *this;
}

template<hxexpected_concept_ T_, hxunexpected_concept_ E_>
hxinline hxattr_flatten bool hxexpected<T_, E_>::equal(const hxexpected& x_) const {
	if(this->has_value() != x_.has_value()) {
		return false;
	}
	if(!this->has_value()) {
		return m_error_ == x_.m_error_;
	}
	return *this->data() == *x_.data();
}

template<hxexpected_concept_ T_, hxunexpected_concept_ E_>
hxinline hxattr_flatten bool hxexpected<T_, E_>::operator==(const T_& value_) const {
	return this->has_value() && (*this->data() == value_);
}

template<hxexpected_concept_ T_, hxunexpected_concept_ E_>
template<typename self_t_, typename callable_t_>
hxinline hxattr_flatten auto hxexpected<T_, E_>::and_then(
		this self_t_&& self_, callable_t_&& callable_)
		-> hxremove_cvref_t<decltype(hxforward<callable_t_>(callable_)(
			hxforward_like<self_t_>(hxdeclval<T_&>())))> {
	if(self_.has_value()) {
		return hxforward<callable_t_>(callable_)(
			hxforward_like<self_t_>(*self_.data()));
	}
	return hxnil;
}

template<hxexpected_concept_ T_, hxunexpected_concept_ E_>
template<typename... args_t_>
hxinline hxattr_flatten T_& hxexpected<T_, E_>::emplace(args_t_&&... args_) noexcept {
	if(this->has_value()) {
		this->data()->T_::~T_();
	}
	T_* const value_ = ::new(static_cast<void*>(this->data()))
		T_(hxforward<args_t_>(args_)...);
	m_error_ = E_(false);
	return *value_;
}

template<hxexpected_concept_ T_, hxunexpected_concept_ E_>
template<typename self_t_>
hxinline hxattr_flatten auto hxexpected<T_, E_>::error(this self_t_&& self_)
		-> decltype(hxforward_like<self_t_>(hxdeclval<E_&>())) {
	hxassertf(static_cast<bool>(self_.m_error_), "wanted_error");
	return hxforward_like<self_t_>(self_.m_error_);
}

template<hxexpected_concept_ T_, hxunexpected_concept_ E_>
hxinline hxattr_flatten hxhash_t hxexpected<T_, E_>::hash(void) const {
	return this->has_value() ? hxkey_hash(*this->data()) : hxhash_t{31u};
}

template<hxexpected_concept_ T_, hxunexpected_concept_ E_>
template<typename self_t_, typename callable_t_>
hxinline hxattr_flatten hxexpected<T_, E_> hxexpected<T_, E_>::or_else(
		this self_t_&& self_, callable_t_&& callable_) {
	if(self_.has_value()) {
		return self_;
	}
	return hxforward<callable_t_>(callable_)();
}

template<hxexpected_concept_ T_, hxunexpected_concept_ E_>
hxinline hxattr_flatten void hxexpected<T_, E_>::reset(void) noexcept {
	if(this->has_value()) {
		this->data()->T_::~T_();
	}
	m_error_ = E_(true);
}

template<hxexpected_concept_ T_, hxunexpected_concept_ E_>
hxinline hxattr_flatten void hxexpected<T_, E_>::reset(const E_& error_) noexcept {
	const bool has_value_ = this->has_value();
	const bool new_has_value_ = !static_cast<bool>(error_);
	if(has_value_ && !new_has_value_) {
		this->data()->T_::~T_();
	} else if(!has_value_ && new_has_value_) {
		::new(static_cast<void*>(this->data())) T_();
	}
	m_error_ = error_;
}

template<hxexpected_concept_ T_, hxunexpected_concept_ E_>
hxinline hxattr_flatten void hxexpected<T_, E_>::swap(hxexpected& x_) noexcept {
	T_* const left_ = this->data();
	T_* const right_ = x_.data();
	const bool left_has_value_ = this->has_value();
	const bool right_has_value_ = x_.has_value();
	if(left_has_value_ && right_has_value_) {
		hxswap(*left_, *right_);
	} else if(left_has_value_) {
		::new(static_cast<void*>(right_)) T_(hxmove(*left_));
		left_->T_::~T_();
	} else if(right_has_value_) {
		::new(static_cast<void*>(left_)) T_(hxmove(*right_));
		right_->T_::~T_();
	}
	hxswap(m_error_, x_.m_error_);
}

template<hxexpected_concept_ T_, hxunexpected_concept_ E_>
template<typename self_t_>
hxinline hxattr_flatten auto hxexpected<T_, E_>::value(this self_t_&& self_)
		-> decltype(hxforward_like<self_t_>(hxdeclval<T_&>())) {
	hxassertf(self_.has_value(), "bad_value");
	return hxforward_like<self_t_>(*self_.data());
}

template<hxexpected_concept_ T_, hxunexpected_concept_ E_>
template<typename self_t_, typename... args_t_>
hxinline hxattr_flatten T_ hxexpected<T_, E_>::value_or(
		this self_t_&& self_, args_t_&&... args_) {
	if(self_.has_value()) {
		return static_cast<T_>(hxforward_like<self_t_>(*self_.data()));
	}
	return T_(hxforward<args_t_>(args_)...);
}

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic pop
#endif

HX_INL_END_
#endif // HX_DOXYGEN_PARSER
