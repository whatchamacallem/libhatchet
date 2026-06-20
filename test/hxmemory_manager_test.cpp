// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

#include <hx/hxutility.h>
#include <hx/hxtest.hpp>

// Using ASSERT* instead of EXPECT* in this file adds coverage for those
// macros. Memory corruption sounds fatal, so that seems appropriate.

// Verify hxnew forwards arguments, catching the bug where args... was used
// instead of hxforward<Args>(args).... A move-only type will fail to compile
// without forwarding, and a moveable type will be copied instead of moved.
TEST(hxmemory_manager_test, hxnew_forward) {
	struct hxtest_move_only {
		explicit hxtest_move_only(int v) : value(v), move_count(0) { }
		hxtest_move_only(hxtest_move_only&& other) noexcept
			: value(other.value), move_count(other.move_count + 1) { }
		hxtest_move_only(const hxtest_move_only&) = delete;
		int value;
		int move_count;
	};

	// If hxforward is missing this test will not compile (move-only type).
	// With hxforward the move constructor is invoked exactly once.
	hxtest_move_only src(42);
	hxtest_move_only* p = hxnew<hxtest_move_only, hxsystem_allocator_heap>(hxmove(src));
	ASSERT_EQ(p->value, 42);
	ASSERT_EQ(p->move_count, 1);
	hxdelete(p);
}

// Verify that new and delete plausibly exist and that hxnullptr compiles.
TEST(hxmemory_manager_test, hxnew) {
	unsigned int* t = new unsigned int(3);
	hxassert_always(t, "new"); // Should be impossible.
	*t = 0xdeadbeefu;
	delete t;
	t = hxnullptr;
	delete t;

	// Prevents Google Tests from fighting with clang tidy over new/delete use.
	// This is test designed to generate link errors and proof of life.
	SUCCEED();
}

TEST(hxmemory_manager_test, bytes) {
const hxsystem_allocator_scope temporary_stack_scope(hxsystem_allocator_temporary_stack);
	hxlog_console("EXPECTING_TEST_WARNINGS\n");
	for(size_t i=10u; i-- != 0u;) {
		void* p = hxmalloc(i);
		ASSERT_NE(p, hxnullptr);
		::memset(p, 0x66, i);
		hxfree(p);
	}
}

TEST(hxmemory_manager_test, string_duplicate) {
const hxsystem_allocator_scope temporary_stack_scope(hxsystem_allocator_temporary_stack);
	// "Allocates a copy of a string using the specified allocator." Duplicate literal into temp arena then release.
	char* p = hxstring_duplicate("str");
	ASSERT_NE(p, hxnullptr);
	ASSERT_STREQ(p, "str");
	hxfree(p);
}

TEST(hxmemory_manager_test, temp_overflow) {
	hxlog_console("EXPECTING_TEST_WARNINGS\n");

	// "Allocates memory of the specified size with a specific allocator and
	// alignment." Request temp_stack byte-count { budget + 1 } using explicit
	// alignment.
	void* p = hxmalloc_ext(HX_MEMORY_BUDGET_TEMPORARY_STACK + 1, hxsystem_allocator_temporary_stack, 1u);
	ASSERT_NE(p, hxnullptr);
	hxfree(p);

const hxsystem_allocator_scope temp(hxsystem_allocator_temporary_stack);
	// Fallback path through default allocator should still succeed for { budget
	// + 1 } bytes.
	p = hxmalloc(HX_MEMORY_BUDGET_TEMPORARY_STACK + 1);
	ASSERT_NE(p, hxnullptr);
	hxfree(p);
}

#if !(HX_MEMORY_MANAGER_DISABLE)

