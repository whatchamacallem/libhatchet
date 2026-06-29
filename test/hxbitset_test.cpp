// SPDX-FileCopyrightText: © 2025 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

#include <hx/hxbitset.hpp>
#include <hx/hxtest.hpp>

HX_NS_USE

hxattr_noinline static void hxtest_gdb_break_hxbitset(void) { }

TEST(hxbitset_test, default_ctor_zero_initializes) {
	const hxbitset<1> b1;
	EXPECT_FALSE(b1[0]);
	const hxbitset<sizeof(size_t) * 8u> bw;
	for(size_t i = 0u; i < sizeof(size_t) * 8u; ++i) {
		EXPECT_FALSE(bw[i]);
	}
	const hxbitset<sizeof(size_t) * 8u + 1u> bw1;
	for(size_t i = 0u; i < sizeof(size_t) * 8u + 1u; ++i) {
		EXPECT_FALSE(bw1[i]);
	}
}

TEST(hxbitset_test, default_ctor_resets_dirty_storage) {
	typedef hxbitset<sizeof(size_t) * 8u + 1u> bitset_t;
	alignas(bitset_t) unsigned char storage[sizeof(bitset_t)];
	volatile unsigned char* dirty = storage;
	for(size_t i = 0u; i < sizeof(bitset_t); ++i) {
		dirty[i] = 0xffu;
	}
	const bitset_t* b = ::new(static_cast<void*>(storage)) bitset_t;
	EXPECT_TRUE(b->none());
	for(size_t i = 0u; i < bitset_t::size(); ++i) {
		EXPECT_FALSE((*b)[i]);
	}
}

TEST(hxbitset_test, copy_ctor_duplicates_bits) {
	hxbitset<sizeof(size_t) * 8u + 3u> src;
	src.set(0u);
	src.set(sizeof(size_t) * 8u);
	src.set(sizeof(size_t) * 8u + 2u);
	const hxbitset<sizeof(size_t) * 8u + 3u> dst(src);
	hxtest_gdb_break_hxbitset();
	EXPECT_TRUE(dst[0u]);
	EXPECT_TRUE(dst[sizeof(size_t) * 8u]);
	EXPECT_TRUE(dst[sizeof(size_t) * 8u + 2u]);
	EXPECT_FALSE(dst[1u]);
	EXPECT_FALSE(dst[sizeof(size_t) * 8u - 1u]);
	EXPECT_FALSE(dst[sizeof(size_t) * 8u + 1u]);
}

TEST(hxbitset_test, assignment_copies_bits) {
	hxbitset<sizeof(size_t) * 8u> src;
	src.set(0u);
	src.set(sizeof(size_t) * 8u - 1u);
	hxbitset<sizeof(size_t) * 8u> dst;
	dst = src;
	EXPECT_TRUE(dst[0u]);
	EXPECT_TRUE(dst[sizeof(size_t) * 8u - 1u]);
	EXPECT_FALSE(dst[1u]);
}

TEST(hxbitset_test, size_t_ctor_initializes_from_value) {
	const volatile size_t one = 1u;
	const hxbitset<sizeof(size_t) * 8u> b(static_cast<size_t>(one));
	EXPECT_TRUE(b[0u]);
	EXPECT_FALSE(b[1u]);
	const volatile size_t all_bits = ~static_cast<size_t>(0u);
	const hxbitset<sizeof(size_t) * 8u> b2(static_cast<size_t>(all_bits));
	EXPECT_TRUE(b2.all());
	const volatile size_t zero = 0u;
	const hxbitset<sizeof(size_t) * 8u> b3(static_cast<size_t>(zero));
	EXPECT_TRUE(b3.none());
	const size_t high_bit = static_cast<size_t>(one) << (sizeof(size_t) * 8u - 1u);
	const hxbitset<sizeof(size_t) * 8u> b4(high_bit);
	EXPECT_TRUE(b4[sizeof(size_t) * 8u - 1u]);
	EXPECT_FALSE(b4[sizeof(size_t) * 8u - 2u]);
}

