// SPDX-FileCopyrightText: © 2025 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

#include <hx/hxdeque.hpp>
#include <hx/hxtest.hpp>

hxattr_noinline static void hxtest_gdb_break_hxdeque(void) {}

namespace {

// Lifecycle tracking helper reused across fixture-based tests.

class hxdeque_test_f* s_hxdeque_current = hxnull;

class hxdeque_test_f : public testing::Test {
public:
	class hxtest_object {
	public:
		hxtest_object(void) {
			++s_hxdeque_current->m_constructed;
			id = s_hxdeque_current->m_next_id--;
			moved_from = false;
		}
		explicit hxtest_object(int32_t x) {
			++s_hxdeque_current->m_constructed;
			id = x;
			moved_from = false;
		}
		hxtest_object(const hxtest_object& x) {
			++s_hxdeque_current->m_constructed;
			id = x.id;
			moved_from = false;
		}
		hxtest_object(hxtest_object&& x) {
			++s_hxdeque_current->m_constructed;
			id = x.id;
			moved_from = false;
			x.id = 0xefef;
			x.moved_from = true;
		}
		~hxtest_object(void) {
			++s_hxdeque_current->m_destructed;
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
		hxassert(s_hxdeque_current == hxnull);
		m_constructed = 0u;
		m_destructed = 0u;
		m_next_id = -1;
		s_hxdeque_current = this;
	}
	~hxdeque_test_f(void) override {
		hxassertrelease(m_constructed == m_destructed,
			"hxdeque_test_f lifecycle mismatch");
		s_hxdeque_current = hxnull;
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
	EXPECT_TRUE(d.push_back(10));
	EXPECT_TRUE(d.push_back(20));
	EXPECT_TRUE(d.push_back(30));
	EXPECT_EQ(d.size(), 3u);
	EXPECT_FALSE(d.empty());

	int v = 0;
	EXPECT_TRUE(d.pop_front(v)); EXPECT_EQ(v, 10);
	EXPECT_TRUE(d.pop_front(v)); EXPECT_EQ(v, 20);
	EXPECT_TRUE(d.pop_front(v)); EXPECT_EQ(v, 30);
	EXPECT_EQ(d.size(), 0u);
	EXPECT_TRUE(d.empty());
}

// push_back returns false when full (full branch taken)
TEST(hxdeque_test, push_back_returns_false_when_full) {
	hxdeque<int, 2u> d;
	EXPECT_TRUE(d.push_back(1));
	EXPECT_TRUE(d.push_back(2));
	EXPECT_TRUE(d.full());
	EXPECT_FALSE(d.push_back(3));
	EXPECT_EQ(d.size(), 2u);
}

// pop_front returns false when empty (empty branch taken)
TEST(hxdeque_test, pop_front_returns_false_when_empty) {
	hxdeque<int, 4u> d;
	int v = 99;
	EXPECT_FALSE(d.pop_front(v));
	EXPECT_EQ(v, 99); // out is untouched
}

// push_front / pop_back: LIFO-like ordering

TEST(hxdeque_test, push_front_pop_back_order) {
	hxdeque<int, 4u> d;
	EXPECT_TRUE(d.push_front(10));
	EXPECT_TRUE(d.push_front(20));
	EXPECT_TRUE(d.push_front(30));
	EXPECT_EQ(d.size(), 3u);

	int v = 0;
	EXPECT_TRUE(d.pop_back(v)); EXPECT_EQ(v, 10);
	EXPECT_TRUE(d.pop_back(v)); EXPECT_EQ(v, 20);
	EXPECT_TRUE(d.pop_back(v)); EXPECT_EQ(v, 30);
	EXPECT_TRUE(d.empty());
}

// push_front returns false when full
TEST(hxdeque_test, push_front_returns_false_when_full) {
	hxdeque<int, 2u> d;
	EXPECT_TRUE(d.push_front(1));
	EXPECT_TRUE(d.push_front(2));
	EXPECT_FALSE(d.push_front(3));
}

// pop_back returns false when empty
TEST(hxdeque_test, pop_back_returns_false_when_empty) {
	hxdeque<int, 4u> d;
	int v = 77;
	EXPECT_FALSE(d.pop_back(v));
	EXPECT_EQ(v, 77);
}

// Ring-buffer wraparound: head and tail cross the physical end of storage

TEST(hxdeque_test, ring_wraparound_push_back_pop_front) {
	// Capacity 4 (mask 3). Fill, drain 3, fill 3 more so tail wraps around.
	hxdeque<int, 4u> d;
	EXPECT_TRUE(d.push_back(1));
	EXPECT_TRUE(d.push_back(2));
	EXPECT_TRUE(d.push_back(3));
	EXPECT_TRUE(d.push_back(4));
	int v = 0;
	EXPECT_TRUE(d.pop_front(v)); EXPECT_EQ(v, 1);
	EXPECT_TRUE(d.pop_front(v)); EXPECT_EQ(v, 2);
	EXPECT_TRUE(d.pop_front(v)); EXPECT_EQ(v, 3);
	// head is now at physical slot 3, tail at 4 (slot 0 after mask)
	EXPECT_TRUE(d.push_back(10));
	EXPECT_TRUE(d.push_back(11));
	EXPECT_TRUE(d.push_back(12));
	// Logical order must be 4, 10, 11, 12
	EXPECT_TRUE(d.pop_front(v)); EXPECT_EQ(v, 4);
	EXPECT_TRUE(d.pop_front(v)); EXPECT_EQ(v, 10);
	EXPECT_TRUE(d.pop_front(v)); EXPECT_EQ(v, 11);
	EXPECT_TRUE(d.pop_front(v)); EXPECT_EQ(v, 12);
	EXPECT_TRUE(d.empty());
}

TEST(hxdeque_test, ring_wraparound_push_front_pop_back) {
	// Capacity 4. Push from front so head moves backwards, crossing slot 0.
	hxdeque<int, 4u> d;
	EXPECT_TRUE(d.push_front(4));
	EXPECT_TRUE(d.push_front(3));
	EXPECT_TRUE(d.push_front(2));
	EXPECT_TRUE(d.push_front(1));
	int v = 0;
	EXPECT_TRUE(d.pop_back(v)); EXPECT_EQ(v, 4);
	EXPECT_TRUE(d.pop_back(v)); EXPECT_EQ(v, 3);
	EXPECT_TRUE(d.push_front(0));
	EXPECT_TRUE(d.push_front(-1));
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
	EXPECT_TRUE(d.emplace_back(42));
	EXPECT_EQ(d.back().id, 42);
	EXPECT_FALSE(d.back().moved_from);
	EXPECT_EQ(d.size(), 1u);
	EXPECT_EQ(m_constructed, 1u);
	EXPECT_EQ(m_destructed, 0u);
}

TEST_F(hxdeque_test_f, emplace_front_constructs_in_place) {
	hxdeque<hxtest_object, 4u> d;
	EXPECT_TRUE(d.emplace_back(10));
	EXPECT_TRUE(d.emplace_front(99));
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
	EXPECT_TRUE(d.emplace_back(3, 7));
	EXPECT_EQ(d.back().x, 3);
	EXPECT_EQ(d.back().y, 7);
}

// front() / back() accessors: both const and non-const overloads

TEST(hxdeque_test, front_and_back_multiple_elements) {
	hxdeque<int, 4u> d;
	EXPECT_TRUE(d.push_back(1));
	EXPECT_TRUE(d.push_back(2));
	EXPECT_TRUE(d.push_back(3));
	EXPECT_EQ(d.front(), 1);
	EXPECT_EQ(d.back(), 3);
}

TEST(hxdeque_test, const_front_and_back) {
	hxdeque<int, 4u> d;
	EXPECT_TRUE(d.push_back(7));
	EXPECT_TRUE(d.push_back(8));
	const hxdeque<int, 4u>& cd = d;
	EXPECT_EQ(cd.front(), 7);
	EXPECT_EQ(cd.back(), 8);
}

// front/back mutability: write through the reference
TEST(hxdeque_test, front_back_reference_mutation) {
	hxdeque<int, 4u> d;
	EXPECT_TRUE(d.push_back(1));
	EXPECT_TRUE(d.push_back(2));
	d.front() = 10;
	d.back()  = 20;
	EXPECT_EQ(d[0], 10);
	EXPECT_EQ(d[1], 20);
}

// operator[] and at(): indexing with logical offset

TEST(hxdeque_test, operator_index_sequential) {
	hxdeque<int, 8u> d;
	for(int i = 0; i < 5; ++i) {
		EXPECT_TRUE(d.push_back(i * 10));
	}
	for(size_t i = 0u; i < 5u; ++i) {
		EXPECT_EQ(d[i], static_cast<int>(i * 10));
	}
}

TEST(hxdeque_test, const_operator_index) {
	hxdeque<int, 4u> d;
	EXPECT_TRUE(d.push_back(3));
	EXPECT_TRUE(d.push_back(6));
	const hxdeque<int, 4u>& cd = d;
	EXPECT_EQ(cd[0], 3);
	EXPECT_EQ(cd[1], 6);
}

// Indexing after ring wraparound: logical indices must map correctly.
TEST(hxdeque_test, operator_index_after_wraparound) {
	hxdeque<int, 4u> d;
	EXPECT_TRUE(d.push_back(1)); EXPECT_TRUE(d.push_back(2));
	EXPECT_TRUE(d.push_back(3)); EXPECT_TRUE(d.push_back(4));
	int v = 0;
	EXPECT_TRUE(d.pop_front(v)); EXPECT_TRUE(d.pop_front(v)); // head is now at slot 2
	EXPECT_TRUE(d.push_back(5)); EXPECT_TRUE(d.push_back(6)); // tail wraps to slots 0, 1
	// Logical: [3, 4, 5, 6]
	EXPECT_EQ(d[0], 3);
	EXPECT_EQ(d[1], 4);
	EXPECT_EQ(d[2], 5);
	EXPECT_EQ(d[3], 6);
}

// clear(): destroys all elements, resets to empty

TEST_F(hxdeque_test_f, clear_destroys_all_elements) {
	hxdeque<hxtest_object, 4u> d;
	EXPECT_TRUE(d.emplace_back(1));
	EXPECT_TRUE(d.emplace_back(2));
	EXPECT_TRUE(d.emplace_back(3));
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
	for(int i = 0; i < 4; ++i) { EXPECT_TRUE(d.emplace_back(i)); }
	hxtest_object out(0);
	// Consume m_constructed from default ctor above
	const size_t baseline = m_constructed;
	EXPECT_TRUE(d.pop_front(out)); // head advances; out receives move
	EXPECT_TRUE(d.pop_front(out));
	// Push two more so tail wraps around
	EXPECT_TRUE(d.emplace_back(100));
	EXPECT_TRUE(d.emplace_back(101));
	EXPECT_EQ(d.size(), 4u);
	const size_t before_clear = m_destructed;
	d.clear();
	// Must have destructed exactly the 4 live elements
	EXPECT_EQ(m_destructed, before_clear + 4u);
	EXPECT_TRUE(d.empty());
	(void)baseline;
}

// Destructor: implicitly calls clear()

TEST_F(hxdeque_test_f, destructor_calls_clear) {
	{
		hxdeque<hxtest_object, 4u> d;
		EXPECT_TRUE(d.emplace_back(7));
		EXPECT_TRUE(d.emplace_back(8));
		EXPECT_EQ(m_constructed, 2u);
	} // destructor fires here
	EXPECT_EQ(m_destructed, 2u);
}

// full() / empty() / size(): state predicate coverage

TEST(hxdeque_test, full_predicate) {
	hxdeque<int, 2u> d;
	EXPECT_FALSE(d.full());
	EXPECT_TRUE(d.push_back(1));
	EXPECT_FALSE(d.full());
	EXPECT_TRUE(d.push_back(2));
	EXPECT_TRUE(d.full());
	int v = 0;
	EXPECT_TRUE(d.pop_front(v));
	EXPECT_FALSE(d.full());
}

// pop_front / pop_back move semantics: moved-from object is destroyed cleanly

TEST_F(hxdeque_test_f, pop_front_moves_element_and_destroys_slot) {
	hxdeque<hxtest_object, 4u> d;
	EXPECT_TRUE(d.emplace_back(55));
	EXPECT_EQ(m_constructed, 1u);

	hxtest_object out(0);
	const size_t c_before = m_constructed;
	const size_t d_before = m_destructed;
	EXPECT_TRUE(d.pop_front(out));
	// The slot destructor is called for the in-place object.
	EXPECT_EQ(m_destructed, d_before + 1u);
	// Assignment increments nothing (uses operator=).
	EXPECT_EQ(m_constructed, c_before);
	EXPECT_EQ(out.id, 55);
	EXPECT_TRUE(d.empty());
}

TEST_F(hxdeque_test_f, pop_back_moves_element_and_destroys_slot) {
	hxdeque<hxtest_object, 4u> d;
	EXPECT_TRUE(d.emplace_back(77));
	hxtest_object out(0);
	const size_t d_before = m_destructed;
	EXPECT_TRUE(d.pop_back(out));
	EXPECT_EQ(m_destructed, d_before + 1u);
	EXPECT_EQ(out.id, 77);
	EXPECT_TRUE(d.empty());
}

// Dynamic storage: reserve, use, and destroy

TEST(hxdeque_test, dynamic_capacity_push_pop) {
	const hxsystem_allocator_scope temporary_stack_scope(hxsystem_allocator_temporary_stack);
	hxdeque<int> d(4u);
	EXPECT_EQ(d.capacity(), 4u);
	for(int i = 0; i < 4; ++i) {
		EXPECT_TRUE(d.push_back(i));
	}
	EXPECT_TRUE(d.full());
	for(int i = 0; i < 4; ++i) {
		int v = 0;
		EXPECT_TRUE(d.pop_front(v));
		EXPECT_EQ(v, i);
	}
	EXPECT_TRUE(d.empty());
}

TEST(hxdeque_test, dynamic_capacity_ring_wraparound) {
	const hxsystem_allocator_scope temporary_stack_scope(hxsystem_allocator_temporary_stack);
	hxdeque<int> d(8u);
	for(int i = 0; i < 8; ++i) { EXPECT_TRUE(d.push_back(i)); }
	int v = 0;
	for(int i = 0; i < 4; ++i) { EXPECT_TRUE(d.pop_front(v)); }
	for(int i = 8; i < 12; ++i) { EXPECT_TRUE(d.push_back(i)); }
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
	EXPECT_TRUE(d.push_back(3));
	EXPECT_TRUE(d.push_front(2));
	EXPECT_TRUE(d.push_back(4));
	EXPECT_TRUE(d.push_front(1));
	// Logical order: 1, 2, 3, 4
	EXPECT_EQ(d.size(), 4u);
	EXPECT_EQ(d[0], 1);
	EXPECT_EQ(d[1], 2);
	EXPECT_EQ(d[2], 3);
	EXPECT_EQ(d[3], 4);
}

TEST(hxdeque_test, interleaved_pop_back_pop_front) {
	hxdeque<int, 4u> d;
	EXPECT_TRUE(d.push_back(10));
	EXPECT_TRUE(d.push_back(20));
	EXPECT_TRUE(d.push_back(30));
	int v = 0;
	EXPECT_TRUE(d.pop_front(v)); EXPECT_EQ(v, 10);
	EXPECT_TRUE(d.pop_back(v));  EXPECT_EQ(v, 30);
	EXPECT_EQ(d.size(), 1u);
	EXPECT_EQ(d.front(), 20);
	EXPECT_EQ(d.back(), 20);
}

// Single-element capacity (capacity == 1): edge case

TEST(hxdeque_test, single_element_capacity) {
	hxdeque<int, 1u> d;
	EXPECT_TRUE(d.empty());
	EXPECT_EQ(d.capacity(), 1u);
	EXPECT_TRUE(d.push_back(42));
	EXPECT_TRUE(d.full());
	EXPECT_FALSE(d.push_back(99));
	EXPECT_FALSE(d.push_front(99));
	EXPECT_EQ(d.front(), 42);
	EXPECT_EQ(d.back(), 42);
	int v = 0;
	EXPECT_TRUE(d.pop_front(v));
	EXPECT_EQ(v, 42);
	EXPECT_TRUE(d.empty());
}

// Capacity-2 full wraparound: both push paths cover the mask boundary

TEST(hxdeque_test, capacity_two_full_wraparound) {
	hxdeque<int, 2u> d;
	EXPECT_TRUE(d.push_back(1)); EXPECT_TRUE(d.push_back(2));
	int v = 0;
	EXPECT_TRUE(d.pop_front(v)); // head at 1
	EXPECT_TRUE(d.push_back(3)); // tail wraps to slot 0
	EXPECT_EQ(d[0], 2);
	EXPECT_EQ(d[1], 3);
	EXPECT_TRUE(d.pop_back(v)); EXPECT_EQ(v, 3);
	EXPECT_TRUE(d.pop_back(v)); EXPECT_EQ(v, 2);
	EXPECT_TRUE(d.empty());
}
