#pragma once
// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

/// \file
/// A 64-bit LCG random number generator useful for test data.

#include "libhatchet.h"

// HX_USE_MACROS_WITH_MODULE allows including macros alongside the module.
#if HX_USE_MACROS_WITH_MODULE
#error Header does not provide macros alone.
#endif

HX_NS_BEGIN_

/// `hxrandom` - 64-bit MMIX LCG. Knuth, D. 2002. (Modified to perturb the
/// return value so that all bits are of equal quality.) Uses a floating point
/// multiply instead of integer modulo when generating numbers in a range.
/// Requires at least 64-bit integer emulation as well. Has a period of `2^64`
/// and passes routine numerical tests with only eight bytes of state while
/// using simple arithmetic. Intended for test data or games, not mathematical
/// applications.
class hxrandom {
public:
	/// Constructor to initialize the random number generator.
	/// - `stream` : Index or seed value for a given stream of random numbers.
	hxinline hxconstexpr hxrandom(uint64_t stream_ = 31u) : m_state_(stream_) { }

	/// Returns [0..2^32).
	hxattr_nodiscard hxinline hxconstexpr
	uint32_t operator()(void) { return this->u32(); }

	/// Returns a random number in the range [base..base+range).
	/// `range(0.0f,10.0f)` returns `0.0f` to `9.999f` and not `10.0f`. Uses a
	/// floating point multiply instead of a divide. `base + size` must not
	/// overflow the type and `size` must be positive.
	/// - `base` : The beginning of the range. e.g., 0.
	/// - `size` : Positive size of the range. e.g., 10 elements.
	template<typename T_>
	hxattr_nodiscard hxinline hxconstexpr
	T_ range(T_ base_, T_ size_) {
		// Use double parameters if you need a bigger size. An emulated
		// floating point multiply is faster and more stable than integer modulo.
		hxassertf(static_cast<float>(size_) < float{0x01000000u},
			"low_precision %f", static_cast<float>(size_)); // 0x1p24f
		return base_ + static_cast<T_>(static_cast<float>(size_) * this->f01());
	}

	/// double version.
	hxattr_nodiscard hxinline hxconstexpr
	double range(double base_, double size_) {
		// Use `uint64_t` parameters if you need a bigger size. An emulated
		// floating point multiply is faster and more stable than integer modulo.
		hxassertf(size_ < double{0x20000000000000ll}, "low_precision %f", size_); // 0x1p53
		return base_ + size_ * this->d01();
	}

	/// int64_t version. Negative or zero size is undefined.
	hxattr_nodiscard hxinline hxconstexpr
	int64_t range(int64_t base_, int64_t size_) {
		hxassertf(size_ > 0, "bad_range %zd", static_cast<hxsize_t>(size_));
		return base_ + static_cast<int64_t>(this->u64() % static_cast<uint64_t>(size_));
	}

	/// uint64_t version. Zero size is undefined.
	hxattr_nodiscard hxinline hxconstexpr
	uint64_t range(uint64_t base_, uint64_t size_) {
		hxassertf(size_ != 0u, "bad_range");
		return base_ + this->u64() % size_;
	}

	/// Reads a specified number of random bytes into the provided buffer.
	/// - `bytes` : Non-null pointer to a buffer large enough for `count` bytes.
	/// - `count` : Number of bytes to read.
	hxinline void read(void* bytes_, hxsize_t count_) hxattr_nonnull(2) {
		uint8_t* chars_ = static_cast<uint8_t*>(bytes_);
		while (count_ >= 4) {
			const uint32_t x_ = this->u32();
			::memcpy(chars_, &x_, 4u);
			chars_ += 4;
			count_ -= 4;
		}
		if (count_ > 0) {
			const uint32_t x_ = this->u32();
			::memcpy(chars_, &x_, static_cast<size_t>(count_));
		}
	}

	/// Returns [0..2^8).
	hxattr_nodiscard hxinline hxconstexpr
	uint8_t u8(void) {
		return static_cast<uint8_t>(this->u32());
	}

	/// Returns [0..2^16).
	hxattr_nodiscard hxinline hxconstexpr
	uint16_t u16(void) {
		return static_cast<uint16_t>(this->u32());
	}

	/// Returns [0..2^32).
	hxattr_nodiscard hxinline hxconstexpr
	uint32_t u32(void) {
		m_state_ = uint64_t{0x5851f42d4c957f2dull} * m_state_ + uint64_t{0x14057b7ef767814full};

		// MODIFICATION: Use the 4 msb bits as a random 0..15 bit variable shift
		// control. Ignores the low 13 bits because they are low quality.
		// Returns 32 bits chosen at a random offset starting between the 13th
		// and 28th bits. 4 bits shift control + 32 returned + up to 15 shifted
		// off + 13 always discarded = 64 bits.
		const uint32_t result_ = static_cast<uint32_t>(m_state_ >> ((m_state_ >> 60) + 13u));
		return result_;
	}

	/// Returns [0..2^64).
	hxattr_nodiscard hxinline hxconstexpr
	uint64_t u64(void) {
		const uint64_t result_ = this->u32() | (static_cast<uint64_t>(this->u32()) << 32);
		return result_;
	}

	/// Returns a float between `[0..1)`. Can safely be used to generate array
	/// indices without overflowing.
	hxattr_nodiscard hxinline hxconstexpr
	float f01(void) {
		// Shift avoids rounding up.
		return static_cast<float>(this->u32() >> 8) * (1.0f / 16777216.0f); // 0x1p-24f
	}

	/// Returns a double between `[0..1)`. Can safely be used to generate array
	/// indices without overflowing.
	hxattr_nodiscard hxinline hxconstexpr
	double d01(void) {
		// Shift avoids rounding up.
		return static_cast<double>(this->u64() >> 11) * (1.0 / 9007199254740992.0); // 0x1p-53
	}

private:
	uint64_t m_state_;
};

HX_NS_END_
