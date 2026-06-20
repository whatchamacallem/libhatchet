// SPDX-FileCopyrightText: © 2025 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

#include <hx/hxbitset.hpp>
#include <hx/hxtest.hpp>

hxattr_noinline static void hxtest_gdb_break_hxbitset(void) {}

// Construction and static properties

TEST(hxbitset_test, default_ctor_zero_initializes) {
	// "Constructs a zero-initialized hxbitset."
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

// Copy construction and assignment

TEST(hxbitset_test, copy_ctor_duplicates_bits) {
	// "Constructs a hxbitset as a copy of x."
	hxbitset<sizeof(size_t) * 8u + 3u> src;
	src.set(0u);
	src.set(sizeof(size_t) * 8u);    // first bit of second word
	src.set(sizeof(size_t) * 8u + 2u); // last valid bit

	const hxbitset<sizeof(size_t) * 8u + 3u> dst(src);
	hxtest_gdb_break_hxbitset();
	EXPECT_TRUE(dst[0u]);
	EXPECT_TRUE(dst[sizeof(size_t) * 8u]);
	EXPECT_TRUE(dst[sizeof(size_t) * 8u + 2u]);
	// Bits that were not set must remain clear.
	EXPECT_FALSE(dst[1u]);
	EXPECT_FALSE(dst[sizeof(size_t) * 8u - 1u]);
	EXPECT_FALSE(dst[sizeof(size_t) * 8u + 1u]);
}

TEST(hxbitset_test, assignment_copies_bits) {
	// "Assigns the bits of x to this hxbitset."
	hxbitset<sizeof(size_t) * 8u> src;
	src.set(0u);
	src.set(sizeof(size_t) * 8u - 1u);

	hxbitset<sizeof(size_t) * 8u> dst;
	dst = src;
	EXPECT_TRUE(dst[0u]);
	EXPECT_TRUE(dst[sizeof(size_t) * 8u - 1u]);
	EXPECT_FALSE(dst[1u]);
}

// size_t constructor

TEST(hxbitset_test, size_t_ctor_initializes_from_value) {
	// "Constructs a hxbitset from a single size_t value." Only valid when
	// bits == sizeof(size_t) * 8.
	const hxbitset<sizeof(size_t) * 8u> b(static_cast<size_t>(1u));
	EXPECT_TRUE(b[0u]);
	EXPECT_FALSE(b[1u]);

	const hxbitset<sizeof(size_t) * 8u> b2(~static_cast<size_t>(0u));
	EXPECT_TRUE(b2.all());

	const hxbitset<sizeof(size_t) * 8u> b3(static_cast<size_t>(0u));
	EXPECT_TRUE(b3.none());

	// High bit only.
	const size_t high_bit = static_cast<size_t>(1u) << (sizeof(size_t) * 8u - 1u);
	const hxbitset<sizeof(size_t) * 8u> b4(high_bit);
	EXPECT_TRUE(b4[sizeof(size_t) * 8u - 1u]);
	EXPECT_FALSE(b4[sizeof(size_t) * 8u - 2u]);
}

// set / reset / test / operator[]

TEST(hxbitset_test, set_all_then_reset_all) {
	// "Sets all bits to 1." / "Clears all bits to 0."
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
	// set() on a bitset whose size is exactly one word must not leave
	// trailing bits set (s_trailing_mask == ~0u in this case).
	hxbitset<sizeof(size_t) * 8u> b;
	b.set();
	EXPECT_TRUE(b.all());
	for(size_t i = 0u; i < sizeof(size_t) * 8u; ++i) {
		EXPECT_TRUE(b[i]);
	}
}

TEST(hxbitset_test, set_single_bit_first_and_last) {
	// "Sets or clears the bit at position pos."
	hxbitset<sizeof(size_t) * 8u + 1u> b;

	// First bit of first word.
	b.set(0u);
	EXPECT_TRUE(b[0u]);
	EXPECT_FALSE(b[1u]);

	// Last bit of first word (off-by-one boundary).
	b.set(sizeof(size_t) * 8u - 1u);
	EXPECT_TRUE(b[sizeof(size_t) * 8u - 1u]);

	// First bit of second word (word boundary crossing).
	b.set(sizeof(size_t) * 8u);
	EXPECT_TRUE(b[sizeof(size_t) * 8u]);

	// Confirm no adjacent contamination.
	EXPECT_FALSE(b[sizeof(size_t) * 8u - 2u]);
}

TEST(hxbitset_test, set_false_clears_bit) {
	// "Sets or clears the bit at position pos." with value=false.
	hxbitset<8u> b;
	b.set();
	b.set(3u, false);
	EXPECT_FALSE(b[3u]);
	EXPECT_TRUE(b[2u]);
	EXPECT_TRUE(b[4u]);
}

TEST(hxbitset_test, reset_single_bit_first_and_last) {
	// "Clears the bit at position pos."
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
	// "Returns the value of the bit at position pos." Both accessors must agree.
	hxbitset<sizeof(size_t) * 8u + 3u> b;
	b.set(0u);
	b.set(sizeof(size_t) * 8u - 1u);
	b.set(sizeof(size_t) * 8u);
	b.set(sizeof(size_t) * 8u + 2u);

	for(size_t i = 0u; i < hxbitset<sizeof(size_t) * 8u + 3u>::size(); ++i) {
		EXPECT_EQ(b.test(i), b[i]);
	}
}

// flip

TEST(hxbitset_test, flip_all_from_zero) {
	// "Flips all bits." Starting from zero must produce all-set.
	hxbitset<sizeof(size_t) * 8u + 1u> b;
	b.flip();
	EXPECT_TRUE(b.all());
	// Trailing bit must not escape: verify via data() that the second word
	// contains exactly 1 (only bit s_bits_per_word is set, none above it).
	EXPECT_EQ(b.data()[1], static_cast<size_t>(1u));
}

TEST(hxbitset_test, flip_all_from_set) {
	// Flipping all bits from fully set must produce all-clear.
	hxbitset<sizeof(size_t) * 8u + 1u> b;
	b.set();
	b.flip();
	EXPECT_TRUE(b.none());
}

TEST(hxbitset_test, flip_all_exact_word_boundary) {
	// flip() on exact word-size bitset must maintain valid state.
	hxbitset<sizeof(size_t) * 8u> b;
	b.flip();
	EXPECT_TRUE(b.all());
	b.flip();
	EXPECT_TRUE(b.none());
}

TEST(hxbitset_test, flip_single_bit_first_and_last) {
	// "Flips the bit at position pos."
	hxbitset<sizeof(size_t) * 8u + 1u> b;

	b.flip(0u);
	EXPECT_TRUE(b[0u]);
	b.flip(0u);
	EXPECT_FALSE(b[0u]);

	b.flip(sizeof(size_t) * 8u - 1u);
	EXPECT_TRUE(b[sizeof(size_t) * 8u - 1u]);

	// Word boundary.
	b.flip(sizeof(size_t) * 8u);
	EXPECT_TRUE(b[sizeof(size_t) * 8u]);

	// Adjacent bits must be unaffected.
	EXPECT_FALSE(b[sizeof(size_t) * 8u - 2u]);
}

// all / any / none

TEST(hxbitset_test, all_returns_true_only_when_every_bit_is_set) {
	// "Returns true if all bits are set."
	hxbitset<sizeof(size_t) * 8u + 1u> b;
	EXPECT_FALSE(b.all());

	b.set();
	EXPECT_TRUE(b.all());

	// Clear exactly one bit: must no longer be all.
	b.reset(0u);
	EXPECT_FALSE(b.all());

	b.set();
	b.reset(sizeof(size_t) * 8u - 1u); // last bit of first word
	EXPECT_FALSE(b.all());

	b.set();
	b.reset(sizeof(size_t) * 8u); // first bit of second word
	EXPECT_FALSE(b.all());
}

TEST(hxbitset_test, any_returns_true_when_at_least_one_bit_is_set) {
	// "Returns true if at least one bit is set."
	hxbitset<sizeof(size_t) * 8u + 1u> b;
	EXPECT_FALSE(b.any());

	b.set(0u);
	EXPECT_TRUE(b.any());

	b.reset();
	b.set(sizeof(size_t) * 8u);
	EXPECT_TRUE(b.any());
}

// Bitwise assign operators

TEST(hxbitset_test, and_assign_clears_bits) {
	// "Applies bitwise AND with x in place."
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
	// "Applies bitwise OR with x in place."
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
	// "Applies bitwise XOR with x in place."
	hxbitset<sizeof(size_t) * 8u + 3u> a;
	hxbitset<sizeof(size_t) * 8u + 3u> b;
	a.set(0u);
	a.set(1u);
	b.set(1u);
	b.set(2u);

	a ^= b;
	// 0 XOR 0 = 0, but a[0] was set.
	EXPECT_TRUE(a[0u]);
	// 1 XOR 1 = 0.
	EXPECT_FALSE(a[1u]);
	// 0 XOR 1 = 1.
	EXPECT_TRUE(a[2u]);
}

// Shift operators

TEST(hxbitset_test, left_shift_zero_is_identity) {
	// "Shifts all bits left by count positions." count==0 returns *this unchanged.
	hxbitset<sizeof(size_t) * 8u + 3u> b;
	b.set(0u);
	b.set(sizeof(size_t) * 8u);
	b <<= 0u;
	EXPECT_TRUE(b[0u]);
	EXPECT_TRUE(b[sizeof(size_t) * 8u]);
	EXPECT_FALSE(b[1u]);
}

TEST(hxbitset_test, left_shift_single_bit_within_word) {
	// Shift by 1 within one word.
	hxbitset<sizeof(size_t) * 8u> b;
	b.set(0u);
	b <<= 1u;
	EXPECT_FALSE(b[0u]);
	EXPECT_TRUE(b[1u]);
}

TEST(hxbitset_test, left_shift_crosses_word_boundary) {
	// Shift bit at position s_bits_per_word-1 by 1 into the next word.
	hxbitset<sizeof(size_t) * 8u + 1u> b;
	b.set(sizeof(size_t) * 8u - 1u);
	b <<= 1u;
	EXPECT_FALSE(b[sizeof(size_t) * 8u - 1u]);
	EXPECT_TRUE(b[sizeof(size_t) * 8u]);
}

TEST(hxbitset_test, left_shift_by_word_multiple) {
	// Shift by exactly s_bits_per_word (bit_shift == 0 branch).
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
	// Shifting by >= bits must produce all zeros.
	hxbitset<sizeof(size_t) * 8u + 3u> b;
	b.set();
	b <<= hxbitset<sizeof(size_t) * 8u + 3u>::size();
	EXPECT_TRUE(b.none());
}

TEST(hxbitset_test, left_shift_off_by_one_at_word_boundary) {
	// Shift by s_bits_per_word - 1: bit 0 should land at s_bits_per_word-1.
	hxbitset<sizeof(size_t) * 8u + 1u> b;
	b.set(0u);
	b <<= sizeof(size_t) * 8u - 1u;
	EXPECT_TRUE(b[sizeof(size_t) * 8u - 1u]);
	EXPECT_FALSE(b[sizeof(size_t) * 8u]);
	EXPECT_FALSE(b[0u]);
}

TEST(hxbitset_test, left_shift_trailing_bits_masked_off) {
	// After shifting, bits beyond bits-1 must be masked to 0.
	hxbitset<sizeof(size_t) * 8u + 1u> b;
	b.set();
	// Shift by 1: bit s_bits_per_word shifts out of range, bit s_bits_per_word-1
	// shifts into bit s_bits_per_word (word[1] bit 0).
	b <<= 1u;
	// bit s_bits_per_word is now set (from bit s_bits_per_word-1).
	EXPECT_TRUE(b[sizeof(size_t) * 8u]);
	EXPECT_EQ(b.data()[1], static_cast<size_t>(1u));
	// bit s_bits_per_word+1 does not exist. The mask keeps word[1] to 1.
	EXPECT_EQ(b.data()[1] & ~static_cast<size_t>(1u), static_cast<size_t>(0u));
}

TEST(hxbitset_test, right_shift_zero_is_identity) {
	// "Shifts all bits right by count positions." count==0 returns *this unchanged.
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
	// Shift bit at s_bits_per_word right by 1 into word 0.
	hxbitset<sizeof(size_t) * 8u + 1u> b;
	b.set(sizeof(size_t) * 8u);
	b >>= 1u;
	EXPECT_FALSE(b[sizeof(size_t) * 8u]);
	EXPECT_TRUE(b[sizeof(size_t) * 8u - 1u]);
}

TEST(hxbitset_test, right_shift_by_word_multiple) {
	// Shift by exactly s_bits_per_word (bit_shift == 0 branch).
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
	// Shift by s_bits_per_word - 1: bit s_bits_per_word-1 should land at 0.
	hxbitset<sizeof(size_t) * 8u + 1u> b;
	b.set(sizeof(size_t) * 8u - 1u);
	b >>= sizeof(size_t) * 8u - 1u;
	EXPECT_TRUE(b[0u]);
	EXPECT_FALSE(b[1u]);
	EXPECT_FALSE(b[sizeof(size_t) * 8u - 1u]);
}

// Equality operators

TEST(hxbitset_test, equal_operator_detects_difference) {
	hxbitset<sizeof(size_t) * 8u + 3u> a;
	const hxbitset<sizeof(size_t) * 8u + 3u> b;
	a.set(0u);
	EXPECT_NE(a, b);
}

#if HX_CPLUSPLUS < 202002L
TEST(hxbitset_test, not_equal_operator_defined_before_cpp20) {
	// "Returns true if any bits differ from those of x."
	hxbitset<sizeof(size_t) * 8u> a;
	hxbitset<sizeof(size_t) * 8u> b;
	a.set(0u);
	EXPECT_NE(a, b);
	hxbitset<sizeof(size_t) * 8u> c(a);
	EXPECT_EQ(a, c);
}
#endif

// load

TEST(hxbitset_test, load_copies_bytes_into_storage) {
	// "Copies len bytes from src into the hxbitset storage."
	hxbitset<8u> b;
	const unsigned char src = static_cast<unsigned char>(0xcbu);
	b.load(reinterpret_cast<const char*>(&src), sizeof src);
	// Verify bit-by-bit that the value 0xcd was loaded.
	for(size_t i = 0u; i < 8u; ++i) {
		EXPECT_EQ(b[i], ((0xcbu >> i) & 1u) != 0u);
	}
}

TEST(hxbitset_test, load_masks_trailing_bits) {
	// load() must mask trailing bits rather than asserting when the source
	// byte has bits set above bits-1.
	hxbitset<3u> b;
	// 0xff has bits 3-7 set which are beyond bits-1 == 2.
	const unsigned char src = static_cast<unsigned char>(0xffu);
	b.load(reinterpret_cast<const char*>(&src), sizeof src);
	// Only the lowest 3 bits must survive.
	EXPECT_TRUE(b[0u]);
	EXPECT_TRUE(b[1u]);
	EXPECT_TRUE(b[2u]);
	EXPECT_TRUE(b.all());
}

TEST(hxbitset_test, load_partial_bytes) {
	// Load fewer than bytes() bytes. The rest of the storage is unspecified
	// but the loaded portion must be correct.
	hxbitset<sizeof(size_t) * 8u * 2u> b;
	size_t val = static_cast<size_t>(1u);
	b.load(reinterpret_cast<const char*>(&val), sizeof(size_t));
	EXPECT_TRUE(b[0u]);
	EXPECT_FALSE(b[1u]);
}

// Single-word exact-boundary bitset (bits == s_bits_per_word)

TEST(hxbitset_test, exact_word_bitset_all_any_none) {
	// s_trailing_mask == ~0u when bits is a multiple of s_bits_per_word.
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
	b.set(sizeof(size_t) * 8u - 1u); // highest bit
	b.flip();
	EXPECT_FALSE(b[sizeof(size_t) * 8u - 1u]);
	for(size_t i = 0u; i < sizeof(size_t) * 8u - 1u; ++i) {
		EXPECT_TRUE(b[i]);
	}
}

// Multi-word bitset with non-zero trailing bits

TEST(hxbitset_test, multi_word_set_clears_trailing_bits_in_last_word) {
	// After set() the raw word beyond bits-1 must be masked to 0.
	hxbitset<sizeof(size_t) * 8u + 1u> b;
	b.set();
	// Only the lowest bit of word[1] should be set (bit s_bits_per_word).
	EXPECT_EQ(b.data()[1], static_cast<size_t>(1u));
}

TEST(hxbitset_test, multi_word_flip_masks_trailing_bits) {
	hxbitset<sizeof(size_t) * 8u + 1u> b;
	b.flip();
	// Word 0 fully set, word 1 has only bit 0 set.
	EXPECT_EQ(b.data()[0], ~static_cast<size_t>(0u));
	EXPECT_EQ(b.data()[1], static_cast<size_t>(1u));
}

// Two-word bitset with odd trailing bits (bits == s_bits_per_word * 2 - 1)

TEST(hxbitset_test, penultimate_word_boundary_bitset) {
	const size_t bits = sizeof(size_t) * 8u * 2u - 1u;
	hxbitset<sizeof(size_t) * 8u * 2u - 1u> b;

	b.set();
	EXPECT_TRUE(b.all());
	// The last word should have all bits except the MSB set.
	EXPECT_EQ(b.data()[1], ~static_cast<size_t>(0u) >> 1u);

	b.reset(bits - 1u); // Highest valid bit.
	EXPECT_FALSE(b[bits - 1u]);
	EXPECT_TRUE(b[bits - 2u]);
	EXPECT_FALSE(b.all());
}

// Chained shifts preserve trailing bit invariant

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

// Smoke test with the minimum-size bitset

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

// Consteval context

#if HX_CPLUSPLUS >= 202302L

namespace {

// Returns the number of bits set after constructing a bitset and calling
// set/reset/flip on individual bits and operators.
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

// Returns true if shift operators and equality work correctly at compile time.
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

// Returns true if bitwise AND and XOR work correctly at compile time.
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

} // namespace {

#endif // HX_CPLUSPLUS >= 202302L
