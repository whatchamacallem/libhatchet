// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

#include "../include/hx/hxprofiler.hpp"
#include "../include/hx/hxconsole.hpp"
#include "../include/hx/hxfile.hpp"

HX_NS_BEGIN_

#if HX_USE_PROFILER

namespace {

// -- Console Commands ---------------------------------------------------------

hxconsole_command_named(*+[](void) -> bool { hxprofiler_start(); return true; }, profilestart);
hxconsole_command_named(*+[](void) -> bool { hxprofiler_stop(); return true; }, profilestop);
hxconsole_command_named(*+[](void) -> bool { hxprofiler_log(); return true; }, profilelog);
#if HX_USE_FILE_IO
hxconsole_command_named(*+[](const char* filename) -> bool {
	hxprofiler_write_to_chrome_tracing(filename); return true; }, profilewrite);
#endif

} // namespace {
namespace hxdetail_ {

// -- hxprofiler_internal_ -----------------------------------------------------

hxprofiler_internal_ hxg_profiler_;


hxprofiler_internal_::hxprofiler_internal_() : m_is_started_(false) { }

void hxprofiler_internal_::start_(void) {
#if HX_USE_THREADS
	const hxunique_lock profiler_lock(hxg_profiler_.m_mutex_);
#endif
	m_records.clear();
	m_is_started_ = true;
}

void hxprofiler_internal_::stop_(void) {
#if HX_USE_THREADS
	const hxunique_lock profiler_lock(hxg_profiler_.m_mutex_);
#endif
	m_is_started_ = false;
}

void hxprofiler_internal_::log_(void) {
#if HX_USE_THREADS
	const hxunique_lock profiler_lock(hxg_profiler_.m_mutex_);
#endif
	m_is_started_ = false;

	hxlog_handler(hxlog_level_console, "[ ");
	for(hxsize_t i = 0; i < m_records.size(); ++i) {
		const hxprofiler_record_& rec = m_records[i];

		if(i != 0) { hxlog_handler(hxlog_level_console, ",\n"); }

		const hxcycles_t delta = rec.m_end_ - rec.m_begin_;
		hxlog_handler(hxlog_level_console, "{ \"name\":\"%s\", \"ms\":%.15g, \"thread\":\"%x\" }",
		rec.m_label_, static_cast<double>(delta) * hxmilliseconds_per_cycle,
		static_cast<unsigned int>(rec.m_thread_id_));
	}
	hxlog_handler(hxlog_level_console, " ]\n");
}

size_t hxprofiler_internal_::size_(void) const {
	return sizeof(hxprofiler_header) + sizeof(hxprofiler_sample) * static_cast<size_t>(m_records.size());
}

#if HX_USE_FILE_IO
void hxprofiler_internal_::write_(hxfile& file) {
#if HX_USE_THREADS
	const hxunique_lock profiler_lock(hxg_profiler_.m_mutex_);
#endif
	m_is_started_ = false;

	const hxprofiler_header header = {
		hxc_profiler_header64, hxc_profiler_version, static_cast<uint32_t>(m_records.size()) };
	file.write(&header, sizeof header);

	hxprofiler_sample samples[16];
	hxsize_t batch_size = 0;

	const hxprofiler_record_* const end = m_records.end();
	for(const hxprofiler_record_* hxrestrict rec = m_records.begin(); rec != end; ++rec) {
		hxprofiler_sample& sample = samples[batch_size];
		::memset(&sample, 0x00, sizeof sample);
		::strncpy(sample.sample_label, rec->m_label_, hxc_profiler_label_max_size);
		sample.sample_begin = rec->m_begin_;
		sample.sample_end = rec->m_end_;
		sample.sample_thread_id = rec->m_thread_id_;

		if(++batch_size == 16) {
			file.write(samples, sizeof samples);
			batch_size = 0;
		}
	}

	if(batch_size != 0) {
		file.write(samples, sizeof(hxprofiler_sample) * static_cast<size_t>(batch_size));
	}
}

// https://ui.perfetto.dev/
void hxprofiler_internal_::write_to_chrome_tracing_(const char* filename) {
#if HX_USE_THREADS
	const hxunique_lock profiler_lock(hxg_profiler_.m_mutex_);
#endif
	m_is_started_ = false;

	hxfile f(hxfile::open_mode_out, "%s", filename);

	f.print("[\n");
	if(!m_records.empty()) {
		const hxcycles_t epoch = m_records[0].m_begin_;
		bool first = true;
		for(hxsize_t i = 0; i < m_records.size(); ++i) {
			const hxprofiler_record_& rec = m_records[i];

			if(rec.m_end_ < rec.m_begin_) { continue; } // GCOV_EXCL_LINE

			if(!first) { f.print(",\n"); }
			first = false;
			f.print("{\"name\":\"%s\",\"cat\":\"PERF\",\"ph\":\"B\",\"pid\":0,\"tid\":%u,\"ts\":%.15g},\n",
				rec.m_label_,
				static_cast<unsigned int>(rec.m_thread_id_),
				static_cast<double>(rec.m_begin_ - epoch) * hxmicroseconds_per_cycle);
			f.print("{\"name\":\"%s\",\"cat\":\"PERF\",\"ph\":\"E\",\"pid\":0,\"tid\":%u,\"ts\":%.15g}",
				rec.m_label_,
				static_cast<unsigned int>(rec.m_thread_id_),
				static_cast<double>(rec.m_end_ - epoch) * hxmicroseconds_per_cycle);
		}
	}
	f.print("\n]\n");

	hxlog_handler(hxlog_level_console, "wrote %s.\n", filename);
}
#endif // HX_USE_FILE_IO
} // hxdetail_
#endif // HX_USE_PROFILER
HX_NS_END_
