#pragma once
// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

/// \file
/// Google Test-compatible framework for writing unit tests. It does not spam
/// your system memory allocator with string operations right after an assert
/// fails. Actually, it never allocates. To disable this header and switch to
/// testing with `<gtest/gtest.h>` directly, use `-DHX_USE_GOOGLE_TEST=1`. Only
/// core features are provided. This framework uses only `operator<` and
/// `operator==` in its assertions. Compatibility with Google Test may require
/// additional relational operators.
///
/// - `TEST(suite, name)` - Defines a test case without a fixture.
/// - `TEST_F(fixture, name)` - Defines a test case using a fixture class.
/// - Use `ASSERT_`* for fatal assertions and `EXPECT_`* for non-fatal.
/// - See `RUN_ALL_TESTS` in `test/hxtest_main.cpp` for example.
///
/// - Simple Test Case (no fixture):
/// ```
///   TEST(Math, Addition) {
///	   int a = 2, b = 3;
///	   EXPECT_EQ(a + b, 5);
///	   EXPECT_TRUE(a < b + 2);
///	   EXPECT_NEAR(3.14, 3.141, 0.01);
///	   SUCCEED();
///   }
/// ```
/// - Fixture-Based Test Case (using `TEST_F`):
/// ```
///   class MyFixture : public testing::Test {
///   public:
///	   void SetUp() { value = 42; }
///	   void TearDown() { EXPECT_EQ(value, 100); }
///	   void set_value(int x) { value = x; }
///	   int value;
///   };
///
///   TEST_F(MyFixture, ValueIsSet) {
///	   EXPECT_EQ(value, 42);
///	   set_value(100);
///	   EXPECT_NE(value, 42);
///   }
/// ```
///
///   | Macro | Assertion |
///   | --- | --- |
///   | `SUCCEED(void)` | Marks the current test as successful without any checks. |
///   | `FAIL(void)` | WARNING. Calls `return`. Marks the current test as failed. |
///   | `ADD_FAILURE(void)` | Adds a non-fatal failure at the current location. |
///   | `ADD_FAILURE_AT(const char* file, int line)` | Adds a non-fatal failure at the specified location. |
///   | `EXPECT_TRUE(bool x)` | Requires that the condition is true. |
///   | `EXPECT_FALSE(bool x)` | Requires that the condition is false. |
///   | `EXPECT_EQ(T a, T b)` | Requires `a == b`. |
///   | `EXPECT_NE(T a, T b)` | Requires `a != b` using `!(a == b)`. |
///   | `EXPECT_LT(T a, T b)` | Requires `a < b`. |
///   | `EXPECT_GT(T a, T b)` | Requires `a > b` using `b < a`. |
///   | `EXPECT_LE(T a, T b)` | Requires `a` ≤ `b` using `!(b < a)`. |
///   | `EXPECT_GE(T a, T b)` | Requires `a` ≥ `b` using `!(a < b)`. |
///   | `EXPECT_NEAR(T expected, T actual, T absolute_range)` | Requires that two values are within a given range. |
///   | `EXPECT_FLOAT_EQ(float a, float b)` | Checks floats for equality within a scaled tolerance. |
///   | `EXPECT_DOUBLE_EQ(double a, double b)` | Checks doubles for equality within a scaled tolerance. |
///   | `EXPECT_STREQ(const char* a, const char* b)` | Requires that two C strings are equal. |
///   | `EXPECT_STRNE(const char* a, const char* b)` | Requires that two C strings differ. |
///   | `ASSERT_TRUE(bool x)` | Requires that the condition is true. |
///   | `ASSERT_FALSE(bool x)` | Requires that the condition is false. |
///   | `ASSERT_EQ(T a, T b)` | Requires `a == b`. |
///   | `ASSERT_NE(T a, T b)` | Requires `a != b` using `!(a == b)`. |
///   | `ASSERT_LT(T a, T b)` | Requires `a < b`. |
///   | `ASSERT_GT(T a, T b)` | Requires `a > b` using `b < a`. |
///   | `ASSERT_LE(T a, T b)` | Requires `a` ≤ `b` using `!(b < a)`. |
///   | `ASSERT_GE(T a, T b)` | Requires `a` ≥ `b` using `!(a < b)`. |
///   | `ASSERT_NEAR(T expected, T actual, T absolute_error)` | Requires that two values are within a given range. |
///   | `ASSERT_FLOAT_EQ(float a, float b)` | Checks floats for equality within a scaled tolerance. |
///   | `ASSERT_DOUBLE_EQ(double a, double b)` | Checks doubles for equality within a scaled tolerance. |
///   | `ASSERT_STREQ(const char* a, const char* b)` | Requires that two C strings are equal. |
///   | `ASSERT_STRNE(const char* a, const char* b)` | Requires that two C strings differ. |
///
/// See: https://google.github.io/googletest/reference/assertions.html

