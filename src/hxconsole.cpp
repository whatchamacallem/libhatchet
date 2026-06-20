// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

#include "../include/hx/hxconsole.hpp"
#include "../include/hx/hxfile.hpp"
#include "../include/hx/hxhash_table.hpp"
#include "../include/hx/hxsort.hpp"
#include "../include/hx/hxarray.hpp"

HX_NS_BEGIN_
#if HX_USE_CONSOLE
namespace hxdetail_ {

// These C library wrappers reduce code bloat and enforce additional
// constraints. The next_ pointer is reset when parse errors or negative numbers
// are encountered.

long hxconsole_strtol_(const char* str, char** next) {
	errno = 0;
	const long v = ::strtol(str, next, 0);
	if(errno == ERANGE) { *next = const_cast<char*>(str); }
	return v;
}

long long hxconsole_strtoll_(const char* str, char** next) {
	errno = 0;
	const long long v = ::strtoll(str, next, 0);
	if(errno == ERANGE) { *next = const_cast<char*>(str); }
	return v;
}

unsigned long hxconsole_strtoul_(const char* str, char** next) {
	// The standard treats negative numbers as large positive values.
	const char* p = str;
	while(hxisspace(*p)) { ++p; }
	if(*p == '-') { hxassert(*next == const_cast<char*>(str)); return 0; }

	errno = 0;
	const unsigned long v = ::strtoul(str, next, 0);
	if(errno == ERANGE) { *next = const_cast<char*>(str); }
	return v;
}

unsigned long long hxconsole_strtoull_(const char* str, char** next) {
	// The standard treats negative numbers as large positive values.
	const char* p = str;
	while(hxisspace(*p)) { ++p; }
	if(*p == '-') { hxassert(*next == const_cast<char*>(str)); return 0; }

	errno = 0;
	const unsigned long long v = ::strtoull(str, next, 0);
	if(errno == ERANGE) { *next = const_cast<char*>(str); }
	return v;
}

} // hxdetail_

// ----------------------------------------------------------------------------
// hxconsole_command_table
//
// Compares command lines to static strings. Hashing stops at the first
// non-printing character on the command line.

namespace {

class hxconsole_less {
public:
	bool operator()(const hxdetail_::hxconsole_hash_table_node_* a,
			const hxdetail_::hxconsole_hash_table_node_* b) const {
		return hxkey_less(a->hash_key().str_, b->hash_key().str_);
	}
};

class hxconsole_command_table
	: public hxhash_table<hxdetail_::hxconsole_hash_table_node_, 2, false, hxdo_not_delete> {
};

// Local static to enforce construction-order.
hxconsole_command_table& hxconsole_commands_(void) {
	static hxconsole_command_table table_;
	return table_;
}

} // namespace {

// ----------------------------------------------------------------------------
// Console API

// hxconsole_register_ is internal only.
void hxdetail_::hxconsole_register_(hxconsole_hash_table_node_* node) {
	hxassertmsg(node->hash_key().str_ && node->command_(), "invalid_parameter");
	hxassertmsg(!hxconsole_commands_().find(node->hash_key()), "command_reregistered %s", node->hash_key().str_);

	hxconsole_commands_().insert(node);
}

// Nodes are statically allocated. Do not delete.
void hxconsole_deregister(const char* id) {
	hxconsole_commands_().release_key(hxdetail_::hxconsole_hash_table_key_(id));
}

bool hxconsole_exec_line(const char* command) {
	// Skip leading whitespace.
	const char* pos = command;
	while(hxisspace(*pos)) {
		++pos;
	}

	// Skip comments and blank lines.
	if(hxdetail_::hxconsole_is_end_of_line_(pos)) {
		return true;
	}

	const hxdetail_::hxconsole_hash_table_node_* node = hxconsole_commands_().find(hxdetail_::hxconsole_hash_table_key_(pos));
	if(node == hxnull) {
		hxwarn_msg(0, "unknown_command %s", command);
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
		hxwarn_msg(result, "command_failed %s", command);
		return result;
	}
#ifdef __cpp_exceptions
	catch (...) {
		hxwarn_msg(0, "unexpected_exception %s", command);
		return false;
	}
#endif
}

