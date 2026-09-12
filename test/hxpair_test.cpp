// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

#include <hx/hxpair.hpp>
#include <hx/hxtest.hpp>

HX_NS_USE

TEST(hxpair_test, three_way_matches_ordering) {
	const hxpair<int32_t, int32_t> a = { 31, 35 };
	const hxpair<int32_t, int32_t> b = { 33, 34 };
	const hxpair<int32_t, int32_t> c = { 31, 36 };
	EXPECT_TRUE((a - b) < 0);
	EXPECT_TRUE((b - a) > 0);
	EXPECT_TRUE((a - c) < 0);
	EXPECT_TRUE((c - a) > 0);
	EXPECT_TRUE((a - a) == 0);
	EXPECT_TRUE(hxthree_way(a, b) < 0);
	EXPECT_TRUE(hxthree_way(b, a) > 0);
	EXPECT_TRUE(hxthree_way(a, a) == 0);
}

#if HX_CPLUSPLUS >= 202002L
TEST(hxpair_test, spaceship_matches_operator_minus) {
	const hxpair<int32_t, int32_t> values[] = {
		{ 31, 31 }, { 31, 32 }, { 32, 31 }, { 32, -31 }
	};
	for(hxsize_t i_ = 0; i_ < hxsize(values); ++i_) {
		for(hxsize_t j_ = 0; j_ < hxsize(values); ++j_) {
			const hxpair<int32_t, int32_t>& a = values[i_];
			const hxpair<int32_t, int32_t>& b = values[j_];
			EXPECT_EQ((a <=> b) < 0, (a - b) < 0);
			EXPECT_EQ((a <=> b) > 0, (a - b) > 0);
			EXPECT_EQ((a <=> b) == 0, (a - b) == 0);
			EXPECT_EQ((a <=> b) < 0, a < b);
			EXPECT_EQ((a <=> b) == 0, a == b);
		}
	}
}
#endif // HX_CPLUSPLUS >= 202002L

TEST(hxpair_test, mixed_field_types_order_and_hash) {
	const hxpair<int32_t, const char*> a = { 31, "alpha" };
	const hxpair<int32_t, const char*> b = { 31, "beta" };
	EXPECT_TRUE(hxkey_less(a, b));
	EXPECT_FALSE(hxkey_less(b, a));
	EXPECT_FALSE(hxkey_equal(a, b));
	EXPECT_TRUE(hxkey_equal(a, a));

	char alpha_storage[] = "alpha"; // NOLINT(misc-const-correctness)
	const hxpair<int32_t, const char*> distinct = { 31, alpha_storage };
	EXPECT_NE(a.b, distinct.b);
	EXPECT_TRUE(hxkey_equal(a, distinct));
	EXPECT_EQ(hxkey_hash(a), hxkey_hash(distinct));
}

TEST(hxpair_test, hash_distinguishes_fields) {
	const hxpair<int32_t, int32_t> a = { 31, 32 };
	const hxpair<int32_t, int32_t> b = { 32, 31 };
	const hxpair<int32_t, int32_t> c = { 31, 33 };
	const hxpair<int32_t, int32_t> d = { 33, 32 };
	EXPECT_EQ(hxkey_hash(a), hxkey_hash(a));
	EXPECT_NE(hxkey_hash(a), hxkey_hash(b));
	EXPECT_NE(hxkey_hash(a), hxkey_hash(c));
	EXPECT_NE(hxkey_hash(a), hxkey_hash(d));
}
