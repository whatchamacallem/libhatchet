// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

#include <hx/hxprofiler.hpp>
#include <hx/hxtask_queue.hpp>
#include <hx/hxconsole.hpp>
#include <hx/hxrandom.hpp>
#include <hx/hxutility.h>
#include <hx/hxtest.hpp>

HX_NS_USE

#if HX_USE_PROFILER
namespace {

const char* hxs_test_labels[] = {
	"Alpha",   "Beta",	 "Gamma",
	"Delta",   "Epsilon",  "Zeta",
	"Eta",	 "Theta",	"Iota",
	"Kappa",   "Lambda",   "Mu",
	"Nu",	   "Xi",	  "Omicron",
	"Pi",	   "Rho",	 "Sigma",
	"Tau",	  "Upsilon", "Phi",
	"Chi",	  "Psi",	 "Omega"
};
const hxsize_t hxs_test_num_labels = hxsize(hxs_test_labels);

class hxprofiler_task_test : public hxtask {
public:
	hxprofiler_task_test(void) :
		m_target_ms(0.0f), m_accumulator(0u), m_label(hxnull) { }
	void construct(const char* label, float target_ms) {
		m_label = label;
		m_target_ms = target_ms;
		m_accumulator = 0;
	}
	const char* get_label(void) const override { return m_label; }
	bool execute(hxtask_queue* q) override {
		(void)q;
		generate_scopes(m_target_ms);
		return true;
	}
	virtual void generate_scopes(float target_ms) {
		const hxcycles_t start_cycles = hxtime_sample_cycles();
		hxcycles_t delta = 0u;
		if(target_ms >= 2.0f) {
			const float subtarget = target_ms / 2.0f;
			const char* sub_label = hxs_test_labels[static_cast<hxsize_t>(subtarget)];
			hxprofile_scope(sub_label);
			generate_scopes(subtarget);
		}
		while(static_cast<double>(delta) * hxmilliseconds_per_cycle < target_ms) {
			const uint32_t ops = (m_accumulator & 0xf) + 1;
			for(uint32_t i = 0; i < ops; ++i) {
				m_accumulator ^= m_test_prng();
			}
			delta = hxtime_sample_cycles() - start_cycles;
		}
	}
private:
	float m_target_ms;
	uint32_t m_accumulator;
	hxrandom m_test_prng;
	const char* m_label;
};
} // namespace

TEST(hxprofiler_test, single_scope_runs_for_1ms) {
	hxprofiler_start();
	{
		hxprofile_scope("1 ms");
		hxprofiler_task_test one;
		one.construct("1 ms", 1.0f);
		one.execute(hxnull);
	}
#if HX_USE_CONSOLE
	const bool is_ok = hxconsole_exec_line("profilelog");
	EXPECT_TRUE(is_ok);
#else
	hxprofiler_log();
	SUCCEED();
#endif
}

TEST(hxprofiler_test, scope_exit_when_records_full_is_dropped) {
	hxprofiler_start();
	for(hxsize_t i = 0; i < static_cast<hxsize_t>(HX_PROFILER_MAX_RECORDS + 8u); ++i) {
		hxprofile_scope("Overflow");
	}
	hxprofiler_start();
	{
		hxprofile_scope("Overflow");
	}
	hxprofiler_log();
	SUCCEED();
}

#if HX_USE_CONSOLE
TEST(hxprofiler_test, start_and_stop_console_commands) {
	EXPECT_TRUE(hxconsole_exec_line("profilestart"));
	EXPECT_TRUE(hxconsole_exec_line("profilestop"));
}
#endif

#if HX_USE_FILE_IO
TEST(hxprofiler_test, write_to_chrome_tracing_command) {
	const hxsystem_allocator_scope temporary_stack_scope(hxsystem_allocator_stack_0);
	hxprofiler_stop();
#if HX_USE_CONSOLE
	hxconsole_exec_line("profilestart");
#else
	hxprofiler_start();
#endif
	hxtask_queue q(hxs_test_num_labels, 2u);
	hxprofiler_task_test tasks[hxs_test_num_labels];
	for(hxsize_t i = hxs_test_num_labels; i-- != 0u; ) {
		tasks[i].construct(hxs_test_labels[i], static_cast<float>(i));
		q.enqueue(tasks + i);
	}
	q.wait_for_all();
#if HX_USE_CONSOLE
	const bool is_ok = hxconsole_exec_line("profilewrite profile.json");
	EXPECT_TRUE(is_ok);
#else
	hxprofiler_write_to_chrome_tracing("profile.json");
	SUCCEED();
#endif
	hxprofiler_log();
}

TEST(hxprofiler_test, write_to_chrome_tracing_with_no_records) {
	const hxsystem_allocator_scope temporary_stack_scope(hxsystem_allocator_stack_0);
	hxprofiler_start();
	hxprofiler_write_to_chrome_tracing("profile_empty.json");
	SUCCEED();
}
#endif // HX_USE_FILE_IO
#endif // HX_USE_PROFILER
