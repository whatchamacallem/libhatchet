// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

#include <hx/hxconsole.hpp>
#include <hx/hxfile.hpp>
#include <hx/hxtest.hpp>

HX_NS_USE
#if HX_USE_CONSOLE

// TEST_F fixtures would not work as the console registers static variables.

namespace {
float hxs_console_test_result_hook = 0.0f;

bool hxconsole_test_register0(float a0, const char* a1) {
	hxs_console_test_result_hook = a0 + static_cast<float>(::strlen(a1));
	return true;
}
bool hxconsole_test_register1(float a0) {
	hxs_console_test_result_hook = a0;
	return true;
}
bool hxconsole_test_register2(float a0) {
	hxs_console_test_result_hook = a0;
	return true;
}
bool hxconsole_test_register3(float, float a1) {
	hxs_console_test_result_hook = a1;
	return true;
}

// --- variable fixtures ---
int8_t   hxs_console_test_i8    = 0;
int16_t  hxs_console_test_i16   = 0;
int32_t  hxs_console_test_i32   = 0;
int64_t  hxs_console_test_i64   = 0;
uint8_t  hxs_console_test_u8    = 0;
uint16_t hxs_console_test_u16   = 0;
uint32_t hxs_console_test_u32   = 0;
uint64_t hxs_console_test_u64   = 0;
float    hxs_console_test_f32   = 0.0f;
double   hxs_console_test_f64   = 0.0;
bool     hxs_console_test_bool  = false;
size_t   hxs_console_test_size  = 0u;
char     hxs_console_test_char  = 0;

// --- per-type function fixtures ---
int8_t   hxs_console_test_fn_i8   = 0;
int16_t  hxs_console_test_fn_i16  = 0;
int32_t  hxs_console_test_fn_i32  = 0;
int64_t  hxs_console_test_fn_i64  = 0;
uint8_t  hxs_console_test_fn_u8   = 0;
uint16_t hxs_console_test_fn_u16  = 0;
uint32_t hxs_console_test_fn_u32  = 0;
uint64_t hxs_console_test_fn_u64  = 0;
char     hxs_console_test_fn_char = 0;
float    hxs_console_test_fn_f32  = 0.0f;
double   hxs_console_test_fn_f64  = 0.0;
bool     hxs_console_test_fn_bool = false;
const char* hxs_console_test_fn_str = hxnull;

bool hxconsole_test_fn_i8(int8_t v_)       { hxs_console_test_fn_i8   = v_; return true; }
bool hxconsole_test_fn_i16(int16_t v_)     { hxs_console_test_fn_i16  = v_; return true; }
bool hxconsole_test_fn_i32(int32_t v_)     { hxs_console_test_fn_i32  = v_; return true; }
bool hxconsole_test_fn_i64(int64_t v_)     { hxs_console_test_fn_i64  = v_; return true; }
bool hxconsole_test_fn_u8(uint8_t v_)      { hxs_console_test_fn_u8   = v_; return true; }
bool hxconsole_test_fn_u16(uint16_t v_)    { hxs_console_test_fn_u16  = v_; return true; }
bool hxconsole_test_fn_u32(uint32_t v_)    { hxs_console_test_fn_u32  = v_; return true; }
bool hxconsole_test_fn_u64(uint64_t v_)    { hxs_console_test_fn_u64  = v_; return true; }
bool hxconsole_test_fn_char(char v_)       { hxs_console_test_fn_char = v_; return true; }
bool hxconsole_test_fn_f32(float v_)       { hxs_console_test_fn_f32  = v_; return true; }
bool hxconsole_test_fn_f64(double v_)      { hxs_console_test_fn_f64  = v_; return true; }
bool hxconsole_test_fn_bool(bool v_)       { hxs_console_test_fn_bool = v_; return true; }
bool hxconsole_test_fn_str(const char* v_) { hxs_console_test_fn_str  = v_; return true; }

// --- multi-arg function fixtures ---
int32_t     hxs_console_test_fn_mixed_i32 = 0;
float       hxs_console_test_fn_mixed_f32 = 0.0f;
const char* hxs_console_test_fn_mixed_str = hxnull;
int32_t     hxs_console_test_fn_ints_i32  = 0;
uint32_t    hxs_console_test_fn_ints_u32  = 0;
bool        hxs_console_test_fn_void_called = false;

bool hxconsole_test_fn_mixed(int32_t a_, float b_, const char* c_) {
	hxs_console_test_fn_mixed_i32 = a_; hxs_console_test_fn_mixed_f32 = b_; hxs_console_test_fn_mixed_str = c_;
	return true;
}
bool hxconsole_test_fn_ints(int32_t a_, uint32_t b_) {
	hxs_console_test_fn_ints_i32 = a_; hxs_console_test_fn_ints_u32 = b_;
	return true;
}
bool hxconsole_test_fn_void(void) { hxs_console_test_fn_void_called = true; return true; }

// --- file test fixtures ---
volatile float hxs_console_test_file_var1 = 0.0f;
volatile float hxs_console_test_file_var2 = 0.0f;

bool hxconsole_test_file_fn(float f_) {
	hxs_console_test_file_var2 = f_;
	return true;
}

static bool hxconsole_test_failing_command(void) { return false; }

} // namespace

// --- registrations ---
hxconsole_command(hxconsole_test_register0);
hxconsole_command(hxconsole_test_register1);
hxconsole_command(hxconsole_test_register2);
hxconsole_command(hxconsole_test_register3);

hxconsole_variable(hxs_console_test_i8);
hxconsole_variable(hxs_console_test_i16);
hxconsole_variable(hxs_console_test_i32);
hxconsole_variable(hxs_console_test_i64);
hxconsole_variable(hxs_console_test_u8);
hxconsole_variable(hxs_console_test_u16);
hxconsole_variable(hxs_console_test_u32);
hxconsole_variable(hxs_console_test_u64);
hxconsole_variable(hxs_console_test_f32);
hxconsole_variable(hxs_console_test_f64);
hxconsole_variable(hxs_console_test_bool);
hxconsole_variable(hxs_console_test_size);
hxconsole_variable(hxs_console_test_char);

