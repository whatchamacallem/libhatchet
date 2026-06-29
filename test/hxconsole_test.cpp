// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

#include <hx/hxconsole.hpp>
#include <hx/hxfile.hpp>
#include <hx/hxtest.hpp>

HX_NS_USE

#if HX_USE_CONSOLE
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

long long          hxs_console_test_fn_ll  = 0;
unsigned long long hxs_console_test_fn_ull = 0;
bool hxconsole_test_fn_ll(long long v_)           { hxs_console_test_fn_ll  = v_; return true; }
bool hxconsole_test_fn_ull(unsigned long long v_) { hxs_console_test_fn_ull = v_; return true; }

int32_t     hxs_console_test_fn_mixed_i32 = 0;
float       hxs_console_test_fn_mixed_f32 = 0.0f;
const char* hxs_console_test_fn_mixed_str = hxnull;
int32_t     hxs_console_test_fn_ints_i32  = 0;
uint32_t    hxs_console_test_fn_ints_u32  = 0;
bool        hxs_console_test_fn_void_called = false;
volatile float hxs_console_test_file_var1 = 0.0f;
volatile float hxs_console_test_file_var2 = 0.0f;

bool hxconsole_test_fn_mixed(int32_t a_, float b_, const char* c_) {
	hxs_console_test_fn_mixed_i32 = a_; hxs_console_test_fn_mixed_f32 = b_; hxs_console_test_fn_mixed_str = c_;
	return true;
}
bool hxconsole_test_fn_ints(int32_t a_, uint32_t b_) {
	hxs_console_test_fn_ints_i32 = a_; hxs_console_test_fn_ints_u32 = b_;
	return true;
}
bool hxconsole_test_fn_void(void) { hxs_console_test_fn_void_called = true; return true; }
bool hxconsole_test_file_fn(float f_) {
	hxs_console_test_file_var2 = f_;
	return true;
}
static bool hxconsole_test_failing_command(void) { return false; }

// GCOVR_EXCL_START
bool hxconsole_test_all_labels(float, double, char, bool, signed char,
		unsigned char, short, unsigned short, int, unsigned int, long,
		unsigned long, long long, unsigned long long, const char*) {
	return true;
}
// GCOVR_EXCL_STOP
} // namespace

hxconsole_command(hxconsole_test_register0);
hxconsole_command(hxconsole_test_register1);
hxconsole_command(hxconsole_test_all_labels);
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
hxconsole_command(hxconsole_test_fn_ll);
hxconsole_command(hxconsole_test_fn_ull);
hxconsole_command(hxconsole_test_fn_mixed);
hxconsole_command(hxconsole_test_fn_ints);
hxconsole_command(hxconsole_test_fn_void);
hxconsole_variable_named(hxs_console_test_file_var1, hxconsole_test_file_var);
hxconsole_command_named(hxconsole_test_file_fn, hxconsole_test_file_fn_name);
hxconsole_command_named(hxconsole_test_failing_command, hxconsole_test_failing_command);

TEST(hxconsole_test, register_command) {
	hxlog_warning("EXPECTING_TEST_WARNINGS");
	hxs_console_test_result_hook = 0.0f;
	EXPECT_TRUE(hxconsole_exec_line("hxconsole_test_register0 77 ..."));
	EXPECT_EQ(80.0f, hxs_console_test_result_hook);
	hxs_console_test_result_hook = 0.0f;
	EXPECT_TRUE(hxconsole_exec_line("hxconsole_test_register0 10.0 A"));
	EXPECT_EQ(11.0f, hxs_console_test_result_hook);
	hxs_console_test_result_hook = 0.0f;
	EXPECT_TRUE(hxconsole_exec_line("hxconsole_test_register1 12.5"));
	EXPECT_EQ(12.5f, hxs_console_test_result_hook);
	hxs_console_test_result_hook = -1.0f;
	EXPECT_FALSE(hxconsole_exec_line("hxconsole_test_register1 "));
	EXPECT_EQ(-1.0f, hxs_console_test_result_hook);
	hxs_console_test_fn_ints_i32 = -2;
	EXPECT_FALSE(hxconsole_exec_line("hxconsole_test_fn_ints 7 "));
	EXPECT_EQ(static_cast<int32_t>(-2), hxs_console_test_fn_ints_i32);
	hxs_console_test_fn_ints_i32 = -2;
	EXPECT_FALSE(hxconsole_exec_line("hxconsole_test_fn_ints 7 8 9 "));
	EXPECT_EQ(static_cast<int32_t>(-2), hxs_console_test_fn_ints_i32);
	EXPECT_FALSE(hxconsole_exec_line("Not_exist"));
	hxconsole_deregister("hxconsole_test_register0");
	EXPECT_FALSE(hxconsole_exec_line("hxconsole_test_register0 77 ..."));
}

