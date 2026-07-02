#pragma once
// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

/// \file
/// Implements a simple console for debugging, remote use or for parsing
/// configuration files. Output is directed to the system log with
/// `hxlog_level_console`. Requires C++20 or later. Set `HX_USE_CONSOLE=2` to
/// enable the debug console. See `hxconsole.cpp` for details. This is optimized
/// for generated code size and not speed.
///
/// Here is how to write lambdas that cease to exist entirely when the console is
/// disabled:
///
/// `hxconsole_command_named(*+[](void) -> bool { return true; }, command_name);`

#include "libhatchet.h"

#if HX_USE_CONSOLE
#if !(HX_USE_MACROS_WITH_MODULE)

#include <limits.h>
#include "hxutility.h"

HX_NS_BEGIN_

// Include internals
#include "detail/hxconsole_detail.hpp"

/// `hxconsole_deregister` - Explicitly deregisters a console symbol.
/// - `id` : Non-null identifier string for the variable or command being
///   removed.
void hxconsole_deregister(const char* id_) hxattr_nonnull(1);

/// `hxconsole_exec_line` - Evaluates a console command to either call a
/// function or set a variable. E.g., `srand 77` or `a_variable 5`.
/// - `command` : Non-null UTF-8 command string executed by the console.
bool hxconsole_exec_line(const char* command_) hxattr_nonnull(1);

/// `hxconsole_help` - Logs every console symbol to the console log.
bool hxconsole_help(void);

#if HX_USE_FILE_IO
class hxfile;
/// `hxconsole_exec_file` - Executes a configuration file that is opened for
/// reading. Ignores blank lines and comments that start with `#`. Disables
/// asserts so that EOF is allowed. WARNING: `file` must not have asserts
/// enabled as this call reads past the end.
/// - `file` : A file containing commands.
bool hxconsole_exec_file(hxfile& file_);

/// `hxconsole_exec_filename` - Opens a configuration file by name and executes
/// it.
/// - `filename` : Non-null UTF-8 path to a file containing commands.
bool hxconsole_exec_filename(const char* filename_) hxattr_nonnull(1);
#endif // HX_USE_FILE_IO

HX_NS_END_
#endif // !HX_USE_MACROS_WITH_MODULE

/// `hxconsole_command` - Registers a function using a global constructor. Use
/// in a global scope. The command uses the same name and arguments as the
/// function. E.g., `hxconsole_command(srand);`
/// - `x` : Valid C identifier that evaluates to a function pointer.
#define hxconsole_command(x_) static HX_NS_PREFIX_ hxdetail_::hxconsole_constructor_ \
	hxg_console_symbol_##x_(HX_NS_PREFIX_ hxdetail_::hxconsole_command_factory_(&(x_)), #x_)

/// `hxconsole_command_named` - Registers a named function using a global
/// constructor. Use in a global scope. The provided name must be a valid C
/// identifier. E.g., `hxconsole_command_named(srand, seed_rand);`
/// - `x` : Any expression that evaluates to a function pointer.
/// - `name` : Valid C identifier that identifies the command.
#define hxconsole_command_named(x_, name_) static HX_NS_PREFIX_ hxdetail_::hxconsole_constructor_ \
	hxg_console_symbol_##name_(HX_NS_PREFIX_ hxdetail_::hxconsole_command_factory_(&(x_)), #name_)

/// `hxconsole_variable` - Registers a variable. Use in a global scope. The
/// command has the same name as the variable. E.g.,
/// ```cpp
///   static bool is_my_hack_enabled=false;
///   hxconsole_variable(is_my_hack_enabled);
/// ```
/// - `x` : Valid C identifier that evaluates to a variable.
#define hxconsole_variable(x_) static HX_NS_PREFIX_ hxdetail_::hxconsole_constructor_ \
	hxg_console_symbol_##x_(HX_NS_PREFIX_ hxdetail_::hxconsole_variable_factory_(&(x_)), #x_)

/// `hxconsole_variable_named` - Registers a named variable. Use in a global
/// scope. The provided name must be a valid C identifier. E.g.,
/// ```cpp
///   static bool is_my_hack_enabled=false;
///   hxconsole_variable_named(is_my_hack_enabled, f_hack); // add "f_hack" to the console.
/// ```
/// - `x` : Any expression that evaluates to a variable.
/// - `name` : Valid C identifier that identifies the variable.
#define hxconsole_variable_named(x_, name_) static HX_NS_PREFIX_ hxdetail_::hxconsole_constructor_ \
	hxg_console_symbol_##name_(HX_NS_PREFIX_ hxdetail_::hxconsole_variable_factory_(&(x_)), #name_)

#else // !HX_USE_CONSOLE

// Avoid complaints about stray ; at global scope.
#define hxconsole_command(x_) static_assert(true, "")
#define hxconsole_command_named(x_, name_) static_assert(true, "")
#define hxconsole_variable(x_) static_assert(true, "")
#define hxconsole_variable_named(x_, name_) static_assert(true, "")

#endif // !HX_USE_CONSOLE
