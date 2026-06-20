// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

#include <hx/hxrandom.hpp>
#include <hx/hxarray.hpp>
#include <hx/hxtest.hpp>

HX_NS_USE

TEST(hxrandom_test, generation) {
	hxrandom rng(1u);

	// "Automatically casts to an unsigned integer or floating point value."
	uint8_t uint8 = 0u;
	uint16_t uint16 = 0u;
	uint32_t uint32 = 0u;
	uint64_t uint64 = 0u;
	uint8 = rng.u8();
	uint16 = rng.u16();
	uint32 = rng();
	uint64 = rng.u64();

	// None of these should be zero on the second sample.
	EXPECT_NE((static_cast<uint64_t>(uint8) * static_cast<uint64_t>(uint16) *
		static_cast<uint64_t>(uint32) * static_cast<uint64_t>(uint64)), 0u);

	for(int s=100; s-- != 0;) {
		// "Automatically casts to an unsigned integer or floating point value."
		// Grab floats in [0..1).
		const float f = rng.f01();
		const double d = rng.d01();

		// WARNING: While 0.0 is legal, it is being treated as an error
		// because it is likely to be so. The odds of hitting zero in the first
		// 200 numbers is effectively zero.
		EXPECT_TRUE(f > 0.0f && f < 1.0f);
		EXPECT_TRUE(d > 0.0 && d < 1.0);
	}
}

TEST(hxrandom_test, read_populates_buffer) {
	hxrandom rng(0x654321u);

	uint8_t buffer[] = {
		0xefu, 0xefu, 0xefu, 0xefu,
		0xefu, 0xefu, 0xefu, 0xefu,
		0xefu
	};
	const size_t size = hxsize(buffer);
	const size_t read_count = size - 2; // 7. Intentionally odd.

	// "Reads a specified number of random bytes into the provided buffer."
	// Little-endian stream should match manual u32 sequence.
	rng.read(buffer, read_count);

	hxrandom verifier(0x654321u);
	size_t remaining = read_count;
	const uint8_t* expected = buffer;

	// This just documents an expected interface and sequence.
	while(remaining >= 4) {
		const uint32_t x = verifier.u32();
		EXPECT_EQ(*expected++, static_cast<uint8_t>(x));
		EXPECT_EQ(*expected++, static_cast<uint8_t>(x >> 8));
		EXPECT_EQ(*expected++, static_cast<uint8_t>(x >> 16));
		EXPECT_EQ(*expected++, static_cast<uint8_t>(x >> 24));
		remaining -= 4;
	}
	if(remaining != 0u) {
		uint32_t x = verifier.u32();
		do {
			EXPECT_EQ(*expected++, static_cast<uint8_t>(x));
			x >>= 8;
		} while(--remaining != 0u)
			/**/;
	}

	// Check for a tail overwrite.
	for(size_t i = read_count; i < size; ++i) {
		EXPECT_EQ(buffer[i], 0xefu);
	}
}

TEST(hxrandom_test, range) {
	hxrandom rng(30000);
	for(int s=100; s-- != 0;) {
		// "Returns a random number in the range [base..base+range)." Validate
		// overloads across integral and floating types.
		EXPECT_TRUE(rng.range('a', static_cast<char>(10)) >= 'a' &&
			rng.range('a', static_cast<char>(10)) < static_cast<char>('a' + 10));
		EXPECT_TRUE(rng.range(1000,100) >= 1000 && rng.range(1000,100) < 1100);
		EXPECT_TRUE(rng.range(1000u,100u) >= 1000u && rng.range(1000u,100u) < 1100u);
		EXPECT_TRUE(rng.range(1000l,100l) >= 1000l && rng.range(1000l,100l) < 1100l);
		EXPECT_TRUE(rng.range(1000ul,100ul) >= 1000ul && rng.range(1000ul,100ul) < 1100ul);
		EXPECT_TRUE(rng.range(1000ll,100ll) >= 1000ll && rng.range(1000ll,100ll) < 1100ll);
		EXPECT_TRUE(rng.range(1000ull,100ull) >= 1000ull && rng.range(1000ull,100ull) < 1100ull);
		EXPECT_TRUE(rng.range(1000.0f,100.0f) >= 1000.0f && rng.range(1000.0f,100.0f) < 1100.0f);
		EXPECT_TRUE(rng.range(1000.0,100.0) >= 1000.0 && rng.range(1000.0,100.0) < 1100.0);

		// Check that the RNG isn't just spitting out zeros.
		EXPECT_TRUE(rng() | rng());
	}
}

TEST(hxrandom_test, histogram) {
	const hxsystem_allocator_scope temporary_stack_scope(hxsystem_allocator_temporary_stack);
	hxrandom rng(40000);
	constexpr int buckets = 1 << 10; // 1k buckets.
	constexpr int iters = 1000;
	constexpr int max = 1100; // 10% above the average maximum.
	hxarray<int, buckets> hist(0);

	for(int i=(buckets*iters); i-- != 0;) {
		// Doesn't require an unsigned type for %. No floating point math is used.
		++hist[static_cast<size_t>(rng() % (buckets - 1))];
	}
	for(size_t i=buckets; i-- != 0u;) {
		EXPECT_LE(hist[i], max);
	}
}

TEST(hxrandom_test, histogram_f) {
	const hxsystem_allocator_scope temporary_stack_scope(hxsystem_allocator_temporary_stack);
	hxrandom rng(40000);
	constexpr int buckets = 1000; // 1k buckets.
	constexpr int iters = 1000;
	constexpr int max = 1150; // 15% above the average maximum.
	hxarray<int, buckets> hist(0);

	for(int i=(buckets*iters); i-- != 0;) {
		// Generate 64-bit doubles.
		++hist[static_cast<size_t>(rng.range(0.0, static_cast<double>(buckets)))];
	}
	for(size_t i=buckets; i-- != 0u;) {
		EXPECT_LE(hist[i], max);
	}
}

#if HX_CPLUSPLUS >= 202302L

namespace {

// Returns true when u64 produces output across a sequence.
consteval bool hxtest_hxrandom_consteval_range(void) {
	hxrandom rng(42u);
	for(int i_ = 0; i_ < 16; ++i_) {
		const uint64_t v_ = rng.u64();
		if(v_ == 0u) {
			return false;
		}
	}
	return true;
}

static_assert(hxtest_hxrandom_consteval_range(),
	"hxrandom consteval: u64 must produce non-zero output across 16 steps");

} // namespace {

#endif // HX_CPLUSPLUS >= 202302L
