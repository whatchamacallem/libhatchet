// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

#include <hx/hxbitset.hpp>
#include <hx/hxslot_map.hpp>
#include <hx/hxrandom.hpp>
#include <hx/hxvector.hpp>
#include "./hxtest_util.hpp"

HX_NS_USE

hxattr_noinline static void hxtest_gdb_break_hxslot_map(void) { }

using hxslot_map_f = hxtest_object_fixture;

#if HX_CPLUSPLUS >= 202302L
TEST_F(hxslot_map_f, value_or_emplaces_fallback) {
	hxslot_map<hxtest_object, 2> m;
	const hxhandle_t handle = m.emplace(10);
	EXPECT_EQ(m.value_or(handle, 14, 17).value(), 10);
	EXPECT_EQ(m.value_or(handle + static_cast<hxhandle_t>(4), 14, 17).value(), 31);
	EXPECT_TRUE(check_stats(3, 2, 0, 2, 1, 0, 0, 0, 0, 0));
}
#endif // HX_CPLUSPLUS >= 202302L

TEST_F(hxslot_map_f, hxslot_map_construction_fixed) {
	const hxslot_map<hxtest_object, 2> m;
	EXPECT_EQ(m.capacity(), hxsize_t{3});
	EXPECT_EQ(m.max_size(), hxsize_t{3});
	EXPECT_EQ(m.size(), hxsize_t{0});
	EXPECT_TRUE(m.empty());
	EXPECT_FALSE(m.full());
	EXPECT_NE(m.data(), static_cast<const hxtest_object*>(hxnull));
	EXPECT_TRUE(check_no_stats());
}

TEST_F(hxslot_map_f, hxslot_map_full) {
	hxslot_map<hxtest_object, 2> m;
	EXPECT_FALSE(m.full());
	m.insert(0);
	EXPECT_FALSE(m.full());
	m.insert(1);
	EXPECT_FALSE(m.full());
	const hxhandle_t h2 = m.insert(2);
	EXPECT_TRUE(m.full());
	EXPECT_TRUE(m.reset(h2));
	EXPECT_FALSE(m.full());
	EXPECT_TRUE(check_stats(3, 1, 0, 3, 0, 0, 0, 0, 0, 0));
}

TEST_F(hxslot_map_f, hxslot_map_construction_dynamic) {
	const hxslot_map<hxtest_object> m;
	EXPECT_EQ(m.capacity(), hxsize_t{0});
	EXPECT_EQ(m.max_size(), hxsize_t{0});
	EXPECT_EQ(m.size(), hxsize_t{0});
	EXPECT_TRUE(m.empty());
	EXPECT_TRUE(m.full());
	EXPECT_TRUE(check_no_stats());
}

TEST_F(hxslot_map_f, hxslot_map_insert_and_get) {
	hxslot_map<hxtest_object, 2> a;
	const hxhandle_t h0 = a.insert(0);
	EXPECT_NE(h0, static_cast<hxhandle_t>(0));
	EXPECT_FALSE(a.empty());
	EXPECT_EQ(a.size(), hxsize_t{1});
	const hxhandle_t h1 = a.insert(1);
	const hxhandle_t h2 = a.insert(2);
	EXPECT_EQ(a.size(), hxsize_t{3});
	EXPECT_NE(h0, h1); EXPECT_NE(h0, h2); EXPECT_NE(h1, h2);

	hxtest_object* const v0 = a.value(h0);
	hxtest_object* const v1 = a.value(h1);
	hxtest_object* const v2 = a.value(h2);
	ASSERT_NE(v0, a.end());
	ASSERT_NE(v1, a.end());
	ASSERT_NE(v2, a.end());
	EXPECT_EQ(v0->value(), (int32_t)0);
	EXPECT_EQ(v1->value(), (int32_t)1);
	EXPECT_EQ(v2->value(), (int32_t)2);

	const hxslot_map<hxtest_object, 2>& cm = a;
	const hxtest_object* const cv1 = cm.value(h1);
	ASSERT_NE(cv1, cm.end());
	EXPECT_EQ(cv1->value(), (int32_t)1);
	const hxslot_map<hxtest_object> b;
	hxtest_gdb_break_hxslot_map();
	EXPECT_TRUE(check_stats(3, 0, 0, 3, 0, 0, 0, 0, 0, 0));
}

