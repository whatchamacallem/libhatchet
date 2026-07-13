#pragma once
// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

/// \file
/// Provides core macros, memory management and feature detection. Requires C99
/// for C. C++11 is minimum for C++. Utilizes language features up to C++20.
/// Inclusion on the compiler search path is not required. However, the headers
/// are intended to be included as follows: `#include <hx/libhatchet.h>`
///
/// Defines logging macros `hxlog`, `hxlog_release`, `hxlog_console`,
/// `hxlog_warning` which vary by `HX_USE_LOGGING` and defines log verbosity
/// { log, console, warning, assert }.
///
/// Assertion macros `hxassert`, `hxassertmsg` are active only when
/// `HX_HARDENING_MODE == HX_HARDENING_MODE_DEBUG`. `hxassert_hard` is active
/// when `HX_HARDENING_MODE != HX_HARDENING_MODE_NONE`.

// Use minimal C style headers. The std:: namespace may not exist. "You can't
// get there from here."
#include <errno.h>
#include <signal.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifndef __wasm__
#if __has_include(<x86intrin.h>)
#include <x86intrin.h>
#elif __has_include(<intrin.h>)
#include <intrin.h>
#endif
#endif

#if defined __STDC_VERSION__ && __STDC_VERSION__ < 202311l
#include <stdbool.h>
#endif

/// `int LIBHATCHET_VER` - One digit major, and two digit minor and patch
/// versions.
#define LIBHATCHET_VER 14900

/// `LIBHATCHET_TAG` - Major, minor and patch version tag name.
#define LIBHATCHET_TAG "v1.49.0"

#if !defined HX_HARDENING_MODE
/// `HX_HARDENING_MODE` - Library hardening level. See the README.md for levels.
#define HX_HARDENING_MODE HX_HARDENING_MODE_DEBUG
#endif

/// `HX_HARDENING_MODE_NONE` - Omits library hardening and disables all asserts.
#define HX_HARDENING_MODE_NONE 0
/// `HX_HARDENING_MODE_STANDARD` - Provides hardening but saves space by
/// omitting verbose output.
#define HX_HARDENING_MODE_STANDARD 1
/// `HX_HARDENING_MODE_VERBOSE` - Provides verbose messages, suitable for
/// internal release.
#define HX_HARDENING_MODE_VERBOSE 2
/// `HX_HARDENING_MODE_DEBUG` - Provides comprehensive asserts and verbose
/// output.
#define HX_HARDENING_MODE_DEBUG 3

/// Compile-time assertion for `HX_HARDENING_MODE` [0..3] range.
#if (HX_HARDENING_MODE) < 0 || (HX_HARDENING_MODE) > 3
#error HX_HARDENING_MODE must be [0..3]. See <hx/hxsettings.h>.
#endif

#include "hxsettings.h"
#if !(HX_USE_MACROS_WITH_MODULE)
#include "hxmemory_manager.h"
#endif

/// `hxinit` - Initializes the platform if needed. Does a quick version check to
/// determine if the platform is already correctly initialized first. See
/// `hxg_init_ver_`. `LIBHATCHET_VER` is checked against the value
/// hxinit_internal was compiled with. WARNING: Call in `main()`. Not thread
/// safe.
#define hxinit() (void)(hxg_init_ver_ == LIBHATCHET_VER || (hxinit_internal(LIBHATCHET_VER), 0))

#if (HX_HARDENING_MODE) == HX_HARDENING_MODE_DEBUG // These are debug facilities.

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

#else // HX_HARDENING_MODE != HX_HARDENING_MODE_DEBUG
#define hxassertmsg(x_, ...) ((void)0)
#define hxassert(x_) ((void)0)
#define hxassert_always(x_, ...) (void)((bool)(x_) \
	|| (hxassert_handler(), 0)) // THIS IS USED AS A COMPILE TIME ASSERT.

#endif // HX_HARDENING_MODE != HX_HARDENING_MODE_DEBUG