hxconsole_command(hxconsole_test_fn_i8);
hxconsole_command(hxconsole_test_fn_i16);
hxconsole_command(hxconsole_test_fn_i32);
hxconsole_command(hxconsole_test_fn_i64);
hxconsole_command(hxconsole_test_fn_u8);
hxconsole_command(hxconsole_test_fn_u16);
hxconsole_command(hxconsole_test_fn_u32);
hxconsole_command(hxconsole_test_fn_u64);
hxconsole_command(hxconsole_test_fn_char);
hxconsole_command(hxconsole_test_fn_f32);
hxconsole_command(hxconsole_test_fn_f64);
hxconsole_command(hxconsole_test_fn_bool);
hxconsole_command(hxconsole_test_fn_str);
hxconsole_command(hxconsole_test_fn_mixed);
hxconsole_command(hxconsole_test_fn_ints);
hxconsole_command(hxconsole_test_fn_void);

hxconsole_variable_named(hxs_console_test_file_var1, hxconsole_test_file_var);
hxconsole_command_named(hxconsole_test_file_fn, hxconsole_test_file_fn_name);
hxconsole_command_named(hxconsole_test_failing_command, hxconsole_test_failing_command);

// register_command

TEST(hxconsole_test, register_command) {
	hxlog_warning("EXPECTING_TEST_WARNINGS\n");

	// 77 + strlen("...") = 80
	hxs_console_test_result_hook = 0.0f;
	EXPECT_TRUE(hxconsole_exec_line("hxconsole_test_register0 77 ..."));
	EXPECT_EQ(80.0f, hxs_console_test_result_hook);

	// Literal float passes through unchanged.
	hxs_console_test_result_hook = 0.0f;
	EXPECT_TRUE(hxconsole_exec_line("hxconsole_test_register1 12.5"));
	EXPECT_EQ(12.5f, hxs_console_test_result_hook);

	// Missing argument -> false, hook unchanged.
	hxs_console_test_result_hook = -1.0f;
	EXPECT_FALSE(hxconsole_exec_line("hxconsole_test_register2 "));
	EXPECT_EQ(-1.0f, hxs_console_test_result_hook);

	// Second argument missing -> false.
	hxs_console_test_result_hook = -2.0f;
	EXPECT_FALSE(hxconsole_exec_line("hxconsole_test_register3 7 "));
	EXPECT_EQ(-2.0f, hxs_console_test_result_hook);

	// Too many arguments -> false.
	hxs_console_test_result_hook = -2.0f;
	EXPECT_FALSE(hxconsole_exec_line("hxconsole_test_register3 7 8 9 "));
	EXPECT_EQ(-2.0f, hxs_console_test_result_hook);

	// Unknown command -> false.
	EXPECT_FALSE(hxconsole_exec_line("Not_exist"));

	// Deregister removes the symbol.
	hxconsole_deregister("hxconsole_test_register0");
	EXPECT_FALSE(hxconsole_exec_line("hxconsole_test_register0 77 ..."));
}

// variable_set: per-type set then verify

TEST(hxconsole_test, variable_set) {
	// int8_t
	hxs_console_test_i8 = 0;
	EXPECT_TRUE(hxconsole_exec_line("hxs_console_test_i8 123"));
	EXPECT_EQ(hxs_console_test_i8, (int8_t)123);

	// int16_t
	hxs_console_test_i16 = 0;
	EXPECT_TRUE(hxconsole_exec_line("hxs_console_test_i16 234"));
	EXPECT_EQ(hxs_console_test_i16, (int16_t)234);

	// int32_t
	hxs_console_test_i32 = 0;
	EXPECT_TRUE(hxconsole_exec_line("hxs_console_test_i32 345"));
	EXPECT_EQ(hxs_console_test_i32, (int32_t)345);

	// int64_t
	hxs_console_test_i64 = 0;
	EXPECT_TRUE(hxconsole_exec_line("hxs_console_test_i64 567"));
	EXPECT_EQ(hxs_console_test_i64, (int64_t)567);

	// uint8_t
	hxs_console_test_u8 = 0;
	EXPECT_TRUE(hxconsole_exec_line("hxs_console_test_u8 12"));
	EXPECT_EQ(hxs_console_test_u8, (uint8_t)12);

	// uint16_t
	hxs_console_test_u16 = 0;
	EXPECT_TRUE(hxconsole_exec_line("hxs_console_test_u16 2345"));
	EXPECT_EQ(hxs_console_test_u16, (uint16_t)2345);

	// uint32_t
	hxs_console_test_u32 = 0;
	EXPECT_TRUE(hxconsole_exec_line("hxs_console_test_u32 3456"));
	EXPECT_EQ(hxs_console_test_u32, (uint32_t)3456);

	// uint64_t
	hxs_console_test_u64 = 0;
	EXPECT_TRUE(hxconsole_exec_line("hxs_console_test_u64 5678"));
	EXPECT_EQ(hxs_console_test_u64, (uint64_t)5678);

	// float
	hxs_console_test_f32 = 0.0f;
	EXPECT_TRUE(hxconsole_exec_line("hxs_console_test_f32 678.0"));
	EXPECT_EQ(hxs_console_test_f32, 678.0f);

	// double
	hxs_console_test_f64 = 0.0;
	EXPECT_TRUE(hxconsole_exec_line("hxs_console_test_f64 789.0"));
	EXPECT_EQ(hxs_console_test_f64, 789.0);

	// bool: 0 -> false, 1 -> true
	hxs_console_test_bool = true;
	EXPECT_TRUE(hxconsole_exec_line("hxs_console_test_bool 0"));
	EXPECT_EQ(hxs_console_test_bool, false);
	EXPECT_TRUE(hxconsole_exec_line("hxs_console_test_bool 1"));
	EXPECT_EQ(hxs_console_test_bool, true);

	// size_t
	hxs_console_test_size = 0u;
	EXPECT_TRUE(hxconsole_exec_line("hxs_console_test_size 1000"));
	EXPECT_EQ(hxs_console_test_size, (size_t)1000);

	// char (int8_t alias on this platform)
	hxs_console_test_char = 0;
	EXPECT_TRUE(hxconsole_exec_line("hxs_console_test_char 65"));
	EXPECT_EQ(hxs_console_test_char, (char)65);

	// Trailing fractional part is a parse error for integer types.
	hxs_console_test_i32 = -1;
	EXPECT_FALSE(hxconsole_exec_line("hxs_console_test_i32 3.5"));
	EXPECT_EQ(hxs_console_test_i32, (int32_t)-1);

	// hex input
	hxs_console_test_i32 = 0;
	EXPECT_TRUE(hxconsole_exec_line("hxs_console_test_i32 0xFF"));
	EXPECT_EQ(hxs_console_test_i32, (int32_t)255);
}