TEST_F(hxslot_map_f, hxslot_map_value_wrong_handle_returns_end) {
	hxslot_map<hxtest_object, 2> m;
	const hxhandle_t h0 = m.insert(5);
	const hxhandle_t stale = h0 + static_cast<hxhandle_t>(4);
	EXPECT_EQ(m.value(stale), m.end());
	const hxslot_map<hxtest_object, 2>& cm = m;
	EXPECT_EQ(cm.value(stale), cm.end());
	EXPECT_TRUE(check_stats(1, 0, 0, 1, 0, 0, 0, 0, 0, 0));
}

TEST_F(hxslot_map_f, hxslot_map_reset_stale_handle_returns_false) {
	hxslot_map<hxtest_object, 2> m;
	const hxhandle_t h0 = m.insert(9);
	EXPECT_TRUE(m.reset(h0));
	EXPECT_EQ(m.size(), hxsize_t{0});
	EXPECT_FALSE(m.reset(h0));
	EXPECT_EQ(m.value(h0), m.end());
	EXPECT_TRUE(check_stats(1, 1, 0, 1, 0, 0, 0, 0, 0, 0));
}

TEST_F(hxslot_map_f, hxslot_map_reset_last_value_skips_move) {
	hxslot_map<hxtest_object, 2> m;
	m.insert(0);
	m.insert(1);
	const hxhandle_t h2 = m.insert(2);
	EXPECT_EQ(m.size(), hxsize_t{3});
	EXPECT_TRUE(m.reset(h2));
	EXPECT_EQ(m.size(), hxsize_t{2});
	EXPECT_EQ(*m.data(), 0);
	EXPECT_EQ(*(m.data() + 1), 1);
	EXPECT_TRUE(check_stats(3, 1, 0, 3, 0, 0, 0, 0, 0, 0));
}

TEST_F(hxslot_map_f, hxslot_map_reset_non_last_value_moves_last_into_hole) {
	hxslot_map<hxtest_object, 2> m;
	const hxhandle_t h0 = m.insert(0);
	const hxhandle_t h1 = m.insert(1);
	const hxhandle_t h2 = m.insert(2);
	EXPECT_TRUE(m.reset(h0));
	EXPECT_EQ(m.size(), hxsize_t{2});
	// The last value (2) is moved into the hole left by h0.
	EXPECT_EQ(*m.data(), 2);
	EXPECT_EQ(*(m.data() + 1), 1);
	hxtest_object* const v1 = m.value(h1);
	hxtest_object* const v2 = m.value(h2);
	ASSERT_NE(v1, m.end());
	ASSERT_NE(v2, m.end());
	EXPECT_EQ(v1->value(), (int32_t)1);
	EXPECT_EQ(v2->value(), (int32_t)2);
	// The moved value's backref must be fixed up so it can be erased correctly.
	EXPECT_TRUE(m.reset(h2));
	EXPECT_EQ(m.size(), hxsize_t{1});
	EXPECT_EQ(*m.data(), 1);
	EXPECT_TRUE(m.reset(h1));
	EXPECT_EQ(m.size(), hxsize_t{0});
	EXPECT_TRUE(check_stats(3, 3, 0, 3, 0, 0, 0, 2, 0, 0));
}

TEST_F(hxslot_map_f, hxslot_map_insert_after_erase_reuses_slot_with_new_generation) {
	hxslot_map<hxtest_object, 2> m;
	const hxhandle_t h0 = m.insert(0);
	const hxhandle_t h1 = m.insert(1);
	const hxhandle_t h2 = m.insert(2);
	EXPECT_TRUE(m.reset(h1));
	const hxhandle_t reused = m.insert(11);
	EXPECT_NE(reused, h1);
	EXPECT_EQ(m.value(reused)->value(), (int32_t)11);
	EXPECT_EQ(m.value(h1), m.end());
	EXPECT_NE(m.value(h0), m.end());
	EXPECT_NE(m.value(h2), m.end());
	EXPECT_EQ(m.size(), hxsize_t{3});
	EXPECT_TRUE(check_stats(4, 1, 0, 4, 0, 0, 0, 1, 0, 0));
}

TEST_F(hxslot_map_f, hxslot_map_full_cycle_reuse_across_all_slots) {
	hxslot_map<hxtest_object, 2> m;
	const hxhandle_t h0 = m.insert(0);
	const hxhandle_t h1 = m.insert(1);
	const hxhandle_t h2 = m.insert(2);
	EXPECT_TRUE(m.reset(h0));
	EXPECT_TRUE(m.reset(h1));
	EXPECT_TRUE(m.reset(h2));
	EXPECT_EQ(m.size(), hxsize_t{0});
	EXPECT_TRUE(m.empty());
	const hxhandle_t r0 = m.insert(10);
	const hxhandle_t r1 = m.insert(11);
	const hxhandle_t r2 = m.insert(12);
	EXPECT_EQ(m.size(), hxsize_t{3});
	EXPECT_EQ(m.value(r0)->value(), (int32_t)10);
	EXPECT_EQ(m.value(r1)->value(), (int32_t)11);
	EXPECT_EQ(m.value(r2)->value(), (int32_t)12);
	EXPECT_TRUE(check_stats(6, 3, 0, 6, 0, 0, 0, 1, 0, 0));
}

