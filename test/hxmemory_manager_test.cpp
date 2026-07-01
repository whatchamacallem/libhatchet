// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

#include <hx/hxutility.h>
#include <hx/hxtest.hpp>
#include "./hxtest_util.hpp"

HX_NS_USE

#if HX_CPLUSPLUS >= 202302L
namespace {
consteval bool hxtest_consteval_delete_deletes_and_is_true(void) {
	const hxconsteval_delete deleter;
	if(!static_cast<bool>(deleter)) { return false; }
	int* value = ::new int(34);
	if(*value != 34) { return false; }
	deleter(value);
	return true;
}
static_assert(hxtest_consteval_delete_deletes_and_is_true(),
	"hxconsteval_delete must report true and free its argument");
} // namespace
#endif // HX_CPLUSPLUS >= 202302L

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

#if (HX_PROVIDE_NEW_DELETE) == 1
TEST(hxmemory_manager_test, new_delete) {
	unsigned int* t = new unsigned int(3);
	hxassert_always(t, "new");
	*t = 0xdeadbeefu;
	delete t;
	t = hxnull;
	delete t;

	unsigned int* u = new unsigned int[1]{33u};
	hxassert_always(u, "new[]");
	u[0] = 0xdeadbeefu;
	delete[] u;
	u = hxnullptr;
	delete[] u;

	SUCCEED();
}
#endif

TEST(hxmemory_manager_test, hxmalloc_allocator_alignment_args_heap) {
#if HX_USE_MEMORY_MANAGER
	void* aligned = hxmalloc(8u, hxsystem_allocator_heap, 64u);
	hxassert_always(aligned, "hxmalloc");
	EXPECT_EQ(reinterpret_cast<uintptr_t>(aligned) & 63u, 0u);
	hxfree(aligned);
#endif

	void* defaulted = hxmalloc(8u, hxsystem_allocator_heap);
	hxassert_always(defaulted, "hxmalloc");
	EXPECT_EQ(reinterpret_cast<uintptr_t>(defaulted)
		& (static_cast<uintptr_t>(hxalignment) - 1u), 0u);
	hxfree(defaulted);
}

TEST(hxmemory_manager_test, hxmalloc_allocator_alignment_args_stack) {
	const hxsystem_allocator_scope temporary_stack_scope(hxsystem_allocator_stack_0);
#if HX_USE_MEMORY_MANAGER
	void* aligned = hxmalloc(8u, hxsystem_allocator_stack_0, 64u);
	hxassert_always(aligned, "hxmalloc");
	EXPECT_EQ(reinterpret_cast<uintptr_t>(aligned) & 63u, 0u);
	hxfree(aligned);
#endif

	void* defaulted = hxmalloc(8u, hxsystem_allocator_stack_0);
	hxassert_always(defaulted, "hxmalloc");
	EXPECT_EQ(reinterpret_cast<uintptr_t>(defaulted)
		& (static_cast<uintptr_t>(hxalignment) - 1u), 0u);
	hxfree(defaulted);
}

TEST(hxmemory_manager_test, placement_new_array_returns_buffer) {
	alignas(unsigned int) unsigned char buffer[4u * sizeof(unsigned int)];
	unsigned int* p = static_cast<unsigned int*>(
		operator new[](sizeof(buffer), static_cast<void*>(buffer)));
	EXPECT_EQ(static_cast<void*>(p), static_cast<void*>(buffer));
	p[0] = 0xdeadbeefu;
	p[3] = 0xfeedface;
	EXPECT_EQ(p[0], 0xdeadbeefu);
	EXPECT_EQ(p[3], 0xfeedface);
}

TEST(hxmemory_manager_test, bytes) {
const hxsystem_allocator_scope temporary_stack_scope(hxsystem_allocator_stack_0);
	hxlog_warning("EXPECTING_TEST_WARNINGS");
	for(size_t i=10u; i != 0u; --i) {
		void* p = hxmalloc(i);
		ASSERT_NE(p, hxnullptr);
		::memset(p, 0x66, i);
		hxfree(p);
	}
}

TEST(hxmemory_manager_test, bytes_single_byte) {
const hxsystem_allocator_scope temporary_stack_scope(hxsystem_allocator_stack_0);
	void* p = hxmalloc(1u);
	ASSERT_NE(p, hxnullptr);
	::memset(p, 0x66, 1u);
	hxfree(p);
}

TEST(hxmemory_manager_test, string_duplicate) {
const hxsystem_allocator_scope temporary_stack_scope(hxsystem_allocator_stack_0);
	char* p = hxstring_duplicate("str");
	ASSERT_NE(p, hxnullptr);
	ASSERT_STREQ(p, "str");
	hxfree(p);
}

TEST(hxmemory_manager_test, string_duplicate_empty) {
const hxsystem_allocator_scope temporary_stack_scope(hxsystem_allocator_stack_0);
	char* p = hxstring_duplicate("");
	ASSERT_NE(p, hxnullptr);
	ASSERT_STREQ(p, "");
	hxfree(p);
}

TEST(hxmemory_manager_test, string_duplicate_single_char) {
const hxsystem_allocator_scope temporary_stack_scope(hxsystem_allocator_stack_0);
	char* p = hxstring_duplicate("x");
	ASSERT_NE(p, hxnullptr);
	ASSERT_STREQ(p, "x");
	hxfree(p);
}

