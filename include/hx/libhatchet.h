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
/// Defines logging macros `hxlog`, `hxlog_release`, `hxlog_console`,
/// `hxlog_warning` which vary by `HX_HARDENING_MODE` level (0–3) and defines log
/// verbosity { log, console, warning, assert }.
///
/// Assertion macros `hxassert`, `hxassertmsg`, `hxassert_hard` are provided
/// for debugging, active when `HX_HARDENING_MODE != HX_HARDENING_MODE_NONE`. `hxinit` initializes the
/// platform and `hxshutdown` releases resources when `HX_HARDENING_MODE != HX_HARDENING_MODE_NONE`.

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
#define LIBHATCHET_VER 13400

/// `LIBHATCHET_TAG` - Major, minor and patch version tag name. Odd numbered
/// minor versions are development branches and their tags end in `-dev`.
#define LIBHATCHET_TAG "v1.34.0"

#if !defined HX_HARDENING_MODE
#if defined NDEBUG
#define HX_HARDENING_MODE HX_HARDENING_MODE_NONE
#else
/// `HX_HARDENING_MODE` - Library hardening level. See the README.md for levels.
#define HX_HARDENING_MODE HX_HARDENING_MODE_DEBUG
#endif
#endif

/// `HX_HARDENING_MODE_NONE` - Omits library hardening and disables all asserts.
#define HX_HARDENING_MODE_NONE     0
/// `HX_HARDENING_MODE_STANDARD` - Provides hardening but saves space by omitting verbose output.
#define HX_HARDENING_MODE_STANDARD 1
/// `HX_HARDENING_MODE_VERBOSE` - Provides verbose messages, suitable for internal release.
#define HX_HARDENING_MODE_VERBOSE  2
/// `HX_HARDENING_MODE_DEBUG` - Provides comprehensive asserts and verbose output.
#define HX_HARDENING_MODE_DEBUG    3

#include "hxsettings.h"
#include "hxmemory_manager.h"

#if HX_CPLUSPLUS
extern "C" {
#endif

/// Compile-time assertion for `HX_HARDENING_MODE` [0..3] range.
#if (HX_HARDENING_MODE) < 0 || (HX_HARDENING_MODE) > 3
#error HX_HARDENING_MODE must be [0..3]. See <hx/hxsettings.h>.
#endif

// libhatchet C and C++ API. Above headers are C and C++ too.

/// `hxlog_level_t` - Runtime setting for verbosity of log messages.
/// Independently controls what messages are compiled in. See
/// `g_hxsettings.log_level`.
enum hxlog_level_t {
	/// Written to `hxout`. Structured output. No automatic newline.
	hxlog_level_log,
	/// Written to `hxerr`. Unstructured informative output including error
	/// messages regarding console commands and `hxtest` results. No automatic
	/// newline. No news is good news.
	hxlog_level_console,
	/// Written to `hxerr`. Warnings about serious problems.
	hxlog_level_warning,
	/// Written to `hxerr`. Reason for abnormal termination or test failure.
	hxlog_level_assert
};

/// `hxnull` - The null pointer value for a given pointer type represented by
/// the numeric constant `0`. The C/C++ language standards explicitly define the
/// meaning of `0` in pointer context as a null pointer of the expected type.
/// However they do not define whether `NULL` is `0` or `((void*)0)`. `hxnull`
/// fills that gap by having an unambiguous type. See `hxnullptr`/`hxnullptr_t` if
/// you need a `std::nullptr` replacement.
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

#if (HX_HARDENING_MODE) == HX_HARDENING_MODE_DEBUG // These are debug facilities.

/// `hxlog(...)` - Enters formatted messages in the system log. Does not add a
/// newline. This is only evaluated when `HX_HARDENING_MODE ==
/// HX_HARDENING_MODE_DEBUG`.
/// - `...` Printf-style formatted log message.
#define hxlog(...) hxlog_handler(hxlog_level_log, __VA_ARGS__)

/// `hxassert(bool x)` - Logs an error and terminates execution if `x` is false.
/// This is only evaluated when `HX_HARDENING_MODE == HX_HARDENING_MODE_DEBUG`.
/// Always evaluates to an expression of type `void`. May be used as a compile
/// time assert after C++23.
/// - `x` : The condition to evaluate.
#define hxassert(x_) (void)((bool)(x_)   /* THIS IS USED AS A COMPILE TIME ASSERT: */  \
	|| (hxlog_handler_(hxlog_level_assert, #x_), hxassert_handler(__FILE__, __LINE__)) \
	|| hxbreakpoint())

/// `hxassertmsg(bool x, ...)` - Logs an error and terminates execution if `x`
/// is false. Does not evaluate message args unless condition fails. This is
/// only evaluated when `HX_HARDENING_MODE == HX_HARDENING_MODE_DEBUG`. Always
/// evaluates to an expression of type `void`. e.g., `hxassertmsg(x == 0, "x:
/// %d", x)`. May be used as a compile time assert after C++23.
/// - `x` : The condition to evaluate.
/// - `...` Printf-style formatted log message.
#define hxassertmsg(x_, ...) (void)((bool)(x_)    /* THIS IS USED AS A COMPILE TIME ASSERT: */ \
	|| (hxlog_handler_(hxlog_level_assert, __VA_ARGS__), hxassert_handler(__FILE__, __LINE__)) \
	|| hxbreakpoint())

/// `hxassert_hard(bool x, ...)` - Logs an error and terminates execution if `x`
/// is false. This is only evaluated when `HX_HARDENING_MODE !=
/// HX_HARDENING_MODE_NONE`. Always evaluates to an expression of type `void`.
/// May be used as a compile time assert after C++23.
/// - `x` : The condition to evaluate.
/// - `...` Printf-style formatted log message.
#define hxassert_hard(x_, ...) (void)((bool)(x_) /* THIS IS USED AS A COMPILE TIME ASSERT: */  \
	|| (hxlog_handler_(hxlog_level_assert, __VA_ARGS__), hxassert_handler(__FILE__, __LINE__)) \
	|| hxbreakpoint())

