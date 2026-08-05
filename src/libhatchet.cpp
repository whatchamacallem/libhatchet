// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

#include "../include/hx/libhatchet.h"
#include "../include/hx/hxfile.hpp"
#include "../include/hx/hxalgorithm.hpp"

#include <stdio.h>
#if defined HX_USE_FLOATING_POINT_TRAPS
#include <fenv.h>
#endif

// This file has C linkage only. HX_NS_BEGIN_ is not used.
HX_NS_USE

extern "C" {

// hxg_init_ver_ should not be explicitly zero-initialized. MSVC handles that
// differently. If non-zero the platform has been initialized without being shut
// down.
int hxg_init_ver_; // Static initialize to 0.

// Allows observation of asserts. Return true to ignore.
bool (*hxg_assert_handler)(void);

// Exception-handling semantics exist in a few places in case they are enabled,
// but you are advised to use -fno-exceptions. This library does not provide the
// exception handling functions expected by the C++ ABI.
#if !(HX_USE_LIBCXX) && defined __cpp_exceptions && !defined __INTELLISENSE__
static_assert(0, "Warning: C++ exceptions are not supported");
#endif

// -- __cxa --------------------------------------------------------------------

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

hxattr_weak int __cxa_guard_acquire(guard_t* guard) {
	uint8_t *status = reinterpret_cast<uint8_t*>(guard);
	if (__atomic_load_n(status, __ATOMIC_ACQUIRE) == 2) { return 0; } // GCOVR_EXCL_LINE

	uint8_t expected = 0;
	while (!__atomic_compare_exchange_n(status, &expected, 1, false, __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE)) { // GCOVR_EXCL_LINE
		// GCOVR_EXCL_START
		if (expected == 2) { return 0; }
		expected = 0;
		// GCOVR_EXCL_STOP
	}
	return 1;
}

hxattr_weak void __cxa_guard_release(guard_t* guard) {
	__atomic_store_n(reinterpret_cast<uint8_t*>(guard), 2, __ATOMIC_RELEASE);
}

// GCOVR_EXCL_START
hxattr_weak void __cxa_guard_abort(guard_t* guard) {
	__atomic_store_n(reinterpret_cast<uint8_t*>(guard), 0, __ATOMIC_RELEASE);
}

hxattr_weak void __cxa_deleted_virtual(void) {
	hxassert_hard(0, "__cxa_deleted_virtual");
}

hxattr_weak void __cxa_pure_virtual(void) {
	hxassert_hard(0, "__cxa_pure_virtual");
}
// GCOVR_EXCL_STOP

#endif

// -- __sanitizer_report_error_summary -----------------------------------------
// Hooks clang's sanitizers into the debugger by overriding a weak library
// symbol in the sanitizer support library. This provides clickable error
// messages in VS Code and is otherwise unused.

void __sanitizer_report_error_summary(const char *error_summary); // NOLINT(bugprone-reserved-identifier)

// GCOVR_EXCL_START
hxattr_weak void __sanitizer_report_error_summary(const char *error_summary) { // NOLINT(bugprone-reserved-identifier)
	// A clickable message has already been printed to standard output.
	hxbreakpoint(); (void)error_summary;
}
// GCOVR_EXCL_STOP

// -- libhatchet ---------------------------------------------------------------
// Initialization, shutdown, exit, assert, and logging.

hxattr_weak void hxinit_internal(int version) {
	// Check if compile time version matches callers.
	hxassert_hard(LIBHATCHET_VER == version, "hxinit binary mismatch");
	hxassert_hard((hxg_init_ver_ == 0) || (hxg_init_ver_ == version), "hxinit after shutdown");
	(void)version;

	if(hxg_init_ver_ == 0) {
		hxsettings_construct_();

#if HX_USE_FLOATING_POINT_TRAPS
		// You need the math library -lm. This is a nonstandard glibc/_GNU_SOURCE extension.
		::feenableexcept(FE_DIVBYZERO | FE_INVALID | FE_OVERFLOW);
#endif

		hxmemory_manager_init_();
		hxg_init_ver_ = LIBHATCHET_VER;
	}
}

hxattr_weak void hxshutdown(void) {
	if(hxg_init_ver_ != 0) { // GCOVR_EXCL_LINE. Should only be called once.
		hxmemory_manager_shut_down_();

#if HX_USE_FLOATING_POINT_TRAPS
		::feenableexcept(0);
#endif

		// Trap reinitialization. This intentionally breaks global destructors
		// that call hxfree. Leak tracking has to run before that. Just don't
		// call hxshutdown() if you don't need this.
		hxg_init_ver_ = 1;
	}
}

#if (HX_HARDENING_MODE) == HX_HARDENING_MODE_DEBUG
hxattr_weak hxattr_noexcept bool hxassert_handler(const char* file, size_t line) {
	for(const char* it = file; *it != '\0'; ++it) {
		if(*it == '/' || *it == '\\') {
			file = it + 1;
		}
	}
	if(hxg_assert_handler != hxnull && hxg_assert_handler()) {
		return true;
	}
	// GCOVR_EXCL_START
	hxlog_handler(hxlog_level_assert, "breakpoint %s(%zu)", file, line);
	// Return to hxbreakpoint at the calling line.
	return false;
	// GCOVR_EXCL_STOP
}
#else
hxattr_weak hxattr_noexcept void hxassert_handler(void) {
	if(hxg_assert_handler != hxnull && hxg_assert_handler()) {
		return;
	}
	hxlog_handler(hxlog_level_assert, "ASSERT_FAIL\n");
	hxexit(EXIT_FAILURE);
}
#endif

hxattr_weak hxattr_noexcept void hxset_assert_handler(bool (*handler)(void)) {
	hxg_assert_handler = handler;
}

hxattr_weak hxattr_noexcept void hxlog_handler(hxlog_level_t level, const char* format, ...) {
	va_list args;
	va_start(args, format);
	hxlog_handler_v(level, format, args);
	va_end(args);
}

hxattr_weak hxattr_noexcept void hxlog_handler_v(hxlog_level_t level, const char* format, va_list args) {
	if(hxg_settings.log_level > level && (hxg_init_ver_ != 0)) { // GCOVR_EXCL_LINE
		return;
	}

	// vsnprintf leaves a trailing NUL that may be overwritten below.
	char line_buf[HX_MAX_LINE];
	int len = ::vsnprintf(line_buf, HX_MAX_LINE-1, format, args);

	// Do not try to print the format string because it may be corrupt.
	// Assume "hxlog_handler_v" will not cause recursion when logged.
	hxassert_hard(len >= 0 && len < ((HX_MAX_LINE)-1), "hxlog_handler_v");
	len = hxclamp(len, 0, (HX_MAX_LINE)-2);

#if HX_USE_FILE_IO
	hxfile& f = level == hxlog_level_log ? hxout : hxerr;
	if(level == hxlog_level_warning) {
		f << "WARNING ";
		line_buf[len++] = '\n';
	}
	else if(level == hxlog_level_assert) {
		f << "ASSERT_FAIL ";
		line_buf[len++] = '\n';
	}
	f.write(line_buf, static_cast<size_t>(len));
#else
	// Fall back to stdout when there is no filesystem.
	if(level == hxlog_level_warning) {
		::fputs("WARNING ", stdout);
		line_buf[len++] = '\n';
	}
	else if(level == hxlog_level_assert) {
		::fputs("ASSERT_FAIL ", stdout);
		line_buf[len++] = '\n';
	}
	::fwrite(line_buf, 1u, static_cast<size_t>(len), stdout);
#endif
}

// Make sure error messages are reported. Does not support coverage testing.
// GCOVR_EXCL_START
hxattr_noreturn hxattr_weak hxattr_noexcept void hxexit(int status) {
#if HX_USE_FILE_IO
	hxout.flush();
	hxerr.flush();
#else
	::fflush(stdout);
#endif
	::_Exit(status);
}
// GCOVR_EXCL_STOP

} // extern "C"
