// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

#include <hx/hxtest.hpp>
#include "./hxctest.h"
#include "./hxtest_util.hpp"

HX_NS_USE

namespace {

template<typename T>
void hxtest_relational(T a, T b) {
	ASSERT_EQ(a, a);
	ASSERT_GE(a, a);
	ASSERT_GE(b, a);
	ASSERT_GT(b, a);
	ASSERT_LE(a, a);
	ASSERT_LE(a, b);
	ASSERT_LT(a, b);
	ASSERT_NE(a, b);
	EXPECT_EQ(a, a);
	EXPECT_GE(a, a);
	EXPECT_GE(b, a);
	EXPECT_GT(b, a);
	EXPECT_LE(a, a);
	EXPECT_LE(a, b);
	EXPECT_LT(a, b);
	EXPECT_NE(a, b);
}
} // namespace

TEST(hxtest_test, relational) {
	hxtest_relational<int>(-1, 0);
	hxtest_relational<long>(-5, -4);
	hxtest_relational<long long>(-9, -8);
	hxtest_relational<unsigned int>(1u, 2u);
	hxtest_relational<unsigned long>(5ul, 6ul);
	hxtest_relational<unsigned long long>(11ull, 12ull);
	hxtest_relational<int8_t>(static_cast<int8_t>(-12), static_cast<int8_t>(-11));
	hxtest_relational<uint8_t>(static_cast<uint8_t>(3), static_cast<uint8_t>(4));
	hxtest_relational<int16_t>(static_cast<int16_t>(-301), static_cast<int16_t>(-300));
	hxtest_relational<uint16_t>(static_cast<uint16_t>(7), static_cast<uint16_t>(8));
	hxtest_relational<int32_t>(static_cast<int32_t>(-70001), static_cast<int32_t>(-70000));
	hxtest_relational<uint32_t>(static_cast<uint32_t>(100), static_cast<uint32_t>(101));
	hxtest_relational<float>(-0.00002f, -0.00001f);
	hxtest_relational<double>(0.0, 1.0);
	hxtest_relational<unsigned int>(0u, 1u);
	hxtest_relational<uint8_t>(static_cast<uint8_t>(0), static_cast<uint8_t>(1));
	ASSERT_STREQ("a", "a");
	ASSERT_STRNE("a", "b");
}

TEST(hxtest_test, eq_and_ne_at_adjacency) {
	EXPECT_EQ(0, 0);
	EXPECT_NE(0, 1);
	EXPECT_NE(1, 0);
	EXPECT_EQ(0u, 0u);
	EXPECT_NE(0u, 1u);
	EXPECT_NE(-1, 0);
	EXPECT_NE(0, -1);
	EXPECT_LT(0, 1);
	EXPECT_GT(1, 0);
	EXPECT_LE(0, 0);
	EXPECT_LE(0, 1);
	EXPECT_GE(1, 1);
	EXPECT_GE(1, 0);
}

TEST(hxtest_test, float_eq) {
	const float third = 1.0f / 3.0f;
	ASSERT_FLOAT_EQ(third + third + third, 1.0f);
	const float a = 0.1f;
	const float b = 0.2f;
	const float c = 0.3f;
	ASSERT_FLOAT_EQ(a + b, c);
	ASSERT_FLOAT_EQ(c - b, a);
	ASSERT_FLOAT_EQ((a + b) - a, b);
	const float tenth = 1.0f / 10.0f;
	ASSERT_FLOAT_EQ(tenth * 10.0f, 1.0f);
	ASSERT_FLOAT_EQ(a * a, 0.01f);

	const float negative = -1.0f / 3.0f;
	ASSERT_FLOAT_EQ(-(third + third + third), negative + negative + negative);
	ASSERT_FLOAT_EQ(-a - b, -c);

	EXPECT_FLOAT_EQ(-1.0f, -1.0f);
	EXPECT_FLOAT_EQ(1.0f, 1.0f);

	const uint32_t one_bits = 0x3f800000u;
	float lo = 0.0f; ::memcpy(&lo, &one_bits, sizeof lo);
	const uint32_t near_bits = one_bits + 4u;
	float near = 0.0f; ::memcpy(&near, &near_bits, sizeof near);
	EXPECT_FLOAT_EQ(lo, near);
	EXPECT_FLOAT_EQ(near, lo);

	const uint32_t neg_lo_bits = 0x80000000u | one_bits;
	float neg_lo = 0.0f; ::memcpy(&neg_lo, &neg_lo_bits, sizeof neg_lo);
	const uint32_t neg_near_bits = 0x80000000u | near_bits;
	float neg_near = 0.0f; ::memcpy(&neg_near, &neg_near_bits, sizeof neg_near);
	EXPECT_FLOAT_EQ(neg_lo, neg_near);
	EXPECT_FLOAT_EQ(neg_near, neg_lo);
}

