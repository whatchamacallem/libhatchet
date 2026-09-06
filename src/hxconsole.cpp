// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

#include "../include/hx/hxconsole.hpp"
#include "../include/hx/hxfile.hpp"
#include "../include/hx/hxhash_table.hpp"
#include "../include/hx/hxsort.hpp"
#include "../include/hx/hxvector.hpp"

// The console tests intentionally generate floating point traps.
#if defined HX_USE_FLOATING_POINT_TRAPS
#include <fenv.h>
#endif

HX_NS_BEGIN_
#if HX_USE_CONSOLE
namespace hxdetail_ {

// These C library wrappers reduce code bloat and enforce additional
// constraints. The next pointer is reset when parse errors, out of range
// values or negative numbers are encountered.

hxattr_cold float hxconsole_strtof_(const char* str, char** next) {
#if defined HX_USE_FLOATING_POINT_TRAPS
	fenv_t fenv_;
	::feholdexcept(&fenv_);
#endif
	errno = 0;
	const float v = ::strtof(str, next);
	if(errno == ERANGE) { *next = const_cast<char*>(str); }
#if defined HX_USE_FLOATING_POINT_TRAPS
	::feclearexcept(FE_ALL_EXCEPT);
	::fesetenv(&fenv_);
#endif
	return v;
}

hxattr_cold double hxconsole_strtod_(const char* str, char** next) {
#if defined HX_USE_FLOATING_POINT_TRAPS
	fenv_t fenv_;
	::feholdexcept(&fenv_);
#endif
	errno = 0;
	const double v = ::strtod(str, next);
	if(errno == ERANGE) { *next = const_cast<char*>(str); }
#if defined HX_USE_FLOATING_POINT_TRAPS
	::feclearexcept(FE_ALL_EXCEPT);
	::fesetenv(&fenv_);
#endif
	return v;
}

hxattr_cold long hxconsole_strtol_(const char* str, char** next, long min, long max) {
	errno = 0;
	const long v = ::strtol(str, next, 0);
	if(errno == ERANGE || v < min || v > max) { *next = const_cast<char*>(str); }
	return v;
}

hxattr_cold long long hxconsole_strtoll_(const char* str, char** next) {
	errno = 0;
	const long long v = ::strtoll(str, next, 0);
	if(errno == ERANGE) { *next = const_cast<char*>(str); }
	return v;
}

hxattr_cold unsigned long hxconsole_strtoul_(const char* str, char** next, unsigned long max) {
	// The standard treats negative numbers as large positive values. This doesn't.
	const char* p = str;
	while(hxisspace(*p)) { ++p; }
	if(*p == '-') {
		hxassert(*next == const_cast<char*>(str));
		return 0;
	}

	errno = 0;
	const unsigned long v = ::strtoul(str, next, 0);
	if(errno == ERANGE || v > max) { *next = const_cast<char*>(str); }
	return v;
}

hxattr_cold unsigned long long hxconsole_strtoull_(const char* str, char** next) {
	// The standard treats negative numbers as large positive values. This doesn't.
	const char* p = str;
	while(hxisspace(*p)) { ++p; }
	if(*p == '-') {
		hxassert(*next == const_cast<char*>(str));
		return 0;
	}

	errno = 0;
	const unsigned long long v = ::strtoull(str, next, 0);
	if(errno == ERANGE) { *next = const_cast<char*>(str); }
	return v;
}

// const char* captures remainder of line including comments starting with #'s.
// Leading whitespace is discarded and the string is allowed to be empty.
template<> hxattr_cold const char* hxconsole_parse_arg_<const char*>(const char* str, char** next) {
	while(hxisspace(*str)) { ++str; }
	*next = const_cast<char*>(str) + ::strlen(str);
	return str;
}

hxattr_cold bool hxconsole_command_::execute_(const char* str) const {
	if(m_execute_(m_target_, str)) {
		return true;
	}
	usage_();
	return false;
}

hxattr_cold void hxconsole_command_::usage_(const char* id) const {
	hxlog_handler(hxlog_level_console, "%s", (id != hxnull) ? id : "usage:");
	for(const char* const* label = m_labels_; *label != hxnull; ++label) {
		hxlog_handler(hxlog_level_console, " %s", *label);
	}
	hxlog_handler(hxlog_level_console, "\n");
}

} // hxdetail_

