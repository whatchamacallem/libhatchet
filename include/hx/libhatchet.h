#pragma once
// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

/// \file hx/libhatchet.h
/// libhatchet. Requires C99 for C. C++11 is minimum for C++. Utilizes
/// language features up to C++20. Inclusion on the compiler search path is not
/// required. However, the headers are intended to be included as follows:
/// `#include <hx/libhatchet.h>`
///
/// Defines logging macros `hxlog`, `hxlogrelease`, `hxlogconsole`,
/// `hxlogwarning` which vary by `HX_RELEASE` level (0–3) and defines log
/// verbosity { log, console, warning, assert }.
///
/// Assertion macros `hxassert`, `hxassertmsg`, `hxassertrelease` are provided
/// for debugging, active when `HX_RELEASE < 3`. `hxinit` initializes the
/// platform and `hxshutdown` releases resources when `HX_RELEASE < 3`.

// Use minimal C style headers. The std:: namespace may not exist. "You can't
// get there from here."
#include <errno.h>
#include <signal.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#if  !defined __STDC_VERSION__ || __STDC_VERSION__ < 202311l
#include <stdbool.h>
#endif

/// `int LIBHATCHET_VER` - One digit major, and two digit minor and patch
/// versions. Odd numbered minor versions are development branches.
#define LIBHATCHET_VER 12200

/// `LIBHATCHET_TAG` - Major, minor and patch version tag name. Odd numbered
/// minor versions are development branches and their tags end in `-dev`.
#define LIBHATCHET_TAG "v1.22.0"

#include "hxsettings.h"
#include "hxmemory_manager.h"

#if HX_CPLUSPLUS
extern "C" {
#endif

/// Compile-time assertion for `HX_RELEASE` [0..3] range.
#if (HX_RELEASE) < 0 || (HX_RELEASE) >= 4
#error HX_RELEASE must be [0..3].
#endif

// libhatchet C and C++ API. Above headers are C and C++ too.

/// `hxloglevel_t` - Runtime setting for verbosity of log messages.
/// Independently controls what messages are compiled in. See
/// `g_hxsettings.log_level`.
enum hxloglevel_t {
	/// Written to `hxout`. Structured output. No automatic newline.
	hxloglevel_log,
	/// Written to `hxerr`. Unstructured informative output including error
	/// messages regarding console commands and `hxtest` results. No automatic
	/// newline. No news is good news.
	hxloglevel_console,
	/// Written to `hxerr`. Warnings about serious problems.
	hxloglevel_warning,
	/// Written to `hxerr`. Reason for abnormal termination or test failure.
	hxloglevel_assert
};

/// `hxnull` - The null pointer value for a given pointer type represented by
/// the numeric constant `0`. The C/C++ language standards explicitly define the
/// meaning of `0` in pointer context as a null pointer of the expected type.
/// However they do not define whether `NULL` is `0` or `((void*)0)`. `hxnull`
/// fills that gap by having an unambiguous type. See `hxnullptr`/`hxnullptr_t` if
/// you need a `std::nullptr_t` replacement.
#define hxnull 0

/// `hxhash_t` - Unsigned 32-bit hash value. Expect collisions.
typedef uint32_t hxhash_t;

/// `hxhash_bits` - Number of bits in `hxhash_t`.
#define hxhash_bits 32u

/// `hxinit` - Initializes the platform if needed. Does a quick version check to
/// determine if the platform is already correctly initialized first. Designed
/// to trigger a link error when used against previous versions. It uses
/// `g_hxinit_ver_` which is renamed to encode the current version. See
/// `g_hxinit_ver_`. As well, `LIBHATCHET_VER` is checked against the value
/// hxinit_internal was compiled with.
#define hxinit() (void)(g_hxinit_ver_ == LIBHATCHET_VER || (hxinit_internal(LIBHATCHET_VER), 0))

#if (HX_RELEASE) == 0 // These are debug facilities.

/// `hxlog(...)` - Enters formatted messages in the system log. Does not add a
/// newline. This is only evaluated when `HX_RELEASE == 0`.
/// - `...` Printf-style formatted log message.
#define hxlog(...) hxloghandler(hxloglevel_log, __VA_ARGS__)

/// `hxassert(bool x)` - Logs an error and terminates execution if `x` is false.
/// This is only evaluated when `HX_RELEASE == 0`. Always evaluates to an
/// expression of type `void`.
/// - `x` : The condition to evaluate.
#define hxassert(x_) (void)((bool)(x_)   /* This is also a compile time assert: */  \
	|| (hxloghandler_(hxloglevel_assert, #x_), hxasserthandler(__FILE__, __LINE__)) \
	|| hxbreakpoint())

/// `hxassertmsg(bool x, ...)` - Logs an error and terminates execution if `x`
/// is false. Does not evaluate message args unless condition fails. This is
/// only evaluated when `HX_RELEASE == 0`. Always evaluates to an expression of
/// type `void`. e.g., `hxassertmsg(x == 0, "x: %d", x)`.
/// - `x` : The condition to evaluate.
/// - `...` Printf-style formatted log message.
#define hxassertmsg(x_, ...) (void)((bool)(x_)    /* This is also a compile time assert: */ \
	|| (hxloghandler_(hxloglevel_assert, __VA_ARGS__), hxasserthandler(__FILE__, __LINE__)) \
	|| hxbreakpoint())

/// `hxassertrelease(bool x, ...)` - Logs an error and terminates execution if
/// `x` is false up to release level 2. This is only evaluated when `HX_RELEASE
/// < 3`. Always evaluates to an expression of type `void`.
/// - `x` : The condition to evaluate.
/// - `...` Printf-style formatted log message.
#define hxassertrelease(x_, ...) (void)((bool)(x_) /* This is also a compile time assert: */ \
	|| (hxloghandler_(hxloglevel_assert, __VA_ARGS__), hxasserthandler(__FILE__, __LINE__))  \
	|| hxbreakpoint())