TEST_F(hxslot_map_f, hxslot_map_clear_on_empty_map_is_noop) {
	hxslot_map<hxtest_object, 2> m;
	m.clear();
	EXPECT_EQ(m.size(), hxsize_t{0});
	EXPECT_TRUE(m.empty());
	EXPECT_TRUE(check_no_stats());
}

TEST_F(hxslot_map_f, hxslot_map_clear_single_value) {
	hxslot_map<hxtest_object, 2> m;
	const hxhandle_t h0 = m.insert(3);
	m.clear();
	EXPECT_EQ(m.size(), hxsize_t{0});
	EXPECT_TRUE(m.empty());
	EXPECT_EQ(m.value(h0), m.end());
	EXPECT_TRUE(check_stats(1, 1, 0, 1, 0, 0, 0, 0, 0, 0));
}

TEST_F(hxslot_map_f, hxslot_map_clear_multiple_values_then_reinsert) {
	hxslot_map<hxtest_object, 2> m;
	const hxhandle_t h0 = m.insert(0);
	const hxhandle_t h1 = m.insert(1);
	const hxhandle_t h2 = m.insert(2);
	m.clear();
	EXPECT_EQ(m.size(), hxsize_t{0});
	EXPECT_TRUE(m.empty());
	EXPECT_EQ(m.value(h0), m.end());
	EXPECT_EQ(m.value(h1), m.end());
	EXPECT_EQ(m.value(h2), m.end());
	const hxhandle_t r0 = m.insert(20);
	const hxhandle_t r1 = m.insert(21);
	const hxhandle_t r2 = m.insert(22);
	EXPECT_EQ(m.size(), hxsize_t{3});
	EXPECT_EQ(m.value(r0)->value(), (int32_t)20);
	EXPECT_EQ(m.value(r1)->value(), (int32_t)21);
	EXPECT_EQ(m.value(r2)->value(), (int32_t)22);
	EXPECT_TRUE(check_stats(6, 3, 0, 6, 0, 0, 0, 0, 0, 0));
}

TEST_F(hxslot_map_f, hxslot_map_destructor_destroys_all_values) {
	{
		hxslot_map<hxtest_object, 2> m1;
		m1.insert(31);

		hxslot_map<hxtest_object, 2> m2;
		m2.insert(32);
		m2.insert(33);
		m2.insert(34);
	}
	EXPECT_TRUE(check_stats(4, 4, 0, 4, 0, 0, 0, 0, 0, 0));
}

TEST_F(hxslot_map_f, hxslot_map_set_size_bits_allocates) {
	hxslot_map<hxtest_object> m;
	m.set_size_bits(2u);
	EXPECT_EQ(m.capacity(), hxsize_t{3});
	EXPECT_EQ(m.size(), hxsize_t{0});
	const hxhandle_t h0 = m.insert(30);
	const hxhandle_t h1 = m.insert(31);
	const hxhandle_t h2 = m.insert(32);
	EXPECT_EQ(m.size(), hxsize_t{3});
	EXPECT_EQ(m.value(h0)->value(), (int32_t)30);
	EXPECT_TRUE(m.reset(h1));
	EXPECT_EQ(m.size(), hxsize_t{2});
	EXPECT_NE(m.value(h0), m.end());
	EXPECT_NE(m.value(h2), m.end());
	EXPECT_TRUE(check_stats(3, 1, 0, 3, 0, 0, 0, 1, 0, 0));
}

TEST_F(hxslot_map_f, hxslot_map_handle_at_returns_matching_handle) {
	hxslot_map<hxtest_object, 2> m;
	const hxhandle_t h0 = m.insert(0);
	const hxhandle_t h1 = m.insert(1);
	const hxhandle_t h2 = m.insert(2);
	EXPECT_EQ(m.handle_at(hxsize_t{0}), h0);
	EXPECT_EQ(m.handle_at(hxsize_t{1}), h1);
	EXPECT_EQ(m.handle_at(hxsize_t{2}), h2);
	EXPECT_TRUE(check_stats(3, 0, 0, 3, 0, 0, 0, 0, 0, 0));
}