// -- hxconsole_command_table --------------------------------------------------
// Compares command lines to static strings. Hashing stops at the first
// non-printing character on the command line.

namespace {
class hxconsole_less {
public:
	hxattr_cold bool operator()(const hxdetail_::hxconsole_hash_table_node_* a,
			const hxdetail_::hxconsole_hash_table_node_* b) const {
		return hxkey_less(a->hash_key().str_, b->hash_key().str_);
	}
};

class hxconsole_command_table
	: public hxhash_table<hxdetail_::hxconsole_hash_table_node_, hxdo_not_delete, false, 3> {
};

// Local static to enforce construction-order. Destruction is no-op.
hxattr_cold hxconsole_command_table& hxconsole_commands_(void) {
	static hxconsole_command_table table_; // GCOVR_EXCL_LINE
	return table_;
}
} // namespace {

// -- Console API --------------------------------------------------------------

// hxconsole_register_ is internal only.
hxattr_cold void hxdetail_::hxconsole_register_(hxconsole_hash_table_node_* node) {
	hxconsole_command_table& commands = hxconsole_commands_();
	hxassertmsg(node->hash_key().str_, "bad_arg");
	if(commands.replace(node)) {
		hxlog_handler(hxlog_level_warning, "command_reregistered %s\n", node->hash_key().str_);
	}
}

// Nodes are statically allocated. Do not delete.
hxattr_cold void hxconsole_deregister(const char* id) {
	hxconsole_commands_().release_key(hxdetail_::hxconsole_hash_table_key_(id));
}

hxattr_cold bool hxconsole_exec_line(const char* command) {
	// Skip leading whitespace.
	const char* pos = command;
	while(hxisspace(*pos)) {
		++pos;
	}

	// Skip comments and blank lines.
	if(hxdetail_::hxconsole_is_end_of_line_(pos)) {
		return true;
	}

	const hxconsole_command_table::const_iterator node =
		hxconsole_commands_().find(hxdetail_::hxconsole_hash_table_key_(pos));
	if(node == hxconsole_commands_().end()) {
		hxwarn(0, "unknown_command %s", command);
		return false;
	}

	// Skip the command name.
	while(hxisgraph(*pos)) {
		++pos;
	}

#ifdef __cpp_exceptions
	try
#endif
	{
		const bool result = node->command_()->execute_(pos);
		hxwarn(result, "command_failed %s", command);
		return result;
	}
#ifdef __cpp_exceptions
	catch (...) {
		hxwarn(0, "unexpected_exception %s", command);
		return false;
	}
#endif
}

// Lists variables and commands in order.
hxattr_cold bool hxconsole_help(void) {
	hxinit(); // GCOVR_EXCL_LINE
	const hxsystem_allocator_scope temp_mem(hxsystem_allocator_heap);
	const hxconsole_command_table& commands = hxconsole_commands_();
	hxvector<const hxdetail_::hxconsole_hash_table_node_*> cmds;
	cmds.reserve(commands.size());
	for(hxconsole_command_table::const_iterator it = commands.cbegin();
			it != commands.cend(); ++it) {
		if(::strncmp(it->hash_key().str_, "hxconsole_test", 14) == 0 ||
				::strncmp(it->hash_key().str_, "hxs_console_test", 16) == 0) {
			continue;
		}
		cmds.push_back(&*it);
	}
	// Do not remove. This is potentially a warning about dead code.
	const hxsize_t skipped = commands.size() - cmds.size();
	hxwarn(skipped == 0, "help_skipped %d test symbols", static_cast<int>(skipped));

	hxinsertion_sort<const hxdetail_::hxconsole_hash_table_node_**, hxconsole_less>(
		cmds.begin(), cmds.end(), hxconsole_less());

	for(hxvector<const hxdetail_::hxconsole_hash_table_node_*>::iterator it = cmds.begin();
			it != cmds.end(); ++it) {
		(*it)->command_()->usage_((*it)->hash_key().str_);
	}
	return true;
}