// variable_query: name with no value logs current value and returns true

TEST(hxconsole_test, variable_query) {
	hxs_console_test_i32 = 42;
	EXPECT_TRUE(hxconsole_exec_line("hxs_console_test_i32"));
	EXPECT_EQ(hxs_console_test_i32, (int32_t)42); // unchanged

	hxs_console_test_f32 = -1.5f;
	EXPECT_TRUE(hxconsole_exec_line("hxs_console_test_f32"));
	EXPECT_EQ(hxs_console_test_f32, -1.5f); // unchanged

	hxs_console_test_bool = true;
	EXPECT_TRUE(hxconsole_exec_line("hxs_console_test_bool"));
	EXPECT_EQ(hxs_console_test_bool, true); // unchanged
}

// variable_overflow: out-of-range returns false, variable unchanged.
// exact boundaries must succeed.

TEST(hxconsole_test, variable_overflow) {
	hxlog_warning("EXPECTING_TEST_WARNINGS\n");

	// int8_t exact boundaries
	hxs_console_test_i8 = 0;
	EXPECT_TRUE(hxconsole_exec_line("hxs_console_test_i8 127"));
	EXPECT_EQ(hxs_console_test_i8, (int8_t)127);
	EXPECT_TRUE(hxconsole_exec_line("hxs_console_test_i8 -128"));
	EXPECT_EQ(hxs_console_test_i8, (int8_t)-128);
	// one beyond
	hxs_console_test_i8 = 99;
	EXPECT_FALSE(hxconsole_exec_line("hxs_console_test_i8 128"));
	EXPECT_EQ(hxs_console_test_i8, (int8_t)99);
	EXPECT_FALSE(hxconsole_exec_line("hxs_console_test_i8 -129"));
	EXPECT_EQ(hxs_console_test_i8, (int8_t)99);
	// out-of-range
	hxs_console_test_i8 = 55;
	EXPECT_FALSE(hxconsole_exec_line("hxs_console_test_i8 200"));
	EXPECT_EQ(hxs_console_test_i8, (int8_t)55);
	EXPECT_FALSE(hxconsole_exec_line("hxs_console_test_i8 -200"));
	EXPECT_EQ(hxs_console_test_i8, (int8_t)55);

	// int16_t exact boundaries
	hxs_console_test_i16 = 0;
	EXPECT_TRUE(hxconsole_exec_line("hxs_console_test_i16 32767"));
	EXPECT_EQ(hxs_console_test_i16, (int16_t)32767);
	EXPECT_TRUE(hxconsole_exec_line("hxs_console_test_i16 -32768"));
	EXPECT_EQ(hxs_console_test_i16, (int16_t)-32768);
	// out-of-range
	hxs_console_test_i16 = 1;
	EXPECT_FALSE(hxconsole_exec_line("hxs_console_test_i16 40000"));
	EXPECT_EQ(hxs_console_test_i16, (int16_t)1);
	EXPECT_FALSE(hxconsole_exec_line("hxs_console_test_i16 -40000"));
	EXPECT_EQ(hxs_console_test_i16, (int16_t)1);

	// int32_t exact boundaries
	hxs_console_test_i32 = 0;
	EXPECT_TRUE(hxconsole_exec_line("hxs_console_test_i32 2147483647"));
	EXPECT_EQ(hxs_console_test_i32, (int32_t)2147483647);
	EXPECT_TRUE(hxconsole_exec_line("hxs_console_test_i32 -2147483648"));
	EXPECT_EQ(hxs_console_test_i32, (int32_t)-2147483648);
	// out-of-range
	hxs_console_test_i32 = 2;
	EXPECT_FALSE(hxconsole_exec_line("hxs_console_test_i32 3000000000"));
	EXPECT_EQ(hxs_console_test_i32, (int32_t)2);
	EXPECT_FALSE(hxconsole_exec_line("hxs_console_test_i32 -3000000000"));
	EXPECT_EQ(hxs_console_test_i32, (int32_t)2);

	// uint8_t exact boundaries
	hxs_console_test_u8 = 1;
	EXPECT_TRUE(hxconsole_exec_line("hxs_console_test_u8 255"));
	EXPECT_EQ(hxs_console_test_u8, (uint8_t)255);
	EXPECT_TRUE(hxconsole_exec_line("hxs_console_test_u8 0"));
	EXPECT_EQ(hxs_console_test_u8, (uint8_t)0);
	// one beyond
	hxs_console_test_u8 = 7;
	EXPECT_FALSE(hxconsole_exec_line("hxs_console_test_u8 256"));
	EXPECT_EQ(hxs_console_test_u8, (uint8_t)7);
	// out-of-range
	EXPECT_FALSE(hxconsole_exec_line("hxs_console_test_u8 300"));
	EXPECT_EQ(hxs_console_test_u8, (uint8_t)7);

	// uint16_t exact boundaries
	hxs_console_test_u16 = 1;
	EXPECT_TRUE(hxconsole_exec_line("hxs_console_test_u16 65535"));
	EXPECT_EQ(hxs_console_test_u16, (uint16_t)65535);
	EXPECT_TRUE(hxconsole_exec_line("hxs_console_test_u16 0"));
	EXPECT_EQ(hxs_console_test_u16, (uint16_t)0);
	// out-of-range
	hxs_console_test_u16 = 3;
	EXPECT_FALSE(hxconsole_exec_line("hxs_console_test_u16 70000"));
	EXPECT_EQ(hxs_console_test_u16, (uint16_t)3);

	// uint32_t exact boundaries
	hxs_console_test_u32 = 1;
	EXPECT_TRUE(hxconsole_exec_line("hxs_console_test_u32 4294967295"));
	EXPECT_EQ(hxs_console_test_u32, (uint32_t)4294967295u);
	EXPECT_TRUE(hxconsole_exec_line("hxs_console_test_u32 0"));
	EXPECT_EQ(hxs_console_test_u32, (uint32_t)0);
	// out-of-range
	hxs_console_test_u32 = 4;
	EXPECT_FALSE(hxconsole_exec_line("hxs_console_test_u32 5000000000"));
	EXPECT_EQ(hxs_console_test_u32, (uint32_t)4);
}

