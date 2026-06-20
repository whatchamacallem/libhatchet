// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

#include "../include/hx/libhatchet.h"
#include "../include/hx/hxarray.hpp"
#include "../include/hx/hxconsole.hpp"
#include "../include/hx/hxfile.hpp"
#include "../include/hx/hxprofiler.hpp"
#include "../include/hx/hxalgorithm.hpp"

#include <stdio.h> // vsnprintf only.

// g_hxinit_ver_ should not be explicitly zero-initialized. MSVC handles that
// differently.
extern "C" {
	// If non-zero the platform has been initialized without being shut down.
	// See `#define g_hxinit_ver_` in hxsettings.h for rationale.
	int g_hxinit_ver_; // Static initialize to 0.

	// Allows observation of asserts. Return true to ignore.
	bool (*g_hxassert_handler)(void);
}

// HX_FLOATING_POINT_TRAPS - Traps (FE_DIVBYZERO|FE_INVALID|FE_OVERFLOW) in
// debug builds so you can safely run without checks in release builds. Use
// -DHX_FLOATING_POINT_TRAPS=0 to disable this debug facility. There is no C++
// standard-conforming way to disable floating point error checking. That
// requires a gcc/clang extension. Using -fno-math-errno and -fno-trapping-math
// will work if you require C++ conforming accuracy without the overhead of
// error checking. -ffast-math includes both of those switches. You need the math
// library -lm. Triggering or explicitly checking for floating point exceptions
// is not recommended.
#if (HX_HARDENING_MODE) == HX_HARDENING_MODE_DEBUG && defined __GLIBC__ && !defined __FAST_MATH__
#include <fenv.h>
#if !defined HX_FLOATING_POINT_TRAPS
#define HX_FLOATING_POINT_TRAPS 1
#endif
#else
#undef HX_FLOATING_POINT_TRAPS
#define HX_FLOATING_POINT_TRAPS 0
#endif

// Exception-handling semantics exist in a few places in case they are enabled,
// but you are advised to use -fno-exceptions. This library does not provide the
// exception handling functions expected by the C++ ABI.
#if !(HX_USE_LIBCXX) && defined __cpp_exceptions && !defined __INTELLISENSE__
static_assert(0, "Warning: C++ exceptions are not supported.");
#endif

// ----------------------------------------------------------------------------

extern "C" {

#if !(HX_USE_LIBCXX)

#if defined __arm__ && !defined __aarch64__
    typedef int guard_t;
#else
    typedef long long guard_t;
#endif

// Support for thread-safe initialization of static variables. Use
// -fno-threadsafe-statics to disable.
int __cxa_guard_acquire(guard_t* guard);
void __cxa_guard_release(guard_t* guard);
void __cxa_guard_abort(guard_t* guard);
void __cxa_deleted_virtual(void);
void __cxa_pure_virtual(void);

int __cxa_guard_acquire(guard_t* guard) {
	uint8_t *status = reinterpret_cast<uint8_t*>(guard);
	if (__atomic_load_n(status, __ATOMIC_ACQUIRE) == 2) { return 0; }

	uint8_t expected = 0;
	while (!__atomic_compare_exchange_n(status, &expected, 1, false, __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE)) {
		if (expected == 2) { return 0; }
		expected = 0;
	}
	return 1;
}

void __cxa_guard_release(guard_t* guard) {
	__atomic_store_n(reinterpret_cast<uint8_t*>(guard), 2, __ATOMIC_RELEASE);
}

void __cxa_guard_abort(guard_t* guard) {
	__atomic_store_n(reinterpret_cast<uint8_t*>(guard), 0, __ATOMIC_RELEASE);
}

void __cxa_deleted_virtual(void) {
	hxassert_hard(0, "__cxa_deleted_virtual");
}

void __cxa_pure_virtual(void) {
	hxassert_hard(0, "__cxa_pure_virtual");
}

#endif

// ----------------------------------------------------------------------------
// Hooks clang's sanitizers into the debugger by overriding a weak library
// symbol in the sanitizer support library. This provides clickable error
// messages in VS Code and is otherwise unused.

void __sanitizer_report_error_summary(const char *error_summary); // NOLINT

void __sanitizer_report_error_summary(const char *error_summary) {
	// A clickable message has already been printed to standard output.
	hxbreakpoint(); (void)error_summary;
}

} // extern "C"

