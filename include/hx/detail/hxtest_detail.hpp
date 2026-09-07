#pragma once
// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

#if !defined LIBHATCHET_VER || (HX_USE_GOOGLE_TEST)
#error Internal. Do not include this file directly.
#endif

#ifndef HX_DOXYGEN_PARSER
HX_NS_BEGIN_
namespace hxdetail_ {

// Use -DHX_TEST_MAX_CASES=N to raise the limit.
#if !defined HX_TEST_MAX_CASES
hxinline_constexpr hxsize_t HX_TEST_MAX_CASES = 1024;
#endif

// Internal. 4 ULPs float comparison.
hxattr_nodiscard bool hxtest_float_eq_(float a_, float b_);

// Internal. 4 ULPs double comparison.
hxattr_nodiscard bool hxtest_double_eq_(double a_, double b_);

// Internal. strcmp wrappers that assert on null arguments.
hxattr_nodiscard bool hxtest_str_eq_(const char* a_, const char* b_);
hxattr_nodiscard bool hxtest_str_ne_(const char* a_, const char* b_);

// hxtest_case_ - Internal. Global constructor.
class hxtest_case_ {
public:
	hxtest_case_(void (*run_function_)(void), const char* suite_name_,
		const char* case_name_, const char* file_name_, int line_number_);
	void (*m_run_)(void);
	const char* m_suite_;
	const char* m_case_;
	const char* m_file_;
	int m_line_;
};

// hxtest_ - Internal. The test tracking and dispatching singleton.
class hxtest_ {
public:
	using test_cases_t_ = hxvector<hxtest_case_*, HX_TEST_MAX_CASES>;

	enum class test_state_ : uint8_t {
		nothing_asserted_,
		pass_,
		fail_
	};

	hxtest_(void);

	// Ensures constructor runs before tests are registered by global constructors.
	static hxtest_& dispatcher_(void);

	// Called by global constructors.
	void add_test_(hxtest_case_* fn_) hxattr_nonnull(2);

	const test_cases_t_& test_cases_(void) const;

	// Assert callback used by macros.
	void condition_check_(bool condition_, const char* file_, int line_,
		const char* message_, bool is_assert_) hxattr_nonnull(3,5);

	static bool filter_(const char* filter_, test_cases_t_& test_cases_) hxattr_nonnull(1);

	int run_all_tests_(void);

private:
	hxtest_(const hxtest_&) = delete;
	void operator=(const hxtest_&) = delete;

	test_cases_t_ m_test_cases_;
	hxtest_case_* m_current_test_;
	test_state_ m_test_state_;
	int m_pass_count_;
	int m_fail_count_;
	int m_total_assert_count_;
	int m_assert_count_;
};

} // hxdetail_
HX_NS_END_
#endif // HX_DOXYGEN_PARSER