TEST(hxconsole_test, variable_set) {
	hxs_console_test_i8 = 0;
	EXPECT_TRUE(hxconsole_exec_line("hxs_console_test_i8 123"));
	EXPECT_EQ(hxs_console_test_i8, static_cast<int8_t>(123));
	hxs_console_test_i16 = 0;
	EXPECT_TRUE(hxconsole_exec_line("hxs_console_test_i16 234"));
	EXPECT_EQ(hxs_console_test_i16, static_cast<int16_t>(234));
	hxs_console_test_i32 = 0;
	EXPECT_TRUE(hxconsole_exec_line("hxs_console_test_i32 345"));
	EXPECT_EQ(hxs_console_test_i32, static_cast<int32_t>(345));
	hxs_console_test_i64 = 0;
	EXPECT_TRUE(hxconsole_exec_line("hxs_console_test_i64 567"));
	EXPECT_EQ(hxs_console_test_i64, static_cast<int64_t>(567));
	hxs_console_test_u8 = 0;
	EXPECT_TRUE(hxconsole_exec_line("hxs_console_test_u8 12"));
	EXPECT_EQ(hxs_console_test_u8, static_cast<uint8_t>(12));
	hxs_console_test_u16 = 0;
	EXPECT_TRUE(hxconsole_exec_line("hxs_console_test_u16 2345"));
	EXPECT_EQ(hxs_console_test_u16, static_cast<uint16_t>(2345));
	hxs_console_test_u32 = 0;
	EXPECT_TRUE(hxconsole_exec_line("hxs_console_test_u32 3456"));
	EXPECT_EQ(hxs_console_test_u32, static_cast<uint32_t>(3456));
	hxs_console_test_u64 = 0;
	EXPECT_TRUE(hxconsole_exec_line("hxs_console_test_u64 5678"));
	EXPECT_EQ(hxs_console_test_u64, static_cast<uint64_t>(5678));
	hxs_console_test_f32 = 0.0f;
	EXPECT_TRUE(hxconsole_exec_line("hxs_console_test_f32 678.0"));
	EXPECT_EQ(hxs_console_test_f32, 678.0f);
	hxs_console_test_f64 = 0.0;
	EXPECT_TRUE(hxconsole_exec_line("hxs_console_test_f64 789.0"));
	EXPECT_EQ(hxs_console_test_f64, 789.0);
	hxs_console_test_bool = true;
	EXPECT_TRUE(hxconsole_exec_line("hxs_console_test_bool 0"));
	EXPECT_EQ(hxs_console_test_bool, false);
	EXPECT_TRUE(hxconsole_exec_line("hxs_console_test_bool 1"));
	EXPECT_EQ(hxs_console_test_bool, true);
	hxs_console_test_size = 0u;
	EXPECT_TRUE(hxconsole_exec_line("hxs_console_test_size 1000"));
	EXPECT_EQ(hxs_console_test_size, static_cast<size_t>(1000));
	hxs_console_test_char = 0;
	EXPECT_TRUE(hxconsole_exec_line("hxs_console_test_char 65"));
	EXPECT_EQ(hxs_console_test_char, static_cast<char>(65));
	hxs_console_test_i32 = -1;
	EXPECT_FALSE(hxconsole_exec_line("hxs_console_test_i32 3.5"));
	EXPECT_EQ(hxs_console_test_i32, static_cast<int32_t>(-1));
	hxs_console_test_i32 = 0;
	EXPECT_TRUE(hxconsole_exec_line("hxs_console_test_i32 0xFF"));
	EXPECT_EQ(hxs_console_test_i32, static_cast<int32_t>(255));
}

TEST(hxconsole_test, variable_query) {
	hxs_console_test_i32 = 34;
	EXPECT_TRUE(hxconsole_exec_line("hxs_console_test_i32"));
	EXPECT_EQ(hxs_console_test_i32, static_cast<int32_t>(34));
	hxs_console_test_f32 = -1.5f;
	EXPECT_TRUE(hxconsole_exec_line("hxs_console_test_f32"));
	EXPECT_EQ(hxs_console_test_f32, -1.5f);
	hxs_console_test_bool = true;
	EXPECT_TRUE(hxconsole_exec_line("hxs_console_test_bool"));
	EXPECT_EQ(hxs_console_test_bool, true);
}

