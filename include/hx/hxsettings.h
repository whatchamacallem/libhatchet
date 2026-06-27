#pragma once
// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

/// \file
/// Compiler detection and target specific C++ feature polyfill. Use `#if
/// HX_...` instead of `#ifdef HX_...` for all `HX_`* macros. `HX_USE_NAMESPACE`
/// can be used to wrap the library in a namespace.

#if !LIBHATCHET_VER
#error #include <hx/libhatchet.h> instead.
#endif

/// \cond HIDDEN
#ifndef __has_include
#define __has_include(x) 0
#endif
/// \endcond

#if defined HX_DOXYGEN_PARSER
/// `HX_CPLUSPLUS` - A version of `__cplusplus` that is defined to `0` when
/// `__cplusplus` is undefined. Allows use in C preprocessor statements without
/// warnings when the compiler is configured to warn about undefined macros.
///
/// -  C++11: 201103L
/// -  C++14: 201402L
/// -  C++17: 201703L
/// -  C++20: 202002L
/// -  C++23: 202302L
#define HX_CPLUSPLUS 202302L // See Doxyfile.
#elif defined __cplusplus
#define HX_CPLUSPLUS __cplusplus
#else
// This is C. Using 0 here avoids warnings.
#define HX_CPLUSPLUS 0
#endif

// -- Target settings for Doxygen ----------------------------------------------
// See the Doxyfile. Run doxygen with no args.
#if defined HX_DOXYGEN_PARSER

/// `HX_USE_THREADS` - `11` indicates C11 threads are in use. `1` is for pthreads
/// and `0` is for no threading.
#define HX_USE_THREADS 11

/// `HX_USE_LIBCXX`: Indicates whether libstdc++/libc++ are present. Set
/// `-DHX_USE_LIBCXX=0` to signal the C++ standard library is not in use. The
/// C++ standard library is not detected automatically because that depends on
/// header include order.
#define HX_USE_LIBCXX 1

/// `hxbreakpoint` - Can be conditionally evaluated with the `&&` and `||`
/// operators. Uses intrinsics when available. (E.g., Clang's.) Raises `SIGTRAP`
/// when `__builtin_debugtrap` is not available.
#define hxbreakpoint() true

/// `hxrestrict` - A pointer attribute indicating that for the lifetime of that
/// pointer, it will be the sole means of accessing the object(s) it points to.
/// Prevents a write iterator from interfering with a read iterator.
#define hxrestrict

/// `hxattr_allocator` - Mark allocator/deallocator pairs for static analysis.
/// See the gcc manual. Must return non-null as well.
#define hxattr_allocator(...)

/// `hxattr_assume` - Tell the optimizer that `condition` is always true.
/// Similar to C++23 `[[assume(condition)]];`.
#define hxattr_assume(...) (void)0

/// `hxattr_cold` - Optimize a function for size.
#define hxattr_cold

/// `hxattr_hot` - Optimize a function more aggressively. Significantly increases
/// code utilization. Adjust implementation according to needs.
#define hxattr_hot

/// `hxattr_nodiscard` - Indicates the caller should not discard the return value.
#define hxattr_nodiscard

/// `hxattr_noexcept` - Use gcc/clang `nothrow` attribute. Unlike `noexcept`
/// this is undefined when violated.
#define hxattr_noexcept

/// `hxattr_nonnull` - Indicates that a function has args that should not be
/// null. Checked by `UBSan`.
#define hxattr_nonnull(...)

/// `hxattr_noinline` - Prevents a function from being inlined.
#define hxattr_noinline

/// `hxattr_noreturn` - Indicates that a function will never return. E.g., by
/// calling `_Exit`.
#define hxattr_noreturn

/// `hxattr_printf` - Indicates to gcc that a function uses `printf`-style
/// formatting so it can type-check the format string.
#define hxattr_printf(...)

/// `hxattr_weak` - Allow calling code to override static library code. Note:
/// MSVC treats all static library symbols as weak.
#define hxattr_weak

/// `hxattr_scanf` - Indicates to gcc that a function uses `scanf`-style
/// formatting so it can type-check the format string.
#define hxattr_scanf(...)

// -- Target settings for MSVC -------------------------------------------------
// MSVC doesn't support C++'s feature test macros very well.
#elif defined _MSC_VER

#if defined __clang__
#error Clang detected masquerading as MSVC - not supported due to intrinsic use.
#endif

#if !defined __cpp_exceptions && !defined _HAS_EXCEPTIONS
#define _HAS_EXCEPTIONS 0
#endif

#if !defined HX_USE_THREADS
#define HX_USE_THREADS 11
#endif

#if !defined HX_USE_LIBCXX
#define HX_USE_LIBCXX 1
#endif