/// `hxassert_always(bool x, ...)` - Logs an error and terminates execution if
/// `x` is false. This is always evaluated regardless of `HX_HARDENING_MODE`.
/// Always evaluates to an expression of type `void`. May be used as a compile
/// time assert after C++23.
/// - `x` : The condition to evaluate.
/// - `...` Printf-style formatted log message.
#define hxassert_always(x_, ...) (void)((bool)(x_) /* THIS IS USED AS A COMPILE TIME ASSERT: */ \
	|| (hxlog_handler_(hxlog_level_assert, __VA_ARGS__), hxassert_handler(__FILE__, __LINE__))  \
	|| hxbreakpoint())

/// Assert handler. Do not call directly, signature changes and then is removed.
/// WARNING: Compile errors from consteval code calling this function are
/// intentional and are how you know a compile time assert has been hit.
bool hxassert_handler(const char* file_, size_t line_) hxattr_noexcept hxattr_nonnull(1) hxattr_cold;

#else // HX_HARDENING_MODE != HX_HARDENING_MODE_DEBUG
#define hxassertmsg(x_, ...) ((void)0)
#define hxassert(x_) ((void)0)
#define hxlog(...) ((void)0)
#define hxassert_always(x_, ...) (void)((bool)(x_) \
	|| (hxassert_handler(), 0)) // THIS IS USED AS A COMPILE TIME ASSERT.

/// WARNING: Compile errors from consteval code calling this function are
/// intentional and are how you know a compile time assert has been hit.
void hxassert_handler(void) hxattr_noexcept hxattr_cold;
#endif // HX_HARDENING_MODE != HX_HARDENING_MODE_DEBUG

#if (HX_HARDENING_MODE) > HX_HARDENING_MODE_STANDARD
/// `hxlog_release(...)` - Enters formatted messages in the system log up to
/// release level 1. No automatic newline. This is only evaluated when
/// `HX_HARDENING_MODE > HX_HARDENING_MODE_STANDARD`.
/// - `...` Printf-style formatted log message.
#define hxlog_release(...) hxlog_handler(hxlog_level_log, __VA_ARGS__)

/// `hxlog_console(...)` - Enters formatted messages in the console system log.
/// This is only evaluated when `HX_HARDENING_MODE > HX_HARDENING_MODE_STANDARD`.
/// - `...` Variadic arguments for the formatted console log message.
#define hxlog_console(...) hxlog_handler(hxlog_level_console, __VA_ARGS__)

/// `hxlog_warning(...)` - Enters formatted warnings in the system log. This is
/// only evaluated when `HX_HARDENING_MODE > HX_HARDENING_MODE_STANDARD`.
/// - `...` Variadic arguments for the formatted warning message.
#define hxlog_warning(...) hxlog_handler(hxlog_level_warning, __VA_ARGS__)

/// `hxwarn_msg(bool x, ...)` - Enters formatted warnings in the system log when
/// `x` is false. This is only evaluated when `HX_HARDENING_MODE >
/// HX_HARDENING_MODE_STANDARD`.
/// - `x` : The condition to evaluate.
/// - `...` Variadic arguments for the formatted warning message.
#define hxwarn_msg(x_, ...) (void)((bool)(x_) \
	|| (hxlog_handler(hxlog_level_warning, __VA_ARGS__), 0))