TEST(hxtest_test, double_eq) {
	const double third = 1.0 / 3.0;
	ASSERT_DOUBLE_EQ(third + third + third, 1.0);
	const double a = 0.1;
	const double b = 0.2;
	const double c = 0.3;
	ASSERT_DOUBLE_EQ(a + b, c);
	ASSERT_DOUBLE_EQ(c - b, a);
	ASSERT_DOUBLE_EQ((a + b) - a, b);
	const double tenth = 1.0 / 10.0;
	ASSERT_DOUBLE_EQ(tenth * 10.0, 1.0);
	ASSERT_DOUBLE_EQ(a * a, 0.01);

	const double negative = -1.0 / 3.0;
	ASSERT_DOUBLE_EQ(-(third + third + third), negative + negative + negative);
	ASSERT_DOUBLE_EQ(-a - b, -c);

	EXPECT_DOUBLE_EQ(-1.0, -1.0);
	EXPECT_DOUBLE_EQ(1.0, 1.0);

	const uint64_t one_bits = 0x3ff0000000000000ull;
	double lo = 0.0; ::memcpy(&lo, &one_bits, sizeof lo);
	const uint64_t near_bits = one_bits + 4ull;
	double near = 0.0; ::memcpy(&near, &near_bits, sizeof near);
	EXPECT_DOUBLE_EQ(lo, near);
	EXPECT_DOUBLE_EQ(near, lo);

	const uint64_t neg_lo_bits = 0x8000000000000000ull | one_bits;
	double neg_lo = 0.0; ::memcpy(&neg_lo, &neg_lo_bits, sizeof neg_lo);
	const uint64_t neg_near_bits = 0x8000000000000000ull | near_bits;
	double neg_near = 0.0; ::memcpy(&neg_near, &neg_near_bits, sizeof neg_near);
	EXPECT_DOUBLE_EQ(neg_lo, neg_near);
	EXPECT_DOUBLE_EQ(neg_near, neg_lo);
}

TEST(hxtest_test, all_tests) {
	hxlog_warning("EXPECTING_TEST_WARNINGS");
	EXPECT_TRUE(hxctest_all());
}

TEST(hxtest_test, succeed) {
	SUCCEED();
}

#if HX_TEST_ERROR_HANDLING
TEST(hxtest_error_handling, fail) {
	hxlog_warning("EXPECTING_TEST_FAILURE");
	SUCCEED();
	FAIL();
	hxassert_always(0, "sys_err FAIL() kept going");
}

TEST(hxtest_error_handling, add_failure) {
	hxlog_warning("EXPECTING_TEST_FAILURE");
	SUCCEED();
	for(int i = 10; i--;) {
		ADD_FAILURE();
	}
	SUCCEED();
}

TEST(hxtest_error_handling, add_failure_at) {
	hxlog_warning("EXPECTING_TEST_FAILURE");
	SUCCEED();
	ADD_FAILURE_AT("fake_file.cpp", 10000);
}

TEST(hxtest_error_handling, nothing_asserted) {
	hxlog_warning("EXPECTING_TEST_FAILURE");
}

TEST(hxtest_error_handling, float_eq_inf_left) {
	hxlog_warning("EXPECTING_TEST_FAILURE");
	const uint32_t inf_bits = 0x7f800000u;
	float inf = 0.0f; ::memcpy(&inf, &inf_bits, sizeof inf);
	EXPECT_FLOAT_EQ(inf, 1.0f);
}

TEST(hxtest_error_handling, float_eq_inf_right) {
	hxlog_warning("EXPECTING_TEST_FAILURE");
	const uint32_t inf_bits = 0x7f800000u;
	float inf = 0.0f; ::memcpy(&inf, &inf_bits, sizeof inf);
	EXPECT_FLOAT_EQ(1.0f, inf);
}

TEST(hxtest_error_handling, double_eq_inf_left) {
	hxlog_warning("EXPECTING_TEST_FAILURE");
	const uint64_t inf_bits = 0x7ff0000000000000ull;
	double inf = 0.0; ::memcpy(&inf, &inf_bits, sizeof inf);
	EXPECT_DOUBLE_EQ(inf, 1.0);
}

TEST(hxtest_error_handling, double_eq_inf_right) {
	hxlog_warning("EXPECTING_TEST_FAILURE");
	const uint64_t inf_bits = 0x7ff0000000000000ull;
	double inf = 0.0; ::memcpy(&inf, &inf_bits, sizeof inf);
	EXPECT_DOUBLE_EQ(1.0, inf);
}

#endif
