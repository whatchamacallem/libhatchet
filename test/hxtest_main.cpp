// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

#include <hx/libhatchet.h>
#include <hx/hxconsole.hpp>
#include <hx/hxfile.hpp>
#include <hx/hxtest.hpp>

HX_NS_USE

int test_main(int argc, char**argv);
bool run_all_tests(void);

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

#if !(HX_USE_GOOGLE_TEST)
// Recognizes "--gtest_break_on_failure" and "--gtest_filter=..." in argv, sets
// hxg_settings from them and then erases them from argv/argc.
void hxtest_handle_gtest_args(int* argc, const char** argv, bool verbose) {
	const char gtest_break_on_failure[] = "--gtest_break_on_failure";
	const char gtest_filter_prefix[] = "--gtest_filter=";
	for(int i = 1; i < *argc; ) {
		bool found = false;
		if(::strcmp(argv[i], gtest_break_on_failure) == 0) {
			hxg_settings.test_break_on_failure = true;
			found = true;
		}
		else if(::strncmp(argv[i], gtest_filter_prefix,
				sizeof(gtest_filter_prefix)-1) == 0) {
			hxassert_always(hxg_settings.test_filter == hxnull, "gtest_filter passed twice");
			hxg_settings.test_filter = argv[i] + sizeof(gtest_filter_prefix) - 1;
			found = true;
		}

		if(found) {
			hxwarn(!verbose, "gtest_arg %s", argv[i]);
			::memmove(argv + i, argv + i + 1,
				sizeof(char*) * static_cast<size_t>(*argc - i));
			--*argc;
		}
		else {
			++i;
		}
	}
}
#endif // !HX_USE_GOOGLE_TEST
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
	// Normally hxassert_handler returning false causes a breakpoint. Forced
	// off here because these calls exercise that return value directly
	// instead of going through a real EXPECT_*/ASSERT_* failure.
	const bool break_on_failure = hxg_settings.test_break_on_failure;
	hxg_settings.test_break_on_failure = false;

	hxset_assert_handler(hxtest_assert_handler);
	EXPECT_TRUE(hxassert_handler("no_slash_or_backslash.cpp", 1u));
	EXPECT_TRUE(hxassert_handler("dir\\backslash_only.cpp", 1u));
	hxset_assert_handler(hxnull);

	hxlog_warning("EXPECTING_ASSERT_FAILURE");
	hxset_assert_handler(hxtest_assert_handler_declines);
	EXPECT_FALSE(hxassert_handler(__FILE__, __LINE__));
	hxset_assert_handler(hxnull);

	EXPECT_TRUE(hxassert_handler(__FILE__, __LINE__));

	hxg_settings.test_break_on_failure = break_on_failure;
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

#if !(HX_USE_GOOGLE_TEST)
TEST(hxtest_main, filter) {
	const hxdetail_::hxtest_::test_cases_t_& all_cases =
		hxdetail_::hxtest_::dispatcher_().test_cases_();

	hxdetail_::hxtest_case_* self_ = hxnull;
	int suite_count = 0;
	for(hxdetail_::hxtest_case_* const* it = all_cases.begin(); it != all_cases.end(); ++it) {
		if(::strcmp((*it)->m_suite_, "hxtest_main") == 0) {
			++suite_count;
			if(::strcmp((*it)->m_case_, "filter") == 0) {
				self_ = *it;
			}
		}
	}
	ASSERT_TRUE(self_ != hxnull);
	EXPECT_GT(all_cases.size(), suite_count);

	{
		hxdetail_::hxtest_::test_cases_t_ copy(all_cases);
		EXPECT_TRUE(hxdetail_::hxtest_::filter_("hxtest_main.filter", copy));
		EXPECT_EQ(copy.size(), 1);
		EXPECT_EQ(copy[0], self_);
	}
	{
		hxdetail_::hxtest_::test_cases_t_ copy(all_cases);
		EXPECT_TRUE(hxdetail_::hxtest_::filter_("hxtest_main.*", copy));
		EXPECT_EQ(copy.size(), suite_count);
		EXPECT_EQ(copy.find(self_) != copy.end(), true);
	}
	{
		hxdetail_::hxtest_::test_cases_t_ copy(all_cases);
		EXPECT_TRUE(hxdetail_::hxtest_::filter_(
			"hxtest_no_such_suite.*:hxtest_main.filter", copy));
		EXPECT_EQ(copy.size(), 1);
		EXPECT_EQ(copy[0], self_);
	}
	{
		hxdetail_::hxtest_::test_cases_t_ copy(all_cases);
		EXPECT_TRUE(hxdetail_::hxtest_::filter_("-hxtest_main.*", copy));
		EXPECT_EQ(copy.size(), all_cases.size() - suite_count);
		EXPECT_EQ(copy.find(self_) == copy.end(), true);
	}
	{
		hxdetail_::hxtest_::test_cases_t_ copy(all_cases);
		EXPECT_FALSE(hxdetail_::hxtest_::filter_("hxtest_no_such_suite_at_all.*", copy));
		EXPECT_TRUE(copy.empty());
	}
	{
		hxdetail_::hxtest_::test_cases_t_ copy(1, self_);
		EXPECT_FALSE(hxdetail_::hxtest_::filter_("-hxtest_no_such_suite.*:hxtest_main.filter", copy));
		EXPECT_TRUE(copy.empty());
	}
	{
		hxdetail_::hxtest_::test_cases_t_ copy(1, self_);
		EXPECT_FALSE(hxdetail_::hxtest_::filter_("hxtest_mainxy", copy));
	}
	{
		hxdetail_::hxtest_::test_cases_t_ copy(1, self_);
		EXPECT_FALSE(hxdetail_::hxtest_::filter_("hxtest_main.y", copy));
	}
	{
		hxdetail_::hxtest_::test_cases_t_ copy(1, self_);
		EXPECT_FALSE(hxdetail_::hxtest_::filter_("hxtest_main.filte1", copy));
	}
}

