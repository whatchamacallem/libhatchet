// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

#include <hx/hxprofiler.hpp>
#include <hx/hxtask_queue.hpp>
#include <hx/hxconsole.hpp>
#include <hx/hxfile.hpp>
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
	hxprofiler_start();
	hxprofiler_write_to_chrome_tracing("profile_empty.json");
	SUCCEED();
}

TEST(hxprofiler_test, write_round_trip_matches_size_and_samples) {
	hxprofiler_start();
	{ hxprofile_scope("Alpha"); }
	{ hxprofile_scope("Fifteen_Letters"); }
	{ hxprofile_scope("Sixteen__Letters"); }
	for(int i = 0; i < 14; ++i) {
		hxprofile_scope("Batch");
	}
	{
		hxfile writer(hxfile::open_mode_out | hxfile::open_mode_asserts, "profile.bin");
		hxprofiler_write(writer);
	}

	alignas(hxprofiler_header) uint8_t buffer[sizeof(hxprofiler_header) + 17u * sizeof(hxprofiler_sample)];
	EXPECT_EQ(hxprofiler_size(), sizeof buffer);
	hxfile reader(hxfile::open_mode_in, "profile.bin");
	EXPECT_EQ(reader.read(buffer, sizeof buffer, sizeof buffer), sizeof buffer);
	uint8_t extra = 0u;
	EXPECT_EQ(reader.read(&extra, sizeof extra, sizeof extra), 0u);
	EXPECT_TRUE(reader.eof());

	const hxprofiler_header* const header = reinterpret_cast<const hxprofiler_header*>(buffer);
	EXPECT_EQ(header->profile_header, hxc_profiler_header64);
	EXPECT_EQ(header->profile_version, hxc_profiler_version);
	EXPECT_EQ(header->sample_size, 17u);

	const hxprofiler_sample* const samples = reinterpret_cast<const hxprofiler_sample*>(header + 1);
	EXPECT_STREQ(samples[0].sample_label, "Alpha");
	EXPECT_STREQ(samples[1].sample_label, "Fifteen_Letters");
	EXPECT_STREQ(samples[2].sample_label, "Sixteen__Letter");
	EXPECT_EQ(samples[2].sample_label[hxc_profiler_label_max_size], '\0');
	EXPECT_LE(samples[0].sample_begin, samples[0].sample_end);
	EXPECT_LE(samples[0].sample_end, samples[1].sample_begin);
	EXPECT_LE(samples[1].sample_end, samples[2].sample_begin);
	EXPECT_LE(samples[2].sample_begin, samples[2].sample_end);
	EXPECT_EQ(samples[0].sample_thread_id, static_cast<uint32_t>(hxthread_id()));
	EXPECT_EQ(samples[2].sample_thread_id, static_cast<uint32_t>(hxthread_id()));

	EXPECT_STREQ(samples[15].sample_label, "Batch");
	EXPECT_STREQ(samples[16].sample_label, "Batch");
	EXPECT_LE(samples[15].sample_end, samples[16].sample_begin);
}
#endif // HX_USE_FILE_IO
#endif // HX_USE_PROFILER
