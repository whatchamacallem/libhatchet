// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

#include <hx/hxutility.h>
#include <hx/hxtest.hpp>
#include <ctype.h>

HX_NS_USE

namespace {

enum class hxutility_test_forward {
	none,
	lvalue,
	const_lvalue,
	rvalue,
	const_rvalue
};
class hxutility_test_forward_t {
public:
	int value;
};

hxutility_test_forward_t hxutility_test_forward_make_forwarded() { return { 11 }; }
hxutility_test_forward_t hxutility_test_forward_make_const_forwarded() { return { 13 }; }
hxutility_test_forward hxutility_test_forward_detect(hxutility_test_forward_t&) {
	return hxutility_test_forward::lvalue;
}
hxutility_test_forward hxutility_test_forward_detect(const hxutility_test_forward_t&) {
	return hxutility_test_forward::const_lvalue;
}
hxutility_test_forward hxutility_test_forward_detect(hxutility_test_forward_t&&) {
	return hxutility_test_forward::rvalue;
}
hxutility_test_forward hxutility_test_forward_detect(const hxutility_test_forward_t&&) {
	return hxutility_test_forward::const_rvalue;
}
template<typename T>
hxutility_test_forward hxutility_test_forward_through_template(T&& value) {
	return hxutility_test_forward_detect(hxforward<T>(value));
}

class hxutility_test_binds_base_t {
public:
	int value;
};
class hxutility_test_binds_derived_t : public hxutility_test_binds_base_t { };
class hxutility_test_binds_ref_conversion_t {
public:
	// GCOVR_EXCL_START. Only used at compile time.
	operator long&() { return value; }
	// GCOVR_EXCL_STOP
	long value;
};
class hxutility_test_binds_value_conversion_t {
public:
	// GCOVR_EXCL_START. Only used at compile time.
	operator long() const { return value; }
	// GCOVR_EXCL_STOP
	long value;
};
} // namespace

static_assert(hxtrue_t::value, "hxtrue_t must report true");
static_assert(!hxfalse_t::value, "hxfalse_t must report false");
static_assert(hxis_same<hxenable_if_t<true, int>, int>(),
	"hxenable_if_t<true> must expose the requested type");
static_assert(hxis_same<hxremove_reference_t<int>, int>(),
	"hxremove_reference leaves non-references untouched");
static_assert(hxis_same<hxremove_reference_t<int&>, int>(),
	"hxremove_reference_t strips lvalue references");
static_assert(hxis_same<hxremove_reference_t<int&&>, int>(),
	"hxremove_reference_t strips rvalue references");
static_assert(hxis_same<hxremove_pointer_t<int*>, int>(),
	"hxremove_pointer should strip pointers");
static_assert(hxis_same<hxremove_pointer_t<int* const>, int>(),
	"hxremove_pointer should ignore const pointers");
static_assert(hxis_same<hxremove_pointer_t<const int*>, const int>(),
	"hxremove_pointer should leave pointed-to qualifiers");
static_assert(hxis_same<hxremove_pointer_t<int* volatile>, int>(),
	"hxremove_pointer_t should ignore volatile pointers");
static_assert(hxis_same<hxremove_pointer_t<int>, int>(),
	"hxremove_pointer_t should leave non-pointers untouched");
static_assert(hxis_lvalue_reference<int&>(),
	"hxis_lvalue_reference should detect lvalues");
static_assert(!hxis_lvalue_reference<int>(),
	"hxis_lvalue_reference should reject non-references");
static_assert(hxis_same<decltype(hxforward_like<int&>(hxdeclval<int&>())), int&>(),
	"hxforward_like should produce mutable lvalues");
static_assert(hxis_same<decltype(hxforward_like<const int&>(hxdeclval<int&>())), const int&>(),
	"hxforward_like should produce const lvalues");
static_assert(hxis_same<decltype(hxforward_like<int>(hxdeclval<int&>())), int&&>(),
	"hxforward_like should produce mutable rvalues");
static_assert(hxis_same<decltype(hxforward_like<const int>(hxdeclval<int&>())), const int&&>(),
	"hxforward_like should produce const rvalues");
static_assert(hxis_rvalue_reference<int&&>(),
	"hxis_rvalue_reference should detect rvalues");
static_assert(!hxis_rvalue_reference<int&>(),
	"hxis_rvalue_reference should reject lvalues");