/// Assert handler. Do not call directly, signature changes and then is removed.
/// NOTA BENE: Compile errors from consteval code calling this function are
/// intentional and are how you know a compile time assert has been hit.
bool hxasserthandler(const char* file_, size_t line_) hxattr_noexcept hxattr_nonnull(1) hxattr_cold;

#else // HX_RELEASE >= 1

// Normal asserts are disabled entirely above HX_RELEASE 0 and below HX_RELEASE 3.
#if (HX_RELEASE) < 3
#define hxassertmsg(x_, ...) ((void)0)
#define hxassert(x_) ((void)0)
#endif // (HX_RELEASE) < 3

#define hxlog(...) ((void)0)

/// NOTA BENE: Compile errors from consteval code calling this function are
/// intentional and are how you know a compile time assert has been hit.
void hxasserthandler(void) hxattr_noexcept hxattr_noreturn hxattr_cold;

#endif // HX_RELEASE >= 1

#if (HX_RELEASE) <= 1
/// `hxlogrelease(...)` - Enters formatted messages in the system log up to
/// release level 1. No automatic newline. This is only evaluated when
/// `HX_RELEASE <= 1`.
/// - `...` Printf-style formatted log message.
#define hxlogrelease(...) hxloghandler(hxloglevel_log, __VA_ARGS__)

/// `hxlogconsole(...)` - Enters formatted messages in the console system log.
/// This is only evaluated when `HX_RELEASE <= 1`.
/// - `...` Variadic arguments for the formatted console log message.
#define hxlogconsole(...) hxloghandler(hxloglevel_console, __VA_ARGS__)

/// `hxlogwarning(...)` - Enters formatted warnings in the system log. This is
/// only evaluated when `HX_RELEASE <= 1`.
/// - `...` Variadic arguments for the formatted warning message.
#define hxlogwarning(...) hxloghandler(hxloglevel_warning, __VA_ARGS__)

