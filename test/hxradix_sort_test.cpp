// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

#include <hx/hxradix_sort.hpp>
#include <hx/hxrandom.hpp>
#include <hx/hxalgorithm.hpp>
#include <hx/hxvector.hpp>
#include <hx/hxtest.hpp>

HX_NS_USE

namespace {
class hxradix_sort_test_f :
	public testing::Test
{
public:
	template<typename key_t>
	class hxtest_object {
	public:
		hxtest_object(key_t k) : id(k) { }
		~hxtest_object(void) { id = static_cast<key_t>(0); }
		bool operator<(const hxtest_object& x) const { return id < x.id; }
		key_t id;
	};
	hxradix_sort_test_f(void) : m_temporary_stack_scope(hxsystem_allocator_stack_0) { }
	template<typename key_t>
	void generate(hxvector<hxtest_object<key_t> >& a, uint32_t size, uint32_t mask, key_t offset) {
		a.reserve(static_cast<hxsize_t>(size));
		for(uint32_t i= size;i--;) {
			const uint32_t x = m_prng() & mask;
			a.push_back(static_cast<key_t>(static_cast<key_t>(x) - offset));
		}
	}
	template<typename key_t>
	static int q_sort_compare(const void* a, const void* b) {
		const hxtest_object<key_t>* lhs = static_cast<const hxtest_object<key_t>*>(a);
		const hxtest_object<key_t>* rhs = static_cast<const hxtest_object<key_t>*>(b);
		if(*lhs < *rhs) { return -1; }
		if(*rhs < *lhs) { return 1; }
		return 0;
	}
	template<typename key_t>
	void test_range_and_type(uint32_t size, uint32_t mask, key_t offset) {
		const hxsystem_allocator_scope temporary_stack_scope_2(hxsystem_allocator_stack_0);
		hxvector<hxtest_object<key_t> > a;
		generate<key_t>(a, size, mask, offset);
		hxvector<hxtest_object<key_t> > b(a);
		::qsort(b.data(), static_cast<size_t>(b.size()), sizeof(hxtest_object<key_t>), q_sort_compare<key_t>);
		hxvector<hxradix_sort_key<key_t, hxtest_object<key_t>*>> rs; rs.reserve(static_cast<hxsize_t>(size));
		for(uint32_t i = size; i--;) {
			rs.emplace_back(a[static_cast<hxsize_t>(i)].id, &a[static_cast<hxsize_t>(i)]);
		}
		hxradix_sort(rs.begin(), rs.end(), hxsystem_allocator_stack_0);
		EXPECT_EQ(b.size(), static_cast<hxsize_t>(size));
		EXPECT_EQ(rs.size(), static_cast<hxsize_t>(size));
		for(uint32_t i=0u; i < size; ++i) {
			EXPECT_EQ(b[static_cast<hxsize_t>(i)].id, rs[static_cast<hxsize_t>(i)].get_value()->id);
		}
		rs.clear();
		for(uint32_t i = size; i--;) {
			rs.push_back(hxradix_sort_key<key_t, hxtest_object<key_t>*>(a[static_cast<hxsize_t>(i)].id, &a[static_cast<hxsize_t>(i)]));
		}
		hxradix_sort11(rs.begin(), rs.end(), hxsystem_allocator_stack_0);
		EXPECT_EQ(b.size(), static_cast<hxsize_t>(size));
		EXPECT_EQ(rs.size(), static_cast<hxsize_t>(size));
		for(uint32_t i=0u; i < size; ++i) {
			EXPECT_EQ(b[static_cast<hxsize_t>(i)].id, rs[static_cast<hxsize_t>(i)].get_value()->id);
		}
	}
	hxsystem_allocator_scope m_temporary_stack_scope;
	hxrandom m_prng;
};
} // namespace {