TEST(hxbitset_test, set_all_then_reset_all) {
	hxbitset<sizeof(size_t) * 8u + 1u> b;
	b.set();
	for(size_t i = 0u; i < hxbitset<sizeof(size_t) * 8u + 1u>::size(); ++i) {
		EXPECT_TRUE(b[i]);
	}
	b.reset();
	for(size_t i = 0u; i < hxbitset<sizeof(size_t) * 8u + 1u>::size(); ++i) {
		EXPECT_FALSE(b[i]);
	}
}

TEST(hxbitset_test, set_all_exact_word_boundary) {
	hxbitset<sizeof(size_t) * 8u> b;
	b.set();
	EXPECT_TRUE(b.all());
	for(size_t i = 0u; i < sizeof(size_t) * 8u; ++i) {
		EXPECT_TRUE(b[i]);
	}
}

TEST(hxbitset_test, set_single_bit_first_and_last) {
	hxbitset<sizeof(size_t) * 8u + 1u> b;
	b.set(0u);
	EXPECT_TRUE(b[0u]);
	EXPECT_FALSE(b[1u]);
	b.set(sizeof(size_t) * 8u - 1u);
	EXPECT_TRUE(b[sizeof(size_t) * 8u - 1u]);
	b.set(sizeof(size_t) * 8u);
	EXPECT_TRUE(b[sizeof(size_t) * 8u]);
	EXPECT_FALSE(b[sizeof(size_t) * 8u - 2u]);
}

TEST(hxbitset_test, set_false_clears_bit) {
	hxbitset<8u> b;
	b.set();
	b.set(3u, false);
	EXPECT_FALSE(b[3u]);
	EXPECT_TRUE(b[2u]);
	EXPECT_TRUE(b[4u]);
}

TEST(hxbitset_test, reset_single_bit_first_and_last) {
	hxbitset<sizeof(size_t) * 8u + 1u> b;
	b.set();
	b.reset(0u);
	EXPECT_FALSE(b[0u]);
	EXPECT_TRUE(b[1u]);
	b.reset(sizeof(size_t) * 8u - 1u);
	EXPECT_FALSE(b[sizeof(size_t) * 8u - 1u]);
	EXPECT_TRUE(b[sizeof(size_t) * 8u - 2u]);
	b.reset(sizeof(size_t) * 8u);
	EXPECT_FALSE(b[sizeof(size_t) * 8u]);
}

TEST(hxbitset_test, test_matches_bracket_operator) {
	hxbitset<sizeof(size_t) * 8u + 3u> b;
	b.set(0u);
	b.set(sizeof(size_t) * 8u - 1u);
	b.set(sizeof(size_t) * 8u);
	b.set(sizeof(size_t) * 8u + 2u);
	for(size_t i = 0u; i < hxbitset<sizeof(size_t) * 8u + 3u>::size(); ++i) {
		EXPECT_EQ(b.test(i), b[i]);
	}
}

TEST(hxbitset_test, flip_all_from_zero) {
	hxbitset<sizeof(size_t) * 8u + 1u> b;
	b.flip();
	EXPECT_TRUE(b.all());
	EXPECT_EQ(b.data()[1], static_cast<size_t>(1u));
}

TEST(hxbitset_test, flip_all_from_set) {
	hxbitset<sizeof(size_t) * 8u + 1u> b;
	b.set();
	b.flip();
	EXPECT_TRUE(b.none());
}

TEST(hxbitset_test, flip_all_exact_word_boundary) {
	hxbitset<sizeof(size_t) * 8u> b;
	b.flip();
	EXPECT_TRUE(b.all());
	b.flip();
	EXPECT_TRUE(b.none());
}

TEST(hxbitset_test, flip_single_bit_first_and_last) {
	hxbitset<sizeof(size_t) * 8u + 1u> b;
	b.flip(0u);
	EXPECT_TRUE(b[0u]);
	b.flip(0u);
	EXPECT_FALSE(b[0u]);
	b.flip(sizeof(size_t) * 8u - 1u);
	EXPECT_TRUE(b[sizeof(size_t) * 8u - 1u]);
	b.flip(sizeof(size_t) * 8u);
	EXPECT_TRUE(b[sizeof(size_t) * 8u]);
	EXPECT_FALSE(b[sizeof(size_t) * 8u - 2u]);
}