#define hxbreakpoint() (__debugbreak(),true)
#define hxrestrict __restrict
#define hxattr_allocator(...)
#define hxattr_assume(condition_) __assume(condition_)
#define hxattr_cold
#define hxattr_hot
#if HX_CPLUSPLUS
#define hxattr_nodiscard [[nodiscard]]
#else
#define hxattr_nodiscard
#endif
#define hxattr_noexcept
#define hxattr_nonnull(...)
#define hxattr_noinline __declspec(noinline)
#if HX_CPLUSPLUS
#define hxattr_noreturn [[noreturn]]
#else
#define hxattr_noreturn
#endif
#define hxattr_printf(...)
#define hxattr_scanf(...)
// #define hxattr_weak __declspec(selectany) is not used as MSVC treats all
// library objects as weak.
#define hxattr_weak

// -- Target settings for Clang and GCC ----------------------------------------
// Further compilers will require customization.
#else // Assume gcc/clang.

// hxthreads.hpp should work in C++11 with pthread.h. _POSIX_THREADS is the
// correct way to observe the -pthread compiler flag.
#if !defined HX_USE_THREADS
#if __has_include(<threads.h>)
#define HX_USE_THREADS 11
#elif defined _POSIX_THREADS
#define HX_USE_THREADS 1
#else
#define HX_USE_THREADS 0
#endif
#endif

#if !defined HX_USE_LIBCXX
#define HX_USE_LIBCXX 1
#elif !(HX_USE_LIBCXX) && !defined HX_SKIP_LIBCXX_CHECK && __has_include(<new>)
#error Use -nostdinc++ to use the C headers instead of the C++ ones.
#endif

// HX_USE_SIGTRAP is not always defined like the other HX_USE_* macros.
#if !defined HX_USE_SIGTRAP && defined __has_builtin && __has_builtin(__builtin_debugtrap)
#define hxbreakpoint() (__builtin_debugtrap(),true)
#else
#define hxbreakpoint() (raise(SIGTRAP),true)
#endif

#define hxrestrict __restrict

// hxattr_allocator - Collection of attributes for allocators.
#if defined __clang__
#define hxattr_allocator(...) \
	__attribute__((returns_nonnull)) __attribute__((warn_unused_result))
#elif __GNUC__ >= 11
#define hxattr_allocator(...) __attribute__((malloc(__VA_ARGS__))) \
	__attribute__((returns_nonnull)) __attribute__((warn_unused_result))
#else
#define hxattr_allocator(...) __attribute__((malloc)) \
	__attribute__((returns_nonnull)) __attribute__((warn_unused_result))
#endif

#if defined __clang__
#define hxattr_assume(condition_) __builtin_assume(condition_)
#else
// The solution for gcc may have side effects. Use with caution.
// #define hxattr_assume(condition_) (void)((bool)(condition_) || (__builtin_unreachable(),0))
#define hxattr_assume(...) (void)0
#endif

// __attribute__ is used because it works in an extern "C" block.
#define hxattr_cold __attribute__((cold))
#define hxattr_hot __attribute__((hot)) __attribute__((flatten))
#define hxattr_nodiscard __attribute__((warn_unused_result))
#define hxattr_noexcept __attribute__((nothrow))
#define hxattr_nonnull(...)__attribute__((nonnull(__VA_ARGS__)))
#define hxattr_noinline __attribute__((noinline))
#define hxattr_noreturn __attribute__((noreturn))
#define hxattr_printf(pos_, start_) __attribute__((format(printf, pos_, start_)))
#define hxattr_scanf(pos_, start_) __attribute__((format(scanf, pos_, start_)))
#define hxattr_weak __attribute__((weak))
#endif // target specific settings

// -- Target independent -------------------------------------------------------
#if HX_CPLUSPLUS >= 202302L
/// `hxconstexpr` - Enables C++23 compatable constexpr usage for functions as
/// that has support for destructors. Falls back to not using constexpr below
/// C++23.
#define hxconstexpr constexpr
#else
#define hxconstexpr
#endif // HX_CPLUSPLUS < 202302L

#if HX_CPLUSPLUS >= 201703L
/// `hxinline_constexpr` - Enables C++17 compatable "inline constexpr" usage for
/// variables so they can be exported from modules. Falls back to not using
/// inline below C++17.
#define hxinline_constexpr inline constexpr
/// `hxif_constexpr` - `if constexpr` on C++17 and later, plain `if` otherwise.
#define hxif_constexpr if constexpr
#else
#define hxinline_constexpr constexpr
#define hxif_constexpr if
#endif

#if !defined HX_USE_MODULE
/// `HX_USE_MODULE` - Setting `-DHX_USE_MODULE=1` when using modules (e.g.
/// `import hx;`) will allow the macros in `<hx/libhatchet.h>`,
/// `<hx/hxconsole.hpp>`, `<hx/hxprofiler.hpp>` and `<hx/hxtest.hpp>` to be
/// textually included alongside `import hx;`. See `src/hxmodule.cppm`.
#define HX_USE_MODULE 0
#endif