/// `hxwarnmsg(bool x, ...)` - Enters formatted warnings in the system log when
/// `x` is false. This is only evaluated when `HX_RELEASE <= 1`.
/// - `x` : The condition to evaluate.
/// - `...` Variadic arguments for the formatted warning message.
#define hxwarnmsg(x_, ...) (void)((bool)(x_) \
	|| (hxloghandler(hxloglevel_warning, __VA_ARGS__), 0))

#else // HX_RELEASE >= 2
#define hxlogrelease(...) ((void)0)
#define hxlogconsole(...) ((void)0)
#define hxlogwarning(...) ((void)0)
#define hxwarnmsg(x_, ...) ((void)0)
#endif

// hxassertrelease has 4 variations. See above. It is only evaluated when
// HX_RELEASE < 3.
#if (HX_RELEASE) == 1
#define hxassertrelease(x_, ...) (void)((bool)(x_)     \
	|| (hxloghandler_(hxloglevel_assert, __VA_ARGS__), \
		hxasserthandler(), 0)) // This is also a compile time assert.
#elif (HX_RELEASE) == 2
#define hxassertrelease(x_, ...) (void)((bool)(x_) \
	|| (hxasserthandler(), 0)) // This is also a compile time assert.
#elif (HX_RELEASE) == 3
// This is an extreme level of optimization where asserts are assumed to be
// valid.
#define hxassert(x_) hxattr_assume(x_)
#define hxassertmsg(x_, ...) hxattr_assume(x_)
#define hxassertrelease(x_, ...) hxattr_assume(x_)
#endif

/// `hxinit_internal` - Internal. Use `hxinit` instead. It checks `g_hxinit_ver_`.
void hxinit_internal(int version_) hxattr_cold;

/// `g_hxinit_ver_` - Internal. Set to current library version by `hxinit`. This
/// is renamed in settings.h to contain the library version in order to cause
/// meaningful link errors when linked against stale binaries. The linker symbol
/// should look like `g_hxinit_ver31700_`.
extern int g_hxinit_ver_;

/// `hxshutdown` - Terminates service. Releases all resources acquired by the
/// platform and confirms all memory allocations have been released. `HX_RELEASE
/// < 3`.
void hxshutdown(void) hxattr_cold;

/// `hxloghandler` - Enters formatted messages in the system log. This is the
/// only access to logging when `HX_RELEASE > 2`.
/// - `level` : The log level (e.g., `hxloglevel_log`, `hxloglevel_warning`).
/// - `format` : Non-null `printf`-style format string.
/// - `...` Additional arguments that must satisfy the format string.
void hxloghandler(enum hxloglevel_t level_, const char* format_, ...) hxattr_noexcept hxattr_printf(2, 3);

/// `hxloghandler_v` - A `va_list` version of `hxloghandler`. This is the only
/// access to logging when `HX_RELEASE > 2`.
/// - `level` : The log level (e.g., `hxloglevel_log`, `hxloglevel_warning`).
/// - `format` : Non-null `printf`-style format string.
/// - `args` : A `va_list` containing the arguments for the format string.
void hxloghandler_v(enum hxloglevel_t level_, const char* format_, va_list args_) hxattr_noexcept hxattr_nonnull(2);

/// \cond INTERNAL
// These allow hxasserthandler and hxloghandler to be used by asserts in C++23.
// WARNING: gcc version 13 was reporting the wrong __cplusplus for C++23.
#if HX_CPLUSPLUS >= 202302L
#define hxconstexpr constexpr

hxattr_noexcept constexpr void hxloghandler_(enum hxloglevel_t level_, const char* format_, ...) {
	if !consteval {
		va_list args_;
		va_start(args_, format_);
		hxloghandler_v(level_, format_, args_);
		va_end(args_);
	}
}

#else
// Fall back to not using hxconstexpr below C++23.
#define hxconstexpr
#define hxloghandler_ hxloghandler
#endif // HX_CPLUSPLUS >= 202302L

/// \endcond

#if HX_CPLUSPLUS
} // extern "C"
#endif