TEST(hxbitset_test, all_returns_true_only_when_every_bit_is_set) {
	hxbitset<sizeof(size_t) * 8u + 1u> b;
	EXPECT_FALSE(b.all());
	b.set();
	EXPECT_TRUE(b.all());
	b.reset(0u);
	EXPECT_FALSE(b.all());
	b.set();
	b.reset(sizeof(size_t) * 8u - 1u);
	EXPECT_FALSE(b.all());
	b.set();
	b.reset(sizeof(size_t) * 8u);
	EXPECT_FALSE(b.all());
}

TEST(hxbitset_test, any_returns_true_when_at_least_one_bit_is_set) {
	hxbitset<sizeof(size_t) * 8u + 1u> b;
	EXPECT_FALSE(b.any());
	b.set(0u);
	EXPECT_TRUE(b.any());
	b.reset();
	b.set(sizeof(size_t) * 8u);
	EXPECT_TRUE(b.any());
}

TEST(hxbitset_test, and_assign_clears_bits) {
	hxbitset<sizeof(size_t) * 8u + 3u> a;
	hxbitset<sizeof(size_t) * 8u + 3u> b;
	a.set();
	b.set(1u);
	b.set(sizeof(size_t) * 8u);
	a &= b;
	EXPECT_FALSE(a[0u]);
	EXPECT_TRUE(a[1u]);
	EXPECT_TRUE(a[sizeof(size_t) * 8u]);
	EXPECT_FALSE(a[sizeof(size_t) * 8u + 2u]);
}

TEST(hxbitset_test, or_assign_sets_bits) {
	hxbitset<sizeof(size_t) * 8u + 3u> a;
	hxbitset<sizeof(size_t) * 8u + 3u> b;
	a.set(0u);
	b.set(sizeof(size_t) * 8u);
	a |= b;
	EXPECT_TRUE(a[0u]);
	EXPECT_TRUE(a[sizeof(size_t) * 8u]);
	EXPECT_FALSE(a[1u]);
}

TEST(hxbitset_test, xor_assign_toggles_bits) {
	hxbitset<sizeof(size_t) * 8u + 3u> a;
	hxbitset<sizeof(size_t) * 8u + 3u> b;
	a.set(0u);
	a.set(1u);
	b.set(1u);
	b.set(2u);
	a ^= b;
	EXPECT_TRUE(a[0u]);
	EXPECT_FALSE(a[1u]);
	EXPECT_TRUE(a[2u]);
}

TEST(hxbitset_test, left_shift_zero_is_identity) {
	hxbitset<sizeof(size_t) * 8u + 3u> b;
	b.set(0u);
	b.set(sizeof(size_t) * 8u);
	b <<= 0u;
	EXPECT_TRUE(b[0u]);
	EXPECT_TRUE(b[sizeof(size_t) * 8u]);
	EXPECT_FALSE(b[1u]);
}

TEST(hxbitset_test, left_shift_single_bit_within_word) {
	hxbitset<sizeof(size_t) * 8u> b;
	b.set(0u);
	b <<= 1u;
	EXPECT_FALSE(b[0u]);
	EXPECT_TRUE(b[1u]);
}

TEST(hxbitset_test, left_shift_crosses_word_boundary) {
	hxbitset<sizeof(size_t) * 8u + 1u> b;
	b.set(sizeof(size_t) * 8u - 1u);
	b <<= 1u;
	EXPECT_FALSE(b[sizeof(size_t) * 8u - 1u]);
	EXPECT_TRUE(b[sizeof(size_t) * 8u]);
}

TEST(hxbitset_test, left_shift_by_word_multiple) {
	hxbitset<sizeof(size_t) * 8u * 2u> b;
	b.set(0u);
	b.set(1u);
	b <<= sizeof(size_t) * 8u;
	EXPECT_FALSE(b[0u]);
	EXPECT_FALSE(b[1u]);
	EXPECT_TRUE(b[sizeof(size_t) * 8u]);
	EXPECT_TRUE(b[sizeof(size_t) * 8u + 1u]);
}

TEST(hxbitset_test, left_shift_by_more_than_bits_zeros_all) {
	hxbitset<sizeof(size_t) * 8u + 3u> b;
	b.set();
	b <<= hxbitset<sizeof(size_t) * 8u + 3u>::size();
	EXPECT_TRUE(b.none());
}