// float_overflow: ERANGE from strtof/strtod resets next and rejects the value.
// This is the only path that exercises the floating-point ERANGE guard.

TEST(hxconsole_test, float_overflow) {
	hxlog_warning("EXPECTING_TEST_WARNINGS\n");

	// float: well beyond FLT_MAX (~3.4e38). Value rejected, variable unchanged.
	hxs_console_test_f32 = 1.0f;
	EXPECT_FALSE(hxconsole_exec_line("hxs_console_test_f32 1e40"));
	EXPECT_EQ(hxs_console_test_f32, 1.0f);

	// double: well beyond DBL_MAX (~1.8e308). Value rejected, variable unchanged.
	hxs_console_test_f64 = 2.0;
	EXPECT_FALSE(hxconsole_exec_line("hxs_console_test_f64 1e400"));
	EXPECT_EQ(hxs_console_test_f64, 2.0);

	// Same guard on the function argument path.
	hxs_console_test_fn_f32 = 3.0f;
	EXPECT_FALSE(hxconsole_exec_line("hxconsole_test_fn_f32 1e40"));
	EXPECT_EQ(hxs_console_test_fn_f32, 3.0f);

	hxs_console_test_fn_f64 = 4.0;
	EXPECT_FALSE(hxconsole_exec_line("hxconsole_test_fn_f64 1e400"));
	EXPECT_EQ(hxs_console_test_fn_f64, 4.0);
}

// hex_overflow: out-of-range hexadecimal input is rejected like decimal input.

TEST(hxconsole_test, hex_overflow) {
	hxlog_warning("EXPECTING_TEST_WARNINGS\n");

	// uint8_t: 0x100 is one beyond UCHAR_MAX.
	hxs_console_test_u8 = 9;
	EXPECT_FALSE(hxconsole_exec_line("hxs_console_test_u8 0x100"));
	EXPECT_EQ(hxs_console_test_u8, (uint8_t)9);

	// uint16_t: 0x10000 is one beyond USHRT_MAX.
	hxs_console_test_u16 = 9;
	EXPECT_FALSE(hxconsole_exec_line("hxs_console_test_u16 0x10000"));
	EXPECT_EQ(hxs_console_test_u16, (uint16_t)9);

	// uint32_t: 0x1FFFFFFFF is one bit beyond UINT32_MAX.
	hxs_console_test_u32 = 9;
	EXPECT_FALSE(hxconsole_exec_line("hxs_console_test_u32 0x1FFFFFFFF"));
	EXPECT_EQ(hxs_console_test_u32, (uint32_t)9);

	// int8_t: 0x80 is one beyond SCHAR_MAX.
	hxs_console_test_i8 = 9;
	EXPECT_FALSE(hxconsole_exec_line("hxs_console_test_i8 0x80"));
	EXPECT_EQ(hxs_console_test_i8, (int8_t)9);
}

// whitespace_dispatch: leading and extra internal whitespace route correctly.

TEST(hxconsole_test, whitespace_dispatch) {
	// Leading whitespace before the command name.
	hxs_console_test_i32 = 0;
	EXPECT_TRUE(hxconsole_exec_line("   hxs_console_test_i32 17"));
	EXPECT_EQ(hxs_console_test_i32, (int32_t)17);

	// Tab as leading whitespace and between command and value.
	hxs_console_test_i32 = 0;
	EXPECT_TRUE(hxconsole_exec_line("\thxs_console_test_i32\t18"));
	EXPECT_EQ(hxs_console_test_i32, (int32_t)18);

	// Multiple spaces between command and multiple arguments.
	hxs_console_test_fn_ints_i32 = 0; hxs_console_test_fn_ints_u32 = 0;
	EXPECT_TRUE(hxconsole_exec_line("hxconsole_test_fn_ints    -5     100"));
	EXPECT_EQ(hxs_console_test_fn_ints_i32, (int32_t)-5);
	EXPECT_EQ(hxs_console_test_fn_ints_u32, (uint32_t)100);
}

// blank_line: empty and whitespace/comment-only lines succeed without dispatch.

TEST(hxconsole_test, blank_line) {
	EXPECT_TRUE(hxconsole_exec_line(""));
	EXPECT_TRUE(hxconsole_exec_line("   "));
	EXPECT_TRUE(hxconsole_exec_line("\t \t"));
	EXPECT_TRUE(hxconsole_exec_line("# just a comment"));
	EXPECT_TRUE(hxconsole_exec_line("   # indented comment"));
}