// ----------------------------------------------------------------------------
// Initialization, shutdown, exit, assert, and logging.

extern "C"
void hxinit_internal(int version) {
	// Check if compiled in expected_version matches callers.
	const long expected_version = LIBHATCHET_VER;
	hxassert_hard(expected_version == version, "LIBHATCHET_VER mismatch.");
	hxassert_hard((g_hxinit_ver_ == 0) || (g_hxinit_ver_ == version), "LIBHATCHET_VER mismatch.");
	(void)version; (void)expected_version;

	if(g_hxinit_ver_ == 0) {
		hxsettings_construct();

#if HX_FLOATING_POINT_TRAPS
		// You need the math library -lm. This is a nonstandard glibc/_GNU_SOURCE extension.
		::feenableexcept(FE_DIVBYZERO | FE_INVALID | FE_OVERFLOW);
#endif

		hxmemory_manager_init();
		g_hxinit_ver_ = LIBHATCHET_VER;
	}
}

extern "C"
hxattr_noexcept void hxlog_handler(hxlog_level_t level, const char* format, ...) {
	va_list args;
	va_start(args, format);
	hxlog_handler_v(level, format, args);
	va_end(args);
}

extern "C"
hxattr_noexcept void hxlog_handler_v(hxlog_level_t level, const char* format, va_list args) {
	if((g_hxinit_ver_ != 0) && g_hxsettings.log_level > level) {
		return;
	}

	// vsnprintf leaves a trailing NUL that may be overwritten below.
	char line_buf[HX_MAX_LINE];
	int len = ::vsnprintf(line_buf, HX_MAX_LINE, format, args);

	// Do not try to print the format string because it may be invalid.
	hxassert_hard(len >= 0 && len < (int)HX_MAX_LINE, "vsnprintf");

	hxfile& f = level == hxlog_level_log ? hxout : hxerr;
	if(level == hxlog_level_warning) {
		f << "WARNING ";
		line_buf[len++] = '\n';
	}
	else if(level == hxlog_level_assert) {
		f.write("ASSERT_FAIL ", (sizeof "ASSERT_FAIL ") - 1u);
		line_buf[len++] = '\n';
	}
	f.write(line_buf, (size_t)len);
}

// HX_HARDENING_MODE != HX_HARDENING_MODE_NONE provides facilities for testing
// teardown. Just call _Exit() otherwise.
extern "C"
void hxshutdown(void) {
	if(g_hxinit_ver_ != 0) {
#if (HX_HARDENING_MODE) != HX_HARDENING_MODE_NONE
		hxmemory_manager_shut_down();
		// Try to trap further activity. This breaks global destructors that call
		// hxfree. There is no easier way to track leaks.
#endif
		g_hxinit_ver_ = 0;
	}
}

extern "C"
hxattr_noexcept void hxset_assert_handler(bool (*handler)(void)) {
	g_hxassert_handler = handler;
}

#if (HX_HARDENING_MODE) == HX_HARDENING_MODE_DEBUG
extern "C"
hxattr_noexcept bool hxassert_handler(const char* file, size_t line) {
	const char* f = hxbasename(file);
	if((g_hxinit_ver_ != 0) && g_hxsettings.asserts_to_be_skipped > 0) {
		--g_hxsettings.asserts_to_be_skipped;
		hxlog_handler(hxlog_level_assert, "skipped %s(%zu)", f, line);
		return true;
	}
	if(g_hxassert_handler != hxnull && g_hxassert_handler()) {
		return true;
	}
	hxlog_handler(hxlog_level_assert, "breakpoint %s(%zu)\n", f, line);
	// Return to hxbreakpoint at the calling line.
	return false;
}
#else
extern "C"
hxattr_noexcept void hxassert_handler(void) {
	if(g_hxassert_handler != hxnull && g_hxassert_handler()) {
		return;
	}
	hxlog_handler(hxlog_level_assert, "assert_fail\n");
	_Exit(EXIT_FAILURE);
}
#endif