TEST(hxbitset_test, left_shift_off_by_one_at_word_boundary) {
	hxbitset<sizeof(size_t) * 8u + 1u> b;
	b.set(0u);
	b <<= sizeof(size_t) * 8u - 1u;
	EXPECT_TRUE(b[sizeof(size_t) * 8u - 1u]);
	EXPECT_FALSE(b[sizeof(size_t) * 8u]);
	EXPECT_FALSE(b[0u]);
}

TEST(hxbitset_test, left_shift_trailing_bits_masked_off) {
	hxbitset<sizeof(size_t) * 8u + 1u> b;
	b.set();
	b <<= 1u;
	EXPECT_TRUE(b[sizeof(size_t) * 8u]);
	EXPECT_EQ(b.data()[1], static_cast<size_t>(1u));
	EXPECT_EQ(b.data()[1] & ~static_cast<size_t>(1u), static_cast<size_t>(0u));
}

TEST(hxbitset_test, right_shift_zero_is_identity) {
	hxbitset<sizeof(size_t) * 8u + 3u> b;
	b.set(0u);
	b.set(sizeof(size_t) * 8u);
	b >>= 0u;
	EXPECT_TRUE(b[0u]);
	EXPECT_TRUE(b[sizeof(size_t) * 8u]);
}

TEST(hxbitset_test, right_shift_single_bit_within_word) {
	hxbitset<sizeof(size_t) * 8u> b;
	b.set(1u);
	b >>= 1u;
	EXPECT_TRUE(b[0u]);
	EXPECT_FALSE(b[1u]);
}

TEST(hxbitset_test, right_shift_crosses_word_boundary) {
	hxbitset<sizeof(size_t) * 8u + 1u> b;
	b.set(sizeof(size_t) * 8u);
	b >>= 1u;
	EXPECT_FALSE(b[sizeof(size_t) * 8u]);
	EXPECT_TRUE(b[sizeof(size_t) * 8u - 1u]);
}

TEST(hxbitset_test, right_shift_by_word_multiple) {
	hxbitset<sizeof(size_t) * 8u * 2u> b;
	b.set(sizeof(size_t) * 8u);
	b.set(sizeof(size_t) * 8u + 1u);
	b >>= sizeof(size_t) * 8u;
	EXPECT_TRUE(b[0u]);
	EXPECT_TRUE(b[1u]);
	EXPECT_FALSE(b[sizeof(size_t) * 8u]);
	EXPECT_FALSE(b[sizeof(size_t) * 8u + 1u]);
}

TEST(hxbitset_test, right_shift_by_more_than_bits_zeros_all) {
	hxbitset<sizeof(size_t) * 8u + 3u> b;
	b.set();
	b >>= hxbitset<sizeof(size_t) * 8u + 3u>::size();
	EXPECT_TRUE(b.none());
}

TEST(hxbitset_test, right_shift_off_by_one_at_word_boundary) {
	hxbitset<sizeof(size_t) * 8u + 1u> b;
	b.set(sizeof(size_t) * 8u - 1u);
	b >>= sizeof(size_t) * 8u - 1u;
	EXPECT_TRUE(b[0u]);
	EXPECT_FALSE(b[1u]);
	EXPECT_FALSE(b[sizeof(size_t) * 8u - 1u]);
}

TEST(hxbitset_test, equal_operator_detects_difference) {
	hxbitset<sizeof(size_t) * 8u + 3u> a;
	const hxbitset<sizeof(size_t) * 8u + 3u> b;
	a.set(0u);
	EXPECT_NE(a, b);
}

#if HX_CPLUSPLUS < 202002L
TEST(hxbitset_test, not_equal_operator_defined_before_cpp20) {
	hxbitset<sizeof(size_t) * 8u> a;
	hxbitset<sizeof(size_t) * 8u> b;
	a.set(0u);
	EXPECT_NE(a, b);
	hxbitset<sizeof(size_t) * 8u> c(a);
	EXPECT_EQ(a, c);
}
#endif

TEST(hxbitset_test, load_copies_bytes_into_storage) {
	hxbitset<8u> b;
	const unsigned char src = static_cast<unsigned char>(0xcbu);
	b.load(&src, sizeof src);
	for(size_t i = 0u; i < 8u; ++i) {
		EXPECT_EQ(b[i], ((0xcbu >> i) & 1u) != 0u);
	}
}