static_assert(hxis_same<hxremove_cv_t<int>, int>(),
	"hxremove_cv_t leaves plain types untouched");
static_assert(hxis_same<hxremove_cv_t<const volatile int>, int>(),
	"hxremove_cv strips const volatile");
static_assert(hxis_same<hxremove_cv_t<const int>, int>(),
	"hxremove_cv_t strips const");
static_assert(hxis_same<hxremove_cv_t<volatile int>, int>(),
	"hxremove_cv_t strips volatile");
static_assert(!hxis_const<int>(), "hxis_const should reject mutable");
static_assert(hxis_const<const int>(),
	"hxis_const should detect const");
static_assert(hxis_void<void>(), "hxis_void should detect void");
static_assert(hxis_void<const void>(),
	"hxis_void should ignore qualifiers");
static_assert(!hxis_void<int>(), "hxis_void should reject others");
static_assert(hxis_integral<int>(),
	"hxis_integral should detect int");
static_assert(hxis_integral<const unsigned long>(),
	"hxis_integral should ignore qualifiers");
static_assert(!hxis_integral<float>(),
	"hxis_integral should reject floats");
static_assert(hxis_floating_point<float>(),
	"hxis_floating_point should detect floats");
static_assert(hxis_floating_point<const long double>(),
	"hxis_floating_point should ignore qualifiers");
static_assert(!hxis_floating_point<int>(),
	"hxis_floating_point should reject ints");
static_assert(!hxis_array<int>(),
	"hxis_array should reject non-arrays");
static_assert(hxis_array<int[4]>(),
	"hxis_array should detect sized arrays");
static_assert(hxis_array<const int[]>(),
	"hxis_array should detect unsized arrays");
static_assert(hxis_pointer<int*>(),
	"hxis_pointer should detect pointers");
static_assert(hxis_pointer<const int*>(),
	"hxis_pointer should ignore qualifiers");
static_assert(!hxis_pointer<int>(),
	"hxis_pointer should reject non-pointers");
static_assert(hxis_same<hxrestrict_t<int>, int>(),
	"hxrestrict_t should leave non-pointers untouched");
static_assert(sizeof(hxrestrict_t<int*>) == sizeof(int*),
	"hxrestrict_t should preserve pointer representation");
static_assert(hxbinds_directly<int, int>(),
	"hxbinds_directly should accept identical types");
static_assert(hxbinds_directly<const int, int>(),
	"hxbinds_directly should accept adding const");
static_assert(!hxbinds_directly<int, const int>(),
	"hxbinds_directly should reject removing const");
static_assert(!hxbinds_directly<const short, int>(),
	"hxbinds_directly should reject a type requiring conversion, even as a const reference");
static_assert(!hxbinds_directly<short, int>(),
	"hxbinds_directly should reject a type requiring conversion");
static_assert(hxbinds_directly<hxutility_test_binds_base_t, hxutility_test_binds_derived_t>(),
	"hxbinds_directly should accept derived to base");
static_assert(!hxbinds_directly<hxutility_test_binds_derived_t, hxutility_test_binds_base_t>(),
	"hxbinds_directly should reject base to derived");
static_assert(hxbinds_directly<long, hxutility_test_binds_ref_conversion_t>(),
	"hxbinds_directly should accept conversion operators returning references");
static_assert(!hxbinds_directly<long, const hxutility_test_binds_ref_conversion_t>(),
	"hxbinds_directly should reject conversion operators requiring non-const");
static_assert(!hxbinds_directly<const long, hxutility_test_binds_value_conversion_t>(),
	"hxbinds_directly should reject conversion operators returning values");

TEST(hxutility_test, type_trait_functions_evaluate_at_runtime) {
	EXPECT_TRUE((hxbinds_directly<int, int>()));
	EXPECT_FALSE((hxbinds_directly<int, const int>()));
	EXPECT_TRUE(hxis_array<int[4]>());
	EXPECT_FALSE(hxis_array<int>());
	EXPECT_TRUE(hxis_const<const int>());
	EXPECT_FALSE(hxis_const<int>());
	EXPECT_TRUE(hxis_floating_point<float>());
	EXPECT_FALSE(hxis_floating_point<int>());
	EXPECT_TRUE(hxis_integral<int>());
	EXPECT_FALSE(hxis_integral<float>());
	EXPECT_TRUE(hxis_lvalue_reference<int&>());
	EXPECT_FALSE(hxis_lvalue_reference<int>());
	EXPECT_TRUE(hxis_pointer<int*>());
	EXPECT_FALSE(hxis_pointer<int>());
	EXPECT_TRUE(hxis_reference<int&>());
	EXPECT_FALSE(hxis_reference<int>());
	EXPECT_TRUE(hxis_rvalue_reference<int&&>());
	EXPECT_FALSE(hxis_rvalue_reference<int&>());
	EXPECT_TRUE((hxis_same<int, int>()));
	EXPECT_FALSE((hxis_same<int, float>()));
	EXPECT_TRUE(hxis_void<void>());
	EXPECT_FALSE(hxis_void<int>());
}

