#pragma once
// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.
// hxconsole inline header and internals. See hxconsole.hpp.

#ifndef LIBHATCHET_VER
#error Internal. Do not include this file directly.
#endif

#ifndef HX_DOXYGEN_PARSER

namespace hxdetail_ {

// Argument parsing. Explicit specializations parse each supported type.
// On overflow next_ is reset to str_ to signal a parse failure.

// These C library wrappers reduce code bloat and enforce additional constraints.
float              hxconsole_strtof_(const char* str_, char** next_);
double             hxconsole_strtod_(const char* str_, char** next_);
long               hxconsole_strtol_(const char* str_, char** next_, long min_, long max_);
long long          hxconsole_strtoll_(const char* str_, char** next_);
unsigned long      hxconsole_strtoul_(const char* str_, char** next_, unsigned long max_);
unsigned long long hxconsole_strtoull_(const char* str_, char** next_);

template<typename arg_t_>
arg_t_ hxconsole_parse_arg_(const char* str_, char** next_) = delete;

// Floating-point types.
template<> inline hxattr_cold float hxconsole_parse_arg_<float>(const char* str_, char** next_) {
	return hxconsole_strtof_(str_, next_);
}
template<> inline hxattr_cold double hxconsole_parse_arg_<double>(const char* str_, char** next_) {
	return hxconsole_strtod_(str_, next_);
}

// char: range CHAR_MIN..CHAR_MAX (platform-defined signedness).
template<> inline hxattr_cold char hxconsole_parse_arg_<char>(const char* str_, char** next_) {
	return static_cast<char>(hxconsole_strtol_(str_, next_, CHAR_MIN, CHAR_MAX));
}

// bool: only 0 or 1 are valid. Anything else is a parse error.
template<> inline hxattr_cold bool hxconsole_parse_arg_<bool>(const char* str_, char** next_) {
	return hxconsole_strtol_(str_, next_, 0l, 1l) != 0l;
}

template<> inline hxattr_cold signed char hxconsole_parse_arg_<signed char>(const char* str_, char** next_) {
	return static_cast<signed char>(hxconsole_strtol_(str_, next_, SCHAR_MIN, SCHAR_MAX));
}
template<> inline hxattr_cold short hxconsole_parse_arg_<short>(const char* str_, char** next_) {
	return static_cast<short>(hxconsole_strtol_(str_, next_, SHRT_MIN, SHRT_MAX));
}
template<> inline hxattr_cold int hxconsole_parse_arg_<int>(const char* str_, char** next_) {
	return static_cast<int>(hxconsole_strtol_(str_, next_, INT_MIN, INT_MAX));
}
template<> inline hxattr_cold long hxconsole_parse_arg_<long>(const char* str_, char** next_) {
	return hxconsole_strtol_(str_, next_, LONG_MIN, LONG_MAX);
}
template<> inline hxattr_cold long long hxconsole_parse_arg_<long long>(const char* str_, char** next_) {
	return hxconsole_strtoll_(str_, next_);
}

template<> inline hxattr_cold unsigned char hxconsole_parse_arg_<unsigned char>(const char* str_, char** next_) {
	return static_cast<unsigned char>(hxconsole_strtoul_(str_, next_, UCHAR_MAX));
}
template<> inline hxattr_cold unsigned short hxconsole_parse_arg_<unsigned short>(const char* str_, char** next_) {
	return static_cast<unsigned short>(hxconsole_strtoul_(str_, next_, USHRT_MAX));
}
template<> inline hxattr_cold unsigned int hxconsole_parse_arg_<unsigned int>(const char* str_, char** next_) {
	return static_cast<unsigned int>(hxconsole_strtoul_(str_, next_, UINT_MAX));
}
template<> inline hxattr_cold unsigned long hxconsole_parse_arg_<unsigned long>(const char* str_, char** next_) {
	return hxconsole_strtoul_(str_, next_, ULONG_MAX);
}
template<> inline hxattr_cold unsigned long long hxconsole_parse_arg_<unsigned long long>(const char* str_, char** next_) {
	return hxconsole_strtoull_(str_, next_);
}

template<> const char* hxconsole_parse_arg_<const char*>(const char* str_, char** next_);

// Argument labels for usage strings.

// GCOVR_EXCL_START
template<typename arg_t_> constexpr const char* hxconsole_arg_label_() = delete;
template<> constexpr const char* hxconsole_arg_label_<float>() { return "f32"; }
template<> constexpr const char* hxconsole_arg_label_<double>() { return "f64"; }
template<> constexpr const char* hxconsole_arg_label_<char>() { return "char"; }
template<> constexpr const char* hxconsole_arg_label_<bool>() { return "bool"; }
template<> constexpr const char* hxconsole_arg_label_<signed char>() { return "i8"; }
template<> constexpr const char* hxconsole_arg_label_<unsigned char>() { return "u8"; }
template<> constexpr const char* hxconsole_arg_label_<short>() { return "i16"; }
template<> constexpr const char* hxconsole_arg_label_<unsigned short>() { return "u16"; }
template<> constexpr const char* hxconsole_arg_label_<int>() { return "i32"; }
template<> constexpr const char* hxconsole_arg_label_<unsigned int>() { return "u32"; }
template<> constexpr const char* hxconsole_arg_label_<long>() { return sizeof(long) == 4u ? "i32" : "i64"; }
template<> constexpr const char* hxconsole_arg_label_<unsigned long>() { return sizeof(long) == 4u ? "u32" : "u64"; }
template<> constexpr const char* hxconsole_arg_label_<long long>() { return "i64"; }
template<> constexpr const char* hxconsole_arg_label_<unsigned long long>() { return "u64"; }
template<> constexpr const char* hxconsole_arg_label_<const char*>() { return "char*"; }
// GCOVR_EXCL_STOP

// C++20 concept for parseable types.

template<typename t_>
concept hxconsole_parseable_ = requires(const char* s_, char** n_) {
	requires hxis_same<decltype(hxconsole_parse_arg_<t_>(s_, n_)), t_>::value;
};

// Checks for printing characters.

inline hxattr_cold bool hxconsole_is_end_of_line_(const char* str_) {
	while(hxisspace(*str_)) { ++str_; }
	return *str_ == '\0' || *str_ == '#'; // Skip comments
}

// The void(void) signature is exempt from function pointer cast warnings.
using hxconsole_fn_t_ = void (*)(void);

// A command's target function or a variable's address.
union hxconsole_target_ {
	hxconsole_fn_t_ fn_;
	volatile void* var_;
};

// Terminal case: all args parsed, check end of line and call.
template<typename fn_t_, typename... parsed_t_>
hxattr_cold bool hxconsole_call_(fn_t_ fn_, const char* pos_, char*, parsed_t_... parsed_) {
	if(hxconsole_is_end_of_line_(pos_)) {
		return fn_(parsed_...);
	}
	return false;
}

// Recursive case: parse one arg, recurse with remaining types.
template<typename first_t_, typename... rest_t_, typename fn_t_, typename... parsed_t_>
hxattr_cold bool hxconsole_call_(fn_t_ fn_, const char* pos_, char* next_, parsed_t_... parsed_) {
	first_t_ val_ = hxconsole_parse_arg_<first_t_>(pos_, &next_);
	// Empty strings are valid string args.
	if constexpr(hxis_same<first_t_, const char*>::value) {
		return hxconsole_call_<rest_t_...>(fn_, next_, next_, parsed_..., val_);
	} else {
		return (pos_ < next_) && hxconsole_call_<rest_t_...>(fn_, next_, next_, parsed_..., val_);
	}
}

// Parses str and calls the target function. Returns false on parse errors.
template<hxconsole_parseable_... args_t_>
hxattr_cold bool hxconsole_execute_(hxconsole_target_ target_, const char* str_) {
	bool (*const fn_)(args_t_...) = reinterpret_cast<bool (*)(args_t_...)>(target_.fn_);
	if constexpr(sizeof...(args_t_) == 0) {
		return hxconsole_is_end_of_line_(str_) && fn_();
	} else {
		char* const next_ = const_cast<char*>(str_);
		return hxconsole_call_<args_t_...>(fn_, str_, next_);
	}
}

// Queries the variable when str is empty and assigns to it otherwise.
template<hxconsole_parseable_ var_t_>
hxattr_cold bool hxconsole_variable_execute_(hxconsole_target_ target_, const char* str_) {
	volatile var_t_* const var_ = static_cast<volatile var_t_*>(target_.var_);
	if(hxconsole_is_end_of_line_(str_)) {
		// 0 parameters is a query.
		hxlog_handler(hxlog_level_console, "%.15g\n", static_cast<double>(*var_));
		return true;
	}
	char* next_ = const_cast<char*>(str_);
	const var_t_ val_ = hxconsole_parse_arg_<var_t_>(str_, &next_);
	if(str_ < next_ && hxconsole_is_end_of_line_(next_)) {
		*var_ = val_;
		return true;
	}
	hxlog_handler(hxlog_level_console, "parse_error: %s\n", str_);
	return false;
}

// A type-erased console command or variable.
class hxconsole_command_ {
public:
	hxconsole_command_(bool (*fn_)(hxconsole_target_, const char*),
			const char* const* labels_, hxconsole_target_ target_)
		: m_execute_(fn_), m_labels_(labels_), m_target_(target_) { }

