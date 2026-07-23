// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.
//
// An interactive Mandelbrot viewer demonstrating hxconsole, hxtask_queue,
// hxprofiler, hxmutex and hxfile.
//
// The Mandelbrot set is a fractal defined in the complex plane, constituting
// one of the most studied and visually recognised objects in mathematics. It is
// formally defined as the set of all complex numbers C for which the sequence
// Z(n+1) = Z(n)² + C, with Z(0) = 0, remains bounded as n tends to infinity.
//
// Use "cat example/example_correct.txt" if you just want to see the output.

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
// Unused. Included to make sure it compiles with HX_USE_MACROS_WITH_MODULE=1.
#include <hx/hxtest.hpp>

namespace {

const double s_example_inv_log2       = 1.4426950408889634;
const double s_example_log_log2       = -0.36651292058166435;
const double s_example_angle_freq     = 6.28318530717958647692 / 16.0;
const double s_example_two_pi_over_3  = 2.09439510239319552;
const double s_example_four_pi_over_3 = 4.18879020478639098;
const double s_example_amplitude      = 127.5;

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
// parallel by example_render via hxtask_queue.
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

		const double col_scale = m_zoom / 80.0;
		const double row_scale = col_scale * 0.25;
		const double upper_origin = m_center_y
			+ (static_cast<double>(m_row * 2) - 39.5) * row_scale;
		const double lower_origin = m_center_y
			+ (static_cast<double>(m_row * 2 + 1) - 39.5) * row_scale;
		char* dst = m_row_buffer;

		for(hxsize_t col = 0; col < 80; ++col) {
			const double real_origin = m_center_x + (static_cast<double>(col) - 39.5) * col_scale;
			unsigned upper_r, upper_g, upper_b;
			unsigned lower_r, lower_g, lower_b;
			this->color(real_origin, upper_origin, upper_r, upper_g, upper_b);
			this->color(real_origin, lower_origin, lower_r, lower_g, lower_b);
			dst += ::sprintf(dst, "\033[48;2;%u;%u;%u;38;2;%u;%u;%um\xe2\x96\x84",
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

// Enqueues all 40 row tasks, waits for completion, then prints the frame.
bool example_render(hx::hxtask_queue& queue, hx::hxarray<example_row_task, 40>& tasks,
		hx::hxarray<hx::hxarray<char, 4096>, 40>& row_storage) {
	hxsize_t max_iter = static_cast<hxsize_t>(50.0 * ::sqrt(::sqrt(1.0 / s_example_zoom))) + 20;
	if(max_iter < 64)   { max_iter = 64; }
	if(max_iter > 4096) { max_iter = 4096; }

	hxprofiler_start();

	for(hxsize_t row = 0; row < 40; ++row) {
		tasks[row].set(row, s_example_center_x, s_example_center_y, s_example_zoom,
			max_iter, row_storage[row].data());
		queue.enqueue(&tasks[row]);
	}
	queue.wait_for_all();

	hxprofiler_stop();
	hxprofiler_write_to_chrome_tracing("profile.json");

	for(hxsize_t row = 0; row < 40; ++row) {
		hx::hxout.print("%s", row_storage[row].data());
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
		"\texit\n");
}

int example_main(void) {
	hxinit();

	struct ::sigaction sa;
	sa.sa_handler = example_notify_sigint;
	::sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0; // No SA_RESTART: SIGINT interrupts blocking fgets
	::sigaction(SIGINT, &sa, hxnull);

	int exit_code = EXIT_SUCCESS;
	if(!hx::hxconsole_exec_filename("example.cfg")) {
		hx::hxout << "error: example.cfg not found or failed to execute\n";
		exit_code = EXIT_FAILURE;
	} else {
		hx::hxarray<hx::hxarray<char, 4096>, 40>* row_storage =
			hx::hxnew<hx::hxarray<hx::hxarray<char, 4096>, 40>>();

		hx::hxtask_queue queue(40, 8);
		hx::hxarray<example_row_task, 40> tasks;

		example_render(queue, tasks, *row_storage);
		example_usage();

		char line[256];
		for(;;) {
			hx::hxout << "> ";
			if(::fgets(line, static_cast<int>(sizeof line), stdin) == hxnull) {
				break;
			}
			if(hx::hxconsole_exec_line(line)) {
				if(s_example_exit) { break; }
				example_render(queue, tasks, *row_storage);
			}
			else {
				example_usage();
			}
		}
		hx::hxdelete(row_storage);
	}

	hxshutdown();
	return exit_code;
}

} // namespace

int main(void) {
	return example_main();
}