// Lists variables and commands in order.
bool hxconsole_help(void) {
	hxinit();
	const hxsystem_allocator_scope temporary_stack(hxsystem_allocator_temporary_stack);
	hxarray<const hxdetail_::hxconsole_hash_table_node_*> cmds;
	cmds.reserve(hxconsole_commands_().size());
	for(hxconsole_command_table::const_iterator it = hxconsole_commands_().cbegin();
			it != hxconsole_commands_().cend(); ++it) {
		if(::strncmp(it->hash_key().str_, "hxconsole_test", 13) == 0 ||
				::strncmp(it->hash_key().str_, "hxs_console_test", 15) == 0) {
			continue;
		}
		cmds.push_back(&*it);
	}

	hxinsertion_sort<const hxdetail_::hxconsole_hash_table_node_**, hxconsole_less>(cmds.begin(), cmds.end(), hxconsole_less());

	for(hxarray<const hxdetail_::hxconsole_hash_table_node_*>::iterator it = cmds.begin();
			it != cmds.end(); ++it) {
		(*it)->command_()->usage_((*it)->hash_key().str_);
	}
	return true;
}

#if HX_USE_FILE_IO
bool hxconsole_exec_file(hxfile& file) {
	char line_buf[HX_MAX_LINE];
	bool result = true;
	while(result && file.getline(line_buf)) {
		result = hxconsole_exec_line(line_buf);
	}
	return result;
}

bool hxconsole_exec_filename(const char* filename) {
	// Please don't assert.
	hxfile file(hxfile::in|hxfile::skip_asserts, "%s", filename);
	hxwarn_msg(file, "cannot open: %s", filename);
	if(file) {
		const bool is_ok = hxconsole_exec_file(file);
		hxwarn_msg(is_ok, "encountering errors: %s", filename);
		return is_ok;
	}
	return false;
}
#endif // HX_USE_FILE_IO

// ----------------------------------------------------------------------------
// Built-in console commands. These are not hooked up as console commands
// automatically for WASM because they didn't seem that useful. WASM will
// require custom plumbing anyway.

#if !defined __wasm__
namespace {

bool hxconsole_peek(uint64_t address, uint32_t bytes) {
	hxhex_dump(reinterpret_cast<const void*>(static_cast<uintptr_t>(address)), bytes, false);
	return true;
}

// Writes bytes from a hex value in little-endian format (LSB first). The value
// repeats every 8 bytes (64 bits) in memory. The hex input is also 64-bit.
bool hxconsole_poke(uint64_t address, uint32_t bytes, uint64_t hex) {
	volatile uint8_t* addr = reinterpret_cast<volatile uint8_t*>(static_cast<uintptr_t>(address));
	while(bytes-- != 0u) {
		*addr++ = static_cast<uint8_t>(hex);
		hex = (hex >> 8) | (hex << 56);
	}
	return true;
}

bool hxconsole_hex_dump(uint64_t address, uint32_t bytes) {
	hxhex_dump(reinterpret_cast<const void*>(static_cast<uintptr_t>(address)), bytes, true);
	return true;
}

bool hxconsole_float_dump(uint64_t address, uint32_t bytes) {
	hxfloat_dump(reinterpret_cast<const float*>(static_cast<uintptr_t>(address)), bytes);
	return true;
}

// List console commands and argument types.
hxconsole_command_named(hxconsole_help, help);

// Write bytes to console.
hxconsole_command_named(hxconsole_peek, peek);

// Write bytes to memory.
hxconsole_command_named(hxconsole_poke, poke);

// Write bytes to console with pretty formatting.
hxconsole_command_named(hxconsole_hex_dump, hexdump);

// Write floats to console.
hxconsole_command_named(hxconsole_float_dump, floatdump);

#if HX_USE_FILE_IO
// Executes commands and settings in a file. Usage: "exec <filename>".
hxconsole_command_named(hxconsole_exec_filename, exec);
#endif // HX_USE_FILE_IO

} // namespace {
#endif // !defined __wasm__
#endif // HX_USE_CONSOLE
HX_NS_END_