TEST(hxbitset_test, load_masks_trailing_bits) {
	hxbitset<3u> b;
	const unsigned char src = static_cast<unsigned char>(0xffu);
	b.load(&src, sizeof src);
	EXPECT_TRUE(b[0u]);
	EXPECT_TRUE(b[1u]);
	EXPECT_TRUE(b[2u]);
	EXPECT_TRUE(b.all());
}

TEST(hxbitset_test, load_partial_bytes) {
	hxbitset<sizeof(size_t) * 8u * 2u> b;
	size_t val = static_cast<size_t>(1u);
	b.load(&val, sizeof(size_t));
	EXPECT_TRUE(b[0u]);
	EXPECT_FALSE(b[1u]);
}

TEST(hxbitset_test, exact_word_bitset_all_any_none) {
	hxbitset<sizeof(size_t) * 8u> b;
	EXPECT_TRUE(b.none());
	EXPECT_FALSE(b.any());
	EXPECT_FALSE(b.all());
	b.set();
	EXPECT_TRUE(b.all());
	EXPECT_TRUE(b.any());
	EXPECT_FALSE(b.none());
	b.reset(0u);
	EXPECT_FALSE(b.all());
	EXPECT_TRUE(b.any());
}

TEST(hxbitset_test, exact_word_bitset_flip_round_trip) {
	hxbitset<sizeof(size_t) * 8u> b;
	b.set(sizeof(size_t) * 8u - 1u);
	b.flip();
	EXPECT_FALSE(b[sizeof(size_t) * 8u - 1u]);
	for(size_t i = 0u; i < sizeof(size_t) * 8u - 1u; ++i) {
		EXPECT_TRUE(b[i]);
	}
}

TEST(hxbitset_test, multi_word_set_clears_trailing_bits_in_last_word) {
	hxbitset<sizeof(size_t) * 8u + 1u> b;
	b.set();
	EXPECT_EQ(b.data()[1], static_cast<size_t>(1u));
}

TEST(hxbitset_test, multi_word_flip_masks_trailing_bits) {
	hxbitset<sizeof(size_t) * 8u + 1u> b;
	b.flip();
	EXPECT_EQ(b.data()[0], ~static_cast<size_t>(0u));
	EXPECT_EQ(b.data()[1], static_cast<size_t>(1u));
}

TEST(hxbitset_test, penultimate_word_boundary_bitset) {
	const size_t bits = sizeof(size_t) * 8u * 2u - 1u;
	hxbitset<sizeof(size_t) * 8u * 2u - 1u> b;
	b.set();
	EXPECT_TRUE(b.all());
	EXPECT_EQ(b.data()[1], ~static_cast<size_t>(0u) >> 1u);
	b.reset(bits - 1u);
	EXPECT_FALSE(b[bits - 1u]);
	EXPECT_TRUE(b[bits - 2u]);
	EXPECT_FALSE(b.all());
}

TEST(hxbitset_test, repeated_left_shift_drains_to_zero) {
	hxbitset<sizeof(size_t) * 8u + 1u> b;
	b.set();
	for(size_t i = 0u; i < hxbitset<sizeof(size_t) * 8u + 1u>::size(); ++i) {
		b <<= 1u;
	}
	EXPECT_TRUE(b.none());
}

TEST(hxbitset_test, repeated_right_shift_drains_to_zero) {
	hxbitset<sizeof(size_t) * 8u + 1u> b;
	b.set();
	for(size_t i = 0u; i < hxbitset<sizeof(size_t) * 8u + 1u>::size(); ++i) {
		b >>= 1u;
	}
	EXPECT_TRUE(b.none());
}

TEST(hxbitset_test, all_fails_when_penultimate_word_cleared) {
	hxbitset<sizeof(size_t) * 8u * 2u + 1u> b;
	b.set();
	b.reset(0u);
	EXPECT_FALSE(b.all());
	b.set();
	b.reset(sizeof(size_t) * 8u);
	EXPECT_FALSE(b.all());
	b.set();
	b.reset(sizeof(size_t) * 8u * 2u);
	EXPECT_FALSE(b.all());
}

