// SPDX-FileCopyrightText: © 2025 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

#include <hx/hxdeque.hpp>
#include <hx/hxtest.hpp>

HX_NS_USE

hxattr_noinline static void hxtest_gdb_break_hxdeque(void) {}

namespace {

// Lifecycle tracking helper reused across fixture-based tests.

class hxdeque_test_f* hxs_deque_current = hxnull;

class hxdeque_test_f : public testing::Test {
public:
	class hxtest_object {
	public:
		hxtest_object(void) {
			++hxs_deque_current->m_constructed;
			id = hxs_deque_current->m_next_id--;
			moved_from = false;
		}
		explicit hxtest_object(int32_t x) {
			++hxs_deque_current->m_constructed;
			id = x;
			moved_from = false;
		}
		hxtest_object(const hxtest_object& x) {
			++hxs_deque_current->m_constructed;
			id = x.id;
			moved_from = false;
		}
		hxtest_object(hxtest_object&& x) {
			++hxs_deque_current->m_constructed;
			id = x.id;
			moved_from = false;
			x.id = 0xefef;
			x.moved_from = true;
		}
		~hxtest_object(void) {
			++hxs_deque_current->m_destructed;
			id = 0xefef;
			moved_from = true;
		}
		void operator=(const hxtest_object& x) {
			id = x.id;
			moved_from = false;
		}
		hxtest_object& operator=(hxtest_object&& x) {
			id = x.id;
			moved_from = false;
			x.id = 0xefef;
			x.moved_from = true;
			return *this;
		}
		bool operator==(int32_t x) const { return id == x; }
		int32_t id;
		bool moved_from;
	};

	hxdeque_test_f(void) {
		hxassert(hxs_deque_current == hxnull);
		m_constructed = 0u;
		m_destructed = 0u;
		m_next_id = -1;
		hxs_deque_current = this;
	}
	~hxdeque_test_f(void) {
		hxassert_always(m_constructed == m_destructed,
			"hxdeque_test_f lifecycle mismatch");
		hxs_deque_current = hxnull;
	}

	size_t m_constructed;
	size_t m_destructed;
	int32_t m_next_id;
};

} // namespace {

// Static-capacity construction and initial state

TEST(hxdeque_test, static_ctor_empty_state) {
	const hxdeque<int, 4u> d;
	EXPECT_EQ(d.size(), 0u);
	EXPECT_EQ(d.capacity(), 4u);
	EXPECT_TRUE(d.empty());
	EXPECT_FALSE(d.full());
}

TEST(hxdeque_test, dynamic_ctor_empty_state) {
	const hxsystem_allocator_scope temporary_stack_scope(hxsystem_allocator_temporary_stack);
	const hxdeque<int> d(4u);
	EXPECT_EQ(d.size(), 0u);
	EXPECT_EQ(d.capacity(), 4u);
	EXPECT_TRUE(d.empty());
	EXPECT_FALSE(d.full());
}

// push_back / pop_front: FIFO ordering

TEST(hxdeque_test, push_back_pop_front_fifo_order) {
	hxdeque<int, 4u> d;
	d.push_back(10);
	d.push_back(20);
	d.push_back(30);
	EXPECT_EQ(d.size(), 3u);
	EXPECT_FALSE(d.empty());

	int v = 0;
	v = d.front(); d.pop_front(); EXPECT_EQ(v, 10);
	v = d.front(); d.pop_front(); EXPECT_EQ(v, 20);
	v = d.front(); d.pop_front(); EXPECT_EQ(v, 30);
	EXPECT_EQ(d.size(), 0u);
	EXPECT_TRUE(d.empty());
}

// push_front / pop_back: LIFO-like ordering

TEST(hxdeque_test, push_front_pop_back_order) {
	hxdeque<int, 4u> d;
	d.push_front(10);
	d.push_front(20);
	d.push_front(30);
	EXPECT_EQ(d.size(), 3u);

	int v = 0;
	v = d.back(); d.pop_back(); EXPECT_EQ(v, 10);
	v = d.back(); d.pop_back(); EXPECT_EQ(v, 20);
	v = d.back(); d.pop_back(); EXPECT_EQ(v, 30);
	EXPECT_TRUE(d.empty());
}

// Ring-buffer wraparound: head and tail cross the physical end of storage

