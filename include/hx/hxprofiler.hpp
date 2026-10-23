#pragma once
// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

/// \file
/// Provides macros for RAII-style profiling (`hxprofile_scope`,
/// `hxprofile_scope_min` with optional cycle cutoffs). Allows exporting raw
/// binary samples (`hxprofiler_write`) and Chrome's tracing format
/// (`hxprofiler_write_to_chrome_tracing`). Profiling is enabled only if
/// `HX_USE_PROFILER` is defined.
///
/// | Macro | Purpose |
/// | --- | --- |
/// | `hxprofile_scope` | Records scoped samples keyed by a string literal. |
/// | `hxprofile_scope_min` | Records scoped samples when the cycle cutoff is met. |
/// | `hxprofiler_start` | Clears existing samples then starts recording. |
/// | `hxprofiler_stop` | Stops recording while retaining samples. |
/// | `hxprofiler_log` | Logs captured samples to the system log. |
/// | `hxprofiler_size` | Returns the size in bytes written by `hxprofiler_write`. |
/// | `hxprofiler_write` | Writes captured samples as binary to an `hxfile`. |
/// | `hxprofiler_write_to_chrome_tracing` | Writes captured samples to Chrome Trace Event JSON. |

#include "libhatchet.h"
#if !(HX_USE_MACROS_WITH_MODULE)

// These are always included so the build doesn't break when the profiler is
// turned off.
#include "hxvector.hpp"
#include "hxthread.hpp"

HX_NS_BEGIN_

#if HX_USE_FILE_IO
class hxfile;
#endif

/// `hxcycles_t` - Stores approximately 1.5 seconds to 209 years worth of
/// processor cycles starting from an unspecified origin and wrapping using
/// unsigned rules. This is intended for profiling, not calendaring. Used by the
/// following include.
using hxcycles_t = size_t;

/// `hxcycles_per_second` - WARNING: Customize for your processor speed. This
/// assumes 2.8 GHz. These constants are primarily used with `printf`, which
/// promotes everything to double anyway.
hxinline_constexpr double hxcycles_per_second = HX_CYCLES_PER_SECOND;

/// `hxmilliseconds_per_cycle` - The number of milliseconds in a single
/// processor cycle, derived from `hxcycles_per_second`.
hxinline_constexpr double hxmilliseconds_per_cycle = 1.0e+3 / hxcycles_per_second;

/// `hxmicroseconds_per_cycle` - The number of microseconds in a single
/// processor cycle, derived from `hxcycles_per_second`.
hxinline_constexpr double hxmicroseconds_per_cycle = 1.0e+6 / hxcycles_per_second;

/// `hxc_profiler_header64` - Magic number beginning raw profiler data.
constexpr uint64_t hxc_profiler_header64 = 0x0123456789abcdefull;

/// `hxc_profiler_label_max_size` - Maximum label length stored in a sample.
constexpr size_t hxc_profiler_label_max_size = 15u;

/// `hxc_profiler_version` - Version of the raw profiler data layout.
constexpr uint32_t hxc_profiler_version = 100u;

/// `hxtime_sample_cycles(void)` - Set up the processor cycle counter for your
/// architecture. This is callable without enabling `HX_USE_PROFILER`.
hxinline hxcycles_t hxtime_sample_cycles(void);

/// `hxprofiler_sample` - Binary record layout for a profiler sample.
class hxprofiler_sample {
public:
	/// Null-terminated label truncated to `hxc_profiler_label_max_size`.
	char sample_label[hxc_profiler_label_max_size + 1u];
	/// Cycle count at scope entry.
	uint64_t sample_begin;
	/// Cycle count at scope exit.
	uint64_t sample_end;
	/// Thread id the scope was sampled on.
	uint32_t sample_thread_id;
};

