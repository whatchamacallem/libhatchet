// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

#include <hx/hxutility.h>
#include <hx/hxtest.hpp>

HX_NS_USE

TEST(hxmemory_manager_test, hxnew_forward) {
	struct hxtest_move_only {
		explicit hxtest_move_only(int v) : value(v), move_count(0) { }
		hxtest_move_only(hxtest_move_only&& other) noexcept
			: value(other.value), move_count(other.move_count + 1) { }
		hxtest_move_only(const hxtest_move_only&) = delete;
		int value;
		int move_count;
	};
	hxtest_move_only src(42);
	hxtest_move_only* p = hxnew<hxtest_move_only, hxsystem_allocator_heap>(hxmove(src));
	ASSERT_EQ(p->value, 42);
	ASSERT_EQ(p->move_count, 1);
	hxdelete(p);
}

TEST(hxmemory_manager_test, hxnew_forward_move_count_is_exactly_one) {
	struct hxtest_count_moves {
		explicit hxtest_count_moves(int v) : value(v), move_count(0) { }
		hxtest_count_moves(hxtest_count_moves&& other) noexcept
			: value(other.value), move_count(other.move_count + 1) { }
		hxtest_count_moves(const hxtest_count_moves&) = delete;
		int value;
		int move_count;
	};
	hxtest_count_moves src(7);
	hxtest_count_moves* p = hxnew<hxtest_count_moves, hxsystem_allocator_heap>(hxmove(src));
	ASSERT_EQ(p->move_count, 1);
	ASSERT_NE(p->move_count, 0);
	ASSERT_EQ(p->value, 7);
	hxdelete(p);
}

TEST(hxmemory_manager_test, hxnew) {
	unsigned int* t = new unsigned int(3);
	hxassert_always(t, "new");
	*t = 0xdeadbeefu;
	delete t;
	t = hxnullptr;
	delete t;
	SUCCEED();
}

TEST(hxmemory_manager_test, bytes) {
const hxsystem_allocator_scope temporary_stack_scope(hxsystem_allocator_temporary_stack);
	hxlog_warning("EXPECTING_TEST_WARNINGS\n");
	for(size_t i=10u; i != 0u; --i) {
		void* p = hxmalloc(i);
		ASSERT_NE(p, hxnullptr);
		::memset(p, 0x66, i);
		hxfree(p);
	}
}

TEST(hxmemory_manager_test, bytes_single_byte) {
const hxsystem_allocator_scope temporary_stack_scope(hxsystem_allocator_temporary_stack);
	void* p = hxmalloc(1u);
	ASSERT_NE(p, hxnullptr);
	::memset(p, 0x66, 1u);
	hxfree(p);
}

TEST(hxmemory_manager_test, string_duplicate) {
const hxsystem_allocator_scope temporary_stack_scope(hxsystem_allocator_temporary_stack);
	char* p = hxstring_duplicate("str");
	ASSERT_NE(p, hxnullptr);
	ASSERT_STREQ(p, "str");
	hxfree(p);
}

TEST(hxmemory_manager_test, string_duplicate_empty) {
const hxsystem_allocator_scope temporary_stack_scope(hxsystem_allocator_temporary_stack);
	char* p = hxstring_duplicate("");
	ASSERT_NE(p, hxnullptr);
	ASSERT_STREQ(p, "");
	hxfree(p);
}

TEST(hxmemory_manager_test, string_duplicate_single_char) {
const hxsystem_allocator_scope temporary_stack_scope(hxsystem_allocator_temporary_stack);
	char* p = hxstring_duplicate("x");
	ASSERT_NE(p, hxnullptr);
	ASSERT_STREQ(p, "x");
	hxfree(p);
}

