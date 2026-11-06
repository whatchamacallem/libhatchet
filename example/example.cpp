// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.
//
// An interactive Mandelbrot viewer demonstrating hxconsole, hxtask_queue,
// hxprofiler, hxmutex, hxfile and hxtest.
//
// The Mandelbrot set is a fractal defined in the complex plane, constituting
// one of the most studied and visually recognised objects in mathematics. It is
// formally defined as the set of all complex numbers C for which the sequence
// Z(n+1) = Z(n)² + C, with Z(0) = 0, remains bounded as n tends to infinity.
//
// Run with no arguments to explore. Otherwise each argument is a console
// command and "hxexample runtests" reports the tests at the end of this file
// with the Test Anything Protocol. See test_target.sh.

#include <signal.h>
#include <stdio.h>
#include <math.h>

#if defined HX_USE_MACROS_WITH_MODULE && (HX_USE_MACROS_WITH_MODULE)
import hx;
#else
#include <hx/hxarray.hpp>
#include <hx/hxfile.hpp>
#include <hx/hxtask_queue.hpp>
#endif

// These provide macros when HX_USE_MACROS_WITH_MODULE=1.
#include <hx/hxconsole.hpp>
#include <hx/hxprofiler.hpp>
#include <hx/hxtest.hpp>

namespace {

const double s_example_inv_log2       = 1.4426950408889634;
const double s_example_log_log2       = -0.36651292058166435;
const double s_example_angle_freq     = 6.28318530717958647692 / 16.0;
const double s_example_two_pi_over_3  = 2.09439510239319552;
const double s_example_four_pi_over_3 = 4.18879020478639098;
const double s_example_amplitude      = 127.5;

// The frame is 80 columns by 40 rows of two pixels each.
constexpr hxsize_t s_example_columns = 80;
constexpr hxsize_t s_example_rows = 40;

// Mutexes are not available in a signal handler.
void example_notify_sigint(int) {
	hxexit(EXIT_FAILURE);
}

// Sets s_example_exit, causing the main loop to break after the current render.
bool s_example_exit = false;
bool example_exit(void) {
	s_example_exit = true;
	return true;
}

// Console variables and functions. Console utilization is intended to be local
// to each translation unit.

double s_example_center_x = 0.0;
double s_example_center_y = 0.0;
double s_example_zoom = 3.0;

bool example_left(double amount)  { s_example_center_x -= amount * s_example_zoom;  return true; }
bool example_right(double amount) { s_example_center_x += amount * s_example_zoom;  return true; }
bool example_up(double amount)	  { s_example_center_y -= amount * s_example_zoom;  return true; }
bool example_down(double amount)  { s_example_center_y += amount * s_example_zoom;  return true; }
bool example_in(double factor)	  { s_example_zoom /= factor;						return true; }
bool example_out(double factor)   { s_example_zoom *= factor;						return true; }

hxconsole_variable_named(s_example_center_x, center_x);
hxconsole_variable_named(s_example_center_y, center_y);
hxconsole_variable_named(s_example_zoom, zoom);

hxconsole_command_named(example_exit, exit);
hxconsole_command_named(example_left, left);
hxconsole_command_named(example_right, right);
hxconsole_command_named(example_up, up);
hxconsole_command_named(example_down, down);
hxconsole_command_named(example_in, in);
hxconsole_command_named(example_out, out);

// hxtask computing one row of the Mandelbrot image. Rows are dispatched in
// parallel by example_compute via hxtask_queue.
class example_row_task : public hx::hxtask {
public:
	void set(hxsize_t row, double center_x, double center_y, double zoom, hxsize_t max_iter, char* row_buffer) {
		m_row		 = row;
		m_center_x   = center_x;
		m_center_y   = center_y;
		m_zoom	     = zoom;
		m_max_iter   = max_iter;
		m_row_buffer = row_buffer;
	}

	void color(double real_origin, double imaginary_origin,
			unsigned& r, unsigned& g, unsigned& b) {
		double real = 0.0;
		double imaginary = 0.0;
		hxsize_t iter = 0;
		while(iter < m_max_iter) {
			const double real_squared = real * real;
			const double imaginary_squared = imaginary * imaginary;
			if(real_squared + imaginary_squared > 4.0) {
				break;
			}
			imaginary = 2.0 * real * imaginary + imaginary_origin;
			real = real_squared - imaginary_squared + real_origin;
			++iter;
		}
		if(iter == m_max_iter) {
			r = 0; g = 0; b = 0;
			return;
		}
		const double mod_squared = real * real + imaginary * imaginary;
		const double log_zn = ::log(mod_squared) * 0.5;
		const double mu = static_cast<double>(iter) + 1.0
			- (::log(log_zn) - s_example_log_log2) * s_example_inv_log2;
		const double angle = mu * s_example_angle_freq;
		r = static_cast<unsigned>(s_example_amplitude
			+ s_example_amplitude * ::sin(angle));
		g = static_cast<unsigned>(s_example_amplitude
			+ s_example_amplitude * ::sin(angle + s_example_two_pi_over_3));
		b = static_cast<unsigned>(s_example_amplitude
			+ s_example_amplitude * ::sin(angle + s_example_four_pi_over_3));
	}

