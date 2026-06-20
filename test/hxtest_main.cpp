// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

// Include everything first to catch conflicts.
#include <hx/libhatchet.h>
#include <hx/hxconsole.hpp>
#include <hx/hxfile.hpp>
#include <hx/hxtest.hpp> // May include Google Test.

#include "hxctest.h"

HX_NS_USE

int test_main(int argc, char**argv);
bool run_all_tests(void);

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

	// volatile prevents the compiler from treating code after hxassert_always
	// as unreachable.
	const volatile bool failing = false;
	hxassert_always((bool)failing, "hxtest_set_assert_handler_intentional");

	hxset_assert_handler(hxnull);
	EXPECT_EQ(hxs_test_assert_handler_count, 1);
}

bool run_all_tests(void) {
	hxlog_console("libhatchet 🪓🪓🪓 " LIBHATCHET_TAG "\n");
	hxlog_console("C++: %d hardening: %d profiler: %d\n",
		static_cast<int>(HX_CPLUSPLUS),
		static_cast<int>(HX_HARDENING_MODE),
		static_cast<int>(HX_USE_PROFILER));

	// RUN_ALL_TESTS is a Google Test symbol.
	const size_t tests_failing = static_cast<size_t>(RUN_ALL_TESTS());

#if HX_TEST_ERROR_HANDLING
	const int hxs_expected_failures = 4;

	hxassert_always(tests_failing == hxs_expected_failures,
		"unexpected_failures Expected exactly %d tests to fail...", hxs_expected_failures);
	// There are no asserts at level 3.
	if(tests_failing == hxs_expected_failures) {
		hxlog_handler(hxlog_level_warning,
			"expected_failures Expected exactly %d tests to fail...", hxs_expected_failures);
	}
	return tests_failing == hxs_expected_failures;
#else
	return tests_failing == 0;
#endif
}

// Console requires C++20.
#if HX_CPLUSPLUS >= 202002L
static bool execute_stdin(void) {
	return hxconsole_exec_file(hxin);
}

// Command line parameter to run all tests.
hxconsole_command_named(run_all_tests, runtests);

// Command line parameter to execute stdin.
hxconsole_command_named(execute_stdin, execstdin);
#endif // HX_CPLUSPLUS >= 202002L

// test_main - Command line console command dispatcher. Each parameter is
// treated as a separate command.
int test_main(int argc, char**argv) {
	hxinit();

	bool is_ok = true;
// Console requires C++20.
#if HX_CPLUSPLUS >= 202002L
	if(argc > 1) {
		for(int i=1; i<argc; ++i) {
			is_ok = is_ok && hxconsole_exec_line(argv[i]);
		}
	}
	else {
		is_ok = is_ok && run_all_tests();
	}
#else
	(void)argc; (void)argv;
	is_ok = is_ok && run_all_tests();
#endif

	// Logging and asserts are actually unaffected by a shutdown.
	hxshutdown();
	return is_ok ? EXIT_SUCCESS : EXIT_FAILURE;
}

// main - calls test_main.
int main(int argc, char**argv) {
	testing::InitGoogleTest(&argc, argv);

#if (HX_USE_GOOGLE_TEST) && ((HX_HARDENING_MODE) == HX_HARDENING_MODE_DEBUG)
	GTEST_FLAG_SET(break_on_failure, true);
#endif

	return test_main(argc, argv);
}