#else // HX_HARDENING_MODE < HX_HARDENING_MODE_VERBOSE
#define hxlog_release(...) ((void)0)
#define hxlog_console(...) ((void)0)
#define hxlog_warning(...) ((void)0)
#define hxwarn_msg(x_, ...) ((void)0)
#endif

// hxassert_hard has 4 variations. See above. It is only evaluated when
// HX_HARDENING_MODE != HX_HARDENING_MODE_NONE.
#if (HX_HARDENING_MODE) == HX_HARDENING_MODE_VERBOSE
#define hxassert_hard(x_, ...) (void)((bool)(x_)         \
	|| (hxlog_handler_(hxlog_level_assert, __VA_ARGS__), \
		hxassert_handler(), 0)) // THIS IS USED AS A COMPILE TIME ASSERT.
#elif (HX_HARDENING_MODE) == HX_HARDENING_MODE_STANDARD
#define hxassert_hard(x_, ...) (void)((bool)(x_) \
	|| (hxassert_handler(), 0)) // THIS IS USED AS A COMPILE TIME ASSERT.
#elif (HX_HARDENING_MODE) == HX_HARDENING_MODE_NONE
#define hxassert_hard(x_, ...) ((void)0)
#endif

/// `hxinit_internal` - Internal. Use `hxinit` instead. It checks `g_hxinit_ver_`.
void hxinit_internal(int version_) hxattr_cold;

/// `g_hxinit_ver_` - Internal. Set to current library version by `hxinit`. This
/// is renamed in settings.h to contain the library version in order to cause
/// meaningful link errors when linked against stale binaries. The linker symbol
/// should look like `g_hxinit_ver31700_`.
extern int g_hxinit_ver_;

/// `hxshutdown` - Terminates service. Releases all resources acquired by the
/// platform and confirms all memory allocations have been released.
/// `HX_HARDENING_MODE != HX_HARDENING_MODE_NONE`.
void hxshutdown(void) hxattr_cold;

/// `hxlog_handler` - Enters formatted messages in the system log. This is the
/// only access to logging when `HX_HARDENING_MODE < HX_HARDENING_MODE_STANDARD`.
/// - `level` : The log level (e.g., `hxlog_level_log`, `hxlog_level_warning`).
/// - `format` : Non-null `printf`-style format string.
/// - `...` Additional arguments that must satisfy the format string.
void hxlog_handler(enum hxlog_level_t level_, const char* format_, ...) hxattr_noexcept hxattr_printf(2, 3);

/// `hxlog_handler_v` - A `va_list` version of `hxlog_handler`. This is the only
/// access to logging when `HX_HARDENING_MODE < HX_HARDENING_MODE_STANDARD`.
/// - `level` : The log level (e.g., `hxlog_level_log`, `hxlog_level_warning`).
/// - `format` : Non-null `printf`-style format string.
/// - `args` : A `va_list` containing the arguments for the format string.
void hxlog_handler_v(enum hxlog_level_t level_, const char* format_, va_list args_) hxattr_noexcept hxattr_nonnull(2);

/// `hxset_assert_handler` - Installs a custom handler called when an assertion
/// fails. Pass `hxnull` to disable. If it returns true the assert will not exit
/// or trigger a breakpoint.
/// - `handler` : Function pointer of type `void (*)(void)`, or `hxnull`.
void hxset_assert_handler(bool (*handler_)(void)) hxattr_noexcept;

/// \cond INTERNAL
// WARNING: gcc version 13 was reporting the wrong __cplusplus for C++23.
#if HX_CPLUSPLUS >= 202302L
// C++11 and C++23 compatable constexpr use are the only ones used. hxconstexpr
// enables C++23 compatable constexpr usage as it has support for destructors.
#define hxconstexpr constexpr

// This allows hxassert_handler and hxlog_handler to be used by compile time
// asserts in C++23.
hxattr_noexcept constexpr void hxlog_handler_(enum hxlog_level_t level_, const char* format_, ...) {
	if !consteval {
		va_list args_;
		va_start(args_, format_);
		hxlog_handler_v(level_, format_, args_);
		va_end(args_);
	}
}
#else
// Fall back to not using hxconstexpr below C++23.
#define hxconstexpr
#define hxlog_handler_ hxlog_handler
#endif // HX_CPLUSPLUS >= 202302L

/// \endcond

#if HX_CPLUSPLUS
} // extern "C"
#endif