/// `hxprofiler_header` - Binary header for raw profiler data. Followed
/// immediately by an `hxprofiler_sample` array of `sample_size` elements.
class hxprofiler_header {
public:
	/// Always `hxc_profiler_header64`.
	uint64_t profile_header;
	/// Always `hxc_profiler_version`.
	uint32_t profile_version;
	/// Number of samples following the header.
	uint32_t sample_size;
};

HX_NS_END_

#endif // !HX_USE_MACROS_WITH_MODULE

/// `hxprofile_scope(const char* label_string_literal)` - Declares an RAII-style
/// profiling sample. WARNING: A pointer to `label_string_literal` is kept.
/// Compiles to a NOP when not in use.
/// - `label_string_literal` : A string literal label for the sample.
#define hxprofile_scope(label_string_literal_) \
	HX_PROFILE_ONLY_(const HX_NS_PREFIX_ hxdetail_::hxprofiler_scope_internal_<> \
		HX_APPEND_COUNTER_(hxprofile_scope_)(label_string_literal_))

/// `hxprofile_scope_min(const char* label_string_literal, hxcycles_t
/// min_cycles)` - Declares an RAII-style profiling sample with a minimum cycle
/// cutoff. WARNING: A pointer to `label_string_literal` is kept. Compiles to a
/// NOP when not in use.
/// - `label_string_literal` : A string literal label for the sample.
/// - `min_cycles` : A minimum number of cycles required for a sample to be
///   recorded. Must be a compile-time constant.
#define hxprofile_scope_min(label_string_literal_, min_cycles_) \
	HX_PROFILE_ONLY_(const HX_NS_PREFIX_ hxdetail_::hxprofiler_scope_internal_<min_cycles_> \
		HX_APPEND_COUNTER_(hxprofile_scope_)(label_string_literal_))

/// `hxprofiler_start(void)` - Clears samples and begins sampling. Compiles to a
/// NOP when not in use.
#define hxprofiler_start() HX_PROFILE_ONLY_(HX_NS_PREFIX_ hxdetail_::hxg_profiler_.start_())

/// `hxprofiler_stop(void)` - Ends sampling. Does not clear samples. Compiles to
/// a NOP when not in use.
#define hxprofiler_stop() HX_PROFILE_ONLY_(HX_NS_PREFIX_ hxdetail_::hxg_profiler_.stop_())

/// `hxprofiler_log(void)` - Stops sampling and writes samples to the system
/// log. Compiles to a NOP when not in use.
#define hxprofiler_log() HX_PROFILE_ONLY_(HX_NS_PREFIX_ hxdetail_::hxg_profiler_.log_())

#if HX_USE_PROFILER
/// `hxprofiler_size(void)` - Returns the size in bytes `hxprofiler_write` will
/// write. Stop sampling before use. Returns `0` when not in use.
#define hxprofiler_size() (HX_NS_PREFIX_ hxdetail_::hxg_profiler_.size_())
#else
#define hxprofiler_size() (0u)
#endif

#if HX_USE_FILE_IO
/// `hxprofiler_write(hxfile& file)` - Stops sampling and writes an
/// `hxprofiler_header` and the samples to `file`. NOP when not in use.
/// - `file` : An `hxfile` open for writing.
#define hxprofiler_write(file_) HX_PROFILE_ONLY_(HX_NS_PREFIX_ hxdetail_::hxg_profiler_.write_(file_))

/// `hxprofiler_write_to_chrome_tracing(const char* filename)` - Stops sampling
/// and writes Chrome Trace Event JSON which is viewable using `speedscope` or
/// over at https://ui.perfetto.dev/. Compiles to a NOP when not in use.
/// - `filename` : Path to the output `.json` file.
#define hxprofiler_write_to_chrome_tracing(filename_) \
	HX_PROFILE_ONLY_(HX_NS_PREFIX_ hxdetail_::hxg_profiler_.write_to_chrome_tracing_(filename_))
#endif
#include "detail/hxprofiler_detail.hpp"