TEST_F(hxradix_sort_test_f, null) {
	hxvector<hxradix_sort_key<uint32_t, const char*>> rs;
	rs.reserve(1);
	hxradix_sort(rs.begin(), rs.end(), hxsystem_allocator_stack_0);
	EXPECT_EQ(rs.size(), 0u);
	EXPECT_TRUE(rs.empty());
	rs.push_back(hxradix_sort_key<uint32_t, const char*>(123u, "s"));
	hxradix_sort(rs.begin(), rs.end(), hxsystem_allocator_stack_0);
	EXPECT_EQ(rs.size(), 1);
	EXPECT_EQ(rs[0].get_value()[0], 's');
	EXPECT_FALSE(rs.empty());
}

TEST_F(hxradix_sort_test_f, null11) {
	hxvector<hxradix_sort_key<uint32_t, const char*>> rs;
	rs.reserve(1);
	hxradix_sort11(rs.begin(), rs.end(), hxsystem_allocator_stack_0);
	EXPECT_EQ(rs.size(), 0u);
	EXPECT_TRUE(rs.empty());
	rs.push_back(hxradix_sort_key<uint32_t, const char*>(123u, "s"));
	hxradix_sort11(rs.begin(), rs.end(), hxsystem_allocator_stack_0);
	EXPECT_EQ(rs.size(), 1);
	EXPECT_EQ(rs[0].get_value()[0], 's');
	EXPECT_FALSE(rs.empty());
}

TEST_F(hxradix_sort_test_f, uint32) {
	test_range_and_type<uint32_t>(20u, 0x7fu, 0u);
	test_range_and_type<uint32_t>(100u, 0x7fu, 0u);
	test_range_and_type<uint32_t>(1000u, 0x7fffu, 0u);
	test_range_and_type<uint32_t>(5000u, ~static_cast<uint32_t>(0), 0u);
}

TEST_F(hxradix_sort_test_f, int32) {
	test_range_and_type<int32_t>(20u, 0x7fu, 0x3f);
	test_range_and_type<int32_t>(100u, 0x7fu, 0x3f);
	test_range_and_type<int32_t>(1000u, 0x7fffu, 0x3fff);
	test_range_and_type<int32_t>(5000u, ~static_cast<uint32_t>(0), 0);
}

TEST_F(hxradix_sort_test_f, float) {
	test_range_and_type<float>(200u, 0x7fu, static_cast<float>(0x3f));
	test_range_and_type<float>(100u, 0x7fu, static_cast<float>(0x3f));
	test_range_and_type<float>(1000u, 0x7fffu, static_cast<float>(0x3fff));
	test_range_and_type<float>(5000u, ~static_cast<uint32_t>(0), 0.0f);
}

TEST_F(hxradix_sort_test_f, types) {
	test_range_and_type<uint8_t>(100u, 0x7fu, 0x3fu);
	test_range_and_type<int8_t>(100u, 0x7fu, 0x3f);
	test_range_and_type<uint16_t>(100u, 0x7fu, 0x3fu);
	test_range_and_type<int16_t>(100u, 0x7fu, 0x3f);
}

TEST_F(hxradix_sort_test_f, insertion_sort_boundary_31) {
	test_range_and_type<uint32_t>(31u, 0xffu, 0u);
}

TEST_F(hxradix_sort_test_f, radix_sort_boundary_32) {
	test_range_and_type<uint32_t>(32u, 0xffu, 0u);
}

TEST_F(hxradix_sort_test_f, radix_sort_boundary_33) {
	test_range_and_type<uint32_t>(33u, 0xffu, 0u);
}

TEST_F(hxradix_sort_test_f, key_set_overwrites_value) {
	hxradix_sort_key<uint32_t, uint32_t> k(1u, 10u);
	EXPECT_EQ(k.get_value(), 10u);
	k.set(2u, 20u);
	EXPECT_EQ(k.get_value(), 20u);
	const hxradix_sort_key<uint32_t, uint32_t>& ck = k;
	EXPECT_EQ(ck.get_value(), 20u);
	EXPECT_EQ(ck.get_modified_key(), 2u);
}