TEST(hxconsole_test, variable_overflow) {
	hxlog_warning("EXPECTING_TEST_WARNINGS");
	hxs_console_test_i8 = 0;
	EXPECT_TRUE(hxconsole_exec_line("hxs_console_test_i8 127"));
	EXPECT_EQ(hxs_console_test_i8, static_cast<int8_t>(127));
	EXPECT_TRUE(hxconsole_exec_line("hxs_console_test_i8 -128"));
	EXPECT_EQ(hxs_console_test_i8, static_cast<int8_t>(-128));
	hxs_console_test_i8 = 99;
	EXPECT_FALSE(hxconsole_exec_line("hxs_console_test_i8 128"));
	EXPECT_EQ(hxs_console_test_i8, static_cast<int8_t>(99));
	EXPECT_FALSE(hxconsole_exec_line("hxs_console_test_i8 -129"));
	EXPECT_EQ(hxs_console_test_i8, static_cast<int8_t>(99));
	hxs_console_test_i8 = 55;
	EXPECT_FALSE(hxconsole_exec_line("hxs_console_test_i8 200"));
	EXPECT_EQ(hxs_console_test_i8, static_cast<int8_t>(55));
	EXPECT_FALSE(hxconsole_exec_line("hxs_console_test_i8 -200"));
	EXPECT_EQ(hxs_console_test_i8, static_cast<int8_t>(55));
	hxs_console_test_i16 = 0;
	EXPECT_TRUE(hxconsole_exec_line("hxs_console_test_i16 32767"));
	EXPECT_EQ(hxs_console_test_i16, static_cast<int16_t>(32767));
	EXPECT_TRUE(hxconsole_exec_line("hxs_console_test_i16 -32768"));
	EXPECT_EQ(hxs_console_test_i16, static_cast<int16_t>(-32768));
	hxs_console_test_i16 = 1;
	EXPECT_FALSE(hxconsole_exec_line("hxs_console_test_i16 40000"));
	EXPECT_EQ(hxs_console_test_i16, static_cast<int16_t>(1));
	EXPECT_FALSE(hxconsole_exec_line("hxs_console_test_i16 -40000"));
	EXPECT_EQ(hxs_console_test_i16, static_cast<int16_t>(1));
	hxs_console_test_i32 = 0;
	EXPECT_TRUE(hxconsole_exec_line("hxs_console_test_i32 2147483647"));
	EXPECT_EQ(hxs_console_test_i32, static_cast<int32_t>(2147483647));
	EXPECT_TRUE(hxconsole_exec_line("hxs_console_test_i32 -2147483648"));
	EXPECT_EQ(hxs_console_test_i32, static_cast<int32_t>(-2147483648));
	hxs_console_test_i32 = 2;
	EXPECT_FALSE(hxconsole_exec_line("hxs_console_test_i32 3000000000"));
	EXPECT_EQ(hxs_console_test_i32, static_cast<int32_t>(2));
	EXPECT_FALSE(hxconsole_exec_line("hxs_console_test_i32 -3000000000"));
	EXPECT_EQ(hxs_console_test_i32, static_cast<int32_t>(2));
	hxs_console_test_u8 = 1;
	EXPECT_TRUE(hxconsole_exec_line("hxs_console_test_u8 255"));
	EXPECT_EQ(hxs_console_test_u8, static_cast<uint8_t>(255));
	EXPECT_TRUE(hxconsole_exec_line("hxs_console_test_u8 0"));
	EXPECT_EQ(hxs_console_test_u8, static_cast<uint8_t>(0));
	hxs_console_test_u8 = 7;
	EXPECT_FALSE(hxconsole_exec_line("hxs_console_test_u8 256"));
	EXPECT_EQ(hxs_console_test_u8, static_cast<uint8_t>(7));
	EXPECT_FALSE(hxconsole_exec_line("hxs_console_test_u8 300"));
	EXPECT_EQ(hxs_console_test_u8, static_cast<uint8_t>(7));
	hxs_console_test_u16 = 1;
	EXPECT_TRUE(hxconsole_exec_line("hxs_console_test_u16 65535"));
	EXPECT_EQ(hxs_console_test_u16, static_cast<uint16_t>(65535));
	EXPECT_TRUE(hxconsole_exec_line("hxs_console_test_u16 0"));
	EXPECT_EQ(hxs_console_test_u16, static_cast<uint16_t>(0));
	hxs_console_test_u16 = 3;
	EXPECT_FALSE(hxconsole_exec_line("hxs_console_test_u16 70000"));
	EXPECT_EQ(hxs_console_test_u16, static_cast<uint16_t>(3));
	hxs_console_test_u32 = 1;
	EXPECT_TRUE(hxconsole_exec_line("hxs_console_test_u32 4294967295"));
	EXPECT_EQ(hxs_console_test_u32, static_cast<uint32_t>(4294967295u));
	EXPECT_TRUE(hxconsole_exec_line("hxs_console_test_u32 0"));
	EXPECT_EQ(hxs_console_test_u32, static_cast<uint32_t>(0));
	hxs_console_test_u32 = 4;
	EXPECT_FALSE(hxconsole_exec_line("hxs_console_test_u32 5000000000"));
	EXPECT_EQ(hxs_console_test_u32, static_cast<uint32_t>(4));
}

TEST(hxconsole_test, float_overflow) {
	hxlog_warning("EXPECTING_TEST_WARNINGS");
	hxs_console_test_f32 = 1.0f;
	EXPECT_FALSE(hxconsole_exec_line("hxs_console_test_f32 1e40"));
	EXPECT_EQ(hxs_console_test_f32, 1.0f);
	hxs_console_test_f64 = 2.0;
	EXPECT_FALSE(hxconsole_exec_line("hxs_console_test_f64 1e400"));
	EXPECT_EQ(hxs_console_test_f64, 2.0);
	hxs_console_test_fn_f32 = 3.0f;
	EXPECT_FALSE(hxconsole_exec_line("hxconsole_test_fn_f32 1e40"));
	EXPECT_EQ(hxs_console_test_fn_f32, 3.0f);
	hxs_console_test_fn_f64 = 4.0;
	EXPECT_FALSE(hxconsole_exec_line("hxconsole_test_fn_f64 1e400"));
	EXPECT_EQ(hxs_console_test_fn_f64, 4.0);
}

TEST(hxconsole_test, hex_overflow) {
	hxlog_warning("EXPECTING_TEST_WARNINGS");
	hxs_console_test_u8 = 9;
	EXPECT_FALSE(hxconsole_exec_line("hxs_console_test_u8 0x100"));
	EXPECT_EQ(hxs_console_test_u8, static_cast<uint8_t>(9));
	hxs_console_test_u16 = 9;
	EXPECT_FALSE(hxconsole_exec_line("hxs_console_test_u16 0x10000"));
	EXPECT_EQ(hxs_console_test_u16, static_cast<uint16_t>(9));
	hxs_console_test_u32 = 9;
	EXPECT_FALSE(hxconsole_exec_line("hxs_console_test_u32 0x1FFFFFFFF"));
	EXPECT_EQ(hxs_console_test_u32, static_cast<uint32_t>(9));
	hxs_console_test_i8 = 9;
	EXPECT_FALSE(hxconsole_exec_line("hxs_console_test_i8 0x80"));
	EXPECT_EQ(hxs_console_test_i8, static_cast<int8_t>(9));
}

