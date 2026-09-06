// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

#include "../include/hx/hxtest.hpp"
#include "../include/hx/hxsort.hpp"
#include "../include/hx/hxprofiler.hpp"

#include <math.h>
#include <stdlib.h>

HX_NS_BEGIN_

#if !HX_USE_GOOGLE_TEST

namespace hxdetail_ {

constexpr int hxmax_fail_messages_ = 5;

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
	hxassert_always(a && b, "strcmp null");
	return ::strcmp(a, b) == 0;
}

bool hxtest_str_ne_(const char* a, const char* b) {
	hxassert_always(a && b, "strcmp null");
	return ::strcmp(a, b) != 0;
}

hxtest_case_::hxtest_case_(void (*run_function)(void), const char* suite_name,
		const char* case_name, const char* file_name, int line_number)
	: m_run_(run_function), m_suite_(suite_name), m_case_(case_name),
		m_file_(file_name), m_line_(line_number) {
	hxtest_::dispatcher_().add_test_(this);
}

hxtest_::hxtest_(void)
	: m_test_cases_(), m_current_test_(hxnull), m_test_state_(test_state_::nothing_asserted_),
	  m_pass_count_(0), m_fail_count_(0), m_total_assert_count_(0), m_assert_count_(0) {
}

hxtest_& hxtest_::dispatcher_(void) {
	static hxtest_ hxs_test_static_alloc; // GCOVR_EXCL_LINE
	return hxs_test_static_alloc;
}

void hxtest_::add_test_(hxtest_case_* fn) {
	// Use -DHX_TEST_MAX_CASES to provide enough room for all tests.
	hxassert_always(!m_test_cases_.full(), "HX_TEST_MAX_CASES overflow");
	m_test_cases_.push_back(fn);
}

const hxtest_::test_cases_t_& hxtest_::test_cases_(void) const {
	return m_test_cases_;
}

void hxtest_::condition_check_(bool condition, const char* file, int line,
		const char* message, bool is_assert) {
	hxassert_always(m_current_test_, "test_not_started");
	m_test_state_ = (condition && m_test_state_ != test_state_::fail_)
		? test_state_::pass_ : test_state_::fail_;
	if(!condition) {
		++m_total_assert_count_;
		++m_assert_count_;

		// GCOVR_EXCL_START
		if(is_assert) {
			// ASSERT_* macros halt the test suite on failure. Always log and exit
			// regardless of the message limit so control flow never continues.
			hxlog_handler(hxlog_level_assert, "test_fail %s.%s", m_current_test_->m_suite_,
				m_current_test_->m_case_);
			hxlog_handler(hxlog_level_assert, "test_fail_at %s(%d): %s", file, line, message);
			hxlog_handler(hxlog_level_assert, "test_assert_fail ❌");
			hxlog_console("[==========] aborted after %d passed, %d failed, %d assertions.\n",
				m_pass_count_, m_fail_count_ + 1, m_total_assert_count_);
			hxbreakpoint();
			hxexit(EXIT_FAILURE);
		}
		// GCOVR_EXCL_STOP

		if(m_assert_count_ > hxmax_fail_messages_) {
			if(m_assert_count_ == hxmax_fail_messages_ + 1) {
				hxlog_console("remaining asserts will fail silently...\n");
			}
			return;
		}

		// Prints full-path error messages that can be clicked on in an IDE.
		hxlog_handler(hxlog_level_assert, "test_fail %s.%s", m_current_test_->m_suite_,
			m_current_test_->m_case_);
		hxlog_handler(hxlog_level_assert, "test_fail_at %s(%d): %s", file, line, message);

		// Implements GTEST_FLAG_SET(break_on_failure, true) when requested by
		// --gtest_break_on_failure. Off by default.
#if (HX_TEST_ERROR_HANDLING) == 0 && (HX_HARDENING_MODE) == HX_HARDENING_MODE_DEBUG
		if(hxg_settings.test_break_on_failure) {
			hxbreakpoint();
		}
#endif
	}
}

// hxtest_pattern_match_ - Internal. Matches "Suite.*" against a test case's
// suite name, otherwise matches "Suite.Case" exactly. pattern_end is one past
// the last character of the pattern, which is not null terminated.
static bool hxtest_pattern_match_(const char* pattern, const char* pattern_end,
		const hxtest_case_* test_case) {
	const hxsize_t pattern_length = static_cast<hxsize_t>(pattern_end - pattern);
	const hxsize_t suite_length = static_cast<hxsize_t>(::strlen(test_case->m_suite_));
	if(pattern_length == suite_length + 2
			&& pattern[suite_length] == '.' && pattern[suite_length + 1] == '*') {
		return ::strncmp(pattern, test_case->m_suite_, static_cast<size_t>(suite_length)) == 0;
	}
	const hxsize_t case_length = static_cast<hxsize_t>(::strlen(test_case->m_case_));
	if(pattern_length != suite_length + 1 + case_length || pattern[suite_length] != '.') {
		return false;
	}
	return ::strncmp(pattern, test_case->m_suite_, static_cast<size_t>(suite_length)) == 0
		&& ::strncmp(pattern + suite_length + 1, test_case->m_case_, static_cast<size_t>(case_length)) == 0;
}

