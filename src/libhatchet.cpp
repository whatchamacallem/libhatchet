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
#if (HX_RELEASE) == 0 && defined __GLIBC__ && !defined __FAST_MATH__
#include <fenv.h>
#if !defined HX_FLOATING_POINT_TRAPS
#define HX_FLOATING_POINT_TRAPS 1
#endif
#else
#undef HX_FLOATING_POINT_TRAPS
#define HX_FLOATING_POINT_TRAPS 0
#endif

// Exception-handling semantics exist in case they are enabled, but you are
// advised to use -fno-exceptions. This library does not provide the exception
// handling functions expected by the C++ ABI. In this codebase memory
// allocation cannot fail, and it encourages allocating enough memory in
// advance. The creation of hxthread.h classes cannot fail. By design there are
// no exceptions to handle, although there are many asserts.
#if HX_NO_LIBCXX && defined __cpp_exceptions && !defined __INTELLISENSE__
static_assert(0, "Warning: C++ exceptions are not supported for embedded use.");
#endif

// ----------------------------------------------------------------------------

extern "C" {

void __sanitizer_report_error_summary(const char *error_summary); // NOLINT

#if HX_NO_LIBCXX

// When not hosted, do not lock around the initialization of function-scope
// static variables. Provide release-mode asserts to enforce that locking is
// unnecessary and to ensure function-static constructors do not _throw_.
// Also provide error handling for virtual tables.

// Provide declarations even though they are part of the ABI.
int __cxa_guard_acquire(size_t *guard);
void __cxa_guard_release(size_t *guard);
void __cxa_guard_abort(uint64_t *guard);
void __cxa_deleted_virtual(void);
void __cxa_pure_virtual(void);

int __cxa_guard_acquire(size_t *guard) {
	// Return 0 if already constructed.
	if(*guard == 1u) { return 0; }

	// Function scope statics must be initialized before calling worker threads.
	// Checks whether the constructor is already in progress and flags any
	// potential race condition or reentrancy.
	hxassertrelease(*guard != 2u, "__cxa_guard_acquire no function scope static lock");
	*guard = 2u;
	return 1; // Signal construction required.
}

void __cxa_guard_release(size_t *guard) {
	// Marks the constructor as done and clears the in-progress flag.
	*guard = 1u;
}

void __cxa_guard_abort(uint64_t *guard) {
	hxassertrelease(0, "__cxa_guard_abort");
	*guard = 0u;
}

void __cxa_deleted_virtual(void) {
	hxassertrelease(0, "__cxa_deleted_virtual");
}

void __cxa_pure_virtual(void) {
	hxassertrelease(0, "__cxa_pure_virtual");
}

#endif

// ----------------------------------------------------------------------------
// Hooks clang's sanitizers into the debugger by overriding a weak library
// symbol in the sanitizer support library. This provides clickable error
// messages in VS Code and is otherwise unused.

void __sanitizer_report_error_summary(const char *error_summary) {
	// A clickable message has already been printed to standard output.
	hxbreakpoint(); (void)error_summary;
}

} // extern "C"

// ----------------------------------------------------------------------------
// Initialization, shutdown, exit, assert, and logging.

extern "C"
void hxinit_internal(int version_) {
	// Check if compiled in expected_version matches callers.
	const long expected_version = LIBHATCHET_VER;
	hxassertrelease(expected_version == version_, "LIBHATCHET_VER mismatch.");
	hxassertrelease((g_hxinit_ver_ == 0) || (g_hxinit_ver_ == version_), "LIBHATCHET_VER mismatch.");
	(void)version_; (void)expected_version;

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
hxattr_noexcept void hxloghandler(hxloglevel_t level, const char* format, ...) {
	va_list args;
	va_start(args, format);
	hxloghandler_v(level, format, args);
	va_end(args);
}

extern "C"
hxattr_noexcept void hxloghandler_v(hxloglevel_t level, const char* format, va_list args) {
	if((g_hxinit_ver_ != 0) && g_hxsettings.log_level > level) {
		return;
	}

	// vsnprintf leaves a trailing NUL that may be overwritten below.
	char line_buf[HX_MAX_LINE];
	int len = ::vsnprintf(line_buf, HX_MAX_LINE, format, args);

	// Do not try to print the format string because it may be invalid.
	hxassertrelease(len >= 0 && len < (int)HX_MAX_LINE, "vsnprintf");

	hxfile& f = level == hxloglevel_log ? hxout : hxerr;
	if(level == hxloglevel_warning) {
		f << "WARNING ";
		line_buf[len++] = '\n';
	}
	else if(level == hxloglevel_assert) {
		f.write("ASSERT_FAIL ", (sizeof "ASSERT_FAIL ") - 1u);
		line_buf[len++] = '\n';
	}
	f.write(line_buf, (size_t)len);
}

// HX_RELEASE < 3 provides facilities for testing teardown. Just call _Exit() otherwise.
extern "C"
void hxshutdown(void) {
	if(g_hxinit_ver_ != 0) {
#if (HX_RELEASE) < 3
		hxmemory_manager_shut_down();
		// Try to trap further activity. This breaks global destructors that call
		// hxfree. There is no easier way to track leaks.
#endif
		g_hxinit_ver_ = 0;
	}
}

#if (HX_RELEASE) == 0
extern "C"
hxattr_noexcept bool hxasserthandler(const char* file, size_t line) {
	const char* f = hxbasename(file);
	if((g_hxinit_ver_ != 0) && g_hxsettings.asserts_to_be_skipped > 0) {
		--g_hxsettings.asserts_to_be_skipped;
		hxloghandler(hxloglevel_assert, "skipped %s(%zu)",
			f, line);
		return true;
	}
	hxloghandler(hxloglevel_assert, "breakpoint %s(%zu)\n",
		f, line);

	// Return to hxbreakpoint at the calling line.
	return false;
}
#else
extern "C"
hxattr_noexcept hxattr_noreturn void hxasserthandler(void) {
	hxloghandler(hxloglevel_assert, "exit\n");
	_Exit(EXIT_FAILURE);
}
#endif