TEST(hxconsole_test, whitespace_dispatch) {
	hxs_console_test_i32 = 0;
	EXPECT_TRUE(hxconsole_exec_line("   hxs_console_test_i32 17"));
	EXPECT_EQ(hxs_console_test_i32, static_cast<int32_t>(17));
	hxs_console_test_i32 = 0;
	EXPECT_TRUE(hxconsole_exec_line("\thxs_console_test_i32\t18"));
	EXPECT_EQ(hxs_console_test_i32, static_cast<int32_t>(18));
	hxs_console_test_fn_ints_i32 = 0; hxs_console_test_fn_ints_u32 = 0;
	EXPECT_TRUE(hxconsole_exec_line("hxconsole_test_fn_ints    -5     100"));
	EXPECT_EQ(hxs_console_test_fn_ints_i32, static_cast<int32_t>(-5));
	EXPECT_EQ(hxs_console_test_fn_ints_u32, static_cast<uint32_t>(100));
}

TEST(hxconsole_test, blank_line) {
	EXPECT_TRUE(hxconsole_exec_line(""));
	EXPECT_TRUE(hxconsole_exec_line("   "));
	EXPECT_TRUE(hxconsole_exec_line("\t \t"));
	EXPECT_TRUE(hxconsole_exec_line("# just a comment"));
	EXPECT_TRUE(hxconsole_exec_line("   # indented comment"));
}

TEST(hxconsole_test, signed_plus_prefix) {
	hxs_console_test_i32 = 0;
	EXPECT_TRUE(hxconsole_exec_line("hxs_console_test_i32 +5"));
	EXPECT_EQ(hxs_console_test_i32, static_cast<int32_t>(5));
	hxs_console_test_u32 = 0;
	EXPECT_TRUE(hxconsole_exec_line("hxs_console_test_u32 +6"));
	EXPECT_EQ(hxs_console_test_u32, static_cast<uint32_t>(6));
}

TEST(hxconsole_test, variable_parse_error) {
	hxlog_warning("EXPECTING_TEST_WARNINGS");
	hxs_console_test_i32 = 10;
	EXPECT_FALSE(hxconsole_exec_line("hxs_console_test_i32 34 garbage"));
	EXPECT_EQ(hxs_console_test_i32, static_cast<int32_t>(10));
	hxs_console_test_i32 = 10;
	EXPECT_FALSE(hxconsole_exec_line("hxs_console_test_i32 1 2"));
	EXPECT_EQ(hxs_console_test_i32, static_cast<int32_t>(10));
	hxs_console_test_i32 = 10;
	EXPECT_FALSE(hxconsole_exec_line("hxs_console_test_i32 abc"));
	EXPECT_EQ(hxs_console_test_i32, static_cast<int32_t>(10));
	hxs_console_test_bool = true;
	EXPECT_FALSE(hxconsole_exec_line("hxs_console_test_bool 2"));
	EXPECT_EQ(hxs_console_test_bool, true);
	EXPECT_FALSE(hxconsole_exec_line("hxs_console_test_bool notanumber"));
	EXPECT_EQ(hxs_console_test_bool, true);
}