// signed_plus_prefix: an explicit '+' sign is accepted for signed and unsigned.

TEST(hxconsole_test, signed_plus_prefix) {
	hxs_console_test_i32 = 0;
	EXPECT_TRUE(hxconsole_exec_line("hxs_console_test_i32 +5"));
	EXPECT_EQ(hxs_console_test_i32, (int32_t)5);

	hxs_console_test_u32 = 0;
	EXPECT_TRUE(hxconsole_exec_line("hxs_console_test_u32 +6"));
	EXPECT_EQ(hxs_console_test_u32, (uint32_t)6);
}

// variable_parse_error: garbage/trailing-garbage/multi-value leave variable unchanged

TEST(hxconsole_test, variable_parse_error) {
	hxlog_warning("EXPECTING_TEST_WARNINGS\n");

	// Trailing garbage: parse advances past "42" but trailing "garbage" is not end-of-line.
	hxs_console_test_i32 = 10;
	EXPECT_FALSE(hxconsole_exec_line("hxs_console_test_i32 42 garbage"));
	EXPECT_EQ(hxs_console_test_i32, (int32_t)10);

	// Multiple values.
	hxs_console_test_i32 = 10;
	EXPECT_FALSE(hxconsole_exec_line("hxs_console_test_i32 1 2"));
	EXPECT_EQ(hxs_console_test_i32, (int32_t)10);

	// Pure garbage (no number at all).
	hxs_console_test_i32 = 10;
	EXPECT_FALSE(hxconsole_exec_line("hxs_console_test_i32 abc"));
	EXPECT_EQ(hxs_console_test_i32, (int32_t)10);

	// bool: only 0 and 1 are valid. 2 and non-numeric strings are parse errors.
	hxs_console_test_bool = true;
	EXPECT_FALSE(hxconsole_exec_line("hxs_console_test_bool 2"));
	EXPECT_EQ(hxs_console_test_bool, true);
	EXPECT_FALSE(hxconsole_exec_line("hxs_console_test_bool notanumber"));
	EXPECT_EQ(hxs_console_test_bool, true);
}

// function_types: single-arg per-type

TEST(hxconsole_test, function_types) {
	// int8_t
	hxs_console_test_fn_i8 = 0;
	EXPECT_TRUE(hxconsole_exec_line("hxconsole_test_fn_i8 42"));
	EXPECT_EQ(hxs_console_test_fn_i8, (int8_t)42);
	EXPECT_TRUE(hxconsole_exec_line("hxconsole_test_fn_i8 -1"));
	EXPECT_EQ(hxs_console_test_fn_i8, (int8_t)-1);
	EXPECT_TRUE(hxconsole_exec_line("hxconsole_test_fn_i8 0x1F"));
	EXPECT_EQ(hxs_console_test_fn_i8, (int8_t)0x1F);

	// int16_t
	hxs_console_test_fn_i16 = 0;
	EXPECT_TRUE(hxconsole_exec_line("hxconsole_test_fn_i16 1000"));
	EXPECT_EQ(hxs_console_test_fn_i16, (int16_t)1000);
	EXPECT_TRUE(hxconsole_exec_line("hxconsole_test_fn_i16 -1000"));
	EXPECT_EQ(hxs_console_test_fn_i16, (int16_t)-1000);

	// int32_t
	hxs_console_test_fn_i32 = 0;
	EXPECT_TRUE(hxconsole_exec_line("hxconsole_test_fn_i32 100000"));
	EXPECT_EQ(hxs_console_test_fn_i32, (int32_t)100000);
	EXPECT_TRUE(hxconsole_exec_line("hxconsole_test_fn_i32 -100000"));
	EXPECT_EQ(hxs_console_test_fn_i32, (int32_t)-100000);
	EXPECT_TRUE(hxconsole_exec_line("hxconsole_test_fn_i32 0xFF"));
	EXPECT_EQ(hxs_console_test_fn_i32, (int32_t)0xFF);

	// int64_t
	hxs_console_test_fn_i64 = 0;
	EXPECT_TRUE(hxconsole_exec_line("hxconsole_test_fn_i64 100000"));
	EXPECT_EQ(hxs_console_test_fn_i64, (int64_t)100000);

	// uint8_t
	hxs_console_test_fn_u8 = 0;
	EXPECT_TRUE(hxconsole_exec_line("hxconsole_test_fn_u8 200"));
	EXPECT_EQ(hxs_console_test_fn_u8, (uint8_t)200);

	// uint16_t
	hxs_console_test_fn_u16 = 0;
	EXPECT_TRUE(hxconsole_exec_line("hxconsole_test_fn_u16 50000"));
	EXPECT_EQ(hxs_console_test_fn_u16, (uint16_t)50000);

	// uint32_t
	hxs_console_test_fn_u32 = 0;
	EXPECT_TRUE(hxconsole_exec_line("hxconsole_test_fn_u32 3000000000"));
	EXPECT_EQ(hxs_console_test_fn_u32, (uint32_t)3000000000u);
	EXPECT_TRUE(hxconsole_exec_line("hxconsole_test_fn_u32 0xDEAD"));
	EXPECT_EQ(hxs_console_test_fn_u32, (uint32_t)0xDEAD);

	// uint64_t (UINT64_MAX)
	hxs_console_test_fn_u64 = 0;
	EXPECT_TRUE(hxconsole_exec_line("hxconsole_test_fn_u64 18446744073709551615"));
	EXPECT_EQ(hxs_console_test_fn_u64, (uint64_t)18446744073709551615ull);

	// char
	hxs_console_test_fn_char = 0;
	EXPECT_TRUE(hxconsole_exec_line("hxconsole_test_fn_char 65"));
	EXPECT_EQ(hxs_console_test_fn_char, (char)65);
	EXPECT_TRUE(hxconsole_exec_line("hxconsole_test_fn_char 127"));
	EXPECT_EQ(hxs_console_test_fn_char, (char)127);

	// float
	hxs_console_test_fn_f32 = 0.0f;
	EXPECT_TRUE(hxconsole_exec_line("hxconsole_test_fn_f32 3.14"));
	EXPECT_TRUE(hxs_console_test_fn_f32 > 3.13f && hxs_console_test_fn_f32 < 3.15f);
	EXPECT_TRUE(hxconsole_exec_line("hxconsole_test_fn_f32 -1.5"));
	EXPECT_EQ(hxs_console_test_fn_f32, -1.5f);

	// double
	hxs_console_test_fn_f64 = 0.0;
	EXPECT_TRUE(hxconsole_exec_line("hxconsole_test_fn_f64 3.14159265358979"));
	EXPECT_TRUE(hxs_console_test_fn_f64 > 3.14159 && hxs_console_test_fn_f64 < 3.14160);

	// bool
	hxs_console_test_fn_bool = false;
	EXPECT_TRUE(hxconsole_exec_line("hxconsole_test_fn_bool 1"));
	EXPECT_EQ(hxs_console_test_fn_bool, true);
	EXPECT_TRUE(hxconsole_exec_line("hxconsole_test_fn_bool 0"));
	EXPECT_EQ(hxs_console_test_fn_bool, false);

	// const char*
	hxs_console_test_fn_str = hxnull;
	EXPECT_TRUE(hxconsole_exec_line("hxconsole_test_fn_str hello world"));
	EXPECT_TRUE(hxs_console_test_fn_str != hxnull && ::strncmp(hxs_console_test_fn_str, "hello", 5) == 0);
}

