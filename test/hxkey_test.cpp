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