TEST_F(hxradix_sort_test_f, key_set_preserves_relative_order) {
	hxradix_sort_key<int32_t, uint32_t> low(-1, 100u);
	const hxradix_sort_key<int32_t, uint32_t> high(1, 200u);
	EXPECT_TRUE(low < high);
	EXPECT_FALSE(high < low);
	low.set(2, 300u);
	EXPECT_FALSE(low < high);
	EXPECT_TRUE(high < low);
	EXPECT_EQ(low.get_value(), 300u);
}

TEST_F(hxradix_sort_test_f, two_elements_sorted) {
	const hxsystem_allocator_scope scope2(hxsystem_allocator_stack_0);
	hxvector<hxradix_sort_key<uint32_t, uint32_t>> rs;
	rs.reserve(2);
	rs.push_back(hxradix_sort_key<uint32_t, uint32_t>(2u, 2u));
	rs.push_back(hxradix_sort_key<uint32_t, uint32_t>(1u, 1u));
	hxradix_sort(rs.begin(), rs.end(), hxsystem_allocator_stack_0);
	EXPECT_EQ(rs[0].get_value(), 1u);
	EXPECT_EQ(rs[1].get_value(), 2u);
}

TEST_F(hxradix_sort_test_f, two_elements_sorted11) {
	const hxsystem_allocator_scope scope2(hxsystem_allocator_stack_0);
	hxvector<hxradix_sort_key<uint32_t, uint32_t>> rs;
	rs.reserve(2);
	rs.push_back(hxradix_sort_key<uint32_t, uint32_t>(2u, 2u));
	rs.push_back(hxradix_sort_key<uint32_t, uint32_t>(1u, 1u));
	hxradix_sort11(rs.begin(), rs.end(), hxsystem_allocator_stack_0);
	EXPECT_EQ(rs[0].get_value(), 1u);
	EXPECT_EQ(rs[1].get_value(), 2u);
}

TEST_F(hxradix_sort_test_f, all_256_low_byte_values) {
	const hxsystem_allocator_scope scope2(hxsystem_allocator_stack_0);
	hxvector<hxradix_sort_key<uint32_t, uint32_t>> rs;
	rs.reserve(256);
	for(uint32_t i = 0u; i < 256u; ++i) {
		rs.push_back(hxradix_sort_key<uint32_t, uint32_t>(255u - i, 255u - i));
	}
	hxradix_sort(rs.begin(), rs.end(), hxsystem_allocator_stack_0);
	for(uint32_t i = 0u; i < 256u; ++i) {
		EXPECT_EQ(rs[static_cast<hxsize_t>(i)].get_value(), i);
	}
}

TEST_F(hxradix_sort_test_f, all_256_low_byte_values11) {
	const hxsystem_allocator_scope scope2(hxsystem_allocator_stack_0);
	hxvector<hxradix_sort_key<uint32_t, uint32_t>> rs;
	rs.reserve(256);
	for(uint32_t i = 0u; i < 256u; ++i) {
		rs.push_back(hxradix_sort_key<uint32_t, uint32_t>(255u - i, 255u - i));
	}
	hxradix_sort11(rs.begin(), rs.end(), hxsystem_allocator_stack_0);
	for(uint32_t i = 0u; i < 256u; ++i) {
		EXPECT_EQ(rs[static_cast<hxsize_t>(i)].get_value(), i);
	}
}

TEST_F(hxradix_sort_test_f, four_pass_upper_bytes_differ) {
	const hxsystem_allocator_scope scope2(hxsystem_allocator_stack_0);
	hxvector<hxradix_sort_key<uint32_t, uint32_t>> rs;
	rs.reserve(32);
	for(uint32_t i = 0u; i < 32u; ++i) {
		const uint32_t key = ((i & 1u) != 0u) ? 0xff000000u + i : i;
		rs.push_back(hxradix_sort_key<uint32_t, uint32_t>(key, key));
	}
	hxradix_sort(rs.begin(), rs.end(), hxsystem_allocator_stack_0);
	for(hxsize_t i = 1; i < rs.size(); ++i) {
		EXPECT_LE(rs[i - 1].get_value(), rs[i].get_value());
	}
}