	bool execute(hx::hxtask_queue*) override {
		hxprofile_scope("row");

		// The pixels are centered on the view.
		const double col_center = 0.5 * static_cast<double>(s_example_columns - 1);
		const double row_center = 0.5 * static_cast<double>(2 * s_example_rows - 1);
		const double col_scale = m_zoom / static_cast<double>(s_example_columns);
		const double row_scale = col_scale * 0.25;
		const double upper_origin = m_center_y
			+ (static_cast<double>(m_row * 2) - row_center) * row_scale;
		const double lower_origin = m_center_y
			+ (static_cast<double>(m_row * 2 + 1) - row_center) * row_scale;
		char* dst = m_row_buffer;

		for(hxsize_t col = 0; col < s_example_columns; ++col) {
			const double real_origin = m_center_x + (static_cast<double>(col) - col_center) * col_scale;
			unsigned upper_r, upper_g, upper_b;
			unsigned lower_r, lower_g, lower_b;
			this->color(real_origin, upper_origin, upper_r, upper_g, upper_b);
			this->color(real_origin, lower_origin, lower_r, lower_g, lower_b);
			dst += ::snprintf(dst, 64u, "\033[48;2;%u;%u;%u;38;2;%u;%u;%um\xe2\x96\x84",
				upper_r, upper_g, upper_b, lower_r, lower_g, lower_b);
		}
		dst[0] = '\033'; dst[1] = '['; dst[2] = '0'; dst[3] = 'm';
		dst[4] = '\n'; dst[5] = '\0';
		return true;
	}