TEST_F(hxslot_map_f, hxslot_map_handle_at_tracks_erase_hole_fill) {
	hxslot_map<hxtest_object, 2> m;
	const hxhandle_t h0 = m.insert(0);
	const hxhandle_t h1 = m.insert(1);
	const hxhandle_t h2 = m.insert(2);
	EXPECT_TRUE(m.reset(h0));
	// The last value (2) is moved into the hole left by h0.
	EXPECT_EQ(m.handle_at(hxsize_t{0}), h2);
	EXPECT_EQ(m.handle_at(hxsize_t{1}), h1);
	EXPECT_TRUE(check_stats(3, 1, 0, 3, 0, 0, 0, 1, 0, 0));
}

TEST_F(hxslot_map_f, hxslot_map_erase_if_removes_matching_values) {
	hxslot_map<hxtest_object, 4> m;
	m.insert(0);
	m.insert(1);
	m.insert(2);
	m.insert(3);
	const hxhandle_t h4 = m.insert(4);
	int visits = 0;
	const hxsize_t removed = m.erase_if([&](const hxtest_object& x) -> bool {
		++visits;
		return (x.value() % 2) == 0;
	});
	EXPECT_EQ(visits, 5);
	EXPECT_EQ(removed, hxsize_t{3});
	EXPECT_EQ(m.size(), hxsize_t{2});
	bool found1 = false, found3 = false;
	for(hxsize_t i = 0; i != m.size(); ++i) {
		if(m.data()[i].value() == 1) { found1 = true; }
		if(m.data()[i].value() == 3) { found3 = true; }
	}
	EXPECT_TRUE(found1);
	EXPECT_TRUE(found3);
	// h4 was erased. Its slot and handle generation must be reusable.
	EXPECT_EQ(m.value(h4), m.end());
	const hxhandle_t r4 = m.insert(14);
	EXPECT_NE(r4, h4);
	EXPECT_EQ(m.value(r4)->value(), (int32_t)14);
	EXPECT_EQ(m.size(), hxsize_t{3});
	EXPECT_TRUE(check_stats(6, 3, 0, 6, 0, 0, 0, 2, 0, 0));
}

TEST_F(hxslot_map_f, hxslot_map_erase_if_none_removed_returns_zero) {
	hxslot_map<hxtest_object, 2> m;
	m.insert(0);
	m.insert(1);
	const hxsize_t removed = m.erase_if([](const hxtest_object&) -> bool { return false; });
	EXPECT_EQ(removed, hxsize_t{0});
	EXPECT_EQ(m.size(), hxsize_t{2});
	EXPECT_TRUE(check_stats(2, 0, 0, 2, 0, 0, 0, 0, 0, 0));
}

TEST_F(hxslot_map_f, hxslot_map_erase_if_all_removed_returns_size) {
	hxslot_map<hxtest_object, 2> m;
	m.insert(0);
	m.insert(1);
	m.insert(2);
	const hxsize_t removed = m.erase_if([](const hxtest_object&) -> bool { return true; });
	EXPECT_EQ(removed, hxsize_t{3});
	EXPECT_EQ(m.size(), hxsize_t{0});
	EXPECT_TRUE(m.empty());
	EXPECT_TRUE(check_stats(3, 3, 0, 3, 0, 0, 0, 0, 0, 0));
}

TEST_F(hxslot_map_f, hxslot_map_erase_if_on_empty_map_is_noop) {
	hxslot_map<hxtest_object, 2> m;
	const hxsize_t removed = m.erase_if([](const hxtest_object&) -> bool { return true; });
	EXPECT_EQ(removed, hxsize_t{0});
	EXPECT_EQ(m.size(), hxsize_t{0});
	EXPECT_TRUE(check_no_stats());
}

TEST_F(hxslot_map_f, hxslot_map_erase_if_removes_last_value_only) {
	hxslot_map<hxtest_object, 2> m;
	m.insert(0);
	m.insert(1);
	const hxhandle_t h2 = m.insert(2);
	const hxsize_t removed = m.erase_if([](const hxtest_object& x) -> bool {
		return x.value() == 2;
	});
	EXPECT_EQ(removed, hxsize_t{1});
	EXPECT_EQ(m.size(), hxsize_t{2});
	EXPECT_EQ(*m.data(), 0);
	EXPECT_EQ(*(m.data() + 1), 1);
	EXPECT_EQ(m.value(h2), m.end());
	EXPECT_TRUE(check_stats(3, 1, 0, 3, 0, 0, 0, 0, 0, 0));
}