TEST(hxdeque_test, ring_wraparound_push_back_pop_front) {
	// Capacity 4 (mask 3). Fill, drain 3, fill 3 more so tail wraps around.
	hxdeque<int, 4u> d;
	d.push_back(1);
	d.push_back(2);
	d.push_back(3);
	d.push_back(4);
	int v = 0;
	v = d.front(); d.pop_front(); EXPECT_EQ(v, 1);
	v = d.front(); d.pop_front(); EXPECT_EQ(v, 2);
	v = d.front(); d.pop_front(); EXPECT_EQ(v, 3);
	// head is now at physical slot 3, tail at 4 (slot 0 after mask)
	d.push_back(10);
	d.push_back(11);
	d.push_back(12);
	// Logical order must be 4, 10, 11, 12
	v = d.front(); d.pop_front(); EXPECT_EQ(v, 4);
	v = d.front(); d.pop_front(); EXPECT_EQ(v, 10);
	v = d.front(); d.pop_front(); EXPECT_EQ(v, 11);
	v = d.front(); d.pop_front(); EXPECT_EQ(v, 12);
	EXPECT_TRUE(d.empty());
}

TEST(hxdeque_test, ring_wraparound_push_front_pop_back) {
	// Capacity 4. Push from front so head moves backwards, crossing slot 0.
	hxdeque<int, 4u> d;
	d.push_front(4);
	d.push_front(3);
	d.push_front(2);
	d.push_front(1);
	int v = 0;
	v = d.back(); d.pop_back(); EXPECT_EQ(v, 4);
	v = d.back(); d.pop_back(); EXPECT_EQ(v, 3);
	d.push_front(0);
	d.push_front(-1);
	// Logical order: -1, 0, 1, 2
	EXPECT_EQ(d[0], -1);
	EXPECT_EQ(d[1],  0);
	EXPECT_EQ(d[2],  1);
	EXPECT_EQ(d[3],  2);

	hxtest_gdb_break_hxdeque();
}

// emplace_back / emplace_front: in-place construction

TEST_F(hxdeque_test_f, emplace_back_constructs_in_place) {
	hxdeque<hxtest_object, 4u> d;
	d.emplace_back(42);
	EXPECT_EQ(d.back().id, 42);
	EXPECT_FALSE(d.back().moved_from);
	EXPECT_EQ(d.size(), 1u);
	EXPECT_EQ(m_constructed, 1u);
	EXPECT_EQ(m_destructed, 0u);
}

TEST_F(hxdeque_test_f, emplace_front_constructs_in_place) {
	hxdeque<hxtest_object, 4u> d;
	d.emplace_back(10);
	d.emplace_front(99);
	EXPECT_EQ(d.size(), 2u);
	// Front is the newly emplaced element.
	EXPECT_EQ(d.front().id, 99);
	EXPECT_EQ(d.back().id, 10);
}

// emplace_back with multiple forwarded args (uses variadic path)
TEST_F(hxdeque_test_f, emplace_back_forward_multiple_args) {
	// Use hxtest_pair_t with a two-arg constructor to exercise variadic forwarding.
	struct hxtest_pair_t {
		hxtest_pair_t(int a, int b) : x(a), y(b) {}
		int x, y;
	};
	hxdeque<hxtest_pair_t, 2u> d;
	d.emplace_back(3, 7);
	EXPECT_EQ(d.back().x, 3);
	EXPECT_EQ(d.back().y, 7);
}

// front() / back() accessors: both const and non-const overloads

TEST(hxdeque_test, front_and_back_multiple_elements) {
	hxdeque<int, 4u> d;
	d.push_back(1);
	d.push_back(2);
	d.push_back(3);
	EXPECT_EQ(d.front(), 1);
	EXPECT_EQ(d.back(), 3);
}

TEST(hxdeque_test, const_front_and_back) {
	hxdeque<int, 4u> d;
	d.push_back(7);
	d.push_back(8);
	const hxdeque<int, 4u>& cd = d;
	EXPECT_EQ(cd.front(), 7);
	EXPECT_EQ(cd.back(), 8);
}

// front/back mutability: write through the reference
TEST(hxdeque_test, front_back_reference_mutation) {
	hxdeque<int, 4u> d;
	d.push_back(1);
	d.push_back(2);
	d.front() = 10;
	d.back()  = 20;
	EXPECT_EQ(d[0], 10);
	EXPECT_EQ(d[1], 20);
}