#if (HX_USE_LOGGING) > 1
/// `hxlog(...)` - Enters formatted messages in the system log. Does not add a
/// newline. Only evaluated if `HX_USE_LOGGING` is `2` or above.
/// - `...` Printf-style formatted log message.
#define hxlog(...) hxlog_handler(hxlog_level_log, __VA_ARGS__)
#else
#define hxlog(...) ((void)0)
#endif

#if HX_USE_LOGGING
/// `hxlog_release(...)` - Enters formatted messages in the system log up to
/// release level 1.
/// - `...` Printf-style formatted log message.
#define hxlog_release(...) hxlog_handler(hxlog_level_log, __VA_ARGS__)

/// `hxlog_console(...)` - Enters formatted messages in the console system log.
/// - `...` Variadic arguments for the formatted console log message.
#define hxlog_console(...) hxlog_handler(hxlog_level_console, __VA_ARGS__)

/// `hxlog_warning(...)` - Enters formatted warnings in the system log.
/// - `...` Variadic arguments for the formatted warning message.
#define hxlog_warning(...) hxlog_handler(hxlog_level_warning, __VA_ARGS__)

/// `hxwarn_msg(bool x, ...)` - Enters formatted warnings in the system log when
/// `x` is false.
/// - `x` : The condition to evaluate.
/// - `...` Variadic arguments for the formatted warning message.
#define hxwarn_msg(x_, ...) (void)((bool)(x_) \
	|| (hxlog_handler(hxlog_level_warning, __VA_ARGS__), 0))

#else // !HX_USE_LOGGING
#define hxlog_release(...) ((void)0)
#define hxlog_console(...) ((void)0)
#define hxlog_warning(...) ((void)0)
#define hxwarn_msg(x_, ...) ((void)0)
#endif // !HX_USE_LOGGING

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

#if !(HX_USE_MACROS_WITH_MODULE)
#if HX_CPLUSPLUS
extern "C" {
#endif

/// `hxlog_level_t` - Runtime setting for verbosity of log messages.
/// Independently controls what messages are compiled in. See
/// `hxg_settings.log_level`.
enum hxlog_level_t {
	/// `hxlog_level_log` - Written to `hxout`. Structured output. No automatic
	/// newline.
	hxlog_level_log,
	/// `hxlog_level_console` - Written to `hxerr`. Unstructured informative
	/// output including error messages regarding console commands and `hxtest`
	/// results. No automatic newline. No news is good news.
	hxlog_level_console,
	/// `hxlog_level_warning` - Written to `hxerr`. Warnings about serious
	/// problems.
	hxlog_level_warning,
	/// `hxlog_level_assert` - Written to `hxerr`. Reason for abnormal
	/// termination or test failure.
	hxlog_level_assert
};

/// `hxsize_t` - A signed size type, same as `ssize_t` or `ptrdiff_t`. Use on a
/// 32-bit system with more than 2 GiB RAM is undefined.
typedef ptrdiff_t hxsize_t;

/// `hxhash_t` - Unsigned 32-bit hash value. Expect collisions.
typedef uint32_t hxhash_t;

/// `hxhandle_t` - An opaque 64-bit handle.
typedef uint64_t hxhandle_t;

/// `hxinit_internal` - Internal. Use `hxinit` instead. It checks `hxg_init_ver_`.
void hxinit_internal(int version_) hxattr_cold;

/// \cond HIDDEN
// `hxg_init_ver_` - Internal. Set to the current library version by `hxinit`.
// It is zero when the platform has not been initialized.
extern int hxg_init_ver_;
/// \endcond

/// `hxshutdown` - Terminates service. Releases all resources acquired by the
/// platform and confirms all memory allocations have been released.
void hxshutdown(void) hxattr_cold;

#if (HX_HARDENING_MODE) == HX_HARDENING_MODE_DEBUG
/// `hxassert_handler` - Assert handler. Do not call directly, signature changes
/// and then is removed.
/// WARNING: Compile errors from consteval code calling this function are
/// intentional and are how you know a compile time assert has been hit.
bool hxassert_handler(const char* file_, size_t line_) hxattr_noexcept hxattr_nonnull(1) hxattr_cold;
#else // HX_HARDENING_MODE != HX_HARDENING_MODE_DEBUG
// Errors from consteval code calling this function are how you know a compile
// time assert has been hit. hxattr_cold tells the compiler this call is
// unlikely.
// WARNING: THIS IS USED AS A COMPILE TIME ASSERT.
void hxassert_handler(void) hxattr_noexcept hxattr_cold;
#endif

/// `hxset_assert_handler` - Installs a custom handler called when an assertion
/// fails. Pass `hxnull` to disable. If it returns true the assert will not exit
/// or trigger a breakpoint.
/// - `handler` : Function pointer of type `bool (*)(void)`, or `hxnull`.
void hxset_assert_handler(bool (*handler_)(void)) hxattr_noexcept;

/// `hxlog_handler` - Enters formatted messages in the system log.
/// - `level` : The log level (e.g., `hxlog_level_log`, `hxlog_level_warning`).
/// - `format` : Non-null `printf`-style format string.
/// - `...` Additional arguments that must satisfy the format string.
void hxlog_handler(enum hxlog_level_t level_, const char* format_, ...) hxattr_noexcept hxattr_printf(2, 3);

/// `hxlog_handler_v` - A `va_list` version of `hxlog_handler`.
/// - `level` : The log level (e.g., `hxlog_level_log`, `hxlog_level_warning`).
/// - `format` : Non-null `printf`-style format string.
/// - `args` : A `va_list` containing the arguments for the format string.
void hxlog_handler_v(enum hxlog_level_t level_, const char* format_, va_list args_) hxattr_noexcept hxattr_nonnull(2);

/// `hxexit(int status)` - Flushes `hxout` and `hxerr` then calls `_Exit`.
/// - `status` : The exit status passed to `_Exit`.
hxattr_noreturn void hxexit(int status_) hxattr_noexcept hxattr_cold;

#if HX_CPLUSPLUS
} // extern "C"
#endif
#endif // !HX_USE_MACROS_WITH_MODULE