#include "libhatchet.h"

// HX_USE_GOOGLE_TEST - Enable this to use Google Test instead of hxtest.hpp.
#if HX_USE_GOOGLE_TEST
#include <gtest/gtest.h>
#else

#if !(HX_USE_MACROS_WITH_MODULE)

#include "detail/hxtest_detail.hpp"

/// `testing` - A partial Google Test reimplementation. Use
/// `-DHX_TEST_MAX_CASES` to provide enough room for all tests.
namespace testing {

/// `Test` - Base class for tests required by Google Test's `TEST_F`.
class Test {
public:
	/// User overridable for tests using `TEST_F`.
	void SetUp(void) { }

	/// User overridable for tests using `TEST_F`.
	void TearDown(void) { }
};

} // namespace testing
#endif // !HX_USE_MACROS_WITH_MODULE

/// \cond HIDDEN
// `HX_TEST_NAME_` - Macro for concatenating three arguments into one name.
#define HX_TEST_NAME_(x_, y_, z_) x_ ## y_ ## __ ## z_ ## _
/// \endcond

/// `TEST(suite_name, case_name)` - Google Test reimplementation. Defines a test
/// case with a suite name and case name.
/// - `suite_name` : A valid C identifier for the test suite.
/// - `case_name` : A valid C identifier for the test case.
#define TEST(suite_name_, case_name_) \
	static void HX_TEST_NAME_(hxtest_, suite_name_, case_name_)(void); \
	static HX_NS_PREFIX_ hxdetail_::hxtest_case_ HX_TEST_NAME_(hxs_test_, suite_name_, case_name_)( \
		HX_TEST_NAME_(hxtest_, suite_name_, case_name_), #suite_name_, #case_name_, __FILE__, __LINE__); \
	static void HX_TEST_NAME_(hxtest_, suite_name_, case_name_)(void)

/// `TEST_F(suite_name, case_name)` - Google Test reimplementation for
/// fixture-based tests. Defines a test case where the `suite_name` is a
/// subclass of `testing::Test`.
/// - `suite_fixture` : The test suite base class used as a fixture.
/// - `case_name` : A valid C identifier for the test case.
#define TEST_F(suite_fixture_, case_name_) \
	class HX_TEST_NAME_(hxtest_f_, suite_fixture_, case_name_) : public suite_fixture_ { \
	public: \
		static void hxrun_(void) { \
			HX_TEST_NAME_(hxtest_f_, suite_fixture_, case_name_) subclass_; \
			subclass_.SetUp(); \
			subclass_.hxrun_test_f_(); \
			subclass_.TearDown(); \
		} \
	private: \
		void hxrun_test_f_(void); \
	}; \
	static HX_NS_PREFIX_ hxdetail_::hxtest_case_ HX_TEST_NAME_(hxs_test_f_, suite_fixture_, case_name_)( \
		HX_TEST_NAME_(hxtest_f_, suite_fixture_, case_name_)::hxrun_, #suite_fixture_, #case_name_, __FILE__, __LINE__); \
	void HX_TEST_NAME_(hxtest_f_, suite_fixture_, case_name_)::hxrun_test_f_(void)

/// `int RUN_ALL_TESTS(...)` - Executes all registered test cases.
/// - `...` : Optional const char* matching a specific test suite to run. (Non-standard.)
#define RUN_ALL_TESTS(...) HX_NS_PREFIX_ hxdetail_::hxtest_::dispatcher_().run_all_tests_(__VA_ARGS__)

/// `void SUCCEED(void)` - Marks the current test as successful without any
/// checks.
#define SUCCEED() HX_NS_PREFIX_ hxdetail_::hxtest_::dispatcher_().condition_check_(true, __FILE__, __LINE__, "SUCCEED()", false)

/// `void FAIL(void)` - WARNING. Calls `return`. Marks the current test as
/// failed.
#define FAIL() do { HX_NS_PREFIX_ hxdetail_::hxtest_::dispatcher_().condition_check_(false, __FILE__, __LINE__, "FAIL()", false); return; } while (0)
/// `void ADD_FAILURE(void)` - Adds a non-fatal failure at the current location.
#define ADD_FAILURE() HX_NS_PREFIX_ hxdetail_::hxtest_::dispatcher_().condition_check_(false, __FILE__, __LINE__, "ADD_FAILURE()", false)
/// `void ADD_FAILURE_AT(const char*, int)` - Adds a non-fatal failure at the
/// specified file and line.
#define ADD_FAILURE_AT(file_, line_) HX_NS_PREFIX_ hxdetail_::hxtest_::dispatcher_().condition_check_(false, (file_), static_cast<int>(line_), "ADD_FAILURE_AT()", false)

/// `void EXPECT_TRUE(bool)` - Requires that the condition is true.
#define EXPECT_TRUE(x_) HX_NS_PREFIX_ hxdetail_::hxtest_::dispatcher_().condition_check_((x_), __FILE__, __LINE__, #x_, false)
/// `void EXPECT_FALSE(bool)` - Requires that the condition is false.
#define EXPECT_FALSE(x_) HX_NS_PREFIX_ hxdetail_::hxtest_::dispatcher_().condition_check_(!(x_), __FILE__, __LINE__, "!" #x_, false)
/// `void EXPECT_NEAR(T expected, T actual, T absolute_range)` - Requires that
/// two values are within a given range.
/// - `expected` : Reference value to compare against.
/// - `actual` : Value being tested.
/// - `absolute_range` : Maximum permitted absolute difference.
#define EXPECT_NEAR(expected_, actual_, absolute_range_) do { \
	const auto hxexp_ = (expected_); const auto hxact_ = (actual_); \
	HX_NS_PREFIX_ hxdetail_::hxtest_::dispatcher_().condition_check_( \
		((hxexp_ < hxact_) ? (hxact_ - hxexp_) : (hxexp_ - hxact_)) <= (absolute_range_), \
		__FILE__, __LINE__, "abs(" #expected_ "-" #actual_ ") <= " #absolute_range_, false); \
	} while(0)
/// `void EXPECT_LT(T a, T b)` - Requires `a < b`.
#define EXPECT_LT(a_, b_) HX_NS_PREFIX_ hxdetail_::hxtest_::dispatcher_().condition_check_((a_) < (b_), __FILE__, __LINE__, #a_ " < " #b_, false)
/// `void EXPECT_GT(T a, T b)` - Requires `a > b` using `b < a`.
#define EXPECT_GT(a_, b_) HX_NS_PREFIX_ hxdetail_::hxtest_::dispatcher_().condition_check_((b_) < (a_), __FILE__, __LINE__, #a_ " > " #b_, false)
/// `void EXPECT_LE(T a, T b)` - Requires `a` ≤ `b` using `!(b < a)`.
#define EXPECT_LE(a_, b_) HX_NS_PREFIX_ hxdetail_::hxtest_::dispatcher_().condition_check_(!((b_) < (a_)), __FILE__, __LINE__, #a_ " <= " #b_, false)
/// `void EXPECT_GE(T a, T b)` - Requires `a` ≥ `b` using `!(a < b)`.
#define EXPECT_GE(a_, b_) HX_NS_PREFIX_ hxdetail_::hxtest_::dispatcher_().condition_check_(!((a_) < (b_)), __FILE__, __LINE__, #a_ " >= " #b_, false)
/// `void EXPECT_EQ(T a, T b)` - Requires `a == b`.
#define EXPECT_EQ(a_, b_) HX_NS_PREFIX_ hxdetail_::hxtest_::dispatcher_().condition_check_((a_) == (b_), __FILE__, __LINE__, #a_ " == " #b_, false)
/// `void EXPECT_NE(T a, T b)` - Requires `a != b` using `!(a == b)`.
#define EXPECT_NE(a_, b_) HX_NS_PREFIX_ hxdetail_::hxtest_::dispatcher_().condition_check_(!((a_) == (b_)), __FILE__, __LINE__, #a_ " != " #b_, false)
/// `void EXPECT_FLOAT_EQ(float a, float b)` - Requires floats for equality
/// within a scaled tolerance.
#define EXPECT_FLOAT_EQ(a_, b_) HX_NS_PREFIX_ hxdetail_::hxtest_::dispatcher_().condition_check_(HX_NS_PREFIX_ hxdetail_::hxtest_float_eq_((a_), (b_)), __FILE__, __LINE__, #a_ " ~= " #b_, false)
/// `void EXPECT_DOUBLE_EQ(double a, double b)` - Requires doubles for equality
/// within a scaled tolerance.
#define EXPECT_DOUBLE_EQ(a_, b_) HX_NS_PREFIX_ hxdetail_::hxtest_::dispatcher_().condition_check_(HX_NS_PREFIX_ hxdetail_::hxtest_double_eq_((a_), (b_)), __FILE__, __LINE__, #a_ " ~= " #b_, false)
/// `void EXPECT_STREQ(const char* a, const char* b)` - Requires that two C
/// strings are equal.
#define EXPECT_STREQ(a_, b_) HX_NS_PREFIX_ hxdetail_::hxtest_::dispatcher_().condition_check_( \
	HX_NS_PREFIX_ hxdetail_::hxtest_str_eq_((a_), (b_)), __FILE__, __LINE__, #a_ " == " #b_, false)
/// `void EXPECT_STRNE(const char* a, const char* b)` - Requires that two C
/// strings differ.
#define EXPECT_STRNE(a_, b_) HX_NS_PREFIX_ hxdetail_::hxtest_::dispatcher_().condition_check_( \
	HX_NS_PREFIX_ hxdetail_::hxtest_str_ne_((a_), (b_)), __FILE__, __LINE__, #a_ " != " #b_, false)
/// `void ASSERT_TRUE(bool)` - Requires that the condition is true.
#define ASSERT_TRUE(x_) HX_NS_PREFIX_ hxdetail_::hxtest_::dispatcher_().condition_check_((x_), __FILE__, __LINE__, #x_, true)
/// `void ASSERT_FALSE(bool)` - Requires that the condition is false.
#define ASSERT_FALSE(x_) HX_NS_PREFIX_ hxdetail_::hxtest_::dispatcher_().condition_check_(!(x_), __FILE__, __LINE__, "!" #x_, true)
/// `void ASSERT_NEAR(T expected, T actual, T absolute_error)` - Requires that
/// two values are within a given range.
/// - `expected` : Reference value to compare against.
/// - `actual` : Value being tested.
/// - `absolute_error` : Maximum permitted absolute difference.
#define ASSERT_NEAR(expected_, actual_, absolute_error_) do { \
	const auto hxexp_ = (expected_); const auto hxact_ = (actual_); \
	HX_NS_PREFIX_ hxdetail_::hxtest_::dispatcher_().condition_check_( \
		((hxexp_ < hxact_) ? (hxact_ - hxexp_) : (hxexp_ - hxact_)) <= (absolute_error_), \
		__FILE__, __LINE__, "abs(" #expected_ " - " #actual_ ") <= " #absolute_error_, true); \
	} while(0)
/// `void ASSERT_LT(T a, T b)` - Requires `a < b`.
#define ASSERT_LT(a_, b_) HX_NS_PREFIX_ hxdetail_::hxtest_::dispatcher_().condition_check_((a_) < (b_), __FILE__, __LINE__, #a_ " < " #b_, true)
/// `void ASSERT_GT(T a, T b)` - Requires `a > b` using `b < a`.
#define ASSERT_GT(a_, b_) HX_NS_PREFIX_ hxdetail_::hxtest_::dispatcher_().condition_check_((b_) < (a_), __FILE__, __LINE__, #a_ " > " #b_, true)
/// `void ASSERT_LE(T a, T b)` - Requires `a` ≤ `b` using `!(b < a)`.
#define ASSERT_LE(a_, b_) HX_NS_PREFIX_ hxdetail_::hxtest_::dispatcher_().condition_check_(!((b_) < (a_)), __FILE__, __LINE__, #a_ " <= " #b_, true)
/// `void ASSERT_GE(T a, T b)` - Requires `a` ≥ `b` using `!(a < b)`.
#define ASSERT_GE(a_, b_) HX_NS_PREFIX_ hxdetail_::hxtest_::dispatcher_().condition_check_(!((a_) < (b_)), __FILE__, __LINE__, #a_ " >= " #b_, true)
/// `void ASSERT_EQ(T a, T b)` - Requires `a == b`.
#define ASSERT_EQ(a_, b_) HX_NS_PREFIX_ hxdetail_::hxtest_::dispatcher_().condition_check_((a_) == (b_), __FILE__, __LINE__, #a_ " == " #b_, true)
/// `void ASSERT_NE(T a, T b)` - Requires `a != b` using `!(a == b)`.
#define ASSERT_NE(a_, b_) HX_NS_PREFIX_ hxdetail_::hxtest_::dispatcher_().condition_check_(!((a_) == (b_)), __FILE__, __LINE__, #a_ " != " #b_, true)
/// `void ASSERT_FLOAT_EQ(float a, float b)` - Requires floats for equality
/// within a scaled tolerance.
#define ASSERT_FLOAT_EQ(a_, b_) HX_NS_PREFIX_ hxdetail_::hxtest_::dispatcher_().condition_check_(HX_NS_PREFIX_ hxdetail_::hxtest_float_eq_((a_), (b_)), __FILE__, __LINE__, #a_ " ~= " #b_, true)
/// `void ASSERT_DOUBLE_EQ(double a, double b)` - Requires doubles for equality
/// within a scaled tolerance.
#define ASSERT_DOUBLE_EQ(a_, b_) HX_NS_PREFIX_ hxdetail_::hxtest_::dispatcher_().condition_check_(HX_NS_PREFIX_ hxdetail_::hxtest_double_eq_((a_), (b_)), __FILE__, __LINE__, #a_ " ~= " #b_, true)
/// `void ASSERT_STREQ(const char* a, const char* b)` - Requires that two C
/// strings are equal.
#define ASSERT_STREQ(a_, b_) HX_NS_PREFIX_ hxdetail_::hxtest_::dispatcher_().condition_check_( \
	HX_NS_PREFIX_ hxdetail_::hxtest_str_eq_((a_), (b_)), __FILE__, __LINE__, #a_ " == " #b_, true)
/// `void ASSERT_STRNE(const char* a, const char* b)` - Requires that two C
/// strings differ.
#define ASSERT_STRNE(a_, b_) HX_NS_PREFIX_ hxdetail_::hxtest_::dispatcher_().condition_check_( \
	HX_NS_PREFIX_ hxdetail_::hxtest_str_ne_((a_), (b_)), __FILE__, __LINE__, #a_ " != " #b_, true)

#endif // !HX_USE_GOOGLE_TEST
