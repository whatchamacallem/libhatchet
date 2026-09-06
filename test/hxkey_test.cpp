// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

#include <hx/hxkey.hpp>
#include <hx/hxtest.hpp>

HX_NS_USE

static_assert(hxis_same<
	decltype(hxkey_equal_t<const volatile char*>{}(
		hxdeclval<const volatile char* const&>(),
		hxdeclval<const volatile char* const&>())), bool>(),
	"hxkey_equal_t must preserve const volatile pointer types");
static_assert(hxis_same<
	decltype(hxkey_equal_t<volatile char&&>{}(
		hxdeclval<const char&>(),
		hxdeclval<const char&>())), bool>(),
	"hxkey_equal_t must handle non-const volatile references.");
static_assert(hxis_same<
	decltype(hxkey_less_t<const volatile char*>{}(
		hxdeclval<const volatile char* const&>(),
		hxdeclval<const volatile char* const&>())), bool>(),
	"hxkey_equal_t must preserve const volatile pointer types");
static_assert(hxis_same<
	decltype(hxkey_less_t<volatile char&&>{}(
		hxdeclval<const char&>(),
		hxdeclval<const char&>())), bool>(),
	"hxkey_less_t must handle non-const volatile references.");

TEST(hxkey_function, hxis_string_detects_char_pointers) {
	EXPECT_TRUE(hxis_string<char*>());
	EXPECT_TRUE(hxis_string<const char*>());
	EXPECT_FALSE(hxis_string<int>());
	EXPECT_FALSE(hxis_string<int*>());
	EXPECT_TRUE(hxis_string<char* const volatile>());
	EXPECT_TRUE(hxis_string<const char* const>());
	EXPECT_TRUE(hxis_string<const char* &>());
	EXPECT_TRUE(hxis_string<const char* &&>());
	EXPECT_TRUE(hxis_string<volatile char*>());
	EXPECT_TRUE(hxis_string<const volatile char* &>());
	EXPECT_FALSE(hxis_string<char>());
	EXPECT_FALSE(hxis_string<char&>());
	EXPECT_TRUE(hxis_string<char[6]>());
	EXPECT_TRUE(hxis_string<volatile char[]>());
	EXPECT_TRUE(hxis_string<const volatile char[6]>());
	EXPECT_TRUE(hxis_string<char(&)[6]>());

	EXPECT_FALSE(hxis_string<char**>());
	EXPECT_FALSE(hxis_string<const char**>());
	EXPECT_FALSE(hxis_string<signed char*>());
	EXPECT_FALSE(hxis_string<unsigned char*>());
	EXPECT_FALSE(hxis_string<wchar_t*>());
	EXPECT_FALSE(hxis_string<void*>());
	EXPECT_FALSE(hxis_string<const void*>());
}

TEST(hxkey_function, equal_char_and_const_char_overloads) {
	char mutable_equal_storage[] = "match"; // NOLINT(misc-const-correctness)
	char mutable_differ_storage[] = "matci"; // NOLINT(misc-const-correctness)
	char* mutable_equal = mutable_equal_storage;
	char* mutable_differ = mutable_differ_storage;
	const char* immutable = "match";
	EXPECT_TRUE(hxkey_equal(immutable, mutable_equal));
	EXPECT_FALSE(hxkey_equal(mutable_differ, immutable));
	EXPECT_TRUE(hxkey_equal(mutable_equal, immutable));
	EXPECT_FALSE(hxkey_equal(mutable_differ, mutable_equal));
}

TEST(hxkey_function, less_char_and_const_char_overloads) {
	char mutable_a_storage[] = "a"; // NOLINT(misc-const-correctness)
	char mutable_b_storage[] = "b"; // NOLINT(misc-const-correctness)
	char* mutable_a = mutable_a_storage;
	char* mutable_b = mutable_b_storage;
	const char* immutable_a = "a";
	const char* immutable_b = "b";
	EXPECT_TRUE(hxkey_less(immutable_a, mutable_b));
	EXPECT_FALSE(hxkey_less(immutable_b, mutable_a));
	EXPECT_TRUE(hxkey_less(mutable_a, immutable_b));
	EXPECT_FALSE(hxkey_less(mutable_b, mutable_a));
}

TEST(hxkey_hash, char_pointer_matches_const_char_pointer) {
	char mutable_hello[] = "hello";
	EXPECT_EQ(hxkey_hash_t<char*>{}(mutable_hello), hxkey_hash_t<const char*>{}("hello"));
}

TEST(hxkey_hash, compare_avalanches) {
	char original[] = "avalanche-test-string-31";
	const hxhash_t h_original = hxkey_hash(static_cast<const char*>(original));
	for(hxsize_t i_ = 0; original[i_] != '\0'; ++i_) {
		char altered[sizeof(original)];
		::memcpy(altered, original, sizeof(original));
		altered[i_] = static_cast<char>(static_cast<unsigned char>(altered[i_]) ^ (1u << (i_ % 4)));
		const hxhash_t h_altered = hxkey_hash(static_cast<const char*>(altered));
		unsigned int differing_bits = 0u;
		for(hxhash_t x_ = h_original ^ h_altered; x_ != 0u; x_ >>= 1u) {
			differing_bits += x_ & 1u;
		}
		EXPECT_GE(differing_bits, 8u);
		EXPECT_LE(differing_bits, 24u);
	}
}