#if !defined HX_USE_CONSOLE
/// `HX_USE_CONSOLE` - Control whether the console is included in the build.
/// C++20 is required to use the console. `0` - Disables the console. `1` -
/// Enables the console. `2` - Enables the debug console. This allows executing
/// files and modifying memory.
#define HX_USE_CONSOLE ((HX_CPLUSPLUS >= 202002L) ? 1 : 0)
#elif (HX_USE_CONSOLE) && HX_CPLUSPLUS && HX_CPLUSPLUS < 202002L
#error The console requires C++20 or later.
#endif
#if (HX_USE_CONSOLE) > 1 && defined __wasm__
// This warning is primarily for security and sanity checking reasons.
#error The debug console is not designed for use with WASM.
#endif

#if !defined HX_USE_LOGGING
/// `HX_USE_LOGGING` - Control whether logging statements are included in the
/// build. Note: `hxlog_handler` is always available and is used by the asserts.
/// `0` - Disables the logging macros. `1` - All logging except `hxlog`. `2` -
/// All logging including `hxlog`. This is the default.
#define HX_USE_LOGGING 2
#endif

#if !defined HX_USE_FILE_IO
/// `HX_USE_FILE_IO` - Select if `hxfile` exists and if it uses C file I/O or
/// POSIX I/O. `stdout` is used for logging even when `hxfile` is disabled. `0`
/// - Disables hxfile. `1` - C file I/O. `2` - POSIX file I/O.
#define HX_USE_FILE_IO 1
#endif

#if !defined HX_USE_PROFILER
/// `HX_USE_PROFILER` - Enable this to use the profiler. - `0` Disables code for
/// capturing profiling data. - `1` Compiles in code. See `hxprofile_scope`.
#define HX_USE_PROFILER 0
#endif

#if !defined HX_CYCLES_PER_SECOND
/// `HX_CYCLES_PER_SECOND` - Configures the CPU cycles per second. This is
/// hardcoded to avoid an OS dependency. WARNING: Processors may change speed.
#define HX_CYCLES_PER_SECOND 3.0e+9
#endif

#if !defined HX_PROFILER_MAX_RECORDS
/// `HX_PROFILER_MAX_RECORDS` - Set to `4096` if not defined. The profiler
/// doesn't reallocate. This is the maximum.
#define HX_PROFILER_MAX_RECORDS 4096
#endif

#if !defined HX_USE_MEMORY_MANAGER
/// `HX_USE_MEMORY_MANAGER` - Used to disable memory management for debugging
/// and for platforms like wasm where extra system allocations are probably
/// cheaper than code size. - `0` : normal target operation - `1` : remove code
/// entirely
#define HX_USE_MEMORY_MANAGER 1
#endif

#if !defined HX_PROVIDE_NEW_DELETE
/// `HX_PROVIDE_NEW_DELETE` - Provide new/delete when the std library is absent
/// unless overridden.
#define HX_PROVIDE_NEW_DELETE !(HX_USE_LIBCXX)
#endif

/// `HX_KIB` - A KiB, 2^10.
#define HX_KIB (1 << 10)

/// `HX_MIB` - A MiB, 2^20.
#define HX_MIB (1 << 20)

/// `HX_GIB` - A GiB, 2^30.
#define HX_GIB (1 << 30)

#if !defined HX_MEMORY_BUDGET_PERMANENT
/// `HX_MEMORY_BUDGET_PERMANENT` - Pool sizes. Defaults to 4 KiB if not defined.
/// Set to 0 to disable.
#define HX_MEMORY_BUDGET_PERMANENT		(4u * HX_KIB)
#endif

#if !defined HX_MEMORY_MAX_STACKS
/// `HX_MEMORY_MAX_STACKS` - The maximum number of temporary stacks that
/// `hxmemory_manager_allocate_stacks` may allocate. Set to 3 for triple
/// buffering if not defined.
#define HX_MEMORY_MAX_STACKS 3u
#endif

#if !defined HX_RADIX_SORT_MIN_SIZE
/// `HX_RADIX_SORT_MIN_SIZE` - Radix sort switches to `hxinsertion_sort` below
/// this size. Set to `32` if not defined.
#define HX_RADIX_SORT_MIN_SIZE 32
#endif

#if !defined HX_MAX_LINE
/// `HX_MAX_LINE` - Set to 2 KiB. Line buffer size for formatted messages printed
/// with this platform. Only allocated on the stack.
#define HX_MAX_LINE (2 * HX_KIB)
#endif

#if !defined HX_USE_GOOGLE_TEST
/// `HX_USE_GOOGLE_TEST` - Switch to using the real Google Test. Defaults to `0`.
#define HX_USE_GOOGLE_TEST 0
#endif

