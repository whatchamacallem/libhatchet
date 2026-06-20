// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

#include "../include/hx/hxprofiler.hpp"
#include "../include/hx/hxconsole.hpp"
#include "../include/hx/hxfile.hpp"

#if HX_PROFILE

namespace {

// ----------------------------------------------------------------------------
// Console commands

#if HX_CPLUSPLUS >= 202002L
bool hxprofile_start_command_(void) { hxprofiler_start(); return true; }

bool hxprofile_stop_command_(void) { hxprofiler_stop(); return true; }

bool hxprofiler_log_command_(void) { hxprofiler_log(); return true; }

bool hxprofiler_write_to_chrome_tracing_command_(const char* filename) {
	hxprofiler_write_to_chrome_tracing(filename);
	return true;
}

hxconsole_command_named(hxprofile_start_command_, profilestart);
hxconsole_command_named(hxprofile_stop_command_, profilestop);
hxconsole_command_named(hxprofiler_log_command_, profilelog);
hxconsole_command_named(hxprofiler_write_to_chrome_tracing_command_, profilewrite);
#endif // HX_CPLUSPLUS >= 202002L

} // namespace {

// ----------------------------------------------------------------------------
// Variables

namespace hxdetail_ {

hxprofiler_internal_ g_hxprofiler_;

// ----------------------------------------------------------------------------
// hxprofiler_internal_

void hxprofiler_internal_::start_(void) {
	HX_PROFILER_LOCK_();
	m_records.clear();
	m_is_started_ = true;
}

void hxprofiler_internal_::stop_(void) {
	HX_PROFILER_LOCK_();
	m_is_started_ = false;
}

void hxprofiler_internal_::log_(void) {
	HX_PROFILER_LOCK_();
	m_is_started_ = false;

	hxlogconsole("[ ");
	for(size_t i = 0; i < m_records.size(); ++i) {
		const hxprofiler_record_& rec = m_records[i];

		if(i != 0) { hxlogconsole(",\n"); }

		const hxcycles_t delta = rec.m_end_ - rec.m_begin_;
		hxlogconsole("{ \"name\":\"%s\", \"ms\":%.15g, \"thread\":\"%x\" }",
		rec.m_label_, static_cast<double>(delta) * hxmilliseconds_per_cycle,
		static_cast<unsigned int>(rec.m_thread_id_));
	}
	hxlogconsole(" ]\n");
}

// ###
// ### NOTA BENE: Only https://ui.perfetto.dev/ is working at the moment.
// ###
void hxprofiler_internal_::write_to_chrome_tracing_(const char* filename) {
	HX_PROFILER_LOCK_();
	m_is_started_ = false;

	hxfile f(hxfile::out, "%s", filename);

	f.print("[\n");
	if(!m_records.empty()) {
		const hxcycles_t epoch = m_records[0].m_begin_;
		for(size_t i = 0; i < m_records.size(); ++i) {
			if(i != 0) { f.print(",\n"); }

			const hxprofiler_record_& rec = m_records[i];

			// Register wrapping can cause bad samples. Meanwhile Chrome has been
			// updated to generate exceptions when any sample has end < begin.
			if(rec.m_end_ < rec.m_begin_) { continue; }

			const char* label = rec.m_label_;
			f.print("{\"name\":\"%s\",\"cat\":\"PERF\",\"ph\":\"B\",\"pid\":0,\"tid\":%u,\"ts\":%.15g},\n",
				label,
				static_cast<unsigned int>(rec.m_thread_id_),
				static_cast<double>(rec.m_begin_ - epoch) * hxmicroseconds_per_cycle);
			f.print("{\"name\":\"%s\",\"cat\":\"PERF\",\"ph\":\"E\",\"pid\":0,\"tid\":%u,\"ts\":%.15g}",
				label,
				static_cast<unsigned int>(rec.m_thread_id_),
				static_cast<double>(rec.m_end_ - epoch) * hxmicroseconds_per_cycle);
		}
	}
	f.print("\n]\n");

	hxlogconsole("wrote %s.\n", filename);
}

} // hxdetail_

#endif // HX_PROFILE