TEST(hxtest_main, hxtest_strip_g_args) {
	const bool break_on_failure = hxg_settings.test_break_on_failure;
	const char* const filter = hxg_settings.test_filter;
	hxg_settings.test_break_on_failure = false;
	hxg_settings.test_filter = hxnull;

	const char* argv[6] = {
		"hxtest",
		"--gtest_break_on_failure",
		"--gtest_filter=hxtest_main.hxtest_strip_g_args",
		"runtests",
		"--generic",
		hxnull
	};
	int argc = 5;

	hxtest_handle_gtest_args(&argc, argv, false);

	EXPECT_EQ(argc, 3);
	EXPECT_STREQ(argv[0], "hxtest");
	EXPECT_STREQ(argv[1], "runtests");
	EXPECT_STREQ(argv[2], "--generic");
	EXPECT_EQ(argv[3], hxnull);
	EXPECT_TRUE(hxg_settings.test_break_on_failure);
	EXPECT_STREQ(hxg_settings.test_filter, "hxtest_main.hxtest_strip_g_args");

	hxg_settings.test_break_on_failure = break_on_failure;
	hxg_settings.test_filter = filter;
}
#endif // !HX_USE_GOOGLE_TEST

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

bool run_all_tests(void) {
	hxlog_console("libhatchet 🪓🪓🪓 " LIBHATCHET_TAG "\n");
	hxlog_console("C++: %d hardening: %d profiler: %d\n",
		static_cast<int>(HX_CPLUSPLUS),
		static_cast<int>(HX_HARDENING_MODE),
		static_cast<int>(HX_USE_PROFILER));

	const int tests_failing = RUN_ALL_TESTS();

#if HX_TEST_ERROR_HANDLING
	if(hxg_settings.test_filter == hxnull) {
		const int expected_failures = 8;
		if(tests_failing == expected_failures) {
			hxlog_handler(hxlog_level_warning,
				"expected_failures Expected exactly %d tests to fail...", expected_failures);
		}
		return tests_failing == expected_failures;
	}
	return true; // GCOVR_EXCL_LINE
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
		is_ok = run_all_tests();
	}
	// GCOVR_EXCL_STOP
#else // !HX_USE_CONSOLE
	(void)argc; (void)argv;
	is_ok = run_all_tests();
#endif

#if HX_USE_MEMORY_MANAGER
	// Two tests are designed to test allocator overflow. Anything else is a
	// bug. See: TEST(hxmemory_manager_test, temp_overflow).
	const hxmemory_manager_stats stats = hxmemory_manager_utilization(true, false);
	hxassert_always(hxg_settings.test_filter != hxnull || stats.allocator_overflows == 2,
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
#else // !HX_USE_GOOGLE_TEST
	hxinit(); // Init hxsettings.
	hxtest_handle_gtest_args(&argc, const_cast<const char**>(argv), true);
#endif
	// Calling hxexit would break coverage.
	return test_main(argc, argv);
}