TEST(hxbitset_test, any_detects_bit_only_in_last_word) {
	hxbitset<sizeof(size_t) * 8u * 2u + 1u> b;
	b.set(sizeof(size_t) * 8u * 2u);
	EXPECT_TRUE(b.any());
}

TEST(hxbitset_test, left_shift_word_multiple_boundary_lo) {
	hxbitset<sizeof(size_t) * 8u * 2u> b;
	b.set(0u);
	b <<= sizeof(size_t) * 8u;
	EXPECT_TRUE(b[sizeof(size_t) * 8u]);
	EXPECT_FALSE(b[0u]);
	EXPECT_FALSE(b[sizeof(size_t) * 8u - 1u]);
}

TEST(hxbitset_test, right_shift_high_carry_boundary) {
	hxbitset<sizeof(size_t) * 8u * 2u> b;
	b.set(sizeof(size_t) * 8u);
	b >>= 1u;
	EXPECT_TRUE(b[sizeof(size_t) * 8u - 1u]);
	EXPECT_FALSE(b[sizeof(size_t) * 8u]);
}

TEST(hxbitset_test, single_bit_full_coverage) {
	hxbitset<1u> b;
	EXPECT_EQ(hxbitset<1u>::size(), 1u);
	EXPECT_EQ(hxbitset<1u>::bytes(), sizeof(size_t));
	EXPECT_TRUE(b.none());
	EXPECT_FALSE(b.any());
	EXPECT_FALSE(b.all());
	b.set(0u);
	EXPECT_TRUE(b[0u]);
	EXPECT_TRUE(b.test(0u));
	EXPECT_TRUE(b.any());
	EXPECT_TRUE(b.all());
	EXPECT_FALSE(b.none());
	b.flip(0u);
	EXPECT_FALSE(b[0u]);
	EXPECT_TRUE(b.none());
	b.flip();
	EXPECT_TRUE(b[0u]);
	b.reset(0u);
	EXPECT_TRUE(b.none());
	b.set();
	EXPECT_TRUE(b.all());
	b <<= 1u;
	EXPECT_TRUE(b.none());
	b.set();
	b >>= 1u;
	EXPECT_TRUE(b.none());
	const hxbitset<1u> c(b);
	EXPECT_EQ(b, c);
}
#if HX_CPLUSPLUS >= 202302L
namespace {

consteval size_t hxtest_hxbitset_consteval_count(void) {
	hxbitset<sizeof(size_t) * 8u + 3u> b;
	b.set();
	b.reset(0u);
	b.reset(sizeof(size_t) * 8u);
	b.flip(1u);
	b.flip(1u);
	hxbitset<sizeof(size_t) * 8u + 3u> mask;
	mask.set(0u);
	mask.set(sizeof(size_t) * 8u);
	b |= mask;
	size_t count = 0u;
	for(size_t i_ = 0u; i_ < hxbitset<sizeof(size_t) * 8u + 3u>::size(); ++i_) {
		if(b[i_]) {
			++count;
		}
	}
	return count;
}
static_assert(hxtest_hxbitset_consteval_count() == sizeof(size_t) * 8u + 3u,
	"hxbitset consteval: set/reset/flip/or must produce all bits set");
consteval bool hxtest_hxbitset_consteval_shift(void) {
	hxbitset<sizeof(size_t) * 8u> a;
	a.set(0u);
	a <<= 1u;
	if(!a[1u] || a[0u]) {
		return false;
	}
	a >>= 1u;
	if(!a[0u] || a[1u]) {
		return false;
	}
	const hxbitset<sizeof(size_t) * 8u> b(a);
	return a == b;
}
static_assert(hxtest_hxbitset_consteval_shift(),
	"hxbitset consteval: shift operators and equality must work");
consteval bool hxtest_hxbitset_consteval_and_xor(void) {
	hxbitset<sizeof(size_t) * 8u> a;
	hxbitset<sizeof(size_t) * 8u> b;
	a.set(0u);
	a.set(1u);
	b.set(1u);
	b.set(2u);
	a &= b;
	if(!a[1u] || a[0u] || a[2u]) {
		return false;
	}
	a ^= b;
	return a[2u] && !a[1u];
}
static_assert(hxtest_hxbitset_consteval_and_xor(),
	"hxbitset consteval: AND and XOR operators must work");
} // namespace
#endif // HX_CPLUSPLUS >= 202302L