TEST(hxutility_test, hxabs_double) {
	const double negative = -34.75;
	const double positive = 34.75;
	EXPECT_DOUBLE_EQ(hxabs(negative), positive);
	EXPECT_DOUBLE_EQ(hxabs(positive), positive);
	EXPECT_DOUBLE_EQ(hxabs(-0.0), -0.0);
}

TEST(hxutility_test, hxforward) {
	EXPECT_EQ(hxutility_test_forward::rvalue,
		hxutility_test_forward_detect(hxforward<hxutility_test_forward_t>(
			hxutility_test_forward_make_forwarded())));
	EXPECT_EQ(hxutility_test_forward::const_rvalue,
		hxutility_test_forward_detect(hxforward<const hxutility_test_forward_t>(
			hxutility_test_forward_make_const_forwarded())));
	hxutility_test_forward_t lvalue = { 7 };
	EXPECT_EQ(hxutility_test_forward::lvalue, hxutility_test_forward_through_template(lvalue));
	const hxutility_test_forward_t const_lvalue = { 9 };
	EXPECT_EQ(hxutility_test_forward::const_lvalue,
		hxutility_test_forward_through_template(const_lvalue));
	EXPECT_EQ(hxutility_test_forward::rvalue,
		hxutility_test_forward_through_template(hxutility_test_forward_make_forwarded()));
	hxutility_test_forward_t movable_value = { 17 };
	EXPECT_EQ(hxutility_test_forward::rvalue,
		hxutility_test_forward_through_template(hxmove(movable_value)));
	const hxutility_test_forward_t const_movable_value = { 19 };
	EXPECT_EQ(hxutility_test_forward::const_rvalue,
		hxutility_test_forward_through_template(hxmove(const_movable_value)));
}

TEST(hxutility_test, hxforward_like) {
	hxutility_test_forward_t value = { 23 };
	EXPECT_EQ(hxutility_test_forward::lvalue,
		hxutility_test_forward_detect(hxforward_like<int&>(value)));
	EXPECT_EQ(hxutility_test_forward::const_lvalue,
		hxutility_test_forward_detect(hxforward_like<const int&>(value)));
	EXPECT_EQ(hxutility_test_forward::rvalue,
		hxutility_test_forward_detect(hxforward_like<int>(value)));
	EXPECT_EQ(hxutility_test_forward::const_rvalue,
		hxutility_test_forward_detect(hxforward_like<const int>(value)));
}

TEST(hxutility_test, hxlog2i_returns_highest_set_bit) {
	// 2^31 is not tested due to potential rounding error in the WASM path.
	for(int i=0u; i < 31; ++i) {
		EXPECT_EQ(hxlog2i(1u << i), i);
	}
	EXPECT_EQ(hxlog2i(1u), 0);
	EXPECT_EQ(hxlog2i(3u), 1);
	EXPECT_EQ(hxlog2i(1u << 31), 31);
}

TEST(hxutility_test, arithmetic_helpers_cover_min_max_abs_clamp) {
	EXPECT_EQ(hxmin(3, 7), 3);
	EXPECT_EQ(hxmax(3, 7), 7);
	EXPECT_EQ(hxabs(-9), 9);
	EXPECT_EQ(hxabs(9), 9);
	EXPECT_EQ(hxclamp(5, 0, 10), 5);
	EXPECT_EQ(hxclamp(-1, 0, 10), 0);
	EXPECT_EQ(hxclamp(11, 0, 10), 10);
	EXPECT_EQ(hxclamp(0, 0, 10), 0);
	EXPECT_EQ(hxclamp(10, 0, 10), 10);
	EXPECT_EQ(hxclamp(-1, 0, 5), 0);
	EXPECT_EQ(hxclamp(6, 0, 5), 5);
	EXPECT_EQ(hxmin(3, 4), 3);
	EXPECT_EQ(hxmax(3, 4), 4);
	EXPECT_EQ(hxmin(4, 3), 3);
	EXPECT_EQ(hxmax(4, 3), 4);
	EXPECT_EQ(hxabs(-1), 1);
	EXPECT_EQ(hxabs(1), 1);
	EXPECT_EQ(hxabs(0), 0);
}

