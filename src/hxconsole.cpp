// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

#include "../include/hx/hxconsole.hpp"
#include "../include/hx/hxfile.hpp"
#include "../include/hx/hxhash_table.hpp"
#include "../include/hx/hxsort.hpp"
#include "../include/hx/hxarray.hpp"

#if HX_CPLUSPLUS >= 202002L

// These C library wrappers reduce code bloat and enforce additional
// constraints. The next_ pointer is reset when parse errors or negative numbers
// are encountered.

namespace hxdetail_ {

long hxconsole_strtol_(const char* str_, char** next_) {
	errno = 0;
	const long v_ = ::strtol(str_, next_, 0);
	if(errno == ERANGE) { *next_ = const_cast<char*>(str_); }
	return v_;
}

long long hxconsole_strtoll_(const char* str_, char** next_) {
	errno = 0;
	const long long v_ = ::strtoll(str_, next_, 0);
	if(errno == ERANGE) { *next_ = const_cast<char*>(str_); }
	return v_;
}

unsigned long hxconsole_strtoul_(const char* str_, char** next_) {
	// The standard treats negative numbers as large positive values.
	const char* p_ = str_;
	while(hxisspace(*p_)) { ++p_; }
	if(*p_ == '-') { hxassert(*next_ == const_cast<char*>(str_)); return 0; }

	errno = 0;
	const unsigned long v_ = ::strtoul(str_, next_, 0);
	if(errno == ERANGE) { *next_ = const_cast<char*>(str_); }
	return v_;
}

unsigned long long hxconsole_strtoull_(const char* str_, char** next_) {
	// The standard treats negative numbers as large positive values.
	const char* p_ = str_;
	while(hxisspace(*p_)) { ++p_; }
	if(*p_ == '-') { hxassert(*next_ == const_cast<char*>(str_)); return 0; }

	errno = 0;
	const unsigned long long v_ = ::strtoull(str_, next_, 0);
	if(errno == ERANGE) { *next_ = const_cast<char*>(str_); }
	return v_;
}

} // hxdetail_

// ----------------------------------------------------------------------------
// hxconsole_command_table_
//
// Compares command lines to static strings. Hashing stops at the first
// non-printing character on the command line.

namespace {

class hxconsole_less_ {
public:
	bool operator()(const hxconsole_hash_table_node_* a,
			const hxconsole_hash_table_node_* b) const {
		return hxkey_less(a->hash_key().str_, b->hash_key().str_);
	}
};

class hxconsole_command_table_
	: public hxhash_table<hxconsole_hash_table_node_, 2, false, hxdo_not_delete> {
};

// Wrapped to enforce a construction-order dependency. Modification of the table
// is not thread safe, and it is normally constructed before main.
hxconsole_command_table_& hxconsole_commands_(void) {
	static hxconsole_command_table_ table_;
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
	hxconsole_commands_().release_key(hxconsole_hash_table_key_(id));
}

bool hxconsole_exec_line(const char* command) {
	// Skip leading whitespace.
	const char* pos = command;
	while(hxisspace(*pos)) {
		++pos;
	}

	// Skip comments and blank lines.
	if(hxconsole_is_end_of_line_(pos)) {
		return true;
	}

	const hxconsole_hash_table_node_* node = hxconsole_commands_().find(hxconsole_hash_table_key_(pos));
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

// ----------------------------------------------------------------------------
// Built-in console commands

// Lists variables and commands in order.
bool hxconsole_help(void) {
#if (HX_HARDENING_MODE) > HX_HARDENING_MODE_STANDARD
	hxinit();
	const hxsystem_allocator_scope temporary_stack(hxsystem_allocator_temporary_stack);
	hxarray<const hxconsole_hash_table_node_*> cmds;
	cmds.reserve(hxconsole_commands_().size());
	for(hxconsole_command_table_::const_iterator it = hxconsole_commands_().cbegin();
			it != hxconsole_commands_().cend(); ++it) {
		if(::strncmp(it->hash_key().str_, "hxconsole_test", 13) == 0 ||
				::strncmp(it->hash_key().str_, "s_hxconsole_test", 15) == 0) {
			continue;
		}
		cmds.push_back(&*it);
	}

	hxinsertion_sort<const hxconsole_hash_table_node_**, hxconsole_less_>(cmds.begin(), cmds.end(), hxconsole_less_());

	for(hxarray<const hxconsole_hash_table_node_*>::iterator it = cmds.begin();
			it != cmds.end(); ++it) {
		(*it)->command_()->usage_((*it)->hash_key().str_);
	}
#endif
	return true;
}

#if (HX_HARDENING_MODE) > HX_HARDENING_MODE_STANDARD && !defined __wasm__

static bool hxconsole_peek(uint64_t address_, uint32_t bytes_) {
	hxhex_dump(reinterpret_cast<const void*>(static_cast<uintptr_t>(address_)), bytes_, false);
	return true;
}

// Writes bytes from a hex value in little-endian format (LSB first). The value
// repeats every 8 bytes (64 bits) in memory. The hex input is also 64-bit.
static bool hxconsole_poke(uint64_t address_, uint32_t bytes_, uint64_t hex_) {
	volatile uint8_t* address = reinterpret_cast<volatile uint8_t*>(static_cast<uintptr_t>(address_));
	while(bytes_-- != 0u) {
		*address++ = static_cast<uint8_t>(hex_);
		hex_ = (hex_ >> 8) | (hex_ << 56);
	}
	return true;
}

static bool hxconsole_hex_dump(uint64_t address_, uint32_t bytes_) {
	hxhex_dump(reinterpret_cast<const void*>(static_cast<uintptr_t>(address_)), bytes_, true);
	return true;
}

static bool hxconsole_float_dump(uint64_t address_, uint32_t bytes_) {
	hxfloat_dump(reinterpret_cast<const float*>(static_cast<uintptr_t>(address_)), bytes_);
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
#endif

// Executes commands and settings in a file. Usage: "exec <filename>".
hxconsole_command_named(hxconsole_exec_filename, exec);

#endif // HX_CPLUSPLUS >= 202002L
