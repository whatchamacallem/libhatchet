// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.
//
// The main program for testing on embedded targets. The full test suite in the
// test directory is too large for a dev board. This is a small suite checking
// what a port depends on. Build it with src/*.cpp and libc. It reports with the
// Test Anything Protocol (TAP). See target/README.md.

#include <stdio.h>
#include <stdlib.h>

// Anything a target has to do around the tests goes in its section here, under a
// macro its SDK defines. Each section defines target_start and target_finish.

// -- Pico SDK -----------------------------------------------------------------

// The Pico SDK defines PICO_ON_DEVICE. See CMakeLists.txt.
#if defined PICO_ON_DEVICE && (PICO_ON_DEVICE)
#include <pico/stdlib.h>
#include <pico/bootrom.h>

static void target_start(void) {
	// Reports over USB serial.
	stdio_init_all();
	while(!stdio_usb_connected()) {
		sleep_ms(100);
	}
	// Terminals can lose what is sent right after connecting and the TAP plan
	// comes first.
	sleep_ms(250);
}

static void target_finish(void) {
	// Reboots into BOOTSEL so that the next build can be flashed without the
	// button.
	::puts("# Rebooting into BOOTSEL...");
	stdio_flush();
	reset_usb_boot(0, 0);
}

// -- Desktop ------------------------------------------------------------------

#else
static void target_start(void) { }
static void target_finish(void) { }
#endif

// -----------------------------------------------------------------------------

#if defined HX_USE_MACROS_WITH_MODULE && (HX_USE_MACROS_WITH_MODULE)
import hx;
#else
#include <hx/hxarray.hpp>
#include <hx/hxbitset.hpp>
#include <hx/hxdeque.hpp>
#include <hx/hxfile.hpp>
#include <hx/hxhash_table.hpp>
#include <hx/hxhash_table_nodes.hpp>
#include <hx/hxrandom.hpp>
#include <hx/hxsort.hpp>
#include <hx/hxtask_queue.hpp>
#include <hx/hxvector.hpp>
#endif

// These provide macros when HX_USE_MACROS_WITH_MODULE=1.
#include <hx/hxconsole.hpp>
#include <hx/hxprofiler.hpp>
#include <hx/hxtest.hpp>

HX_NS_USE

namespace {

class target_node : public hxhash_table_node_integer<int32_t> {
public:
	explicit target_node(int32_t key) : hxhash_table_node_integer<int32_t>(key) { }
};

// Each task writes to itself because they may run on different threads.
class target_task : public hxtask {
public:
	bool execute(hxtask_queue*) override {
		hxprofile_scope("target_task");
		m_executed = true;
		return true;
	}

	bool m_executed = false;
};

} // namespace

// hxrandom produces the same stream on 32-bit and 64-bit targets.
TEST(target, integers) {
	volatile uint64_t value = 0x123456789abcdef0ull;
	EXPECT_EQ(value * 3u, 0x369d0369d0369cd0ull);
	EXPECT_EQ(value >> 36, 0x1234567ull);

	hxrandom random(31u);
	EXPECT_EQ(random(), 0xf9067d9eu);
	EXPECT_EQ(random(), 0x9c69afb5u);
}

// libc for embedded targets is often built without floating point formatting.
// The C++ <math.h> includes a <new> that conflicts with the hx placement new when
// an SDK requires the C++ headers, so this calls libm with the builtin.
TEST(target, floating_point) {
	volatile double two = 2.0;
	EXPECT_NEAR(__builtin_sqrt(two), 1.41421356, 0.00000001);

	char buffer[16];
	EXPECT_EQ(::snprintf(buffer, sizeof buffer, "%.3f", __builtin_sqrt(two)), 5);
	EXPECT_STREQ(buffer, "1.414");
}

TEST(target, allocators) {
	int* const heap = hxnew<int>(7);
	EXPECT_EQ(*heap, 7);
	hxdelete(heap);

	void* const aligned = hxmalloc_ext(24u, hxslab_allocator_heap, 16u);
	EXPECT_EQ(reinterpret_cast<uintptr_t>(aligned) % 16u, 0u);
	hxfree(aligned);

	// Tests run with stack 0 as the current allocator.
	const hxslab_allocator_scope scope(hxslab_allocator_stack_0);
	hxvector<int> temporary;
	temporary.reserve(16u);
	EXPECT_EQ(temporary.capacity(), 16u);
}

TEST(target, containers) {
	const int values[] = { 5, 3, 8, 1 };

	hxarray<int, 4> array(values);
	hxsort(array.begin(), array.end());
	EXPECT_EQ(array[0], 1);
	EXPECT_EQ(array[3], 8);

	hxvector<int> vector;
	vector.reserve(4u);
	hxdeque<int, 4> deque;
	for(int value : values) {
		vector.push_back(value);
		deque.push_front(value);
	}
	EXPECT_EQ(vector.back(), 1);
	EXPECT_EQ(deque.front(), 1);
	EXPECT_EQ(deque.back(), 5);

	// Spans more than one word on every target.
	hxbitset<70> bits;
	bits.set(69);
	EXPECT_TRUE(bits.test(69));
	EXPECT_FALSE(bits.test(5));

	hxhash_table<target_node, hxdefault_delete, 4> table;
	for(int value : values) {
		table.insert(hxnew<target_node>(value));
	}
	EXPECT_EQ(table.size(), 4u);
	EXPECT_TRUE(table.find(8) != table.end());
	EXPECT_TRUE(table.find(2) == table.end());
}

TEST(target, tasks) {
	// The thread pool is ignored when HX_USE_THREADS=0.
	hxtask_queue queue(4, 2);
	target_task tasks[4];
	for(target_task& task : tasks) {
		queue.enqueue(&task);
	}
	queue.wait_for_all();
	for(const target_task& task : tasks) {
		EXPECT_TRUE(task.m_executed);
	}
}

#if HX_USE_CONSOLE
namespace {
double s_target_value = 0.0;

bool target_add(double amount) {
	s_target_value += amount;
	return true;
}

hxconsole_variable_named(s_target_value, target_value);
hxconsole_command_named(target_add, target_add);
} // namespace

TEST(target, console) {
	EXPECT_TRUE(hxconsole_exec_line("target_value 1.5"));
	EXPECT_TRUE(hxconsole_exec_line("target_add 0.25"));
	EXPECT_DOUBLE_EQ(s_target_value, 1.75);
}

#if HX_USE_FILE_IO
TEST(target, console_file) {
	{
		hxfile file(hxfile::open_mode_out, "%s", "hxtest_target.cfg");
		ASSERT_TRUE(file.print("target_value 2\ntarget_add 0.5\n"));
	}
	ASSERT_TRUE(hxconsole_exec_filename("hxtest_target.cfg"));
	EXPECT_DOUBLE_EQ(s_target_value, 2.5);
}
#endif // HX_USE_FILE_IO
#endif // HX_USE_CONSOLE

// Runs the tests and returns an exit code.
int main(void) {
	target_start();

	hxinit();

	// RUN_ALL_TESTS requires a stack. TAP does not allow anything before the
	// version it starts with.
	const size_t stack_sizes[] = { 4u * HX_KIB };
	hxslab_allocator_allocate_stacks(stack_sizes);

	const int tests_failing = RUN_ALL_TESTS();
	::puts("# libhatchet 🪓🪓🪓 " LIBHATCHET_TAG);

	hxshutdown();

	target_finish();

	return (tests_failing == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
