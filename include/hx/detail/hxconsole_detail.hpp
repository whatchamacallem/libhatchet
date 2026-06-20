#pragma once
// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.
// hxconsole inline header and internals. See hxconsole.hpp.

#ifndef LIBHATCHET_VER
#error Internal. Do not include this file directly.
#endif

namespace hxdetail_ {

// ----------------------------------------------------------------------------
// Argument parsing. Explicit specializations parse each supported type.
// On overflow next_ is reset to str_ to signal a parse failure.

// These C library wrappers reduce code bloat and enforce additional constraints.
long               hxconsole_strtol_(const char* str_, char** next_);
long long          hxconsole_strtoll_(const char* str_, char** next_);
unsigned long      hxconsole_strtoul_(const char* str_, char** next_);
unsigned long long hxconsole_strtoull_(const char* str_, char** next_);

template<typename arg_t_>
arg_t_ hxconsole_parse_arg_(const char* str_, char** next_) = delete;

// Floating-point types.
template<> inline float hxconsole_parse_arg_<float>(const char* str_, char** next_) {
	return ::strtof(str_, next_);
}
template<> inline double hxconsole_parse_arg_<double>(const char* str_, char** next_) {
	return ::strtod(str_, next_);
}

// char: range CHAR_MIN..CHAR_MAX (platform-defined signedness).
template<> inline char hxconsole_parse_arg_<char>(const char* str_, char** next_) {
	const long v_ = hxconsole_strtol_(str_, next_);
	if(v_ < CHAR_MIN || v_ > CHAR_MAX) { *next_ = const_cast<char*>(str_); }
	return static_cast<char>(v_);
}

// bool: only 0 or 1 are valid. Anything else is a parse error.
template<> inline bool hxconsole_parse_arg_<bool>(const char* str_, char** next_) {
	const long v_ = hxconsole_strtol_(str_, next_);
	if(v_ != 0 && v_ != 1) { *next_ = const_cast<char*>(str_); }
	return v_ != 0;
}

// Signed integers: parse as long, then range-check into target width.
template<> inline int8_t hxconsole_parse_arg_<int8_t>(const char* str_, char** next_) {
	const long v_ = hxconsole_strtol_(str_, next_);
	if(v_ < SCHAR_MIN || v_ > SCHAR_MAX) { *next_ = const_cast<char*>(str_); }
	return static_cast<int8_t>(v_);
}
template<> inline int16_t hxconsole_parse_arg_<int16_t>(const char* str_, char** next_) {
	const long v_ = hxconsole_strtol_(str_, next_);
	if(v_ < SHRT_MIN || v_ > SHRT_MAX) { *next_ = const_cast<char*>(str_); }
	return static_cast<int16_t>(v_);
}
template<> inline int32_t hxconsole_parse_arg_<int32_t>(const char* str_, char** next_) {
	const long v_ = hxconsole_strtol_(str_, next_);
	if(v_ < static_cast<long>(INT32_MIN) || v_ > static_cast<long>(INT32_MAX)) { *next_ = const_cast<char*>(str_); }
	return static_cast<int32_t>(v_);
}
template<> inline int64_t hxconsole_parse_arg_<int64_t>(const char* str_, char** next_) {
	const long long v_ = hxconsole_strtoll_(str_, next_);
	return static_cast<int64_t>(v_);
}

// Unsigned integers: parse as unsigned long, then range-check. Negative inputs are rejected.
template<> inline uint8_t hxconsole_parse_arg_<uint8_t>(const char* str_, char** next_) {
	const unsigned long v_ = hxconsole_strtoul_(str_, next_);
	if(v_ > UCHAR_MAX) { *next_ = const_cast<char*>(str_); }
	return static_cast<uint8_t>(v_);
}
template<> inline uint16_t hxconsole_parse_arg_<uint16_t>(const char* str_, char** next_) {
	const unsigned long v_ = hxconsole_strtoul_(str_, next_);
	if(v_ > USHRT_MAX) { *next_ = const_cast<char*>(str_); }
	return static_cast<uint16_t>(v_);
}
template<> inline uint32_t hxconsole_parse_arg_<uint32_t>(const char* str_, char** next_) {
	const unsigned long v_ = hxconsole_strtoul_(str_, next_);
	if(v_ > static_cast<unsigned long>(UINT32_MAX)) { *next_ = const_cast<char*>(str_); }
	return static_cast<uint32_t>(v_);
}
template<> inline uint64_t hxconsole_parse_arg_<uint64_t>(const char* str_, char** next_) {
	const unsigned long long v_ = hxconsole_strtoull_(str_, next_);
	return static_cast<uint64_t>(v_);
}

// const char* captures remainder of line including comments starting with #'s.
// Leading whitespace is discarded and string may be empty.
template<> inline const char* hxconsole_parse_arg_<const char*>(const char* str_, char** next_) {
	while(hxisspace(*str_)) { ++str_; }
	const char* result_ = str_;
	while(*str_ != '\0') { ++str_; }
	*next_ = const_cast<char*>(str_);
	return result_;
}

// ----------------------------------------------------------------------------
// Argument labels for usage strings.

template<typename arg_t_> const char* hxconsole_arg_label_() = delete;
template<> inline const char* hxconsole_arg_label_<float>() { return "f32"; }
template<> inline const char* hxconsole_arg_label_<double>() { return "f64"; }
template<> inline const char* hxconsole_arg_label_<char>() { return "char"; }
template<> inline const char* hxconsole_arg_label_<bool>() { return "bool"; }
template<> inline const char* hxconsole_arg_label_<int8_t>() { return "i8"; }
template<> inline const char* hxconsole_arg_label_<uint8_t>() { return "u8"; }
template<> inline const char* hxconsole_arg_label_<int16_t>() { return "i16"; }
template<> inline const char* hxconsole_arg_label_<uint16_t>() { return "u16"; }
template<> inline const char* hxconsole_arg_label_<int32_t>() { return "i32"; }
template<> inline const char* hxconsole_arg_label_<uint32_t>() { return "u32"; }
template<> inline const char* hxconsole_arg_label_<int64_t>() { return "i64"; }
template<> inline const char* hxconsole_arg_label_<uint64_t>() { return "u64"; }
template<> inline const char* hxconsole_arg_label_<const char*>() { return "char*"; }

// ----------------------------------------------------------------------------
// C++20 concept for parseable types.

template<typename t_>
concept hxconsole_parseable_ = requires(const char* s_, char** n_) {
	requires hxis_same<decltype(hxconsole_parse_arg_<t_>(s_, n_)), t_>::value;
};

// ----------------------------------------------------------------------------
// Checks for printing characters.

inline bool hxconsole_is_end_of_line_(const char* str_) {
	while(hxisspace(*str_)) { ++str_; }
	return *str_ == '\0' || *str_ == '#'; // Skip comments
}

// ----------------------------------------------------------------------------
// hxconsole_command_ base class.

class hxconsole_command_ {
public:
	virtual bool execute_(const char* str_) = 0; // Return false for parse errors.
	virtual void usage_(const char* id_=hxnull) = 0; // Expects command name.
};

// ----------------------------------------------------------------------------
// Single variadic command template. Replaces hxconsole_command0_ through
// hxconsole_command4_.

template<hxconsole_parseable_... args_t_>
class hxconsole_command_impl_ : public hxconsole_command_ {
public:
	hxconsole_command_impl_(bool(*fn_)(args_t_...)) : m_fn_(fn_) { }

