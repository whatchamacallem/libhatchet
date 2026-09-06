// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

#include <hx/libhatchet.h>
#include <hx/hxconsole.hpp>
#include <hx/hxfile.hpp>
#include <hx/hxtest.hpp>

HX_NS_USE

int test_main(int argc, char**argv);
bool run_all_tests(const char* test_suite_filter);

namespace {

int hxs_test_assert_handler_count = 0;

bool hxtest_assert_handler(void) {
	++hxs_test_assert_handler_count;
	return true;
}

#if (HX_HARDENING_MODE) == HX_HARDENING_MODE_DEBUG
bool hxtest_assert_handler_declines(void) {
	++hxs_test_assert_handler_count;
	return false;
}
#endif // (HX_HARDENING_MODE) == HX_HARDENING_MODE_DEBUG

#if HX_USE_CONSOLE
bool hxs_test_console_fn_void_called = false;
bool hxtest_console_fn_void(void) { hxs_test_console_fn_void_called = true; return true; }
bool hxtest_console_failing_command(void) { return false; }
#endif // HX_USE_CONSOLE
} // namespace

TEST(hxtest_main, set_assert_handler) {
	hxlog_warning("EXPECTING_ASSERT_FAILURE");
	hxs_test_assert_handler_count = 0;
	hxset_assert_handler(hxtest_assert_handler);
	const volatile bool failing = false;
	hxassert_always((bool)failing, "hxset_assert_handler should have fired");
	hxset_assert_handler(hxnull);
	EXPECT_EQ(hxs_test_assert_handler_count, 1);

#if (HX_HARDENING_MODE) == HX_HARDENING_MODE_DEBUG
	// Normally hxassert_handler returning false causes a breakpoint.

	hxset_assert_handler(hxtest_assert_handler);
	EXPECT_TRUE(hxassert_handler("no_slash_or_backslash.cpp", 1u));
	EXPECT_TRUE(hxassert_handler("dir\\backslash_only.cpp", 1u));
	hxset_assert_handler(hxnull);

	hxlog_warning("EXPECTING_ASSERT_FAILURE");
	hxset_assert_handler(hxtest_assert_handler_declines);
	EXPECT_FALSE(hxassert_handler(__FILE__, __LINE__));
	hxset_assert_handler(hxnull);

	hxlog_warning("EXPECTING_ASSERT_FAILURE");
	EXPECT_FALSE(hxassert_handler(__FILE__, __LINE__));
#endif
}

TEST(hxtest_main, hxinit_internal_already_initialized) {
	hxinit_internal(LIBHATCHET_VER);
	EXPECT_EQ(hxg_init_ver_, LIBHATCHET_VER);
}

TEST(hxtest_main, hxshutdown_not_initialized) {
	const int previous = hxg_init_ver_;
	hxg_init_ver_ = 0;
	hxshutdown();
	EXPECT_EQ(hxg_init_ver_, 0);
	hxg_init_ver_ = previous;
}

// This is here because trailing underscores are allowed in this file.
#if HX_USE_CONSOLE
TEST(hxconsole_test, register_command_replaces_duplicate) {
	hxlog_warning("EXPECTING_TEST_WARNINGS");
	static const hxdetail_::hxconsole_constructor_ first_(
		hxdetail_::hxconsole_command_factory_(&hxtest_console_fn_void),
		"hxconsole_test_reregistered");
	hxs_test_console_fn_void_called = false;
	EXPECT_TRUE(hxconsole_exec_line("hxconsole_test_reregistered"));
	EXPECT_TRUE(hxs_test_console_fn_void_called);

	static const hxdetail_::hxconsole_constructor_ second_(
		hxdetail_::hxconsole_command_factory_(&hxtest_console_failing_command),
		"hxconsole_test_reregistered");

	hxs_test_console_fn_void_called = false;
	EXPECT_FALSE(hxconsole_exec_line("hxconsole_test_reregistered"));
	EXPECT_FALSE(hxs_test_console_fn_void_called);
}
#endif // HX_USE_CONSOLE

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
	const int expected_failures = 8;
	hxassert_always(tests_failing == expected_failures,
		"stray_failures want %d", expected_failures);
	if(tests_failing == expected_failures) {
		hxlog_handler(hxlog_level_warning,
			"expected_failures Expected exactly %d tests to fail...", expected_failures);
	}
	return tests_failing == expected_failures;
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

	// Allocate the temporary stacks used by the tests. That 335u is actually
	// very precise. See TEST(hxmemory_manager_test, temp_overflow);
	const size_t stack_sizes[] = { 335u * HX_KIB, 2u * HX_KIB, 1u * HX_KIB };
	hxmemory_manager_allocate_stacks(stack_sizes);

	bool is_ok = true;
#if HX_USE_CONSOLE
	if(argc > 1) {
		for(int i=1; i<argc; ++i) {
			is_ok = is_ok && hxconsole_exec_line(argv[i]);
		}
	}
	// GCOVR_EXCL_START
	else {
		is_ok = run_all_tests(hxnull);
	}
	// GCOVR_EXCL_STOP
#else // !HX_USE_CONSOLE
	(void)argc; (void)argv;
	is_ok = run_all_tests(hxnull);
#endif

#if HX_USE_MEMORY_MANAGER
	// Two tests are designed to test allocator overflow. Anything else is a
	// bug. See: TEST(hxmemory_manager_test, temp_overflow). Skip when only
	// listing tests as gtest_discover_tests does, since nothing has run.
	const hxmemory_manager_stats stats = hxmemory_manager_utilization(true, false);
	hxassert_always(stats.allocator_overflows == 2,
		"allocator_overflow count %zu", stats.allocator_overflows);
#endif

	hxshutdown();
	return is_ok ? EXIT_SUCCESS : EXIT_FAILURE;
}

int main(int argc, char**argv) {
#if (HX_USE_GOOGLE_TEST)
	testing::InitGoogleTest(&argc, argv);
	if(GTEST_FLAG_GET(list_tests)) {
		const int dont_care = RUN_ALL_TESTS(); (void)dont_care;
		return EXIT_SUCCESS;
	}
#if (HX_HARDENING_MODE) == HX_HARDENING_MODE_DEBUG
	GTEST_FLAG_SET(break_on_failure, true);
#endif
#endif
	// Calling hxexit would break coverage.
	return test_main(argc, argv);
}