TEST(hxconsole_test, function_types) {
	hxs_console_test_fn_i8 = 0;
	EXPECT_TRUE(hxconsole_exec_line("hxconsole_test_fn_i8 34"));
	EXPECT_EQ(hxs_console_test_fn_i8, static_cast<int8_t>(34));
	EXPECT_TRUE(hxconsole_exec_line("hxconsole_test_fn_i8 -1"));
	EXPECT_EQ(hxs_console_test_fn_i8, static_cast<int8_t>(-1));
	EXPECT_TRUE(hxconsole_exec_line("hxconsole_test_fn_i8 0x1F"));
	EXPECT_EQ(hxs_console_test_fn_i8, static_cast<int8_t>(0x1F));
	hxs_console_test_fn_i16 = 0;
	EXPECT_TRUE(hxconsole_exec_line("hxconsole_test_fn_i16 1000"));
	EXPECT_EQ(hxs_console_test_fn_i16, static_cast<int16_t>(1000));
	EXPECT_TRUE(hxconsole_exec_line("hxconsole_test_fn_i16 -1000"));
	EXPECT_EQ(hxs_console_test_fn_i16, static_cast<int16_t>(-1000));
	hxs_console_test_fn_i32 = 0;
	EXPECT_TRUE(hxconsole_exec_line("hxconsole_test_fn_i32 100000"));
	EXPECT_EQ(hxs_console_test_fn_i32, static_cast<int32_t>(100000));
	EXPECT_TRUE(hxconsole_exec_line("hxconsole_test_fn_i32 -100000"));
	EXPECT_EQ(hxs_console_test_fn_i32, static_cast<int32_t>(-100000));
	EXPECT_TRUE(hxconsole_exec_line("hxconsole_test_fn_i32 0xFF"));
	EXPECT_EQ(hxs_console_test_fn_i32, static_cast<int32_t>(0xFF));
	hxs_console_test_fn_i64 = 0;
	EXPECT_TRUE(hxconsole_exec_line("hxconsole_test_fn_i64 100000"));
	EXPECT_EQ(hxs_console_test_fn_i64, static_cast<int64_t>(100000));
	hxs_console_test_fn_u8 = 0;
	EXPECT_TRUE(hxconsole_exec_line("hxconsole_test_fn_u8 200"));
	EXPECT_EQ(hxs_console_test_fn_u8, static_cast<uint8_t>(200));
	hxs_console_test_fn_u16 = 0;
	EXPECT_TRUE(hxconsole_exec_line("hxconsole_test_fn_u16 50000"));
	EXPECT_EQ(hxs_console_test_fn_u16, static_cast<uint16_t>(50000));
	hxs_console_test_fn_u32 = 0;
	EXPECT_TRUE(hxconsole_exec_line("hxconsole_test_fn_u32 3000000000"));
	EXPECT_EQ(hxs_console_test_fn_u32, static_cast<uint32_t>(3000000000u));
	EXPECT_TRUE(hxconsole_exec_line("hxconsole_test_fn_u32 0xDEAD"));
	EXPECT_EQ(hxs_console_test_fn_u32, static_cast<uint32_t>(0xDEAD));
	hxs_console_test_fn_u64 = 0;
	EXPECT_TRUE(hxconsole_exec_line("hxconsole_test_fn_u64 18446744073709551615"));
	EXPECT_EQ(hxs_console_test_fn_u64, static_cast<uint64_t>(18446744073709551615ull));
	hxs_console_test_fn_char = 0;
	EXPECT_TRUE(hxconsole_exec_line("hxconsole_test_fn_char 65"));
	EXPECT_EQ(hxs_console_test_fn_char, static_cast<char>(65));
	EXPECT_TRUE(hxconsole_exec_line("hxconsole_test_fn_char 127"));
	EXPECT_EQ(hxs_console_test_fn_char, static_cast<char>(127));
	hxs_console_test_fn_f32 = 0.0f;
	EXPECT_TRUE(hxconsole_exec_line("hxconsole_test_fn_f32 3.14"));
	EXPECT_TRUE(hxs_console_test_fn_f32 > 3.13f && hxs_console_test_fn_f32 < 3.15f);
	EXPECT_TRUE(hxconsole_exec_line("hxconsole_test_fn_f32 -1.5"));
	EXPECT_EQ(hxs_console_test_fn_f32, -1.5f);
	hxs_console_test_fn_f64 = 0.0;
	EXPECT_TRUE(hxconsole_exec_line("hxconsole_test_fn_f64 3.14159265358979"));
	EXPECT_TRUE(hxs_console_test_fn_f64 > 3.14159 && hxs_console_test_fn_f64 < 3.14160);
	hxs_console_test_fn_bool = false;
	EXPECT_TRUE(hxconsole_exec_line("hxconsole_test_fn_bool 1"));
	EXPECT_EQ(hxs_console_test_fn_bool, true);
	EXPECT_TRUE(hxconsole_exec_line("hxconsole_test_fn_bool 0"));
	EXPECT_EQ(hxs_console_test_fn_bool, false);
	hxs_console_test_fn_str = hxnull;
	EXPECT_TRUE(hxconsole_exec_line("hxconsole_test_fn_str hello world"));
	EXPECT_TRUE(hxs_console_test_fn_str != hxnull && ::strncmp(hxs_console_test_fn_str, "hello", 5) == 0);
}

