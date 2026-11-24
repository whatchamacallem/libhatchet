#pragma once
// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

/// \file
///
/// `hxtest.hpp` is recommended instead of this code because it is a mature
/// test suite that produces the same outputs. This is for use where `hxtest`
/// would not fit into memory or otherwise be too cumbersome.
///
/// TAP 14 testcases. SKIP and TODO are explicitly not implemented. No excuses
/// are allowed when testing. The consumer can check for a certain number of
/// expected failures, when that is needed for full test coverage.
///
/// When `HX_USE_TEST_FRAMEWORK=0` the producer writes the TAP version and
/// numbers from 1. `HX_USE_TEST_FRAMEWORK=1` numbers the tests from 2, after
/// the test hxboot reports. Above that writes nothing.

#include <hx/libhatchet.h>

#include <stdio.h>

/// `TAP_TEST_PLAN()` - Reports the plan, which ends the TAP 14 document.
/// Follows the last `TAP_TEST_CASE`.
#define TAP_TEST_PLAN() hxtap::test_plan_(__COUNTER__ - 1)

/// `TAP_TEST_CASE(callable)` - Calls the testcase `fn` and reports it
/// numbered by `__COUNTER__`. Clears the `bool passed` in scope when it
/// fails.
/// - `callable` : The testcase.
#define TAP_TEST_CASE(callable_) passed &= hxtap::test_case_(__COUNTER__, #callable_, callable_())

/// `TAP_CHECK(int x)` - Returns `false` from the testcase when `x` is false.
/// - `x` : The condition the testcase requires.
#define TAP_CHECK(x_) if (!(x_)) return false

/// \cond HIDDEN
namespace hxtap {

void test_write_(const char* text_, size_t count_);

inline bool test_case_(int number_, const char* name_, bool result_) {
	(void)number_; (void)name_;
#if (HX_USE_TEST_FRAMEWORK) < 2
	if((HX_USE_TEST_FRAMEWORK) == 0 && number_ == 1) {
		test_write_("TAP version 14\n", sizeof "TAP version 14\n" - 1u);
	}
	char line[HX_MAX_LINE];
	const int length = ::snprintf(line, sizeof line, "%s %d - %s\n",
		result_ ? "ok" : "not ok", number_ + (HX_USE_TEST_FRAMEWORK), name_);
	hxassertf(length > 0 && static_cast<size_t>(length) < sizeof line, "test_case_ %s", name_);
	test_write_(line, static_cast<size_t>(length));
#endif
	return result_;
}

inline void test_plan_(int count_) {
	(void)count_;
#if (HX_USE_TEST_FRAMEWORK) < 2
	char line[16];
	const int length = ::snprintf(line, sizeof line, "1..%d\n",
		count_ + (HX_USE_TEST_FRAMEWORK));
	hxassertf(length > 0 && static_cast<size_t>(length) < sizeof line, "test_plan_ %d", count_);
	test_write_(line, static_cast<size_t>(length));
#endif
}

} // namespace hxtap
/// \endcond
