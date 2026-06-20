#pragma once
// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

/// \file hxconsole.hpp Implements a simple console for debugging, remote use
/// or for parsing configuration files. Output is directed to the system log
/// with `hxlog_level_console`. Requires C++20 or later.

#include "libhatchet.h"

#if HX_CPLUSPLUS >= 202002L
#if !(HX_USE_MODULE)

#include <limits.h>
#include "hxutility.h"

HX_NS_BEGIN_

// Include internals
#include "detail/hxconsole_detail.hpp"

class hxfile;

/// `hxconsole_deregister` - Explicitly deregisters a console symbol.
/// - `id` : Non-null identifier string for the variable or command being
///   removed.
void hxconsole_deregister(const char* id_) hxattr_nonnull(1);

/// `hxconsole_exec_line` - Evaluates a console command to either call a function
/// or set a variable. e.g., `srand 77` or `a_variable 5`.
/// - `command` : Non-null UTF-8 command string executed by the console.
bool hxconsole_exec_line(const char* command_) hxattr_nonnull(1);

/// `hxconsole_exec_file` - Executes a configuration file that is opened for
/// reading. Ignores blank lines and comments that start with `#`.
/// - `file` : A file containing commands.
bool hxconsole_exec_file(hxfile& file_);

/// `hxconsole_exec_filename` - Opens a configuration file by name and executes it.
/// - `filename` : Non-null UTF-8 path to a file containing commands.
bool hxconsole_exec_filename(const char* filename_) hxattr_nonnull(1);

/// `hxconsole_help` - Logs every console symbol to the console log when
/// `HX_HARDENING_MODE > HX_HARDENING_MODE_STANDARD`. Otherwise returns true
/// without producing output.
bool hxconsole_help(void);

HX_NS_END_
#endif // !HX_USE_MODULE

/// `hxconsole_command` - Registers a function using a global constructor. Use in
/// a global scope. The command uses the same name and arguments as the function.
/// - `x` : Valid C identifier that evaluates to a function pointer.
///   e.g., `hxconsole_command(srand);`
#define hxconsole_command(x_) static HX_NS_PREFIX_ hxdetail_::hxconsole_constructor_ \
	hxg_console_symbol_##x_(HX_NS_PREFIX_ hxdetail_::hxconsole_command_factory_(&(x_)), #x_)

/// `hxconsole_command_named` - Registers a named function using a global
/// constructor. Use in a global scope. The provided name must be a valid C
/// identifier.
/// - `x` : Any expression that evaluates to a function pointer.
/// - `name` : Valid C identifier that identifies the command.
///   e.g., `hxconsole_command_named(srand, seed_rand);`
#define hxconsole_command_named(x_, name_) static HX_NS_PREFIX_ hxdetail_::hxconsole_constructor_ \
	hxg_console_symbol_##name_(HX_NS_PREFIX_ hxdetail_::hxconsole_command_factory_(&(x_)), #name_)

/// `hxconsole_variable` - Registers a variable. Use in a global scope. The
/// command has the same name as the variable.
/// - `x` : Valid C identifier that evaluates to a variable.
///   e.g.,
/// ```cpp
///   static bool is_my_hack_enabled=false;
///   hxconsole_variable(is_my_hack_enabled);
/// ```
#define hxconsole_variable(x_) static HX_NS_PREFIX_ hxdetail_::hxconsole_constructor_ \
	hxg_console_symbol_##x_(HX_NS_PREFIX_ hxdetail_::hxconsole_variable_factory_(&(x_)), #x_)

/// `hxconsole_variable_named` - Registers a named variable. Use in a global
/// scope. The provided name must be a valid C identifier.
/// - `x` : Any expression that evaluates to a variable.
/// - `name` : Valid C identifier that identifies the variable.
///   e.g.,
/// ```cpp
///   static bool is_my_hack_enabled=false;
///   hxconsole_variable_named(is_my_hack_enabled, f_hack); // add "f_hack" to the console.
/// ```
#define hxconsole_variable_named(x_, name_) static HX_NS_PREFIX_ hxdetail_::hxconsole_constructor_ \
	hxg_console_symbol_##name_(HX_NS_PREFIX_ hxdetail_::hxconsole_variable_factory_(&(x_)), #name_)

#endif // HX_CPLUSPLUS >= 202002L