// function_overflow: exact boundary succeeds, one-beyond fails

TEST(hxconsole_test, function_overflow) {
	hxlog_warning("EXPECTING_TEST_WARNINGS\n");

	// int8_t
	hxs_console_test_fn_i8 = 0;
	EXPECT_TRUE(hxconsole_exec_line("hxconsole_test_fn_i8 127"));
	EXPECT_EQ(hxs_console_test_fn_i8, (int8_t)127);
	EXPECT_TRUE(hxconsole_exec_line("hxconsole_test_fn_i8 -128"));
	EXPECT_EQ(hxs_console_test_fn_i8, (int8_t)-128);
	hxs_console_test_fn_i8 = 0;
	EXPECT_FALSE(hxconsole_exec_line("hxconsole_test_fn_i8 128"));
	EXPECT_EQ(hxs_console_test_fn_i8, (int8_t)0);
	EXPECT_FALSE(hxconsole_exec_line("hxconsole_test_fn_i8 -129"));
	EXPECT_EQ(hxs_console_test_fn_i8, (int8_t)0);

	// int16_t
	hxs_console_test_fn_i16 = 0;
	EXPECT_TRUE(hxconsole_exec_line("hxconsole_test_fn_i16 32767"));
	EXPECT_EQ(hxs_console_test_fn_i16, (int16_t)32767);
	EXPECT_TRUE(hxconsole_exec_line("hxconsole_test_fn_i16 -32768"));
	EXPECT_EQ(hxs_console_test_fn_i16, (int16_t)-32768);
	hxs_console_test_fn_i16 = 0;
	EXPECT_FALSE(hxconsole_exec_line("hxconsole_test_fn_i16 32768"));
	EXPECT_EQ(hxs_console_test_fn_i16, (int16_t)0);

	// uint8_t
	hxs_console_test_fn_u8 = 0;
	EXPECT_TRUE(hxconsole_exec_line("hxconsole_test_fn_u8 255"));
	EXPECT_EQ(hxs_console_test_fn_u8, (uint8_t)255);
	EXPECT_TRUE(hxconsole_exec_line("hxconsole_test_fn_u8 0"));
	EXPECT_EQ(hxs_console_test_fn_u8, (uint8_t)0);
	hxs_console_test_fn_u8 = 0;
	EXPECT_FALSE(hxconsole_exec_line("hxconsole_test_fn_u8 256"));
	EXPECT_EQ(hxs_console_test_fn_u8, (uint8_t)0);

	// uint16_t
	hxs_console_test_fn_u16 = 0;
	EXPECT_TRUE(hxconsole_exec_line("hxconsole_test_fn_u16 65535"));
	EXPECT_EQ(hxs_console_test_fn_u16, (uint16_t)65535);
	hxs_console_test_fn_u16 = 0;
	EXPECT_FALSE(hxconsole_exec_line("hxconsole_test_fn_u16 65536"));
	EXPECT_EQ(hxs_console_test_fn_u16, (uint16_t)0);

	// int32_t (errno=ERANGE path)
	hxs_console_test_fn_i32 = 0;
	EXPECT_FALSE(hxconsole_exec_line("hxconsole_test_fn_i32 2147483648"));
	EXPECT_EQ(hxs_console_test_fn_i32, (int32_t)0);
	EXPECT_FALSE(hxconsole_exec_line("hxconsole_test_fn_i32 -2147483649"));
	EXPECT_EQ(hxs_console_test_fn_i32, (int32_t)0);

	// uint32_t (errno=ERANGE path)
	hxs_console_test_fn_u32 = 0;
	EXPECT_FALSE(hxconsole_exec_line("hxconsole_test_fn_u32 4294967296"));
	EXPECT_EQ(hxs_console_test_fn_u32, (uint32_t)0);

	// int64_t
	hxs_console_test_fn_i64 = 0;
	EXPECT_TRUE(hxconsole_exec_line("hxconsole_test_fn_i64 9223372036854775807"));
	EXPECT_EQ(hxs_console_test_fn_i64, (int64_t)9223372036854775807ll);
	hxs_console_test_fn_i64 = 0;
	EXPECT_FALSE(hxconsole_exec_line("hxconsole_test_fn_i64 9223372036854775808"));
	EXPECT_EQ(hxs_console_test_fn_i64, (int64_t)0);

	// uint64_t
	hxs_console_test_fn_u64 = 0;
	EXPECT_TRUE(hxconsole_exec_line("hxconsole_test_fn_u64 18446744073709551615"));
	EXPECT_EQ(hxs_console_test_fn_u64, (uint64_t)18446744073709551615ull);
	hxs_console_test_fn_u64 = 0;
	EXPECT_FALSE(hxconsole_exec_line("hxconsole_test_fn_u64 18446744073709551616"));
	EXPECT_EQ(hxs_console_test_fn_u64, (uint64_t)0);

	// char
	hxs_console_test_fn_char = 0;
	EXPECT_TRUE(hxconsole_exec_line("hxconsole_test_fn_char 127"));
	EXPECT_EQ(hxs_console_test_fn_char, (char)127);
	hxs_console_test_fn_char = 0;
	EXPECT_FALSE(hxconsole_exec_line("hxconsole_test_fn_char 128"));
	EXPECT_EQ(hxs_console_test_fn_char, (char)0);

	// char: signed-char platforms only
#if CHAR_MIN < 0
	hxs_console_test_fn_char = 0;
	EXPECT_TRUE(hxconsole_exec_line("hxconsole_test_fn_char -128"));
	EXPECT_EQ(hxs_console_test_fn_char, (char)-128);
	hxs_console_test_fn_char = 0;
	EXPECT_FALSE(hxconsole_exec_line("hxconsole_test_fn_char -129"));
	EXPECT_EQ(hxs_console_test_fn_char, (char)0);
#endif
}