// This test case documents a contract between the system allocators and the
// rest of the program.
class hxmemory_manager_test_f :
	public testing::Test
{
public:
	static void test_memory_allocator_normal(hxsystem_allocator_t id) {
		uintptr_t start_count = 0;
		uintptr_t start_bytes = 0;

	{
		const hxsystem_allocator_scope allocator_scope(id);

			// "Gets the number of allocations made when this scope was entered." Snapshot counters for later diff.
			start_count = allocator_scope.get_initial_allocation_count();
			start_bytes = allocator_scope.get_initial_bytes_allocated();

			{
				// Google Test spams new/delete with std::string operations.
				const hxsystem_allocator_scope gtest_spam_guard(hxsystem_allocator_heap);
				// "Gets the total number of allocations outstanding for this memory allocator." Expect no incidental churn before our own allocations.
				ASSERT_EQ(allocator_scope.get_current_allocation_count(), start_count);
				ASSERT_EQ(allocator_scope.get_current_bytes_allocated(), start_bytes);
			}

			void* ptr1 = hxmalloc(100);
			void* ptr2 = hxmalloc(200);
			::memset(ptr1, 0x33, 100);
			::memset(ptr2, 0x33, 200);

			{
				const hxsystem_allocator_scope gtest_spam_guard(hxsystem_allocator_heap);
				// Check delta counters: 2x allocations should advance outstanding count while preserving initial snapshot.
				ASSERT_EQ(allocator_scope.get_initial_allocation_count(), start_count);
				ASSERT_EQ(allocator_scope.get_current_allocation_count(), 2u + start_count);
				if(allocator_scope.get_current_bytes_allocated() != 0) {
					// Allocators are not required to track byes outstanding.
					// But they have to get it right when they do.
					ASSERT_EQ(allocator_scope.get_initial_bytes_allocated(), start_bytes);
					ASSERT_NEAR(
						static_cast<double>(allocator_scope.get_current_bytes_allocated()),
						static_cast<double>(start_bytes + 300u),
						static_cast<double>(2u * HX_ALIGNMENT));
				}
			}

			// Allows quiet deletion of a permanent resource.
			g_hxsettings.deallocate_permanent = true;
			hxfree(ptr1);
			hxfree(ptr2);
			g_hxsettings.deallocate_permanent = false;
		}

		// hxsystem_allocator_permanent does not free.
		if(id != hxsystem_allocator_permanent) {
			// Fresh scope should be reset to original counters once previous block exits.
			const hxsystem_allocator_scope allocator_scope(id);

			const hxsystem_allocator_scope gtest_spam_guard(hxsystem_allocator_heap);
			ASSERT_EQ(allocator_scope.get_initial_allocation_count(), start_count);
			ASSERT_EQ(allocator_scope.get_initial_bytes_allocated(), start_bytes);
		}
	}

	static void test_memory_allocator_leak(void) {
#if (HX_HARDENING_MODE) == HX_HARDENING_MODE_DEBUG
		void* ptr2 = hxnull;
		const int asserts_allowed = g_hxsettings.asserts_to_be_skipped;

		{
			const hxsystem_allocator_scope allocator_scope(hxsystem_allocator_temporary_stack);

			// The temp stack is expected to be empty for this test.
			ASSERT_EQ(0u, allocator_scope.get_initial_allocation_count());
			ASSERT_EQ(0u, allocator_scope.get_initial_bytes_allocated());
			ASSERT_EQ(0u, allocator_scope.get_current_allocation_count());
			ASSERT_EQ(0u, allocator_scope.get_current_bytes_allocated());

			void* ptr1 = hxmalloc(100);
			ptr2 = hxmalloc(200);
			::memset(ptr1, 0x33, 100);
			::memset(ptr2, 0x33, 200);

			hxfree(ptr1); // Only free one allocation.

			// Prepare to trigger an assert when the scope closes.
			g_hxsettings.asserts_to_be_skipped = 1;
		}
		ASSERT_EQ(g_hxsettings.asserts_to_be_skipped, 0); // hxassert was hit; the leak occurred in the scope.

		{
			const hxsystem_allocator_scope allocator_scope(hxsystem_allocator_temporary_stack);

			// The allocator knows it has an outstanding allocation.
			ASSERT_EQ(allocator_scope.get_initial_allocation_count(), 1);

			// However, the allocated memory was reset.
			ASSERT_EQ(allocator_scope.get_initial_bytes_allocated(), 0);

			// Trigger the assert that catches late deletes.
			g_hxsettings.asserts_to_be_skipped = 1;
			hxfree(ptr2);
		}

		// hxassert was hit. The free happened after the scope closed.
		ASSERT_EQ(g_hxsettings.asserts_to_be_skipped, 0);

		g_hxsettings.asserts_to_be_skipped = asserts_allowed;
#endif
	}
};

TEST_F(hxmemory_manager_test_f, execute) {
	// The API should still work while stubbed out.
	for(size_t i = 0; i < hxsystem_allocator_current; ++i) {
		test_memory_allocator_normal(static_cast<hxsystem_allocator_t>(i));
	}

	// Leak checking requires the memory manager.
#if !(HX_MEMORY_MANAGER_DISABLE)
	hxlog("EXPECTING_TEST_FAILURE\n");

	// Only the temporary stack asserts all allocations are to be freed.
	test_memory_allocator_leak();
#endif
}

#endif // !HX_MEMORY_MANAGER_DISABLE