#if HX_USE_FILE_IO
hxattr_cold bool hxconsole_exec_file(hxfile& file) {
	// POSIX would require a system call per-character if this didn't read past
	// the end.
	char line_buf[HX_MAX_LINE];
	bool result = true;
	size_t carried = 0u;

	while(result) {
		const size_t want = HX_MAX_LINE - 1u - carried;
		const size_t got = file.read(line_buf + carried, want, want);
		hxassertmsg(got <= want, "read_overrun got %zu want %zu", got, want);

		// The carried bytes are already known not to contain a newline.
		char* line_begin = line_buf;
		char* scan = line_buf + carried;
		char* const end = scan + got;
		while(result) {
			char* const newline = static_cast<char*>(
				::memchr(scan, '\n', static_cast<size_t>(end - scan)));
			if(newline == hxnull) {
				break;
			}
			*newline = '\0';
			result = hxconsole_exec_line(line_begin);
			line_begin = newline + 1;
			scan = newline + 1;
		}

		carried = static_cast<size_t>(end - line_begin);
		if(file.eof() || carried >= (HX_MAX_LINE - 1u)) {
			if(result && carried != 0u) {
				*end = '\0';
				result = hxconsole_exec_line(line_begin);
			}
			if(file.eof()) {
				break;
			}
			carried = 0u;
		}
		else {
			::memmove(line_buf, line_begin, carried);
		}
	}
	return result;
}

hxattr_cold bool hxconsole_exec_filename(const char* filename) {
	hxfile file(hxfile::open_mode_in, "%s", filename);
	hxwarn(file, "cannot open: %s", filename);
	if(file) {
		const bool is_ok = hxconsole_exec_file(file);
		hxwarn(is_ok, "encountering errors: %s", filename);
		return is_ok;
	}
	return false;
}
#endif // HX_USE_FILE_IO

// -- Built-in console commands -----------------------------------------------
// These are not hooked up as console commands automatically for WASM because
// they didn't seem that useful. WASM will require custom plumbing anyway.

// These are considered too dangerous to enable by default. HX_USE_CONSOLE=2
// enables the debug console.
#endif // HX_USE_CONSOLE
#if (HX_USE_CONSOLE) > 1
namespace {

hxattr_cold bool hxconsole_peek(uint64_t address, uint32_t bytes) {
	hxhex_view(reinterpret_cast<const void*>(static_cast<uintptr_t>(address)), bytes, false);
	return true;
}

// Writes bytes from a hex value in little-endian format (LSB first). The value
// repeats every 8 bytes (64 bits) in memory. The hex input is also 64-bit.
hxattr_cold bool hxconsole_poke(uint64_t address, uint32_t bytes, uint64_t hex) {
	volatile uint8_t* addr = reinterpret_cast<volatile uint8_t*>(static_cast<uintptr_t>(address));
	while(bytes-- != 0u) {
		*addr++ = static_cast<uint8_t>(hex);
		hex = (hex >> 8) | (hex << 56);
	}
	return true;
}

hxattr_cold bool hxconsole_hex_view(uint64_t address, uint32_t bytes) {
	hxhex_view(reinterpret_cast<const void*>(static_cast<uintptr_t>(address)), bytes, true);
	return true;
}

hxattr_cold bool hxconsole_float_view(uint64_t address, uint32_t size) {
	hxfloat_view(reinterpret_cast<const float*>(static_cast<uintptr_t>(address)), size);
	return true;
}

// List console commands and argument types.
hxconsole_command_named(hxconsole_help, help);

// Write bytes to console.
hxconsole_command_named(hxconsole_peek, peek);

// Write bytes to memory.
hxconsole_command_named(hxconsole_poke, poke);

// Write bytes to console with pretty formatting.
hxconsole_command_named(hxconsole_hex_view, hexview);

// Write floats to console.
hxconsole_command_named(hxconsole_float_view, floatview);

#if HX_USE_FILE_IO
// Executes commands and settings in a file. Usage: "exec <filename>".
hxconsole_command_named(hxconsole_exec_filename, exec);
#endif // HX_USE_FILE_IO

} // namespace {
#endif // (HX_USE_CONSOLE) > 1
HX_NS_END_