	// Parses and executes a command line. Returns false on failure.
	bool execute_(const char* str_) const;

	// Prints id (or "usage:") followed by the argument labels.
	void usage_(const char* id_=hxnull) const;

private:
	bool (*m_execute_)(hxconsole_target_ target_, const char* str_);
	const char* const* m_labels_;
	hxconsole_target_ m_target_;
};

// Returns the null terminated usage label table for a signature.
template<hxconsole_parseable_... args_t_>
hxattr_cold const char* const* hxconsole_labels_(void) {
	static constexpr const char* labels_[] = { hxconsole_arg_label_<args_t_>()..., hxnull };
	return labels_;
}

// Single factory function. The compiler deduces args_t_... from the function
// pointer.
template<hxconsole_parseable_... args_t_>
hxattr_cold hxconsole_command_ hxconsole_command_factory_(bool(*fn_)(args_t_...)) {
	const hxconsole_target_ target_ = { .fn_ = reinterpret_cast<hxconsole_fn_t_>(fn_) };
	return hxconsole_command_(&hxconsole_execute_<args_t_...>, hxconsole_labels_<args_t_...>(),
		target_);
}

inline constexpr const char* hxg_console_variable_factory_labels_[] = { "<optional-value>", hxnull };

template<hxconsole_parseable_ var_t_>
hxattr_cold hxconsole_command_ hxconsole_variable_factory_(volatile var_t_* var_) {
	const hxconsole_target_ target_ = { .var_ = var_ };
	return hxconsole_command_(&hxconsole_variable_execute_<var_t_>, hxg_console_variable_factory_labels_,
		target_);
}

// ERROR: Pointers cannot be console variables.
template<typename var_t_>
void hxconsole_variable_factory_(var_t_** var_) = delete;
template<typename var_t_>
void hxconsole_variable_factory_(const var_t_** var_) = delete;

// Hash table infrastructure.

// Wrap the string literal type because it is not used normally.
class hxconsole_hash_table_key_ {
public:
	explicit hxconsole_hash_table_key_(const char* s_) : str_(s_) { }
	const char* str_;
};

// Uses FNV-1a string hashing. Stops at whitespace.
inline hxattr_cold hxhash_t hxkey_hash(hxconsole_hash_table_key_ k_) {
	hxhash_t x_ = static_cast<hxhash_t>(0x811c9dc5);
	while(hxisgraph(*k_.str_)) {
		x_ ^= static_cast<hxhash_t>(*k_.str_++);
		x_ *= static_cast<hxhash_t>(0x01000193);
	}
	return x_;
}

// A version of ::strcmp that stops at the first non-graphical characters.
inline hxattr_cold bool hxkey_equal(hxconsole_hash_table_key_ a_, hxconsole_hash_table_key_ b_) {
	while(hxisgraph(*a_.str_) && *a_.str_ == *b_.str_) { ++a_.str_; ++b_.str_; }
	return !hxisgraph(*a_.str_) && !hxisgraph(*b_.str_);
}

// this is how to write a hash node without including hash table code.
class hxconsole_hash_table_node_ {
public:
	using key_t = hxconsole_hash_table_key_;