TEST(hxconsole_test, function_overflow) {
	hxlog_warning("EXPECTING_TEST_WARNINGS");
	hxs_console_test_fn_i8 = 0;
	EXPECT_TRUE(hxconsole_exec_line("hxconsole_test_fn_i8 127"));
	EXPECT_EQ(hxs_console_test_fn_i8, static_cast<int8_t>(127));
	EXPECT_TRUE(hxconsole_exec_line("hxconsole_test_fn_i8 -128"));
	EXPECT_EQ(hxs_console_test_fn_i8, static_cast<int8_t>(-128));
	hxs_console_test_fn_i8 = 0;
	EXPECT_FALSE(hxconsole_exec_line("hxconsole_test_fn_i8 128"));
	EXPECT_EQ(hxs_console_test_fn_i8, static_cast<int8_t>(0));
	EXPECT_FALSE(hxconsole_exec_line("hxconsole_test_fn_i8 -129"));
	EXPECT_EQ(hxs_console_test_fn_i8, static_cast<int8_t>(0));
	hxs_console_test_fn_i16 = 0;
	EXPECT_TRUE(hxconsole_exec_line("hxconsole_test_fn_i16 32767"));
	EXPECT_EQ(hxs_console_test_fn_i16, static_cast<int16_t>(32767));
	EXPECT_TRUE(hxconsole_exec_line("hxconsole_test_fn_i16 -32768"));
	EXPECT_EQ(hxs_console_test_fn_i16, static_cast<int16_t>(-32768));
	hxs_console_test_fn_i16 = 0;
	EXPECT_FALSE(hxconsole_exec_line("hxconsole_test_fn_i16 32768"));
	EXPECT_EQ(hxs_console_test_fn_i16, static_cast<int16_t>(0));
	hxs_console_test_fn_u8 = 0;
	EXPECT_TRUE(hxconsole_exec_line("hxconsole_test_fn_u8 255"));
	EXPECT_EQ(hxs_console_test_fn_u8, static_cast<uint8_t>(255));
	EXPECT_TRUE(hxconsole_exec_line("hxconsole_test_fn_u8 0"));
	EXPECT_EQ(hxs_console_test_fn_u8, static_cast<uint8_t>(0));
	hxs_console_test_fn_u8 = 0;
	EXPECT_FALSE(hxconsole_exec_line("hxconsole_test_fn_u8 256"));
	EXPECT_EQ(hxs_console_test_fn_u8, static_cast<uint8_t>(0));
	hxs_console_test_fn_u16 = 0;
	EXPECT_TRUE(hxconsole_exec_line("hxconsole_test_fn_u16 65535"));
	EXPECT_EQ(hxs_console_test_fn_u16, static_cast<uint16_t>(65535));
	hxs_console_test_fn_u16 = 0;
	EXPECT_FALSE(hxconsole_exec_line("hxconsole_test_fn_u16 65536"));
	EXPECT_EQ(hxs_console_test_fn_u16, static_cast<uint16_t>(0));
	hxs_console_test_fn_i32 = 0;
	EXPECT_FALSE(hxconsole_exec_line("hxconsole_test_fn_i32 2147483648"));
	EXPECT_EQ(hxs_console_test_fn_i32, static_cast<int32_t>(0));
	EXPECT_FALSE(hxconsole_exec_line("hxconsole_test_fn_i32 -2147483649"));
	EXPECT_EQ(hxs_console_test_fn_i32, static_cast<int32_t>(0));
	hxs_console_test_fn_u32 = 0;
	EXPECT_FALSE(hxconsole_exec_line("hxconsole_test_fn_u32 4294967296"));
	EXPECT_EQ(hxs_console_test_fn_u32, static_cast<uint32_t>(0));
	hxs_console_test_fn_i64 = 0;
	EXPECT_TRUE(hxconsole_exec_line("hxconsole_test_fn_i64 9223372036854775807"));
	EXPECT_EQ(hxs_console_test_fn_i64, static_cast<int64_t>(9223372036854775807ll));
	hxs_console_test_fn_i64 = 0;
	EXPECT_FALSE(hxconsole_exec_line("hxconsole_test_fn_i64 9223372036854775808"));
	EXPECT_EQ(hxs_console_test_fn_i64, static_cast<int64_t>(0));
	hxs_console_test_fn_u64 = 0;
	EXPECT_TRUE(hxconsole_exec_line("hxconsole_test_fn_u64 18446744073709551615"));
	EXPECT_EQ(hxs_console_test_fn_u64, static_cast<uint64_t>(18446744073709551615ull));
	hxs_console_test_fn_u64 = 0;
	EXPECT_FALSE(hxconsole_exec_line("hxconsole_test_fn_u64 18446744073709551616"));
	EXPECT_EQ(hxs_console_test_fn_u64, static_cast<uint64_t>(0));
	hxs_console_test_fn_char = 0;
	EXPECT_TRUE(hxconsole_exec_line("hxconsole_test_fn_char 127"));
	EXPECT_EQ(hxs_console_test_fn_char, static_cast<char>(127));
	hxs_console_test_fn_char = 0;
	EXPECT_FALSE(hxconsole_exec_line("hxconsole_test_fn_char 128"));
	EXPECT_EQ(hxs_console_test_fn_char, static_cast<char>(0));
#if CHAR_MIN < 0
	hxs_console_test_fn_char = 0;
	EXPECT_TRUE(hxconsole_exec_line("hxconsole_test_fn_char -128"));
	EXPECT_EQ(hxs_console_test_fn_char, static_cast<char>(-128));
	hxs_console_test_fn_char = 0;
	EXPECT_FALSE(hxconsole_exec_line("hxconsole_test_fn_char -129"));
	EXPECT_EQ(hxs_console_test_fn_char, static_cast<char>(0));
#endif
}

TEST(hxconsole_test, function_multi_arg) {
	hxs_console_test_fn_mixed_i32 = 0; hxs_console_test_fn_mixed_f32 = 0.0f; hxs_console_test_fn_mixed_str = hxnull;
	EXPECT_TRUE(hxconsole_exec_line("hxconsole_test_fn_mixed 7 2.5 hello"));
	EXPECT_EQ(hxs_console_test_fn_mixed_i32, static_cast<int32_t>(7));
	EXPECT_EQ(hxs_console_test_fn_mixed_f32, 2.5f);
	EXPECT_TRUE(hxs_console_test_fn_mixed_str != hxnull && ::strncmp(hxs_console_test_fn_mixed_str, "hello", 5) == 0);
	hxs_console_test_fn_ints_i32 = 0; hxs_console_test_fn_ints_u32 = 0;
	EXPECT_TRUE(hxconsole_exec_line("hxconsole_test_fn_ints -5 100"));
	EXPECT_EQ(hxs_console_test_fn_ints_i32, static_cast<int32_t>(-5));
	EXPECT_EQ(hxs_console_test_fn_ints_u32, static_cast<uint32_t>(100));
	hxs_console_test_fn_void_called = false;
	EXPECT_TRUE(hxconsole_exec_line("hxconsole_test_fn_void"));
	EXPECT_TRUE(hxs_console_test_fn_void_called);
}