TEST_F(hxslot_map_f, hxslot_map_erase_if_matches_histogram_of_random_sequence) {
	static const hxsize_t count = static_cast<hxsize_t>(200);
	static const int num_buckets = 30;
	// Five deleted sequences of length 1-5. Some are adjacent (5-7, 8-9 form
	// one run of 5), some are isolated (0, 20, 25-26).
	static const int starts[5] = { 0, 5, 8, 20, 25 };
	static const int lengths[5] = { 1, 3, 2, 1, 2 };

	hxbitset<static_cast<size_t>(num_buckets)> is_target;
	for(int seq = 0; seq != 5; ++seq) {
		for(int i = 0; i != lengths[seq]; ++i) {
			is_target.set(static_cast<size_t>(starts[seq]) + static_cast<size_t>(i));
		}
	}

	hxslot_map<hxtest_object, 8> m;
	hxrandom rng(777u);

	int histogram[num_buckets] = { };
	for(hxsize_t i = 0; i != count; ++i) {
		const int bucket = rng.range(0, num_buckets);
		++histogram[bucket];
		m.insert(bucket);
	}
	EXPECT_EQ(m.size(), count);

	hxsize_t expected_removed = 0;
	for(int bucket = 0; bucket != num_buckets; ++bucket) {
		if(is_target.test(static_cast<size_t>(bucket))) {
			expected_removed += static_cast<hxsize_t>(histogram[bucket]);
		}
	}

	const hxsize_t removed = m.erase_if([&](const hxtest_object& x) -> bool {
		return is_target.test(static_cast<size_t>(x.value()));
	});
	EXPECT_EQ(removed, expected_removed);
	EXPECT_EQ(m.size(), count - expected_removed);

	int remaining[num_buckets] = { };
	for(hxsize_t i = 0; i != m.size(); ++i) {
		++remaining[m.data()[i].value()];
	}
	for(int bucket = 0; bucket != num_buckets; ++bucket) {
		if(is_target.test(static_cast<size_t>(bucket))) {
			EXPECT_EQ(remaining[bucket], 0);
		}
		else {
			EXPECT_EQ(remaining[bucket], histogram[bucket]);
		}
	}
	EXPECT_TRUE(check_stats(200, 58, 0, 200, 0, 0, 0, 58, 0, 0));
}

TEST_F(hxslot_map_f, hxslot_map_random_mutation_stays_valid) {
	struct hxlive_entry_test_t {
		hxhandle_t handle;
		int value;
	};

	static const hxsize_t capacity = static_cast<hxsize_t>(15);
	hxslot_map<hxtest_object, 4> m;
	hxvector<hxlive_entry_test_t> live;
	live.reserve(capacity);
	hxrandom rng(12345u);
	int next_value = 1;

	for(int step = 0; step < 20000; ++step) {
		const bool should_insert = live.empty()
			|| (live.size() < capacity && rng.range(0, 2) == 0);
		if(should_insert) {
			const int value = next_value++;
			const hxhandle_t h = m.insert(value);
			live.push_back(hxlive_entry_test_t{h, value});
		}
		else {
			const hxsize_t victim = static_cast<hxsize_t>(
				rng.range<uint32_t>(0u, static_cast<uint32_t>(live.size())));
			const hxhandle_t h = live[victim].handle;
			EXPECT_TRUE(m.reset(h));
			EXPECT_EQ(m.value(h), m.end());
			live[victim] = live[live.size() - 1];
			live.pop_back();
		}

		ASSERT_EQ(m.size(), live.size());
		ASSERT_EQ(m.empty(), live.empty());
		for(hxsize_t i = 0; i != live.size(); ++i) {
			hxtest_object* const v = m.value(live[i].handle);
			ASSERT_NE(v, m.end());
			EXPECT_EQ(v->value(), live[i].value);
		}

		const hxtest_object* const data = m.data();
		for(hxsize_t i = 0; i != live.size(); ++i) {
			bool found = false;
			for(hxsize_t j = 0; j != live.size(); ++j) {
				if(data[i].value() == live[j].value) {
					found = true;
					break;
				}
			}
			EXPECT_TRUE(found);
		}
	}

	for(hxsize_t i = 0; i != live.size(); ++i) {
		EXPECT_TRUE(m.reset(live[i].handle));
	}
	live.clear();
	EXPECT_TRUE(m.empty());
	EXPECT_EQ(m.size(), hxsize_t{0});
	EXPECT_TRUE(check_stats(10002, 10002, 0, 10002, 0, 0, 0, 7744, 0, 0));
}