#if (HX_HARDENING_MODE) == HX_HARDENING_MODE_DEBUG && defined __GLIBC__ && !defined __FAST_MATH__
#if !defined HX_USE_FLOATING_POINT_TRAPS
/// `HX_USE_FLOATING_POINT_TRAPS` - Traps `(FE_DIVBYZERO|FE_INVALID|FE_OVERFLOW)` in
/// glibc debug builds. There are a number of relevant compiler flags.
#define HX_USE_FLOATING_POINT_TRAPS 1
#endif
#else
#undef HX_USE_FLOATING_POINT_TRAPS
#define HX_USE_FLOATING_POINT_TRAPS 0
#endif

/// Converts its arg into a string.
#define HX_QUOTE(x_) #x_

/// \cond HIDDEN
// `HX_TEST_ERROR_HANDLING` - Tests that the failure of tests is handled
// correctly. Set to `0` if not defined. Set by `testerrorhandling.sh` and
// `coverage.sh`.
#if !defined HX_TEST_ERROR_HANDLING
#define HX_TEST_ERROR_HANDLING 0
#endif

#if HX_CPLUSPLUS
// HX_USE_* feature test flags must not be empty as that is evaluated as 0.
#define HX_CHECK_USE_(x_) static_assert(HX_QUOTE(x_)[0] != 0, #x_ " must not be empty.");
HX_CHECK_USE_(HX_PROVIDE_NEW_DELETE)
HX_CHECK_USE_(HX_TEST_ERROR_HANDLING)
HX_CHECK_USE_(HX_USE_CONSOLE)
HX_CHECK_USE_(HX_USE_FILE_IO)
HX_CHECK_USE_(HX_USE_FLOATING_POINT_TRAPS)
HX_CHECK_USE_(HX_USE_GOOGLE_TEST)
HX_CHECK_USE_(HX_USE_LIBCXX)
HX_CHECK_USE_(HX_USE_LOGGING)
HX_CHECK_USE_(HX_USE_MEMORY_MANAGER)
HX_CHECK_USE_(HX_USE_MODULE)
HX_CHECK_USE_(HX_USE_PROFILER)
HX_CHECK_USE_(HX_USE_THREADS)
#endif

// HX_APPEND_COUNTER - Used to generate unique identifiers. This is weird
// because the ## operator happens before macro arg evaluation and both happen
// before general macro evaluation.
#define HX_APPEND_COUNTER2_(x_, y_) x_ ## y_
#define HX_APPEND_COUNTER_(x_, y_) HX_APPEND_COUNTER2_(x_, y_)
#define HX_APPEND_COUNTER(x_) HX_APPEND_COUNTER_(x_, __COUNTER__)

// HX_BEGIN_INL_/HX_END_INL_ - These allow excluding .inl files from the module
// export block. See hxmodule.cppm for details.
#if !defined HX_BEGIN_INL_
#define HX_BEGIN_INL_
#define HX_END_INL_
#endif

// HX_USE_NAMESPACE - Wraps the entire library in a namespace when HX_USE_NAMESPACE is
// defined as a valid namespace identifier.
#if HX_CPLUSPLUS && defined HX_USE_NAMESPACE
HX_CHECK_USE_(HX_USE_NAMESPACE)
#define HX_NS_BEGIN_  namespace HX_USE_NAMESPACE {
#define HX_NS_END_    }
#define HX_NS_PREFIX_ HX_USE_NAMESPACE::
#define HX_NS_USE     using namespace HX_USE_NAMESPACE;
#else
#define HX_NS_BEGIN_
#define HX_NS_END_
#define HX_NS_PREFIX_
#define HX_NS_USE
#endif
/// \endcond

#if !(HX_USE_MODULE)
#if HX_CPLUSPLUS
extern "C" {
#endif

/// `hxsettings` - Constructed by first call to `hxinit` which happens when on
/// or before the system memory allocators construct. Not thread safe.
struct hxsettings {
	/// `log_level` - Logging level for the application (e.g., verbosity of
	/// logs).
	uint8_t log_level;

	/// `deallocate_permanent` - Allows deallocation of permanent resources at
	/// system shut down.
	bool deallocate_permanent;

#if (HX_HARDENING_MODE) == HX_HARDENING_MODE_DEBUG
	/// `asserts_to_be_skipped` - Number of asserts to skip, useful for testing
	/// assert behavior.
	int asserts_to_be_skipped;
#endif
};

/// `hxg_settings` - Global class constructed by `hxinit`.
extern struct hxsettings hxg_settings;

/// `hxsettings_construct` - Internal. Used to reset settings at startup.
void hxsettings_construct_(void);

#if HX_CPLUSPLUS
} // extern "C"
#endif
#endif // HX_USE_MODULE
