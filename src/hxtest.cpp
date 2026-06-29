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

bool hxtest_float_eq_(float a, float b) {
	uint32_t ua = 0u; ::memcpy(&ua, &a, sizeof ua);
	uint32_t ub = 0u; ::memcpy(&ub, &b, sizeof ub);

	// Comparisons involving non-finite numbers are always false.
	if ((ua & 0x7f800000u) == 0x7f800000u) {
		return false;
	}
	if ((ub & 0x7f800000u) == 0x7f800000u) {
		return false;
	}
	if(a == b) {
		return true;
	}

	const uint32_t sign_mask = 1u << 31;
	const uint32_t ba = ((ua & sign_mask) != 0u) ? (~ua + 1u) : (sign_mask | ua);
	const uint32_t bb = ((ub & sign_mask) != 0u) ? (~ub + 1u) : (sign_mask | ub);
	const uint32_t delta = (ba >= bb) ? (ba - bb) : (bb - ba);

	return delta <= 4u; // 4 ULPs.
}

bool hxtest_double_eq_(double a, double b) {
	uint64_t ua = 0u; ::memcpy(&ua, &a, sizeof ua);
	uint64_t ub = 0u; ::memcpy(&ub, &b, sizeof ub);

	// Comparisons involving non-finite numbers are always false.
	if ((ua & 0x7ff0000000000000ull) == 0x7ff0000000000000ull) {
		return false;
	}
	if ((ub & 0x7ff0000000000000ull) == 0x7ff0000000000000ull) {
		return false;
	}
	if(a == b) { return true; }

	const uint64_t sign_mask = 1ull << 63;
	const uint64_t ba = ((ua & sign_mask) != 0ull) ? (~ua + 1ull) : (sign_mask | ua);
	const uint64_t bb = ((ub & sign_mask) != 0ull) ? (~ub + 1ull) : (sign_mask | ub);
	const uint64_t delta = (ba >= bb) ? (ba - bb) : (bb - ba);

	return delta <= 4u; // 4 ULPs.
}

bool hxtest_str_eq_(const char* a, const char* b) {
	hxassert_always(a && b, "strcmp Null arg.");
	return ::strcmp(a, b) == 0;
}

bool hxtest_str_ne_(const char* a, const char* b) {
	hxassert_always(a && b, "strcmp Null arg.");
	return ::strcmp(a, b) != 0;
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
	hxassert_always(m_num_test_cases_ < (int)HX_TEST_MAX_CASES, "HX_TEST_MAX_CASES overflow");
	m_test_cases_[m_num_test_cases_++] = fn;
}

void hxtest_::condition_check_(bool condition, const char* file, size_t line, const char* message, bool is_assert) {
	hxassert_always(m_current_test_, "test_not_started");
	m_test_state_ = (condition && m_test_state_ != test_state_fail_) ? test_state_pass_ : test_state_fail_;
	if(!condition) {
		++m_total_assert_count_;
		++m_assert_count_;

		if(is_assert) {
			// ASSERT_* macros halt the test suite on failure. Always log and exit
			// regardless of the message limit so control flow never continues.
// GCOVR_EXCL_START
			hxlog_handler(hxlog_level_assert, "test_fail %s.%s", m_current_test_->suite_(), m_current_test_->case_());
			hxlog_handler(hxlog_level_assert, "test_fail_at %s(%zu): %s", file, line, message);
			hxlog_handler(hxlog_level_assert, "test_assert_fail ❌");
			hxlog_console("[==========] aborted after %d passed, %d failed, %d assertions.\n",
				m_pass_count_, m_fail_count_ + 1, m_total_assert_count_);
			hxbreakpoint();
			hxexit(EXIT_FAILURE);
// GCOVR_EXCL_STOP
		}

		if(m_assert_count_ > max_fail_messages_) {
			if(m_assert_count_ == max_fail_messages_ + 1) {
				hxlog_console("remaining asserts will fail silently...\n");
			}
			return;
		}

		// Prints full-path error messages that can be clicked on in an IDE.
		hxlog_handler(hxlog_level_assert, "test_fail %s.%s", m_current_test_->suite_(), m_current_test_->case_());
		hxlog_handler(hxlog_level_assert, "test_fail_at %s(%zu): %s", file, line, message);

		// Debug builds always set breakpoints on unexpected failures.
		// Implements GTEST_FLAG_SET(break_on_failure, true);
#if (HX_TEST_ERROR_HANDLING) == 0 && (HX_HARDENING_MODE) == HX_HARDENING_MODE_DEBUG
		hxbreakpoint();
#endif
	}
}

int hxtest_::run_all_tests_(const char* test_suite_filter) {
	hxinit(); // RUN_ALL_TESTS could be called first.

	if(test_suite_filter != hxnull && test_suite_filter[0] == 0) {
		test_suite_filter = hxnull;
	}
	hxlog_console("[==========] Running tests: %s\n", (test_suite_filter ? test_suite_filter : "All"));

	m_pass_count_ = m_fail_count_ = 0;
	m_total_assert_count_ = 0;

	// Breaking hxinsertion_sort breaks everything.
	hxinsertion_sort(m_test_cases_, m_test_cases_ + m_num_test_cases_, hxtest_case_sort);

	// Starting point. Expected to reset to zero after each test.
	hxassert_always(hxmemory_manager_utilization(true, false).allocations_outstanding == 0u,
		"test_leaks Temp stacks are expected to be empty when running tests");

	for(hxtest_case_interface_** it = m_test_cases_; it != (m_test_cases_ + m_num_test_cases_); ++it) {
		if((test_suite_filter == hxnull)
				|| (::strcmp(test_suite_filter, (*it)->suite_()) == 0)) {
			hxlog_console("[ RUN      ] %s.%s\n", (*it)->suite_(), (*it)->case_());
			m_current_test_ = *it;
			m_test_state_ = test_state_nothing_asserted_;
			m_assert_count_ = 0;

#ifdef __cpp_exceptions
			try
#endif
			{
				(*it)->run_test_();

				// Expect the test to use another scope to reset the stack if needed.
				hxmemory_manager_stats stats = hxmemory_manager_utilization(true, false);
				if(stats.allocations_outstanding != 0u || stats.bytes_outstanding != 0u) {
// GCOVR_EXCL_START
					this->condition_check_(false, (*it)->file_(), (*it)->line_(),
						"test_leaks All tests must reset the temp stack.", true);
// GCOVR_EXCL_STOP
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

	hxmemory_manager_utilization(false, true);

	hxlog_console("[==========] skipped %d tests. failed %d assertions.\n",
		m_num_test_cases_ - m_pass_count_ - m_fail_count_, m_total_assert_count_);

// GCOVR_EXCL_START
	hxassert_always(m_pass_count_ + m_fail_count_, "nothing_tested");

	if(m_pass_count_ != 0 && m_fail_count_ == 0) {
		// This is Google Test style. If only it were green.
		hxlog_handler(hxlog_level_console, "[  PASSED  ] %d test%s.\n", m_pass_count_,
			((m_pass_count_ != 1) ? "s" : ""));
// GCOVR_EXCL_STOP
	}
	else {
		hxlog_handler(hxlog_level_console, "%d FAILED TEST%s ❌\n", m_fail_count_,
			m_fail_count_ == 1 ? "" : "S");
		// Count nothing tested as one failure.
		m_fail_count_ = hxmax(m_fail_count_, 1);
	}
	return m_fail_count_;
}

} // hxdetail_

#endif

HX_NS_END_