/// `hxnull` - The null pointer value for a given pointer type represented by
/// the numeric constant `0`. The C/C++ language standards explicitly define the
/// meaning of `0` in pointer context as a null pointer of the expected type.
/// However they do not define whether `NULL` is `0` or `((void*)0)`. `hxnull`
/// fills that gap by having an unambiguous type. See `hxnullptr`/`hxnullptr_t`
/// if you need a `std::nullptr_t` replacement.
#define hxnull 0

#if HX_CPLUSPLUS
#if !(HX_USE_MACROS_WITH_MODULE)
/// `hxhash_bits` - Number of bits in `hxhash_t`.
hxinline_constexpr hxhash_t hxhash_bits = 32u;

/// `hxnull_handle` - A handle that will never refer to a valid object.
hxinline_constexpr hxhandle_t hxnull_handle = 0u;

/// `hxsizeof` - Returns the size of a type or expression as `hxsize_t`.
template<typename T_> constexpr hxsize_t hxsizeof(void) { return static_cast<hxsize_t>(sizeof(T_)); }
template<typename T_> constexpr hxsize_t hxsizeof(T_&) { return static_cast<hxsize_t>(sizeof(T_)); }
#endif // !HX_USE_MACROS_WITH_MODULE
#else
#define hxhash_bits 32u
#define hxnull_handle 0u
#define hxsizeof(x) (hxsize_t)sizeof(x)
#endif

/// \cond HIDDEN
// WARNING: gcc version 13 was reporting the wrong __cplusplus for C++23.
#if HX_CPLUSPLUS >= 202302L
#if !(HX_USE_MACROS_WITH_MODULE)
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
#endif // !HX_USE_MACROS_WITH_MODULE
#else // HX_CPLUSPLUS < 202302L
// Disable the compile time version of hxlog_handler_.
#define hxlog_handler_ hxlog_handler
#endif // HX_CPLUSPLUS < 202302L
/// \endcond
