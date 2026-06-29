// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

#include <hx/hxkey.hpp>
#include <hx/hxtest.hpp>

HX_NS_USE

static_assert(hxis_same<
	decltype(hxkey_equal_t<const volatile char*>{}(
		hxdeclval<const volatile char* const&>(),
		hxdeclval<const volatile char* const&>())),
	bool>::value,
	"hxkey_equal_t must preserve const volatile pointer types");
static_assert(hxis_same<
	decltype(hxkey_equal_t<volatile char&&>{}(
		hxdeclval<const char&>(),
		hxdeclval<const char&>())),
	bool>::value,
	"hxkey_equal_t must handle non-const volatile references.");
static_assert(hxis_same<
	decltype(hxkey_less_t<const volatile char*>{}(
		hxdeclval<const volatile char* const&>(),
		hxdeclval<const volatile char* const&>())),
	bool>::value,
	"hxkey_equal_t must preserve const volatile pointer types");
static_assert(hxis_same<
	decltype(hxkey_less_t<volatile char&&>{}(
		hxdeclval<const char&>(),
		hxdeclval<const char&>())),
	bool>::value,
	"hxkey_less_t must handle non-const volatile references.");

TEST(hxkey_function, char_pointer_dispatch) {
	const hxkey_equal_t<char*> equal_fn;
	const hxkey_less_t<char*> less_fn;
	char alpha[] = "alpha";
	char beta[] = "beta";
	char alpha_duplicate[] = "alpha";
	EXPECT_TRUE(equal_fn(alpha, alpha_duplicate));
	EXPECT_FALSE(equal_fn(alpha, beta));
	EXPECT_TRUE(less_fn(alpha, beta));
	EXPECT_FALSE(less_fn(beta, alpha));
}

TEST(hxkey_function, equal_identical_content_different_address) {
	char a[] = "abc";
	char b[] = "abc";
	EXPECT_TRUE(hxkey_equal(static_cast<const char*>(a), static_cast<const char*>(b)));
}

TEST(hxkey_function, equal_const_char_and_char_overload) {
	char mutable_equal[] = "match";
	char mutable_differ[] = "matci";
	const char* immutable = "match";
	EXPECT_TRUE(hxkey_equal(immutable, mutable_equal));
	EXPECT_FALSE(hxkey_equal(immutable, mutable_differ));
}

TEST(hxkey_function, equal_char_and_const_char_overload) {
	char mutable_equal[] = "match";
	char mutable_differ[] = "matci";
	const char* immutable = "match";
	EXPECT_TRUE(hxkey_equal(mutable_equal, immutable));
	EXPECT_FALSE(hxkey_equal(mutable_differ, immutable));
}

TEST(hxkey_function, equal_strings_differing_in_last_char) {
	const char* a = "ab0";
	const char* b = "ab1";
	EXPECT_FALSE(hxkey_equal(a, b));
}

TEST(hxkey_function, equal_empty_string_is_equal_to_itself) {
	const char* a = "";
	const char* b = "";
	EXPECT_TRUE(hxkey_equal(a, b));
}

TEST(hxkey_function, equal_empty_vs_nonempty) {
	const char* a = "";
	const char* b = "x";
	EXPECT_FALSE(hxkey_equal(a, b));
	EXPECT_FALSE(hxkey_equal(b, a));
}

TEST(hxkey_function, less_equal_strings_is_not_less) {
	char a[] = "same";
	char b[] = "same";
	EXPECT_FALSE(hxkey_less(static_cast<const char*>(a), static_cast<const char*>(b)));
}

TEST(hxkey_function, less_const_char_and_char_overload) {
	char mutable_a[] = "a";
	char mutable_b[] = "b";
	const char* immutable_a = "a";
	const char* immutable_b = "b";
	EXPECT_TRUE(hxkey_less(immutable_a, mutable_b));
	EXPECT_FALSE(hxkey_less(immutable_b, mutable_a));
	EXPECT_FALSE(hxkey_less(immutable_a, mutable_a));
}

TEST(hxkey_function, less_char_and_const_char_overload) {
	char mutable_a[] = "a";
	char mutable_b[] = "b";
	const char* immutable_a = "a";
	const char* immutable_b = "b";
	EXPECT_TRUE(hxkey_less(mutable_a, immutable_b));
	EXPECT_FALSE(hxkey_less(mutable_b, immutable_a));
	EXPECT_FALSE(hxkey_less(mutable_a, immutable_a));
}

TEST(hxkey_function, less_adjacent_chars_ordered_correctly) {
	const char* a = "a";
	const char* b = "b";
	EXPECT_TRUE(hxkey_less(a, b));
	EXPECT_FALSE(hxkey_less(b, a));
}

TEST(hxkey_function, less_differs_only_in_last_char) {
	const char* a = "ab0";
	const char* b = "ab1";
	EXPECT_TRUE(hxkey_less(a, b));
	EXPECT_FALSE(hxkey_less(b, a));
}

TEST(hxkey_hash, empty_string_differs_from_single_char) {
	const hxhash_t h_empty = hxkey_hash("");
	const hxhash_t h_a     = hxkey_hash("a");
	EXPECT_NE(h_empty, h_a);
}

TEST(hxkey_hash, last_char_affects_hash) {
	const hxhash_t h_ab0 = hxkey_hash("ab0");
	const hxhash_t h_ab1 = hxkey_hash("ab1");
	EXPECT_NE(h_ab0, h_ab1);
}

TEST(hxkey_hash, first_char_affects_hash) {
	const hxhash_t h_abc = hxkey_hash("abc");
	const hxhash_t h_xbc = hxkey_hash("xbc");
	EXPECT_NE(h_abc, h_xbc);
}

TEST(hxkey_hash, order_sensitive) {
	const hxhash_t h_ab = hxkey_hash("ab");
	const hxhash_t h_ba = hxkey_hash("ba");
	EXPECT_NE(h_ab, h_ba);
}

TEST(hxkey_hash, same_string_same_hash) {
	EXPECT_EQ(hxkey_hash("hello"), hxkey_hash("hello"));
}