	const char* get_label(void) const override { return "row"; }

private:
	hxsize_t m_row;
	double m_center_x;
	double m_center_y;
	double m_zoom;
	hxsize_t m_max_iter;
	char* m_row_buffer;
};

using example_frame_t = hx::hxarray<hx::hxarray<char, 4096>, s_example_rows>;
using example_tasks_t = hx::hxarray<example_row_task, s_example_rows>;

// Enqueues a task per row, waits for completion and profiles the frame.
void example_compute(hx::hxtask_queue& queue, example_tasks_t& tasks, example_frame_t& frame) {
	hxsize_t max_iter = static_cast<hxsize_t>(50.0 * ::sqrt(::sqrt(1.0 / s_example_zoom))) + 20;
	if(max_iter < 64)   { max_iter = 64; }
	if(max_iter > 4096) { max_iter = 4096; }

	hxprofiler_start();

	for(hxsize_t row = 0; row < s_example_rows; ++row) {
		tasks[row].set(row, s_example_center_x, s_example_center_y, s_example_zoom,
			max_iter, frame[row].data());
		queue.enqueue(&tasks[row]);
	}
	queue.wait_for_all();

	hxprofiler_stop();
}

// Computes and prints a frame and writes the profile of computing it.
bool example_render(hx::hxtask_queue& queue, example_tasks_t& tasks, example_frame_t& frame) {
	example_compute(queue, tasks, frame);
	hxprofiler_write_to_chrome_tracing("profile.json");

	for(hxsize_t row = 0; row < s_example_rows; ++row) {
		hx::hxout.print("%s", frame[row].data());
	}

	hx::hxout.print("center (%.6g, %.6g) zoom %.6g\n",
		s_example_center_x, s_example_center_y, s_example_zoom);
	return true;
}

void example_usage(void) {
	puts("Commands are:\n"
		"\tcenter_x <optional-f64>\n"
		"\tcenter_y <optional-f64>\n"
		"\tzoom <optional-f64>\n"
		"\tleft f64\n"
		"\tright f64\n"
		"\tup f64\n"
		"\tdown f64\n"
		"\tin f64\n"
		"\tout f64\n"
		"\truntests\n"
		"\texit\n");
}

int example_main(int argc, char** argv) {
	hxinit();

	struct ::sigaction sa;
	sa.sa_handler = example_notify_sigint;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0; // No SA_RESTART: SIGINT interrupts blocking fgets
	::sigaction(SIGINT, &sa, hxnull);

	int exit_code = EXIT_SUCCESS;
	if(argc > 1) {
		// Each argument is a console command.
		for(int i = 1; i < argc; ++i) {
			if(!hx::hxconsole_exec_line(argv[i])) {
				exit_code = EXIT_FAILURE;
			}
		}
	}
	else if(!hx::hxconsole_exec_filename("example.cfg")) {
		hx::hxout << "error: example.cfg not found or failed to execute\n";
		exit_code = EXIT_FAILURE;
	}
	else {
		example_frame_t* frame = hx::hxnew<example_frame_t>();

		hx::hxtask_queue queue(static_cast<int32_t>(s_example_rows), 8);
		example_tasks_t tasks;

		example_render(queue, tasks, *frame);
		example_usage();

		char line[256];
		for(;;) {
			hx::hxout << "> ";
			if(::fgets(line, static_cast<int>(sizeof line), stdin) == hxnull) {
				break;
			}
			if(hx::hxconsole_exec_line(line)) {
				if(s_example_exit) { break; }
				example_render(queue, tasks, *frame);
			}
			else {
				example_usage();
			}
		}
		hx::hxdelete(frame);
	}

	hxshutdown();
	return exit_code;
}

// -- Tests ---------------------------------------------------------------------
// "hxexample runtests" runs these with hxtest, which reports them with the Test
// Anything Protocol by default. See HX_USE_TEST_FRAMEWORK.

bool example_run_tests(void) {
	// The tests run on a temporary stack, which also holds the frame they
	// render. The stacks can only be allocated once.
	static bool s_example_stacks_allocated = false;
	if(!s_example_stacks_allocated) {
		s_example_stacks_allocated = true;
		const size_t stack_sizes[] = { 256u * HX_KIB };
		hx::hxslab_allocator_allocate_stacks(stack_sizes);
	}
	return RUN_ALL_TESTS() == 0;
}

hxconsole_command_named(example_run_tests, runtests);

TEST(example, color) {
	example_row_task task;
	task.set(0, 0.0, 0.0, 3.0, 64, hxnull);
	unsigned r = 1u, g = 1u, b = 1u;

	// 0 never escapes and is black.
	task.color(0.0, 0.0, r, g, b);
	EXPECT_EQ(r, 0u);
	EXPECT_EQ(g, 0u);
	EXPECT_EQ(b, 0u);

	// 2 escapes on the first iteration and is colored.
	task.color(2.0, 0.0, r, g, b);
	EXPECT_LE(r, 255u);
	EXPECT_LE(g, 255u);
	EXPECT_LE(b, 255u);
	EXPECT_NE(r + g + b, 0u);
}

TEST(example, commands) {
	// The console binds the variables and commands that move the view.
	EXPECT_TRUE(hx::hxconsole_exec_line("center_x 0"));
	EXPECT_TRUE(hx::hxconsole_exec_line("center_y 0"));
	EXPECT_TRUE(hx::hxconsole_exec_line("zoom 2"));
	EXPECT_TRUE(hx::hxconsole_exec_line("right 0.5"));
	EXPECT_TRUE(hx::hxconsole_exec_line("down 0.25"));
	EXPECT_TRUE(hx::hxconsole_exec_line("in 4"));
	EXPECT_DOUBLE_EQ(s_example_center_x, 1.0);
	EXPECT_DOUBLE_EQ(s_example_center_y, 0.5);
	EXPECT_DOUBLE_EQ(s_example_zoom, 0.5);
}

TEST(example, config) {
	// example.cfg is a list of console commands setting the initial view.
	ASSERT_TRUE(hx::hxconsole_exec_filename("example.cfg"));
	EXPECT_DOUBLE_EQ(s_example_center_x, -0.7269);
	EXPECT_DOUBLE_EQ(s_example_center_y, 0.1889);
	EXPECT_DOUBLE_EQ(s_example_zoom, 0.006);
}

TEST(example, render) {
	ASSERT_TRUE(hx::hxconsole_exec_filename("example.cfg"));

	example_frame_t* frame = hx::hxnew<example_frame_t>();
	{
		hx::hxtask_queue queue(static_cast<int32_t>(s_example_rows), 8);
		example_tasks_t tasks;
		example_compute(queue, tasks, *frame);
	}

	// Every cell is a background color, a foreground color and a lower half
	// block. The view has points in the set, which are black, and points
	// outside it.
	int black = 0;
	int colored = 0;
	for(hxsize_t row = 0; row < s_example_rows; ++row) {
		const char* cursor = (*frame)[row].data();
		for(hxsize_t col = 0; col < s_example_columns; ++col) {
			unsigned rgb[6] = { };
			int length = 0;
			EXPECT_EQ(::sscanf(cursor, "\033[48;2;%u;%u;%u;38;2;%u;%u;%um\xe2\x96\x84%n",
				rgb + 0, rgb + 1, rgb + 2, rgb + 3, rgb + 4, rgb + 5, &length), 6);
			if(length == 0) {
				break; // Reported above.
			}
			for(int i = 0; i < 6; ++i) {
				EXPECT_LE(rgb[i], 255u);
			}
			black += (rgb[0] | rgb[1] | rgb[2]) == 0u;
			black += (rgb[3] | rgb[4] | rgb[5]) == 0u;
			colored += (rgb[0] | rgb[1] | rgb[2]) != 0u;
			colored += (rgb[3] | rgb[4] | rgb[5]) != 0u;
			cursor += length;
		}
		EXPECT_STREQ(cursor, "\033[0m\n");
	}
	EXPECT_GT(black, 0);
	EXPECT_GT(colored, 0);

	hx::hxdelete(frame);
}

} // namespace

int main(int argc, char** argv) {
	return example_main(argc, argv);
}