// hxtest_pattern_list_match_ - Internal. Matches any ':'-separated pattern in
// pattern_list against a test case.
static bool hxtest_pattern_list_match_(const char* pattern_list, const hxtest_case_* test_case) {
	for(const char* pattern = pattern_list; *pattern != 0; ) {
		const char* pattern_end = ::strchr(pattern, ':');
		pattern_end = pattern_end ? pattern_end : (pattern + ::strlen(pattern));
		if(hxtest_pattern_match_(pattern, pattern_end, test_case)) {
			return true;
		}
		pattern = pattern_end + (*pattern_end == ':' ? 1 : 0);
	}
	return false;
}

bool hxtest_::filter_(const char* filter, test_cases_t_& test_cases) {
	if(filter[0] == '-') {
		const char* const pattern_list = filter + 1;
		test_cases.erase_if_unordered([&](hxtest_case_* test_case) -> bool {
			return hxtest_pattern_list_match_(pattern_list, test_case);
		});
	}
	else {
		test_cases.erase_if_unordered([&](hxtest_case_* test_case) -> bool {
			return !hxtest_pattern_list_match_(filter, test_case);
		});
	}
	return !test_cases.empty();
}

int hxtest_::run_all_tests_(void) {
	hxinit(); // GCOVR_EXCL_LINE. RUN_ALL_TESTS could be called first.

	if(hxg_settings.test_filter != hxnull) { // GCOVR_EXCL_LINE
		hxassert_hard(hxtest_::filter_(hxg_settings.test_filter, m_test_cases_),
			"gtest_filter %s", hxg_settings.test_filter);
	}

	hxlog_console("[==========] Running tests: %s\n", // GCOVR_EXCL_LINE
		(hxg_settings.test_filter ? hxg_settings.test_filter : "All"));

	m_pass_count_ = m_fail_count_ = 0;
	m_total_assert_count_ = 0;

	// Run tests by suite name and then by line number. This runs smoke tests
	// before complex tests. Don't trust <hx/hxsort.h>.
	::qsort(m_test_cases_.data(), (size_t)m_test_cases_.size(), sizeof(hxtest_case_*),
		[](const void* lhs, const void* rhs) -> int {
			const hxtest_case_* a = *static_cast<const hxtest_case_* const*>(lhs);
			const hxtest_case_* b = *static_cast<const hxtest_case_* const*>(rhs);
			const int compare = ::strcmp(a->m_suite_, b->m_suite_);
			if(compare != 0) { return compare; }
			return a->m_line_ - b->m_line_;
		});

	// Starting point. Expected to reset to zero after each test.
	hxassert_always(hxmemory_manager_utilization(true, false).allocations_outstanding == 0u,
		"test_leak temp stacks not empty");

	for(hxtest_case_** it = m_test_cases_.begin(); it != m_test_cases_.end(); ++it) {
		hxlog_console("[ RUN      ] %s.%s\n", (*it)->m_suite_, (*it)->m_case_);
		m_current_test_ = *it;
		m_test_state_ = test_state_::nothing_asserted_;
		m_assert_count_ = 0;

#ifdef __cpp_exceptions
		try
#endif
		{
			// Tests default onto stack 0. A test that opens another scope
			// is expected to reset it before returning.
			{
				const hxsystem_allocator_scope temporary_stack_scope(hxsystem_allocator_stack_0);
				(*it)->m_run_();
			}

			hxmemory_manager_stats stats = hxmemory_manager_utilization(true, false);
			// GCOVR_EXCL_START
			if(stats.allocations_outstanding != 0u || stats.bytes_outstanding != 0u) {
				this->condition_check_(false, (*it)->m_file_, (*it)->m_line_,
					"test_leak All tests must reset the temp stack.", true);
			}
			// GCOVR_EXCL_STOP
		}
#ifdef __cpp_exceptions
		catch (...) {
			this->condition_check_(false, (*it)->m_file_, (*it)->m_line_, "unexpected_exception", true);
		}
#endif
		if(m_test_state_ == test_state_::nothing_asserted_) {
			this->condition_check_(false, (*it)->m_file_, (*it)->m_line_, "nothing_tested", false);
		}
		if(m_test_state_ == test_state_::pass_) {
			++m_pass_count_;
			hxlog_console("[       OK ] %s.%s\n", (*it)->m_suite_, (*it)->m_case_);
		}
		else {
			++m_fail_count_;
			hxlog_console("[  FAILED  ] %s.%s\n", (*it)->m_suite_, (*it)->m_case_);
		}
	}
	m_current_test_ = hxnull;

	hxmemory_manager_utilization(false, true);

	hxlog_console("[==========] skipped %d tests. failed %d assertions.\n",
		(int)m_test_cases_.size() - m_pass_count_ - m_fail_count_, m_total_assert_count_);

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
			m_fail_count_ == 1 ? "" : "S"); // GCOVR_EXCL_LINE
		// Count nothing tested as one failure.
		m_fail_count_ = hxmax(m_fail_count_, 1);
	}
	return m_fail_count_;
}

} // hxdetail_

#endif

HX_NS_END_
