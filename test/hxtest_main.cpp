// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

// Include everything first to catch conflicts.
#include <hx/libhatchet.h>
#include <hx/hxconsole.hpp>
#include <hx/hxfile.hpp>
#include <hx/hxtest.hpp> // May include Google Test.

#include "hxctest.h"

int test_main(int argc, char**argv);
bool run_all_tests(void);

bool run_all_tests(void) {
	hxlogconsole("libhatchet 🪓🪓🪓 " LIBHATCHET_TAG "\n");
	hxlogconsole("release: %d profile: %d\n",
		static_cast<int>(HX_RELEASE),
		static_cast<int>(HX_PROFILE));

	// RUN_ALL_TESTS is a Google Test symbol.
	const size_t tests_failing = static_cast<size_t>(RUN_ALL_TESTS());

#if HX_TEST_ERROR_HANDLING
	const int s_hxexpected_failures = 4;

	hxassertrelease(tests_failing == s_hxexpected_failures,
		"unexpected_failures Expected exactly %d tests to fail...", s_hxexpected_failures);
	// There are no asserts at level 3.
	if(tests_failing == s_hxexpected_failures) {
		hxloghandler(hxloglevel_warning,
			"expected_failures Expected exactly %d tests to fail...", s_hxexpected_failures);
	}
	return tests_failing == s_hxexpected_failures;
#else
	return tests_failing == 0;
#endif
}

#if HX_CPLUSPLUS >= 202002L
static bool execute_stdin(void) {
	return hxconsole_exec_file(hxin);
}

// Command line parameter to run all tests.
hxconsole_command_named(run_all_tests, runtests);

// Command line parameter to execute stdin.
hxconsole_command_named(execute_stdin, execstdin);
#endif // HX_CPLUSPLUS >= 202002L

// test_main - Command line console command dispatcher. Each parameter is treated
// as a separate command.
int test_main(int argc, char**argv) {
	hxinit();

	bool is_ok = true;
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
#if (HX_RELEASE) < 3
	hxshutdown();
#endif
	return is_ok ? EXIT_SUCCESS : EXIT_FAILURE;
}

// main - calls test_main.
int main(int argc, char**argv) {
	testing::InitGoogleTest(&argc, argv);

#if (HX_USE_GOOGLE_TEST) && (HX_RELEASE) == 0
	GTEST_FLAG_SET(break_on_failure, true);
#endif

	return test_main(argc, argv);
}