// function_multi_arg

TEST(hxconsole_test, function_multi_arg) {
	// mixed types
	hxs_console_test_fn_mixed_i32 = 0; hxs_console_test_fn_mixed_f32 = 0.0f; hxs_console_test_fn_mixed_str = hxnull;
	EXPECT_TRUE(hxconsole_exec_line("hxconsole_test_fn_mixed 7 2.5 hello"));
	EXPECT_EQ(hxs_console_test_fn_mixed_i32, (int32_t)7);
	EXPECT_EQ(hxs_console_test_fn_mixed_f32, 2.5f);
	EXPECT_TRUE(hxs_console_test_fn_mixed_str != hxnull && ::strncmp(hxs_console_test_fn_mixed_str, "hello", 5) == 0);

	// all-integer
	hxs_console_test_fn_ints_i32 = 0; hxs_console_test_fn_ints_u32 = 0;
	EXPECT_TRUE(hxconsole_exec_line("hxconsole_test_fn_ints -5 100"));
	EXPECT_EQ(hxs_console_test_fn_ints_i32, (int32_t)-5);
	EXPECT_EQ(hxs_console_test_fn_ints_u32, (uint32_t)100);

	// zero-arg
	hxs_console_test_fn_void_called = false;
	EXPECT_TRUE(hxconsole_exec_line("hxconsole_test_fn_void"));
	EXPECT_TRUE(hxs_console_test_fn_void_called);
}

// function_arity: too few/too many/wrong-type/zero-arg with args

TEST(hxconsole_test, function_arity) {
	hxlog_warning("EXPECTING_TEST_WARNINGS\n");

	// Too few args.
	hxs_console_test_fn_mixed_i32 = -1; hxs_console_test_fn_mixed_f32 = -1.0f;
	EXPECT_FALSE(hxconsole_exec_line("hxconsole_test_fn_mixed 1"));
	EXPECT_EQ(hxs_console_test_fn_mixed_i32, (int32_t)-1);

	// Too many args (trailing data after last parsed arg).
	hxs_console_test_fn_f32 = -1.0f;
	EXPECT_FALSE(hxconsole_exec_line("hxconsole_test_fn_f32 1.0 2.0"));
	EXPECT_EQ(hxs_console_test_fn_f32, -1.0f);

	// Too many args for two-arg function.
	hxs_console_test_fn_i32 = -1;
	EXPECT_FALSE(hxconsole_exec_line("hxconsole_test_fn_i32 1 2"));
	EXPECT_EQ(hxs_console_test_fn_i32, (int32_t)-1);

	// Wrong type / unparseable.
	hxs_console_test_fn_i32 = -1;
	EXPECT_FALSE(hxconsole_exec_line("hxconsole_test_fn_i32 notanumber"));
	EXPECT_EQ(hxs_console_test_fn_i32, (int32_t)-1);

	// Unknown command.
	EXPECT_FALSE(hxconsole_exec_line("nonexistent_fn"));

	// Zero-arg with extra data.
	hxs_console_test_fn_void_called = false;
	EXPECT_FALSE(hxconsole_exec_line("hxconsole_test_fn_void 1"));
	EXPECT_FALSE(hxs_console_test_fn_void_called);
}

// comment_lines: '#' comment character in variable and command lines

TEST(hxconsole_test, comment_lines) {
	// Variable set followed by a comment.
	hxs_console_test_i32 = 0;
	EXPECT_TRUE(hxconsole_exec_line("hxs_console_test_i32 42 # comment"));
	EXPECT_EQ(hxs_console_test_i32, (int32_t)42);

	// Zero-arg command followed by a comment.
	hxs_console_test_fn_void_called = false;
	EXPECT_TRUE(hxconsole_exec_line("hxconsole_test_fn_void # this is a comment"));
	EXPECT_TRUE(hxs_console_test_fn_void_called);
}

// unsigned_negative: negative inputs are rejected for unsigned integer types.