// operator[] and at(): indexing with logical offset

TEST(hxdeque_test, operator_index_sequential) {
	hxdeque<int, 8u> d;
	for(int i = 0; i < 5; ++i) {
		d.push_back(i * 10);
	}
	for(size_t i = 0u; i < 5u; ++i) {
		EXPECT_EQ(d[i], static_cast<int>(i * 10));
	}
}

TEST(hxdeque_test, const_operator_index) {
	hxdeque<int, 4u> d;
	d.push_back(3);
	d.push_back(6);
	const hxdeque<int, 4u>& cd = d;
	EXPECT_EQ(cd[0], 3);
	EXPECT_EQ(cd[1], 6);
}

// Indexing after ring wraparound: logical indices must map correctly.
TEST(hxdeque_test, operator_index_after_wraparound) {
	hxdeque<int, 4u> d;
	d.push_back(1); d.push_back(2);
	d.push_back(3); d.push_back(4);
	d.pop_front(); d.pop_front(); // head is now at slot 2
	d.push_back(5); d.push_back(6); // tail wraps to slots 0, 1
	// Logical: [3, 4, 5, 6]
	EXPECT_EQ(d[0], 3);
	EXPECT_EQ(d[1], 4);
	EXPECT_EQ(d[2], 5);
	EXPECT_EQ(d[3], 6);
}

// clear(): destroys all elements, resets to empty

TEST_F(hxdeque_test_f, clear_destroys_all_elements) {
	hxdeque<hxtest_object, 4u> d;
	d.emplace_back(1);
	d.emplace_back(2);
	d.emplace_back(3);
	EXPECT_EQ(m_constructed, 3u);
	EXPECT_EQ(m_destructed, 0u);
	d.clear();
	EXPECT_EQ(m_destructed, 3u);
	EXPECT_EQ(d.size(), 0u);
	EXPECT_TRUE(d.empty());
}

TEST_F(hxdeque_test_f, clear_empty_deque_is_noop) {
	hxdeque<hxtest_object, 4u> d;
	d.clear(); // must not crash or double-destruct
	EXPECT_EQ(m_destructed, 0u);
}

// clear() iterates using (m_head + i) & m_mask so a wrapped buffer must
// be destroyed correctly.
TEST_F(hxdeque_test_f, clear_after_ring_wraparound_destroys_all) {
	hxdeque<hxtest_object, 4u> d;
	for(int i = 0; i < 4; ++i) { d.emplace_back(i); }
	d.pop_front(); // head advances
	d.pop_front();
	// Push two more so tail wraps around
	d.emplace_back(100);
	d.emplace_back(101);
	EXPECT_EQ(d.size(), 4u);
	const size_t before_clear = m_destructed;
	d.clear();
	// Must have destructed exactly the 4 live elements
	EXPECT_EQ(m_destructed, before_clear + 4u);
	EXPECT_TRUE(d.empty());
}

// Destructor: implicitly calls clear()

TEST_F(hxdeque_test_f, destructor_calls_clear) {
	{
		hxdeque<hxtest_object, 4u> d;
		d.emplace_back(7);
		d.emplace_back(8);
		EXPECT_EQ(m_constructed, 2u);
	} // destructor fires here
	EXPECT_EQ(m_destructed, 2u);
}

// full() / empty() / size(): state predicate coverage

TEST(hxdeque_test, full_predicate) {
	hxdeque<int, 2u> d;
	EXPECT_FALSE(d.full());
	d.push_back(1);
	EXPECT_FALSE(d.full());
	d.push_back(2);
	EXPECT_TRUE(d.full());
	d.pop_front();
	EXPECT_FALSE(d.full());
}

// pop_front / pop_back: destructor is called on removal

TEST_F(hxdeque_test_f, pop_front_destroys_slot) {
	hxdeque<hxtest_object, 4u> d;
	d.emplace_back(55);
	EXPECT_EQ(m_constructed, 1u);

	const size_t d_before = m_destructed;
	d.pop_front();
	EXPECT_EQ(m_destructed, d_before + 1u);
	EXPECT_TRUE(d.empty());
}