TEST(hxconsole_test, function_arity) {
	hxlog_warning("EXPECTING_TEST_WARNINGS");
	hxs_console_test_fn_mixed_i32 = -1; hxs_console_test_fn_mixed_f32 = -1.0f;
	EXPECT_FALSE(hxconsole_exec_line("hxconsole_test_fn_mixed 1"));
	EXPECT_EQ(hxs_console_test_fn_mixed_i32, static_cast<int32_t>(-1));
	hxs_console_test_fn_f32 = -1.0f;
	EXPECT_FALSE(hxconsole_exec_line("hxconsole_test_fn_f32 1.0 2.0"));
	EXPECT_EQ(hxs_console_test_fn_f32, -1.0f);
	hxs_console_test_fn_i32 = -1;
	EXPECT_FALSE(hxconsole_exec_line("hxconsole_test_fn_i32 1 2"));
	EXPECT_EQ(hxs_console_test_fn_i32, static_cast<int32_t>(-1));
	hxs_console_test_fn_i32 = -1;
	EXPECT_FALSE(hxconsole_exec_line("hxconsole_test_fn_i32 notanumber"));
	EXPECT_EQ(hxs_console_test_fn_i32, static_cast<int32_t>(-1));
	EXPECT_FALSE(hxconsole_exec_line("nonexistent_fn"));
	hxs_console_test_fn_void_called = false;
	EXPECT_FALSE(hxconsole_exec_line("hxconsole_test_fn_void 1"));
	EXPECT_FALSE(hxs_console_test_fn_void_called);
}

TEST(hxconsole_test, comment_lines) {
	hxs_console_test_i32 = 0;
	EXPECT_TRUE(hxconsole_exec_line("hxs_console_test_i32 34 # comment"));
	EXPECT_EQ(hxs_console_test_i32, static_cast<int32_t>(34));
	hxs_console_test_fn_void_called = false;
	EXPECT_TRUE(hxconsole_exec_line("hxconsole_test_fn_void # this is a comment"));
	EXPECT_TRUE(hxs_console_test_fn_void_called);
}

TEST(hxconsole_test, unsigned_negative) {
	hxlog_warning("EXPECTING_TEST_WARNINGS");
	hxs_console_test_u8 = 7;
	EXPECT_FALSE(hxconsole_exec_line("hxs_console_test_u8 -1"));
	EXPECT_EQ(hxs_console_test_u8, static_cast<uint8_t>(7));
	hxs_console_test_u16 = 77;
	EXPECT_FALSE(hxconsole_exec_line("hxs_console_test_u16 -11"));
	EXPECT_EQ(hxs_console_test_u16, static_cast<uint16_t>(77));
	hxs_console_test_u32 = 777;
	EXPECT_FALSE(hxconsole_exec_line("hxs_console_test_u32 -111"));
	EXPECT_EQ(hxs_console_test_u32, static_cast<uint32_t>(777));
	hxs_console_test_u64 = 7777;
	EXPECT_FALSE(hxconsole_exec_line("hxs_console_test_u64 -1111"));
	EXPECT_EQ(hxs_console_test_u64, static_cast<uint64_t>(7777));
}

TEST(hxconsole_test, string_edge_cases) {
	hxs_console_test_fn_str = hxnull;
	EXPECT_TRUE(hxconsole_exec_line("hxconsole_test_fn_str   "));
	EXPECT_TRUE(hxs_console_test_fn_str != hxnull && *hxs_console_test_fn_str == '\0');
	hxs_console_test_fn_str = hxnull;
	EXPECT_TRUE(hxconsole_exec_line("hxconsole_test_fn_str hello#world"));
	EXPECT_TRUE(hxs_console_test_fn_str != hxnull
		&& ::strncmp(hxs_console_test_fn_str, "hello#world", 11) == 0);
}

TEST(hxconsole_test, variable_size_t_zero_boundary) {
	hxs_console_test_size = 99u;
	EXPECT_TRUE(hxconsole_exec_line("hxs_console_test_size 0"));
	EXPECT_EQ(hxs_console_test_size, static_cast<size_t>(0));
	hxs_console_test_size = 0u;
	EXPECT_TRUE(hxconsole_exec_line("hxs_console_test_size 1"));
	EXPECT_EQ(hxs_console_test_size, static_cast<size_t>(1));
}

TEST(hxconsole_test, function_overflow_signed_min_unsigned_zero_boundary) {
	hxs_console_test_fn_i8 = 50;
	EXPECT_TRUE(hxconsole_exec_line("hxconsole_test_fn_i8 -128"));
	EXPECT_EQ(hxs_console_test_fn_i8, static_cast<int8_t>(-128));
	hxs_console_test_fn_u8 = 99;
	EXPECT_TRUE(hxconsole_exec_line("hxconsole_test_fn_u8 0"));
	EXPECT_EQ(hxs_console_test_fn_u8, static_cast<uint8_t>(0));
}

TEST(hxconsole_test, long_long_parsing) {
	hxs_console_test_fn_ll = 0;
	EXPECT_TRUE(hxconsole_exec_line("hxconsole_test_fn_ll -123"));
	EXPECT_EQ(hxs_console_test_fn_ll, -123ll);
	hxs_console_test_fn_ll = 0;
	EXPECT_TRUE(hxconsole_exec_line("hxconsole_test_fn_ll 456"));
	EXPECT_EQ(hxs_console_test_fn_ll, 456ll);
}