TEST(hxconsole_test, unsigned_negative) {
	hxlog_warning("EXPECTING_TEST_WARNINGS\n");

	hxs_console_test_u8 = 7;
	EXPECT_FALSE(hxconsole_exec_line("hxs_console_test_u8 -1"));
	EXPECT_EQ(hxs_console_test_u8, (uint8_t)7);

	hxs_console_test_u16 = 77;
	EXPECT_FALSE(hxconsole_exec_line("hxs_console_test_u16 -11"));
	EXPECT_EQ(hxs_console_test_u16, (uint16_t)77);

	hxs_console_test_u32 = 777;
	EXPECT_FALSE(hxconsole_exec_line("hxs_console_test_u32 -111"));
	EXPECT_EQ(hxs_console_test_u32, (uint32_t)777);

	hxs_console_test_u64 = 7777;
	EXPECT_FALSE(hxconsole_exec_line("hxs_console_test_u64 -1111"));
	EXPECT_EQ(hxs_console_test_u64, (uint64_t)7777);
}

// string_edge_cases: const char* argument with only whitespace or embedded '#'

TEST(hxconsole_test, string_edge_cases) {
	// all-whitespace string arg -> empty string (pointer to '\0').
	hxs_console_test_fn_str = hxnull;
	EXPECT_TRUE(hxconsole_exec_line("hxconsole_test_fn_str   "));
	EXPECT_TRUE(hxs_console_test_fn_str != hxnull && *hxs_console_test_fn_str == '\0');

	// embedded '#' is captured verbatim (not treated as comment).
	hxs_console_test_fn_str = hxnull;
	EXPECT_TRUE(hxconsole_exec_line("hxconsole_test_fn_str hello#world"));
	EXPECT_TRUE(hxs_console_test_fn_str != hxnull
		&& ::strncmp(hxs_console_test_fn_str, "hello#world", 11) == 0);
}


// null_test

#if defined __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-zero-length"
#endif

TEST(hxconsole_test, null_test) {
	const uint8_t prev = hxg_settings.log_level;
	hxg_settings.log_level = hxlog_level_warning;
	hxlog_handler(hxlog_level_console, "test_hidden\n");
	hxg_settings.log_level = prev;
	hxlog("");
	SUCCEED();
}

#if defined __GNUC__
#pragma GCC diagnostic pop
#endif

#if (HX_USE_CONSOLE) > 1
#if HX_USE_FILE_IO

TEST(hxconsole_test, file_test) {
	{
		hxfile f(hxfile::out, "hxconsole_test_file_test.txt");
		f << "hxconsole_test_file_var 3\n"
			"  # comment!\n"
			"\n"
			"hxconsole_test_file_var 78\n"
			"hxconsole_test_file_fn_name 89\n"
			"\n";
	}
	EXPECT_TRUE(hxconsole_exec_line("exec hxconsole_test_file_test.txt"));
	EXPECT_EQ(hxs_console_test_file_var1, 78.0f);
	EXPECT_EQ(hxs_console_test_file_var2, 89.0f);
}

TEST(hxconsole_test, file_fail) {
	hxlog_warning("EXPECTING_TEST_WARNINGS\n");

	{
		hxfile(hxfile::out, "hxconsole_test_file_test.txt") << "<unknown symbols>\n";
	}
	EXPECT_FALSE(hxconsole_exec_filename("hxconsole_test_file_test.txt"));

	{
		hxfile(hxfile::out, "hxconsole_test_file_test.txt") << "exec\n";
	}
	EXPECT_FALSE(hxconsole_exec_filename("hxconsole_test_file_test.txt"));

	{
		hxfile(hxfile::out, "hxconsole_test_file_test.txt") << "hxconsole_test_failing_command\n";
	}
	EXPECT_FALSE(hxconsole_exec_filename("hxconsole_test_file_test.txt"));

	// issue 5: mid-file failure stops execution. Line 3 must not run.
	hxs_console_test_file_var1 = 0.0f;
	{
		hxfile(hxfile::out, "hxconsole_test_file_test.txt")
			<< "hxconsole_test_file_var 5\n"
			   "<unknown command>\n"
			   "hxconsole_test_file_var 99\n";
	}
	EXPECT_FALSE(hxconsole_exec_filename("hxconsole_test_file_test.txt"));
	EXPECT_EQ(hxs_console_test_file_var1, 5.0f);

	// issue 6: missing file returns false.
	EXPECT_FALSE(hxconsole_exec_filename("__nonexistent_hxconsole_test__.txt"));
}

// file_peek_poke / file_peek_poke_floats
TEST(hxconsole_test, file_peek_poke) {
	uint32_t target[] = { 111, 777, 333 };
	{
		hxfile f(hxfile::out, "hxconsole_test_file_test.txt");
		f.print("peek 0x%zx 4\n", reinterpret_cast<size_t>(target));
		f.print("poke 0x%zx 4 0xde\n", reinterpret_cast<size_t>(target + 1));
		f.print("hexdump 0x%zx 12\n", reinterpret_cast<size_t>(target));
	}
	EXPECT_TRUE(hxconsole_exec_line("exec hxconsole_test_file_test.txt"));
	EXPECT_EQ(target[0], 111);
	EXPECT_EQ(target[1], 222);
	EXPECT_EQ(target[2], 333);
}

TEST(hxconsole_test, file_peek_poke_floats) {
	float target[] = { 111.0f, 777.0f, 333.0f };
	{
		hxfile f(hxfile::out, "hxconsole_test_file_test.txt");
		f.print("poke 0x%zx 4 0x435E0000\n", reinterpret_cast<size_t>(target + 1));
		f.print("floatdump 0x%zx 3\n", reinterpret_cast<size_t>(target));
	}
	EXPECT_TRUE(hxconsole_exec_line("exec hxconsole_test_file_test.txt"));
	EXPECT_EQ(target[0], 111.0f);
	EXPECT_EQ(target[1], 222.0f);
	EXPECT_EQ(target[2], 333.0f);
}
#endif
#endif // HX_USE_FILE_IO
#endif // (HX_USE_CONSOLE) > 1
