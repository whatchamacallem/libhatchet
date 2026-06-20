#pragma once
// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

/// \file hxinitializer_list.hpp Implements or includes std::initializer_list
/// depending on HX_USE_LIBCXX.

#include "libhatchet.h"

// HX_USE_MODULE allows including macros in addition to the hx module.
#if HX_USE_MODULE
#error Header does not provide macros only.
#endif

#if HX_USE_LIBCXX
#include <initializer_list>
#else // !HX_USE_LIBCXX

/// \cond HIDDEN
namespace std {

template<typename T>
class initializer_list {
public:
	using value_type = T;
	using reference = const T&;
	using const_reference = const T&;
	using size_type = size_t;
	using iterator = const T*;
	using const_iterator = const T*;

	constexpr initializer_list(void) noexcept : m_begin(0), m_size(0u) { }
	constexpr size_t size(void) const noexcept { return m_size; }
	constexpr const T* begin(void) const noexcept { return m_begin; }
	constexpr const T* end(void) const noexcept { return m_begin + m_size; }

private:
	constexpr initializer_list(const T* begin_, size_t size_) noexcept
		: m_begin(begin_), m_size(size_) { }

	const T* m_begin;
	size_t m_size;
};

} // namespace std
/// \endcond

#endif // !HX_USE_LIBCXX