TEST(hxutility_test, hxswap_memcpy) {
	struct hxutility_test_memcpy_record_t {
		int32_t first;
		int32_t second;
	} first = { 1, 2 }, second = { 3, 4 };
	hxswap_memcpy(first, second);
	EXPECT_EQ(first.first, 3);
	EXPECT_EQ(first.second, 4);
	EXPECT_EQ(second.first, 1);
	EXPECT_EQ(second.second, 2);
}

TEST(hxutility_test, hxswap_exchanges_values) {
	int a = 7;
	int b = 11;
	hxswap(a, b);
	EXPECT_EQ(a, 11);
	EXPECT_EQ(b, 7);
}

TEST(hxutility_test, hxswap_uses_move) {
	struct hxutility_test_move_record_t {
		int32_t id;
		int32_t moves;
		hxutility_test_move_record_t(int32_t i) : id(i), moves(0) { }
		hxutility_test_move_record_t(hxutility_test_move_record_t&& x) noexcept
			: id(x.id), moves(x.moves + 1) { x.id = -1; }
		void operator=(hxutility_test_move_record_t&& x) noexcept {
			id = x.id; moves = x.moves + 1; x.id = -1;
		}
	} a(5), b(9);
	hxswap(a, b);
	EXPECT_EQ(a.id, 9);
	EXPECT_EQ(b.id, 5);
	EXPECT_TRUE(a.moves > 0);
	EXPECT_TRUE(b.moves > 0);
}

TEST(hxutility_test, hxisspace) {
	for (int c = 0; c < 128; ++c) {
		const bool hx = hxisspace(static_cast<char>(c));
		const bool st = ::isspace(static_cast<unsigned char>(c)) != 0;
		EXPECT_EQ(hx, st);
	}
	for (int c = 128; c < 256; ++c) {
		const bool hx = hxisspace(static_cast<char>(c));
		EXPECT_EQ(hx, false);
	}

	EXPECT_FALSE(hxisspace(static_cast<char>(0x08)));
	EXPECT_TRUE(hxisspace(static_cast<char>(0x09)));
	EXPECT_TRUE(hxisspace(static_cast<char>(0x0D)));
	EXPECT_FALSE(hxisspace(static_cast<char>(0x0E)));
	EXPECT_TRUE(hxisspace(' '));
	EXPECT_FALSE(hxisspace(static_cast<char>(0x1F)));
	EXPECT_FALSE(hxisspace(static_cast<char>(0x21)));
}

TEST(hxutility_test, hxisgraph) {
	for (int c = 0; c <= 255; ++c) {
		const bool hx = hxisgraph(static_cast<char>(c));
		if (c < 0x80) {
			const bool st = ::isgraph(c) != 0;
			EXPECT_EQ(hx, st);
		}
		else {
			EXPECT_EQ(hx, true);
		}
	}

	EXPECT_FALSE(hxisgraph(static_cast<char>(0x20)));
	EXPECT_TRUE(hxisgraph(static_cast<char>(0x21)));
	EXPECT_TRUE(hxisgraph(static_cast<char>(0x7E)));
	EXPECT_FALSE(hxisgraph(static_cast<char>(0x7F)));
	EXPECT_TRUE(hxisgraph(static_cast<char>(0x80)));
	EXPECT_TRUE(hxisgraph(static_cast<char>(0xFF)));
}

TEST(hxutility_test, hxhex_view) {
	const uint8_t bytes[32] = { 0 };
	hxhex_view(bytes, 0u, false);
	hxhex_view(bytes, sizeof bytes, false);
	hxhex_view(bytes, sizeof bytes, true);
	SUCCEED();
}

TEST(hxutility_test, hxfloat_view) {
	const float floats[8] = { 0.0f };
	hxfloat_view(floats, 0u);
	hxfloat_view(floats, 4u);
	hxfloat_view(floats, sizeof floats / sizeof floats[0]);
	SUCCEED();
}