	hxattr_cold hxconsole_hash_table_node_(hxconsole_hash_table_key_ key_,
			const hxconsole_command_& command_)
			: m_hash_next_(hxnull), m_key_(key_), m_hash_(hxkey_hash(key_)), m_command_(command_) {
#if (HX_HARDENING_MODE) == HX_HARDENING_MODE_DEBUG
		const char* k_ = key_.str_;
		while(hxisgraph(*k_)) {
			++k_;
		}
		hxassertmsg(*k_ == '\0', "bad_console_symbol \"%s\"", key_.str_);
#endif
	}

	// Boilerplate required by hxhash_table.
	hxconsole_hash_table_node_* hash_next(void) const { return m_hash_next_; }
	void set_hash_next(hxconsole_hash_table_node_* next_) { m_hash_next_ = next_; }

	const hxconsole_hash_table_key_& hash_key(void) const { return m_key_; }
	hxhash_t hash_value(void) const { return m_hash_; }
	static hxhash_t hash_value(hxconsole_hash_table_key_ key_) { return hxkey_hash(key_); }
	const hxconsole_command_* command_(void) const { return &m_command_; }

private:
	hxconsole_hash_table_node_* m_hash_next_;
	hxconsole_hash_table_key_ m_key_;
	hxhash_t m_hash_;
	hxconsole_command_ m_command_;
};

void hxconsole_register_(hxconsole_hash_table_node_* node);

// registers a console command using a global variable without memory allocations.
// There is no reason to deregister or destruct anything.
class hxconsole_constructor_ {
public:
	hxattr_cold hxconsole_constructor_(const hxconsole_command_& command_, const char* id_)
			: m_node_(hxconsole_hash_table_key_(id_), command_) {
		hxconsole_register_(&m_node_);
	}

private:
	hxconsole_hash_table_node_ m_node_;
};

} // hxdetail_
#endif // HX_DOXYGEN_PARSER
