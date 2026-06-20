// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

#include <hx/libhatchet.h>
#include <hx/hxconsole.hpp>
#include <hx/hxfile.hpp>
#include <hx/hxtest.hpp>
#include "hxctest.h"

HX_NS_USE

int test_main(int argc, char**argv);
bool run_all_tests(const char* test_suite_filter);

namespace {

int hxs_test_assert_handler_count = 0;

bool hxtest_assert_handler(void) {
	++hxs_test_assert_handler_count;
	return true;
}
} // namespace

TEST(hxtest_main, set_assert_handler) {
	hxs_test_assert_handler_count = 0;
	hxset_assert_handler(hxtest_assert_handler);
	const volatile bool failing = false;
	hxassert_always((bool)failing, "hxtest_set_assert_handler_intentional");
	hxset_assert_handler(hxnull);
	EXPECT_EQ(hxs_test_assert_handler_count, 1);
}
bool run_all_tests(const char* test_suite_filter) {
	hxlog_console("libhatchet 🪓🪓🪓 " LIBHATCHET_TAG "\n");
	hxlog_console("C++: %d hardening: %d profiler: %d\n",
		static_cast<int>(HX_CPLUSPLUS),
		static_cast<int>(HX_HARDENING_MODE),
		static_cast<int>(HX_USE_PROFILER));
#if !(HX_USE_GOOGLE_TEST)
	const int tests_failing = RUN_ALL_TESTS(test_suite_filter);
#else
	const int tests_failing = RUN_ALL_TESTS(); (void)test_suite_filter;
#endif
#if HX_TEST_ERROR_HANDLING
	const int hxs_expected_failures = 4;
	hxassert_always(tests_failing == hxs_expected_failures,
		"unexpected_failures Expected exactly %d tests to fail...", hxs_expected_failures);
	if(tests_failing == hxs_expected_failures) {
		hxlog_handler(hxlog_level_warning,
			"expected_failures Expected exactly %d tests to fail...", hxs_expected_failures);
	}
	return tests_failing == hxs_expected_failures;
#else
	return tests_failing == 0;
#endif
}
hxconsole_command_named(run_all_tests, runtests);
#if HX_USE_FILE_IO
hxconsole_command_named(*+[](void) -> bool { return hxconsole_exec_file(hxin); }, execstdin);
#endif // HX_USE_FILE_IO
int test_main(int argc, char**argv) {
	hxinit();
	bool is_ok = true;
#if HX_USE_CONSOLE
	if(argc > 1) {
		for(int i=1; i<argc; ++i) {
			is_ok = is_ok && hxconsole_exec_line(argv[i]);
		}
	}
	else {
		is_ok = run_all_tests(hxnull);
	}
#else // !HX_USE_CONSOLE
	(void)argc; (void)argv;
	is_ok = run_all_tests(hxnull);
#endif
	hxshutdown();
	if(!is_ok) {
		hxexit(EXIT_FAILURE);
	}
	return is_ok ? EXIT_SUCCESS : EXIT_FAILURE;
}
int main(int argc, char**argv) {
	testing::InitGoogleTest(&argc, argv);
#if (HX_USE_GOOGLE_TEST) && ((HX_HARDENING_MODE) == HX_HARDENING_MODE_DEBUG)
	GTEST_FLAG_SET(break_on_failure, true);
#endif
	return test_main(argc, argv);
}
