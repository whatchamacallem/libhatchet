// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

#include <hx/hxrandom.hpp>
#include <hx/hxarray.hpp>
#include <hx/hxtest.hpp>

HX_NS_USE

TEST(hxrandom_test, generation) {
	hxrandom rng(1);
	uint8_t uint8 = 0u;
	uint16_t uint16 = 0u;
	uint32_t uint32 = 0u;
	uint64_t uint64 = 0u;
	uint8 = rng.u8();
	uint16 = rng.u16();
	uint32 = rng();
	uint64 = rng.u64();
	EXPECT_NE((static_cast<uint64_t>(uint8) * static_cast<uint64_t>(uint16) *
		static_cast<uint64_t>(uint32) * static_cast<uint64_t>(uint64)), 0u);
	for(int s=100; s-- != 0;) {
		const float f = rng.f01();
		const double d = rng.d01();
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
	const hxsize_t size = hxsize(buffer);
	const hxsize_t read_count = size - 2;
	rng.read(buffer, read_count);
	hxrandom verifier(0x654321u);
	hxsize_t remaining = read_count;
	const uint8_t* expected = buffer;
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
	for(hxsize_t i = read_count; i < size; ++i) {
		EXPECT_EQ(buffer[i], 0xefu);
	}
}

TEST(hxrandom_test, read_exactly_four_bytes) {
	uint8_t buf[5] = { 0xefu, 0xefu, 0xefu, 0xefu, 0xefu };
	hxrandom rng(0xabcdu);
	rng.read(buf, 4);
	hxrandom verifier(0xabcdu);
	const uint32_t word = verifier.u32();
	EXPECT_EQ(buf[0], static_cast<uint8_t>(word));
	EXPECT_EQ(buf[1], static_cast<uint8_t>(word >> 8));
	EXPECT_EQ(buf[2], static_cast<uint8_t>(word >> 16));
	EXPECT_EQ(buf[3], static_cast<uint8_t>(word >> 24));
	EXPECT_EQ(buf[4], 0xefu);
}

TEST(hxrandom_test, read_exactly_one_byte) {
	uint8_t buf[2] = { 0xefu, 0xefu };
	hxrandom rng(0x1111u);
	rng.read(buf, 1);
	hxrandom verifier(0x1111u);
	const uint32_t word = verifier.u32();
	EXPECT_EQ(buf[0], static_cast<uint8_t>(word));
	EXPECT_EQ(buf[1], 0xefu);
}

TEST(hxrandom_test, read_exactly_eight_bytes) {
	uint8_t buf[9];
	for(hxsize_t i = 0; i < 9; ++i) { buf[i] = 0xefu; }
	hxrandom rng(0xdeadu);
	rng.read(buf, 8);
	hxrandom verifier(0xdeadu);
	const uint32_t w0 = verifier.u32();
	const uint32_t w1 = verifier.u32();
	EXPECT_EQ(buf[0], static_cast<uint8_t>(w0));
	EXPECT_EQ(buf[1], static_cast<uint8_t>(w0 >> 8));
	EXPECT_EQ(buf[2], static_cast<uint8_t>(w0 >> 16));
	EXPECT_EQ(buf[3], static_cast<uint8_t>(w0 >> 24));
	EXPECT_EQ(buf[4], static_cast<uint8_t>(w1));
	EXPECT_EQ(buf[5], static_cast<uint8_t>(w1 >> 8));
	EXPECT_EQ(buf[6], static_cast<uint8_t>(w1 >> 16));
	EXPECT_EQ(buf[7], static_cast<uint8_t>(w1 >> 24));
	EXPECT_EQ(buf[8], 0xefu);
}

TEST(hxrandom_test, f01_bounds) {
	hxrandom rng(99999u);
	float max_val = 0.0f;
	float min_val = 1.0f;
	for(int i = 10000; i-- != 0;) {
		const float f = rng.f01();
		if(f > max_val) { max_val = f; }
		if(f < min_val) { min_val = f; }
	}
	EXPECT_LT(max_val, 1.0f);
	EXPECT_GE(min_val, 0.0f);
	EXPECT_GT(max_val, 0.5f);
}

TEST(hxrandom_test, d01_bounds) {
	hxrandom rng(77777u);
	double max_val = 0.0;
	double min_val = 1.0;
	for(int i = 10000; i-- != 0;) {
		const double d = rng.d01();
		if(d > max_val) { max_val = d; }
		if(d < min_val) { min_val = d; }
	}
	EXPECT_LT(max_val, 1.0);
	EXPECT_GE(min_val, 0.0);
	EXPECT_GT(max_val, 0.5);
}

TEST(hxrandom_test, range) {
	hxrandom rng(30000);
	for(int s=100; s-- != 0;) {
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
		EXPECT_TRUE(rng() | rng());
	}
}

TEST(hxrandom_test, histogram) {
	const hxsystem_allocator_scope temporary_stack_scope(hxsystem_allocator_temporary_stack);
	hxrandom rng(40000);
	constexpr int buckets = 1 << 10;
	constexpr int iters = 1000;
	constexpr int max = 1100;
	hxarray<int, buckets> hist(0);
	for(int i=(buckets*iters); i-- != 0;) {
		++hist[static_cast<hxsize_t>(rng() % (buckets - 1))];
	}
	for(hxsize_t i=buckets; i-- != 0u;) {
		EXPECT_LE(hist[i], max);
	}
}

TEST(hxrandom_test, histogram_f) {
	const hxsystem_allocator_scope temporary_stack_scope(hxsystem_allocator_temporary_stack);
	hxrandom rng(40000);
	constexpr int buckets = 1000;
	constexpr int iters = 1000;
	constexpr int max = 1150;
	hxarray<int, buckets> hist(0);
	for(int i=(buckets*iters); i-- != 0;) {
		++hist[static_cast<hxsize_t>(rng.range(0.0, static_cast<double>(buckets)))];
	}
	for(hxsize_t i=buckets; i-- != 0u;) {
		EXPECT_LE(hist[i], max);
	}
}
#if HX_CPLUSPLUS >= 202302L
namespace {

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
} // namespace
#endif // HX_CPLUSPLUS >= 202302L