TEST_F(hxdeque_test_f, pop_back_destroys_slot) {
	hxdeque<hxtest_object, 4u> d;
	d.emplace_back(77);
	const size_t d_before = m_destructed;
	d.pop_back();
	EXPECT_EQ(m_destructed, d_before + 1u);
	EXPECT_TRUE(d.empty());
}

// Dynamic storage: reserve, use, and destroy

TEST(hxdeque_test, dynamic_capacity_push_pop) {
	const hxsystem_allocator_scope temporary_stack_scope(hxsystem_allocator_temporary_stack);
	hxdeque<int> d(4u);
	EXPECT_EQ(d.capacity(), 4u);
	for(int i = 0; i < 4; ++i) {
		d.push_back(i);
	}
	EXPECT_TRUE(d.full());
	for(int i = 0; i < 4; ++i) {
		const int v = d.front();
		d.pop_front();
		EXPECT_EQ(v, i);
	}
	EXPECT_TRUE(d.empty());
}

TEST(hxdeque_test, dynamic_capacity_ring_wraparound) {
	const hxsystem_allocator_scope temporary_stack_scope(hxsystem_allocator_temporary_stack);
	hxdeque<int> d(8u);
	for(int i = 0; i < 8; ++i) { d.push_back(i); }
	for(int i = 0; i < 4; ++i) { d.pop_front(); }
	for(int i = 8; i < 12; ++i) { d.push_back(i); }
	// Logical: [4, 5, 6, 7, 8, 9, 10, 11]
	for(int i = 0; i < 8; ++i) {
		EXPECT_EQ(d[static_cast<size_t>(i)], i + 4);
	}
}

// reserve(): deferred dynamic allocation

TEST(hxdeque_test, reserve_sets_capacity_and_mask) {
	const hxsystem_allocator_scope temporary_stack_scope(hxsystem_allocator_temporary_stack);
	hxdeque<int> d;
	EXPECT_EQ(d.capacity(), 0u);
	d.reserve(8u);
	EXPECT_EQ(d.capacity(), 8u);
	EXPECT_TRUE(d.empty());
	EXPECT_FALSE(d.full());
}

// Mixed push_back / push_front interleaving

TEST(hxdeque_test, interleaved_push_back_push_front) {
	hxdeque<int, 8u> d;
	d.push_back(3);
	d.push_front(2);
	d.push_back(4);
	d.push_front(1);
	// Logical order: 1, 2, 3, 4
	EXPECT_EQ(d.size(), 4u);
	EXPECT_EQ(d[0], 1);
	EXPECT_EQ(d[1], 2);
	EXPECT_EQ(d[2], 3);
	EXPECT_EQ(d[3], 4);
}

TEST(hxdeque_test, interleaved_pop_back_pop_front) {
	hxdeque<int, 4u> d;
	d.push_back(10);
	d.push_back(20);
	d.push_back(30);
	int v = 0;
	v = d.front(); d.pop_front(); EXPECT_EQ(v, 10);
	v = d.back();  d.pop_back();  EXPECT_EQ(v, 30);
	EXPECT_EQ(d.size(), 1u);
	EXPECT_EQ(d.front(), 20);
	EXPECT_EQ(d.back(), 20);
}

// Single-element capacity (capacity == 1): edge case

TEST(hxdeque_test, single_element_capacity) {
	hxdeque<int, 1u> d;
	EXPECT_TRUE(d.empty());
	EXPECT_EQ(d.capacity(), 1u);
	d.push_back(42);
	EXPECT_TRUE(d.full());
	EXPECT_EQ(d.front(), 42);
	EXPECT_EQ(d.back(), 42);
	int v = 0;
	v = d.front(); d.pop_front();
	EXPECT_EQ(v, 42);
	EXPECT_TRUE(d.empty());
}

// Capacity-2 full wraparound: both push paths cover the mask boundary

TEST(hxdeque_test, capacity_two_full_wraparound) {
	hxdeque<int, 2u> d;
	d.push_back(1); d.push_back(2);
	int v = 0;
	d.pop_front(); // head at 1
	d.push_back(3); // tail wraps to slot 0
	EXPECT_EQ(d[0], 2);
	EXPECT_EQ(d[1], 3);
	v = d.back(); d.pop_back(); EXPECT_EQ(v, 3);
	v = d.back(); d.pop_back(); EXPECT_EQ(v, 2);
	EXPECT_TRUE(d.empty());
}