	bool execute_(const char* str_) override {
		if constexpr (sizeof...(args_t_) == 0) {
			if(hxconsole_is_end_of_line_(str_)) {
				return m_fn_();
			}
			usage_();
			return false;
		} else {
			char* next_ = const_cast<char*>(str_);
			const bool ok_ = call_<args_t_...>(m_fn_, str_, next_);
			if(!ok_) { usage_(); }
			return ok_;
		}
	}

	void usage_(const char* id_=hxnull) override {
		hxlog_handler(hxlog_level_console, "%s", (id_ != hxnull) ? id_ : "usage:");
		if constexpr (sizeof...(args_t_) == 0) {
			hxlog_handler(hxlog_level_console, "\n");
		} else {
			(hxlog_handler(hxlog_level_console, " %s", hxconsole_arg_label_<args_t_>()), ...);
			hxlog_handler(hxlog_level_console, "\n");
		}
	}

private:
	// Terminal: all args parsed, check end of line and call.
	template<typename fn_t_, typename... parsed_t_>
	static bool call_(fn_t_ fn_, const char* pos_, char*, parsed_t_... parsed_) {
		if(hxconsole_is_end_of_line_(pos_)) {
			return fn_(parsed_...);
		}
		return false;
	}

	// Recursive: parse one arg, recurse with remaining types.
	template<typename first_t_, typename... rest_t_, typename fn_t_, typename... parsed_t_>
	static bool call_(fn_t_ fn_, const char* pos_, char* next_, parsed_t_... parsed_) {
		first_t_ val_ = hxconsole_parse_arg_<first_t_>(pos_, &next_);
		if(pos_ < next_) {
			return call_<rest_t_...>(fn_, next_, next_, parsed_..., val_);
		}
		return false;
	}

	bool(*m_fn_)(args_t_...);
};

// ----------------------------------------------------------------------------
// Variable template.

template<typename var_t_>
class hxconsole_variable_ : public hxconsole_command_ {
public:
	hxconsole_variable_(volatile var_t_* var_) : m_var_(var_) { }