TEST(hxmemory_manager_test, temp_overflow) {
	hxlog_warning("EXPECTING_TEST_WARNINGS");
	const size_t temp_stack_size = 1u * HX_MIB;
	void* p = hxmalloc_ext(temp_stack_size + 1, hxsystem_allocator_stack_0, 1u);
	ASSERT_NE(p, hxnullptr);
	hxfree(p);
	// This time without alignment.
	const hxsystem_allocator_scope temp(hxsystem_allocator_stack_0);
	p = hxmalloc(temp_stack_size + 1);
	ASSERT_NE(p, hxnullptr);
	hxfree(p);
}

TEST(hxmemory_manager_test, additional_stacks_addressable) {
	const hxsystem_allocator_t stack_1 = hxsystem_allocator_stack_0 + 1;
	const hxsystem_allocator_t stack_2 = hxsystem_allocator_stack_0 + 2;
	const hxsystem_allocator_scope scope_1(stack_1);
	const hxsystem_allocator_scope scope_2(stack_2);
	void* p1 = hxmalloc_ext(64u, stack_1, hxalignment);
	void* p2 = hxmalloc_ext(64u, stack_2, hxalignment);
	ASSERT_NE(p1, hxnullptr);
	ASSERT_NE(p2, hxnullptr);
	ASSERT_NE(p1, p2);
	::memset(p1, 0x55, 64u);
	::memset(p2, 0xaa, 64u);
	hxfree(p1);
	hxfree(p2);
}

TEST(hxmemory_manager_test, additional_stacks_scope_current) {
	const hxsystem_allocator_t stack_1 = hxsystem_allocator_stack_0 + 1;
	const hxsystem_allocator_scope temporary_stack_scope(stack_1);
	void* p = hxmalloc(128u);
	ASSERT_NE(p, hxnullptr);
	::memset(p, 0x66, 128u);
	hxfree(p);
}

#if HX_USE_MEMORY_MANAGER

TEST(hxmemory_manager_test, additional_stacks_reset_on_scope_close) {
	const hxsystem_allocator_t stack_1 = hxsystem_allocator_stack_0 + 1;
	size_t baseline_bytes = 0u;
	size_t baseline_count = 0u;
	{
		const hxsystem_allocator_scope temporary_stack_scope(stack_1);
		baseline_bytes = temporary_stack_scope.get_current_bytes_allocated();
		baseline_count = temporary_stack_scope.get_current_allocation_count();
		void* p = hxmalloc(16u);
		ASSERT_NE(p, hxnullptr);
		ASSERT_EQ(temporary_stack_scope.get_current_allocation_count(), baseline_count + 1u);
		ASSERT_GT(temporary_stack_scope.get_current_bytes_allocated(), baseline_bytes);
		hxfree(p);
	}
	const hxsystem_allocator_scope temporary_stack_scope(stack_1);
	ASSERT_EQ(temporary_stack_scope.get_current_allocation_count(), baseline_count);
	ASSERT_EQ(temporary_stack_scope.get_current_bytes_allocated(), baseline_bytes);
}

TEST(hxmemory_manager_test, additional_stacks_are_independent) {
	const hxsystem_allocator_t stack_1 = hxsystem_allocator_stack_0 + 1;
	const hxsystem_allocator_t stack_2 = hxsystem_allocator_stack_0 + 2;
	const hxsystem_allocator_scope scope_1(stack_1);
	void* p1 = hxmalloc(16u);
	ASSERT_NE(p1, hxnullptr);
	ASSERT_EQ(scope_1.get_current_allocation_count(), 1u);
	{
		const hxsystem_allocator_scope scope_2(stack_2);
		ASSERT_EQ(scope_2.get_current_allocation_count(), 0u);
		void* p2 = hxmalloc(16u);
		ASSERT_NE(p2, hxnullptr);
		ASSERT_EQ(scope_2.get_current_allocation_count(), 1u);
		hxfree(p2);
	}
	ASSERT_EQ(scope_1.get_current_allocation_count(), 1u);
	hxfree(p1);
}

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
		{
			const hxtest_skip_asserts skip(1);
			{
				const hxsystem_allocator_scope allocator_scope(hxsystem_allocator_stack_0);
				ASSERT_EQ(0u, allocator_scope.get_initial_allocation_count());
				ASSERT_EQ(0u, allocator_scope.get_initial_bytes_allocated());
				ASSERT_EQ(0u, allocator_scope.get_current_allocation_count());
				ASSERT_EQ(0u, allocator_scope.get_current_bytes_allocated());
				void* ptr1 = hxmalloc(100);
				ptr2 = hxmalloc(200);
				::memset(ptr1, 0x33, 100);
				::memset(ptr2, 0x33, 200);
				hxfree(ptr1);
			}
			ASSERT_EQ(skip.remaining(), 0);
		}
		{
			const hxtest_skip_asserts skip(1);
			{
				const hxsystem_allocator_scope allocator_scope(hxsystem_allocator_stack_0);
				ASSERT_EQ(allocator_scope.get_initial_allocation_count(), 1);
				ASSERT_EQ(allocator_scope.get_initial_bytes_allocated(), 0);
				hxfree(ptr2);
			}
			ASSERT_EQ(skip.remaining(), 0);
		}
#endif
	}
};

TEST_F(hxmemory_manager_test_f, execute) {
	for(size_t i = 0; i <= hxsystem_allocator_stack_0; ++i) {
		test_memory_allocator_normal(static_cast<hxsystem_allocator_t>(i));
	}
	hxlog_warning("EXPECTING_TEST_FAILURE");
	test_memory_allocator_leak();
}
#endif // HX_USE_MEMORY_MANAGER
