// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

#include "../include/hx/hxtest.hpp"
#include "../include/hx/hxsort.hpp"

#include <math.h>

HX_NS_BEGIN_

#if !HX_USE_GOOGLE_TEST

namespace hxdetail_ {

// hxtest_*_eq_ - ULP-based floating point equality (GoogleTest-compatible)
// Compares IEEE-754 numbers by units in the last place (ULPs) with a fixed
// threshold of 4 ULPs for gtest's `EXPECT_FLOAT_EQ` / `EXPECT_DOUBLE_EQ`.
// Unlike Google Test, this fails on any non-finite values because comparing
// test data with infinity indicates a possible issue.

#if defined __clang__
__attribute__((optnone))
#endif
bool hxtest_float_eq_(float a, float b) {
#ifndef __FAST_MATH__
	// No :: prefix on isfinite because isfinite is a macro in C.
	// -fno-fast-math also explicitly breaks isfinite.
	if(!isfinite(a) || !isfinite(b)) { return false; }
#endif
	if(a == b) { return true; }

	uint32_t ua = 0u; ::memcpy(&ua, &a, sizeof ua);
	uint32_t ub = 0u; ::memcpy(&ub, &b, sizeof ub);

	const uint32_t sign_mask = 1u << 31;
	const uint32_t ba = ((ua & sign_mask) != 0u) ? (~ua + 1u) : (sign_mask | ua);
	const uint32_t bb = ((ub & sign_mask) != 0u) ? (~ub + 1u) : (sign_mask | ub);
	const uint32_t delta = (ba >= bb) ? (ba - bb) : (bb - ba);

	return delta <= 4u; // 4 ULPs.
}

#if defined __clang__
__attribute__((optnone))
#endif
bool hxtest_double_eq_(double a, double b) {
#ifndef __FAST_MATH__
	// -fno-fast-math explicitly breaks isfinite.
	if(!isfinite(a) || !isfinite(b)) { return false; }
#endif
	if(a == b) { return true; }

	uint64_t ua = 0u; ::memcpy(&ua, &a, sizeof ua);
	uint64_t ub = 0u; ::memcpy(&ub, &b, sizeof ub);

	const uint64_t sign_mask = 1ull << 63;
	const uint64_t ba = ((ua & sign_mask) != 0u) ? (~ua + 1u) : (sign_mask | ua);
	const uint64_t bb = ((ub & sign_mask) != 0u) ? (~ub + 1u) : (sign_mask | ub);
	const uint64_t delta = (ba >= bb) ? (ba - bb) : (bb - ba);

	return delta <= 4u; // 4 ULPs.
}

static bool hxtest_case_sort(const hxtest_case_interface_* a, const hxtest_case_interface_* b) {
	// Run tests by suite name and then by line number. Runs smoke tests before
	// complex tests in the order written.
	const int compare = ::strcmp(a->suite_(), b->suite_());
	if(compare == 0) { return a->line_() < b->line_(); }
	return compare < 0;
}

hxtest_::hxtest_(void) {
	::memset(static_cast<void*>(this), 0x00, sizeof *this);
}

hxtest_& hxtest_::dispatcher_(void) {
	static hxtest_ hxs_test_static_alloc;
	return hxs_test_static_alloc;
}

void hxtest_::add_test_(hxtest_case_interface_* fn) {
	// Use -DHX_TEST_MAX_CASES to provide enough room for all tests.
	hxassert_always(m_num_test_cases_ < HX_TEST_MAX_CASES, "HX_TEST_MAX_CASES overflow\n");
	if(m_num_test_cases_ < HX_TEST_MAX_CASES) {
		m_test_cases_[m_num_test_cases_++] = fn;
	}
}

// Message is required to end with an \n. Returns hxdev_null on success and
// hxerr otherwise.
void hxtest_::condition_check_(bool condition, const char* file, size_t line, const char* message, bool is_assert) {
	hxassert_always(m_current_test_, "test_not_started");
	m_test_state_ = (condition && m_test_state_ != test_state_fail_) ? test_state_pass_ : test_state_fail_;
		if(!condition) {
			++m_total_assert_count_;
			if(++m_assert_count_ >= max_fail_messages_) {
				if(m_assert_count_ == max_fail_messages_) {
					hxlog_console("remaining asserts will fail silently...\n");
				}
				return;
			}

		// Prints full-path error messages that can be clicked on in an IDE.
		m_current_test_->case_();
		m_current_test_->suite_();
		hxlog_handler(hxlog_level_assert, "test_fail %s.%s", m_current_test_->suite_(), m_current_test_->case_());
		hxlog_handler(hxlog_level_assert, "test_fail_at %s(%zu): %s", file, line, message);

		if(is_assert) {
			// ASSERT_* macros halt the test suite on failure.
			hxlog_handler(hxlog_level_assert, "test_assert_fail ❌");
			hxbreakpoint();
			::_Exit(EXIT_FAILURE);
		}
		else {
			// Debug builds always set breakpoints on unexpected failures.
			// Implements GTEST_FLAG_SET(break_on_failure, true);
#if (HX_TEST_ERROR_HANDLING) == 0 && (HX_HARDENING_MODE) == HX_HARDENING_MODE_DEBUG
			hxbreakpoint();
#endif
		}
		return;
	}
}

size_t hxtest_::run_all_tests_(const char* test_suite_filter) {
	hxinit(); // RUN_ALL_TESTS could be called first.
	hxlog_console("[==========] Running tests: %s\n", (test_suite_filter ? test_suite_filter : "All"));

	m_test_suite_filter_ = test_suite_filter;
	m_pass_count_ = m_fail_count_ = 0u;
	m_total_assert_count_ = 0u;

	// Breaking hxinsertion_sort breaks everything.
	hxinsertion_sort(m_test_cases_, m_test_cases_ + m_num_test_cases_, hxtest_case_sort);

	// Starting point. Expected to reset to zero after each test.
	hxsystem_allocator_scope temporary_stack_base(hxsystem_allocator_temporary_stack);

	hxassert_always(temporary_stack_base.get_current_allocation_count() == 0u
				&& temporary_stack_base.get_current_bytes_allocated() == 0u,
		"test_leaks Temp stack is expected to be empty when running tests.");

	for(hxtest_case_interface_** it = m_test_cases_; it != (m_test_cases_ + m_num_test_cases_); ++it) {
		if((m_test_suite_filter_ == hxnull)
				|| (::strcmp(m_test_suite_filter_, (*it)->suite_()) == 0)) {
			hxlog_console("[ RUN      ] %s.%s\n", (*it)->suite_(), (*it)->case_());
			m_current_test_ = *it;
			m_test_state_ = test_state_nothing_asserted_;
			m_assert_count_ = 0u;

#ifdef __cpp_exceptions
			try
#endif
			{
				(*it)->run_test_();

				// Expect the test to use another scope to reset the stack if needed.
				const size_t t_count = temporary_stack_base.get_current_allocation_count();
				const size_t t_bytes = temporary_stack_base.get_current_bytes_allocated();
				if((t_count != 0u) || (t_bytes != 0u)) {
					this->condition_check_(false, (*it)->file_(), (*it)->line_(),
						"test_leaks All tests must reset the temp stack.", true);
				}
			}
#ifdef __cpp_exceptions
			catch (...) {
				this->condition_check_(false, (*it)->file_(), (*it)->line_(), "unexpected_exception", true);
			}
#endif
			if(m_test_state_ == test_state_nothing_asserted_) {
				this->condition_check_(false, (*it)->file_(), (*it)->line_(), "nothing_tested", false);
			}
			if(m_test_state_ == test_state_pass_) {
				++m_pass_count_;
				hxlog_console("[       OK ] %s.%s\n", (*it)->suite_(), (*it)->case_());
			}
			else {
				++m_fail_count_;
				hxlog_console("[  FAILED  ] %s.%s\n", (*it)->suite_(), (*it)->case_());
			}
		}
	}
	m_current_test_ = hxnull;

	hxlog_console("[==========] skipped %zu tests. failed %zu assertions.\n",
		m_num_test_cases_ - m_pass_count_ - m_fail_count_, m_total_assert_count_);

	hxwarn_msg(m_pass_count_ + m_fail_count_, "nothing_tested");

	if(m_pass_count_ != 0u && m_fail_count_ == 0u) {
		// This is Google Test style. If only it were green.
		hxlog_handler(hxlog_level_console, "[  PASSED  ] %zu test%s.\n", m_pass_count_,
			((m_pass_count_ != 1u) ? "s" : ""));
	}
	else {
		hxlog_handler(hxlog_level_console, "%zu FAILED TEST%s ❌\n", m_fail_count_,
			m_fail_count_ == 1u ? "" : "S");
		// Count nothing tested as one failure.
		m_fail_count_ = hxmax(m_fail_count_, (size_t)1u);
	}
	return m_fail_count_;
}

} // hxdetail_

#endif

HX_NS_END_