	bool execute_(const char* str_) override {
		if(hxconsole_is_end_of_line_(str_)) {
			// 0 parameters is a query.
			hxlog_handler(hxlog_level_console, "%.15g\n", static_cast<double>(*m_var_));
			return true;
		}
		char* next_ = const_cast<char*>(str_);
		var_t_ val_ = hxconsole_parse_arg_<var_t_>(str_, &next_);
		if(str_ < next_ && hxconsole_is_end_of_line_(next_)) {
			*m_var_ = val_;
			return true;
		}
		hxlog_handler(hxlog_level_console, "parse error at: %s\n", str_);
		return false;
	}

	void usage_(const char* id_) override {
		hxlog_handler(hxlog_level_console, "%s <optional-value>\n", (id_ != hxnull) ? id_ : "usage:");
	}
private:
	volatile var_t_* m_var_;
};

// ----------------------------------------------------------------------------
// Single factory function. The compiler deduces args_t_... from the function
// pointer.

template<typename... args_t_>
inline hxconsole_command_impl_<args_t_...> hxconsole_command_factory_(bool(*fn_)(args_t_...)) {
	return hxconsole_command_impl_<args_t_...>(fn_);
}

template<typename var_t_>
inline hxconsole_variable_<var_t_> hxconsole_variable_factory_(volatile var_t_* var_) {
	return hxconsole_variable_<var_t_>(var_);
}

// ERROR: Pointers cannot be console variables.
template<typename var_t_>
inline void hxconsole_variable_factory_(volatile var_t_** var_) = delete;
template<typename var_t_>
inline void hxconsole_variable_factory_(const volatile var_t_** var_) = delete;

// ----------------------------------------------------------------------------
// Hash table infrastructure (unchanged).

// Wrap the string literal type because it is not used normally.
class hxconsole_hash_table_key_ {
public:
	explicit hxconsole_hash_table_key_(const char* s_) : str_(s_) { }
	const char* str_;
};

// Uses FNV-1a string hashing. Stops at whitespace.
inline hxhash_t hxkey_hash(hxconsole_hash_table_key_ k_) {
	hxhash_t x_ = static_cast<hxhash_t>(0x811c9dc5);
	while(hxisgraph(*k_.str_)) {
		x_ ^= static_cast<hxhash_t>(*k_.str_++);
		x_ *= static_cast<hxhash_t>(0x01000193);
	}
	return x_;
}

// A version of ::strcmp that stops at the first non-graphical characters.
inline bool hxkey_equal(hxconsole_hash_table_key_ a_, hxconsole_hash_table_key_ b_) {
	while(hxisgraph(*a_.str_) && *a_.str_ == *b_.str_) { ++a_.str_; ++b_.str_; }
	return !hxisgraph(*a_.str_) && !hxisgraph(*b_.str_);
};

// this is how to write a hash node without including hash table code.
class hxconsole_hash_table_node_ {
public:
	using key_t = hxconsole_hash_table_key_;

	hxconsole_hash_table_node_(hxconsole_hash_table_key_ key_)
			: m_hash_next_(hxnull), m_key_(key_), m_hash_(hxkey_hash(key_)), m_command_(hxnull) {
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
	hxconsole_hash_table_node_*& hash_next(void) { return m_hash_next_; }

	const hxconsole_hash_table_key_& hash_key(void) const { return m_key_; }
	hxhash_t hash_value(void) const { return m_hash_; }
	static hxhash_t hash_value(hxconsole_hash_table_key_ key_) { return hxkey_hash(key_); }
	hxconsole_command_* command_(void) const { return m_command_; }
	void set_command_(hxconsole_command_* x_) { m_command_ = x_; }

private:
	hxconsole_hash_table_node_* m_hash_next_;
	hxconsole_hash_table_key_ m_key_;
	hxhash_t m_hash_;
	hxconsole_command_* m_command_;
};

void hxconsole_register_(hxconsole_hash_table_node_* node);

// registers a console command using a global variable without memory allocations.
// There is no reason to deregister or destruct anything.
class hxconsole_constructor_ {
public:
	template<typename command_t_>
	hxconsole_constructor_(command_t_ fn_, const char* id_)
			: m_node_(hxconsole_hash_table_key_(id_)) {
		static_assert(sizeof(command_t_) <= sizeof(m_storage_), "command_storage_overflow");
		::new(m_storage_ + 0) command_t_(hxmove(fn_));
		m_node_.set_command_(reinterpret_cast<command_t_*>(m_storage_ + 0));
		hxconsole_register_(&m_node_);
	}

private:
	// Provide static storage instead of using allocator before main.
	// Two pointers: vtable ptr + one fn or data ptr (sizeof(void*) each).
	// Sufficient for hxconsole_command_impl_ and hxconsole_variable_ on
	// ILP32, LLP64, and LP64. Enforced by static_assert in the constructor.
	hxconsole_hash_table_node_ m_node_;
	char m_storage_[sizeof(void*) + sizeof(void*)];
};

} // hxdetail_