TEST(hxmemory_manager_test, temp_overflow) {
	hxlog_warning("EXPECTING_TEST_WARNINGS\n");
	void* p = hxmalloc_ext(HX_MEMORY_BUDGET_TEMPORARY_STACK + 1, hxsystem_allocator_temporary_stack, 1u);
	ASSERT_NE(p, hxnullptr);
	hxfree(p);
const hxsystem_allocator_scope temp(hxsystem_allocator_temporary_stack);
	p = hxmalloc(HX_MEMORY_BUDGET_TEMPORARY_STACK + 1);
	ASSERT_NE(p, hxnullptr);
	hxfree(p);
}
#if HX_USE_MEMORY_MANAGER
class hxmemory_manager_test_f :
	public testing::Test
{
public:
	static void test_memory_allocator_normal(hxsystem_allocator_t id) {
		uintptr_t start_count = 0;
		uintptr_t start_bytes = 0;
	{
		const hxsystem_allocator_scope allocator_scope(id);
			start_count = allocator_scope.get_initial_allocation_count();
			start_bytes = allocator_scope.get_initial_bytes_allocated();
			{
				const hxsystem_allocator_scope gtest_spam_guard(hxsystem_allocator_heap);
				ASSERT_EQ(allocator_scope.get_current_allocation_count(), start_count);
				ASSERT_EQ(allocator_scope.get_current_bytes_allocated(), start_bytes);
			}
			void* ptr1 = hxmalloc(100);
			void* ptr2 = hxmalloc(200);
			::memset(ptr1, 0x33, 100);
			::memset(ptr2, 0x33, 200);
			{
				const hxsystem_allocator_scope gtest_spam_guard(hxsystem_allocator_heap);
				ASSERT_EQ(allocator_scope.get_initial_allocation_count(), start_count);
				ASSERT_EQ(allocator_scope.get_current_allocation_count(), 2u + start_count);
				if(allocator_scope.get_current_bytes_allocated() != 0) {
					ASSERT_EQ(allocator_scope.get_initial_bytes_allocated(), start_bytes);
					ASSERT_NEAR(
						static_cast<double>(allocator_scope.get_current_bytes_allocated()),
						static_cast<double>(start_bytes + 300u),
						static_cast<double>(2u * hxalignment));
				}
			}
			hxg_settings.deallocate_permanent = true;
			hxfree(ptr1);
			hxfree(ptr2);
			hxg_settings.deallocate_permanent = false;
		}
		if(id != hxsystem_allocator_permanent) {
			const hxsystem_allocator_scope allocator_scope(id);
			const hxsystem_allocator_scope gtest_spam_guard(hxsystem_allocator_heap);
			ASSERT_EQ(allocator_scope.get_initial_allocation_count(), start_count);
			ASSERT_EQ(allocator_scope.get_initial_bytes_allocated(), start_bytes);
		}
	}
	static void test_memory_allocator_leak(void) {
#if (HX_HARDENING_MODE) == HX_HARDENING_MODE_DEBUG
		void* ptr2 = hxnull;
		const int asserts_allowed = hxg_settings.asserts_to_be_skipped;
		{
			const hxsystem_allocator_scope allocator_scope(hxsystem_allocator_temporary_stack);
			ASSERT_EQ(0u, allocator_scope.get_initial_allocation_count());
			ASSERT_EQ(0u, allocator_scope.get_initial_bytes_allocated());
			ASSERT_EQ(0u, allocator_scope.get_current_allocation_count());
			ASSERT_EQ(0u, allocator_scope.get_current_bytes_allocated());
			void* ptr1 = hxmalloc(100);
			ptr2 = hxmalloc(200);
			::memset(ptr1, 0x33, 100);
			::memset(ptr2, 0x33, 200);
			hxfree(ptr1);
			hxg_settings.asserts_to_be_skipped = 1;
		}
		ASSERT_EQ(hxg_settings.asserts_to_be_skipped, 0);
		{
			const hxsystem_allocator_scope allocator_scope(hxsystem_allocator_temporary_stack);
			ASSERT_EQ(allocator_scope.get_initial_allocation_count(), 1);
			ASSERT_EQ(allocator_scope.get_initial_bytes_allocated(), 0);
			hxg_settings.asserts_to_be_skipped = 1;
			hxfree(ptr2);
		}
		ASSERT_EQ(hxg_settings.asserts_to_be_skipped, 0);
		hxg_settings.asserts_to_be_skipped = asserts_allowed;
#endif
	}
};

TEST_F(hxmemory_manager_test_f, execute) {
	for(size_t i = 0; i < hxsystem_allocator_current; ++i) {
		test_memory_allocator_normal(static_cast<hxsystem_allocator_t>(i));
	}
	hxlog_warning("EXPECTING_TEST_FAILURE\n");
	test_memory_allocator_leak();
}
#endif // HX_USE_MEMORY_MANAGER