TEST(hxconsole_test, unsigned_long_long_parsing) {
	hxs_console_test_fn_ull = 0;
	EXPECT_TRUE(hxconsole_exec_line("hxconsole_test_fn_ull 789"));
	EXPECT_EQ(hxs_console_test_fn_ull, 789ull);
}

TEST(hxconsole_test, unsigned_long_long_rejects_negative) {
	hxlog_warning("EXPECTING_TEST_WARNINGS");
	hxs_console_test_fn_ull = 55;
	EXPECT_FALSE(hxconsole_exec_line("hxconsole_test_fn_ull -1"));
	EXPECT_EQ(hxs_console_test_fn_ull, 55ull);
}

TEST(hxconsole_test, help_logs_commands_and_variables) {
	EXPECT_TRUE(hxconsole_help());
}

TEST(hxconsole_test, usage_messages) {
	hxlog_warning("EXPECTING_TEST_WARNINGS");
	EXPECT_FALSE(hxconsole_exec_line("hxconsole_test_all_labels 1"));
	EXPECT_FALSE(hxconsole_exec_line("hxs_console_test_i32 not_a_number"));
}

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
		hxfile f(hxfile::open_mode_out, "hxconsole_test_file_test.txt");
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
	hxlog_warning("EXPECTING_TEST_WARNINGS");
	{
		hxfile(hxfile::open_mode_out, "hxconsole_test_file_test.txt") << "<unknown symbols>\n";
	}
	EXPECT_FALSE(hxconsole_exec_filename("hxconsole_test_file_test.txt"));
	{
		hxfile(hxfile::open_mode_out, "hxconsole_test_file_test.txt") << "exec\n";
	}
	EXPECT_FALSE(hxconsole_exec_filename("hxconsole_test_file_test.txt"));
	{
		hxfile(hxfile::open_mode_out, "hxconsole_test_file_test.txt") << "hxconsole_test_failing_command\n";
	}
	EXPECT_FALSE(hxconsole_exec_filename("hxconsole_test_file_test.txt"));
	hxs_console_test_file_var1 = 0.0f;
	{
		hxfile(hxfile::open_mode_out, "hxconsole_test_file_test.txt")
			<< "hxconsole_test_file_var 5\n"
			   "<unknown command>\n"
			   "hxconsole_test_file_var 99\n";
	}
	EXPECT_FALSE(hxconsole_exec_filename("hxconsole_test_file_test.txt"));
	EXPECT_EQ(hxs_console_test_file_var1, 5.0f);
	EXPECT_FALSE(hxconsole_exec_filename("__nonexistent_hxconsole_test__.txt"));
}

TEST(hxconsole_test, file_peek_poke) {
	uint32_t target[] = { 111, 777, 333 };
	{
		hxfile f(hxfile::open_mode_out, "hxconsole_test_file_test.txt");
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
		hxfile f(hxfile::open_mode_out, "hxconsole_test_file_test.txt");
		f.print("poke 0x%zx 4 0x435E0000\n", reinterpret_cast<size_t>(target + 1));
		f.print("floatdump 0x%zx 3\n", reinterpret_cast<size_t>(target));
	}
	EXPECT_TRUE(hxconsole_exec_line("exec hxconsole_test_file_test.txt"));
	EXPECT_EQ(target[0], 111.0f);
	EXPECT_EQ(target[1], 222.0f);
	EXPECT_EQ(target[2], 333.0f);
}

TEST(hxconsole_test, file_final_line_without_newline) {
	hxs_console_test_file_var1 = 0.0f;
	{
		hxfile f(hxfile::open_mode_out, "hxconsole_test_file_test.txt");
		f << "hxconsole_test_file_var 41\n"
			 "hxconsole_test_file_var 42";
	}
	EXPECT_TRUE(hxconsole_exec_filename("hxconsole_test_file_test.txt"));
	EXPECT_EQ(hxs_console_test_file_var1, 42.0f);
}

TEST(hxconsole_test, file_partial_line_carried_across_reads) {
	hxs_console_test_file_var1 = 0.0f;
	{
		hxfile f(hxfile::open_mode_out, "hxconsole_test_file_test.txt");
		for(hxsize_t i = 0; i < 200; ++i) {
			f << "hxconsole_test_file_var 7\n";
		}
		f << "hxconsole_test_file_var 55\n";
	}
	EXPECT_TRUE(hxconsole_exec_filename("hxconsole_test_file_test.txt"));
	EXPECT_EQ(hxs_console_test_file_var1, 55.0f);
}

TEST(hxconsole_test, file_overlong_line_flushed) {
	hxs_console_test_file_var1 = 0.0f;
	{
		hxfile f(hxfile::open_mode_out, "hxconsole_test_file_test.txt");
		f << "hxconsole_test_file_var 12\n";
		for(hxsize_t i = 0; i < (HX_MAX_LINE * 2); ++i) {
			f << " ";
		}
		f << "\nhxconsole_test_file_var 63\n";
	}
	EXPECT_TRUE(hxconsole_exec_filename("hxconsole_test_file_test.txt"));
	EXPECT_EQ(hxs_console_test_file_var1, 63.0f);
}
#endif
#endif // HX_USE_FILE_IO
#endif // (HX_USE_CONSOLE) > 1
