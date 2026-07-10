// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

#include <hx/hxfile.hpp>
#include <hx/hxutility.h>
#include <hx/hxtest.hpp>
#include "./hxtest_util.hpp"

#if HX_USE_FILE_IO
#if defined __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-zero-length"
#endif

HX_NS_USE

#if !defined _MSC_VER && !defined __wasm__
static_assert(sizeof(size_t) != 4 || (
		sizeof(hxfile) == 8u),
	"hxfile must pack an intptr_t handle and four uint8_t/bool flags with no"
	" padding");
static_assert(sizeof(size_t) != 8 || (
		sizeof(hxfile) == 16u),
	"hxfile must pack an intptr_t handle and four uint8_t/bool flags with"
	" padding only to the alignment of intptr_t");
#endif

TEST(hxfile_test, constructor_and_open_success) {
	const hxfile f(hxfile::open_mode_in | hxfile::open_mode_out | hxfile::open_mode_asserts,
		"hxfile_test_ctor.bin");
	EXPECT_TRUE(f.is_open());
	EXPECT_FALSE(f.fail());
	EXPECT_EQ(f.mode(), hxfile::open_mode_in | hxfile::open_mode_out | hxfile::open_mode_asserts);
#if (HX_USE_FILE_IO) == 2
	const hxfile g(hxfile::open_mode_out, static_cast<intptr_t>(-1));
#else
	const hxfile g(hxfile::open_mode_out, static_cast<intptr_t>(0));
#endif
	EXPECT_FALSE(g.is_open());
	EXPECT_EQ(g.mode(), hxfile::open_mode_out);
	hxfile h;
	EXPECT_FALSE(h.is_open());
	EXPECT_TRUE(h.open(hxfile::open_mode_out | hxfile::open_mode_asserts, "hxfile_test_open%d.txt", 1));
	EXPECT_TRUE(h.is_open());
	EXPECT_FALSE(h.fail());
}

TEST(hxfile_test, constructor_and_open_failure) {
	const hxfile empty_name(hxfile::open_mode_in, "");
	EXPECT_TRUE(empty_name.fail());
	EXPECT_FALSE(empty_name.is_open());
	EXPECT_EQ(empty_name, false);
	const hxfile missing(hxfile::open_mode_in, "test-file-does-not-exist-%d", 123);
	EXPECT_TRUE(missing.fail());
	EXPECT_FALSE(missing.is_open());
	EXPECT_EQ((bool)(missing.mode() & hxfile::open_mode_asserts), false);
	EXPECT_EQ((bool)(missing.mode() & hxfile::open_mode_in), true);
	const hxfile none_mode(hxfile::open_mode_none | hxfile::open_mode_asserts, "hxfile_test_none.txt");
	EXPECT_FALSE(none_mode.is_open());
	EXPECT_TRUE(none_mode.fail());
}

TEST(hxfile_test, assert_open_fails_fires_when_asserts_set) {
	hxlog_warning("EXPECTING_ASSERT_FAILURE");
	const hxtest_skip_asserts skip(1);
	const hxfile f(hxfile::open_mode_in | hxfile::open_mode_asserts, "test-file-does-not-exist-assert");
#if (HX_HARDENING_MODE) != HX_HARDENING_MODE_NONE
	EXPECT_EQ(skip.remaining(), 0);
#endif
	EXPECT_TRUE(f.fail());
	EXPECT_FALSE(f.is_open());
}

TEST(hxfile_test, assert_open_fails_honors_default) {
	const hxtest_skip_asserts skip(1);
	const hxfile f(hxfile::open_mode_in, "test-file-does-not-exist-assert");
	EXPECT_EQ(skip.remaining(), 1);
	EXPECT_TRUE(f.fail());
	EXPECT_FALSE(f.is_open());
}

TEST(hxfile_test, operator_bool_and_close) {
	hxfile f(hxfile::open_mode_out | hxfile::open_mode_asserts, "hxfile_test_operator_bool.bin");
	EXPECT_TRUE(f);
	EXPECT_TRUE(static_cast<bool>(f));
	f.set_fail();
	EXPECT_FALSE(f);
	f.clear();
	EXPECT_TRUE(f);
	f.close();
	EXPECT_FALSE(f);
	EXPECT_FALSE(f.is_open());
}

TEST(hxfile_test, set_fail_and_clear) {
	hxfile f(hxfile::open_mode_in | hxfile::open_mode_out | hxfile::open_mode_asserts, "hxfile_test_set_fail.txt");
	EXPECT_TRUE(f.is_open());
	EXPECT_FALSE(f.fail());
	f.set_fail();
	EXPECT_TRUE(f.fail());
	f.clear();
	EXPECT_FALSE(f.fail());
	hxfile g;
	EXPECT_FALSE(g.is_open());
	g.clear();
	EXPECT_FALSE(g.fail());
}

TEST(hxfile_test, move_constructor_and_assignment) {
	hxfile ft(hxfile::open_mode_out | hxfile::open_mode_asserts, "hxfile_test_move.bin");
	EXPECT_TRUE(ft.is_open());
	hxfile f(hxmove(ft));
	EXPECT_TRUE(f.is_open());
	EXPECT_TRUE(f.write1(int32_t{42}));
	f.close();
	hxfile fin(hxfile::open_mode_in, "hxfile_test_move.bin");
	f = hxmove(fin);
	EXPECT_TRUE(f.is_open());
	int32_t value = 0;
	EXPECT_TRUE(f.read1(value));
	EXPECT_EQ(value, 42);
}

TEST(hxfile_test, set_pos_failure_leaves_fail_and_eof) {
	hxfile f(hxfile::open_mode_in | hxfile::open_mode_out, "hxfile_test_set_pos_fail.bin");
	EXPECT_TRUE(f.write1(int32_t{1}));
	EXPECT_FALSE(f.set_pos(static_cast<size_t>(-1)));
	EXPECT_TRUE(f.fail());
}

TEST(hxfile_test, get_pos_and_set_pos) {
	struct hxfile_test_arbitrary_t { uint32_t x; };
	const hxfile_test_arbitrary_t a { 0xefefefefu };
	const hxfile_test_arbitrary_t b { 0x01020304u };
	hxfile_test_arbitrary_t c { 0x0u };
	hxfile f(hxfile::open_mode_in | hxfile::open_mode_out | hxfile::open_mode_asserts, "hxfile_test_offset.bin");
	f.write1(a);
	f.write1(b);
	f.write1(a);
	EXPECT_EQ(f.get_pos(), 12u);
	EXPECT_TRUE(f.set_pos(4));
	EXPECT_FALSE(f.fail());
	EXPECT_FALSE(f.eof());
	f.read1(c);
	EXPECT_EQ(b.x, c.x);
	EXPECT_EQ(f.get_pos(), 8u);
}

TEST(hxfile_test, read_returns_byte_count_and_updates_flags) {
	const char filename[] = "hxfile_test_read_eq.bin";
	const uint8_t data[4] = { 0x01u, 0x02u, 0x03u, 0x04u };
	{
		hxfile writer(hxfile::open_mode_out | hxfile::open_mode_asserts, filename);
		EXPECT_EQ(writer.write(data, sizeof data), sizeof data);
	}
	hxfile reader(hxfile::open_mode_in | hxfile::open_mode_asserts, filename);
	uint8_t buf[4] = { 0u, 0u, 0u, 0u };
	EXPECT_EQ(reader.read(buf, sizeof buf, sizeof buf), sizeof buf);
	EXPECT_FALSE(reader.fail());
	EXPECT_EQ(buf[0], 0x01u);
	EXPECT_EQ(buf[3], 0x04u);
}

TEST(hxfile_test, read_short_sets_fail_and_eof) {
	const char filename[] = "hxfile_test_empty.bin";
	{
		const hxfile writer(hxfile::open_mode_out | hxfile::open_mode_asserts, filename);
		EXPECT_TRUE(writer.is_open());
	}
	hxfile reader(hxfile::open_mode_in, filename);
	uint8_t buffer[4];
	EXPECT_EQ(reader.read(buffer, sizeof buffer, sizeof buffer), 0u);
	EXPECT_TRUE(reader.fail());
	EXPECT_TRUE(reader.eof());
}

TEST(hxfile_test, read_overflow_sets_fail_without_eof) {
	const char filename[] = "hxfile_test_empty2.bin";
	{
		const hxfile writer(hxfile::open_mode_out | hxfile::open_mode_asserts, filename);
		EXPECT_TRUE(writer.is_open());
	}
	hxfile reader(hxfile::open_mode_in, filename);
	uint8_t buffer[4];
	EXPECT_EQ(reader.read(buffer, sizeof buffer, sizeof buffer + 1u), 0u);
	EXPECT_TRUE(reader.fail());
	EXPECT_FALSE(reader.eof());
}

TEST(hxfile_test, assert_read_overflow_fires_when_asserts_set) {
	hxlog_warning("EXPECTING_ASSERT_FAILURE");
	const char filename[] = "hxfile_test_assert_read_overflow.bin";
	const uint8_t data[4] = { 1u, 2u, 3u, 4u };
	hxfile writer(hxfile::open_mode_out | hxfile::open_mode_asserts, filename);
	writer.write(data, sizeof data);

	hxfile reader(hxfile::open_mode_in | hxfile::open_mode_asserts, filename);
	uint8_t buf[2] = { 0u, 0u };
	const hxtest_skip_asserts skip(1);
	EXPECT_EQ(reader.read(buf, sizeof buf, sizeof buf + 1u), 0u);
#if (HX_HARDENING_MODE) != HX_HARDENING_MODE_NONE
	EXPECT_EQ(skip.remaining(), 0);
#endif
	EXPECT_TRUE(reader.fail());
	EXPECT_FALSE(reader.eof());
}

TEST(hxfile_test, assert_read_overflow_honors_default) {
	const char filename[] = "hxfile_test_assert_read_overflow.bin";
	const uint8_t data[4] = { 1u, 2u, 3u, 4u };
	hxfile writer(hxfile::open_mode_out | hxfile::open_mode_asserts, filename);
	writer.write(data, sizeof data);

	hxfile reader(hxfile::open_mode_in, filename);
	uint8_t buf[2] = { 0u, 0u };
	const hxtest_skip_asserts skip(1);
	EXPECT_EQ(reader.read(buf, sizeof buf, sizeof buf + 1u), 0u);
	EXPECT_EQ(skip.remaining(), 1);
	EXPECT_TRUE(reader.fail());
	EXPECT_FALSE(reader.eof());
}

TEST(hxfile_test, assert_read_short_fires_when_asserts_set) {
	hxlog_warning("EXPECTING_ASSERT_FAILURE");
	const char filename[] = "hxfile_test_assert_read_short.bin";
	const hxfile writer(hxfile::open_mode_out | hxfile::open_mode_asserts, filename);
	EXPECT_TRUE(writer.is_open());

	hxfile reader(hxfile::open_mode_in | hxfile::open_mode_asserts, filename);
	uint8_t buf[4] = { 0u, 0u, 0u, 0u };
	const hxtest_skip_asserts skip(1);
	EXPECT_EQ(reader.read(buf, sizeof buf, sizeof buf), 0u);
#if (HX_HARDENING_MODE) == HX_HARDENING_MODE_DEBUG
	EXPECT_EQ(skip.remaining(), 0);
#endif
	EXPECT_TRUE(reader.fail());
	EXPECT_TRUE(reader.eof());
}

TEST(hxfile_test, assert_read_short_honors_default) {
	const char filename[] = "hxfile_test_assert_read_short.bin";
	const hxfile writer(hxfile::open_mode_out | hxfile::open_mode_asserts, filename);
	EXPECT_TRUE(writer.is_open());

	hxfile reader(hxfile::open_mode_in, filename);
	uint8_t buf[4] = { 0u, 0u, 0u, 0u };
	const hxtest_skip_asserts skip(1);
	EXPECT_EQ(reader.read(buf, sizeof buf, sizeof buf), 0u);
	EXPECT_EQ(skip.remaining(), 1);
	EXPECT_TRUE(reader.fail());
	EXPECT_TRUE(reader.eof());
}

TEST(hxfile_test, read1_and_write1_round_trip) {
	hxfile f(hxfile::open_mode_in | hxfile::open_mode_out | hxfile::open_mode_asserts, "hxfile_test_read1_write1.bin");
	const int32_t value = -12345;
	EXPECT_TRUE(f.write1(value));
	EXPECT_TRUE(f.set_pos(0));
	int32_t round_tripped = 0;
	EXPECT_TRUE(f.read1(round_tripped));
	EXPECT_EQ(round_tripped, value);
}

TEST(hxfile_test, write_returns_byte_count_and_resets_fail) {
	hxfile f(hxfile::open_mode_out | hxfile::open_mode_asserts, "hxfile_test_write.bin");
	f.set_fail();
	const char bytes[3] = { 'x', 'y', 'z' };
	EXPECT_EQ(f.write(bytes, sizeof bytes), sizeof bytes);
	EXPECT_FALSE(f.fail());
}

TEST(hxfile_test, dev_null_writes_are_skipped) {
	const char bytes[4] = { 'a', 'b', 'c', 'd' };
	EXPECT_EQ(hxdev_null.write(bytes, sizeof bytes), sizeof bytes);
	EXPECT_FALSE(hxdev_null.fail());
	EXPECT_TRUE(hxdev_null.print("ignored %d", 5));
	EXPECT_FALSE(hxdev_null.fail());
}

TEST(hxfile_test, flush_succeeds_and_is_safe_on_dev_null) {
	hxfile f(hxfile::open_mode_out | hxfile::open_mode_asserts, "hxfile_test_flush.bin");
	EXPECT_TRUE(f.write1(int32_t{1}));
	EXPECT_TRUE(f.flush());
	EXPECT_FALSE(f.fail());
	EXPECT_TRUE(hxdev_null.flush());
}

TEST(hxfile_test, print_writes_formatted_text) {
	hxfile f(hxfile::open_mode_out | hxfile::open_mode_asserts, "hxfile_test_print.txt");
	EXPECT_TRUE(f.print("(%d,%d)", 30, 70));
	EXPECT_FALSE(f.fail());
	EXPECT_EQ(f.get_pos(), 7u);
}

TEST(hxfile_test, print_max_line_boundary) {
	char str[HX_MAX_LINE + 1];
	::memset(str, 'a', sizeof str);
	str[HX_MAX_LINE - 1] = '\0';
	{
		hxfile f(hxfile::open_mode_out | hxfile::open_mode_asserts, "hxfile_test_print_limit.txt");
		EXPECT_TRUE(f.print("%s", str));
		EXPECT_FALSE(f.fail());
		EXPECT_EQ(f.get_pos(), static_cast<size_t>(HX_MAX_LINE - 1));
	}
	str[HX_MAX_LINE - 1] = 'a';
	str[HX_MAX_LINE] = '\0';
	{
		hxfile f(hxfile::open_mode_out | hxfile::open_mode_asserts, "hxfile_test_print_limit.txt");
		EXPECT_TRUE(f.print("%s", str));
		EXPECT_FALSE(f.fail());
		EXPECT_EQ(f.get_pos(), static_cast<size_t>(HX_MAX_LINE));
	}
}

TEST(hxfile_test, print_malformed_format_returns_false) {
	wchar_t bad[2];
	bad[0] = static_cast<wchar_t>(0xD800);
	bad[1] = L'\0';
	hxfile f(hxfile::open_mode_out, "hxfile_test_print_bad.txt");
	EXPECT_TRUE(f.is_open());
	EXPECT_FALSE(f.print("%ls", bad));
	EXPECT_TRUE(f.fail());
}

TEST(hxfile_test, scan_reads_formatted_text) {
	{
		hxfile writer(hxfile::open_mode_out | hxfile::open_mode_asserts, "hxfile_test_scan.txt");
		EXPECT_TRUE(writer.print("(%d,%d)", 30, 70));
	}
	hxfile reader(hxfile::open_mode_in | hxfile::open_mode_asserts, "hxfile_test_scan.txt");
	int thirty = 0;
	int seventy = 0;
	EXPECT_EQ(reader.scan("(%d,%d)", &thirty, &seventy), 2);
	EXPECT_EQ(thirty, 30);
	EXPECT_EQ(seventy, 70);
	EXPECT_FALSE(reader.fail());
	EXPECT_FALSE(reader.eof());
}

TEST(hxfile_test, scan_eof_sets_fail_and_eof) {
	const char filename[] = "hxfile_test_scan_eof.txt";
	const hxfile writer(hxfile::open_mode_out | hxfile::open_mode_asserts, filename);
	EXPECT_TRUE(writer.is_open());
	hxfile reader(hxfile::open_mode_in, filename);
	int scanned = 0;
	const int result = reader.scan("%d", &scanned);
	EXPECT_LT(result, 0);
	EXPECT_TRUE(reader.fail());
	EXPECT_TRUE(reader.eof());
}

TEST(hxfile_test, assert_scan_eof_fires_when_asserts_set) {
	hxlog_warning("EXPECTING_ASSERT_FAILURE");
	const char filename[] = "hxfile_test_assert_scan_eof.txt";
	const hxfile writer(hxfile::open_mode_out | hxfile::open_mode_asserts, filename);
	EXPECT_TRUE(writer.is_open());

	hxfile reader(hxfile::open_mode_in | hxfile::open_mode_asserts, filename);
	int scanned = 0;
	const hxtest_skip_asserts skip(1);
	EXPECT_LT(reader.scan("%d", &scanned), 0);
#if (HX_HARDENING_MODE) != HX_HARDENING_MODE_NONE
	EXPECT_EQ(skip.remaining(), 0);
#endif
	EXPECT_TRUE(reader.fail());
	EXPECT_TRUE(reader.eof());
}

TEST(hxfile_test, assert_scan_eof_honors_default) {
	const char filename[] = "hxfile_test_assert_scan_eof.txt";
	const hxfile writer(hxfile::open_mode_out | hxfile::open_mode_asserts, filename);
	EXPECT_TRUE(writer.is_open());

	hxfile reader(hxfile::open_mode_in, filename);
	int scanned = 0;
	const hxtest_skip_asserts skip(1);
	EXPECT_LT(reader.scan("%d", &scanned), 0);
	EXPECT_EQ(skip.remaining(), 1);
	EXPECT_TRUE(reader.fail());
	EXPECT_TRUE(reader.eof());
}

TEST(hxfile_test, stream_insertion_and_extraction_operators) {
	class hxfile_test_record {
	public:
		uint32_t a;
		int16_t b;
		uint8_t c;
		int8_t d;
	};
	{
		hxfile f(hxfile::open_mode_out | hxfile::open_mode_asserts, "hxfile_test_stream.bin");
		const hxfile_test_record x { 77777u, -555, 77u, -55 };
		const int a = -3;
		f << x << a;
		EXPECT_FALSE(f.fail());
	}
	hxfile f(hxfile::open_mode_in | hxfile::open_mode_asserts, "hxfile_test_stream.bin");
	hxfile_test_record y;
	::memset(&y, 0x00, sizeof y);
	int b = 0;
	f >> y >> b;
	EXPECT_EQ(y.a, 77777u);
	EXPECT_EQ(y.b, -555);
	EXPECT_EQ(y.c, 77u);
	EXPECT_EQ(y.d, -55);
	EXPECT_EQ(b, -3);
	EXPECT_FALSE(f.fail());
	EXPECT_FALSE(f.eof());
}

TEST(hxfile_test, stream_insertion_of_string_literal) {
	{
		hxfile f(hxfile::open_mode_out | hxfile::open_mode_asserts, "hxfile_test_stream_literal.txt");
		f << "hxfile_test_stream_literal.txt";
		EXPECT_FALSE(f.fail());
	}
	hxfile f(hxfile::open_mode_in | hxfile::open_mode_asserts, "hxfile_test_stream_literal.txt");
	char buf[32];
	::memset(buf, 0x00, sizeof buf);
	EXPECT_EQ(f.read(buf, sizeof buf, sizeof "hxfile_test_stream_literal.txt" - 1u), sizeof "hxfile_test_stream_literal.txt" - 1u);
	EXPECT_STREQ(buf, "hxfile_test_stream_literal.txt");
}

TEST(hxfile_test, hxin_hxout_hxerr_smoke_test) {
	hxout << "smoke test hxout" << ".";
	hxout.print("..");
	hxout << hxendl;
	EXPECT_TRUE(hxout.flush());
	EXPECT_FALSE(hxout.fail());
	hxerr << "smoke test hxerr" << ".";
	hxerr.print("..");
	hxerr << hxendl;
	EXPECT_TRUE(hxerr.flush());
	EXPECT_FALSE(hxerr.fail());
	EXPECT_TRUE(hxin.is_open());
}

#if (HX_USE_FILE_IO) != 2
TEST(hxfile_test, getline_single_byte_buffer_reads_nothing) {
	const char filename[] = "hxfile_test_getline_single.txt";
	{
		hxfile writer(hxfile::open_mode_out | hxfile::open_mode_asserts, filename);
		writer.write("ab\n", 3u);
	}
	hxfile reader(hxfile::open_mode_in | hxfile::open_mode_asserts, filename);
	char buf[2] = { '\xff', '\xff' };
	EXPECT_TRUE(reader.getline(buf, 1));
	EXPECT_EQ(buf[0], '\0');
	EXPECT_EQ(buf[1], '\xff');
	EXPECT_FALSE(reader.fail());
	EXPECT_FALSE(reader.eof());
	EXPECT_TRUE(reader.getline(buf + 1, 1));
	EXPECT_EQ(buf[1], '\0');
}

TEST(hxfile_test, getline_multi_chunk) {
	const char filename[] = "hxfile_test_getline_long.txt";
	{
		hxfile writer(hxfile::open_mode_out | hxfile::open_mode_asserts, filename);
		char content[HX_MAX_LINE + 1];
		::memset(content, 'x', HX_MAX_LINE);
		content[HX_MAX_LINE] = '\n';
		writer.write(content, static_cast<size_t>(HX_MAX_LINE + 1));
		EXPECT_FALSE(writer.fail());
	}
	hxfile reader(hxfile::open_mode_in | hxfile::open_mode_asserts, filename);
	char line[HX_MAX_LINE + 2];
	::memset(line, 0, sizeof line);
	EXPECT_TRUE(reader.getline(line, static_cast<int>(sizeof line)));
	EXPECT_FALSE(reader.fail());
	EXPECT_EQ(line[0], 'x');
	EXPECT_EQ(line[HX_MAX_LINE - 1], 'x');
	EXPECT_EQ(line[HX_MAX_LINE], '\n');
	EXPECT_EQ(line[HX_MAX_LINE + 1], '\0');
}

TEST(hxfile_test, getline_newline_excluded_by_buffer_size) {
	const char filename[] = "hxfile_test_getline_excl.txt";
	{
		hxfile writer(hxfile::open_mode_out | hxfile::open_mode_asserts, filename);
		writer.write("hello\nworld\n", 12u);
		EXPECT_FALSE(writer.fail());
	}
	hxfile reader(hxfile::open_mode_in | hxfile::open_mode_asserts, filename);
	char line[12];
	EXPECT_TRUE(reader.getline(line, 6));
	EXPECT_FALSE(reader.fail());
	EXPECT_EQ(line[0], 'h');
	EXPECT_EQ(line[4], 'o');
	EXPECT_EQ(line[5], '\0');
	EXPECT_TRUE(reader.getline(line, static_cast<int>(sizeof line)));
	EXPECT_EQ(line[0], '\n');
	EXPECT_EQ(line[1], '\0');
}

TEST(hxfile_test, getline_buffer_fills_without_newline) {
	const char filename[] = "hxfile_test_getline_noeol.txt";
	{
		hxfile writer(hxfile::open_mode_out | hxfile::open_mode_asserts, filename);
		writer.write("abcdefghij", 10u);
		EXPECT_FALSE(writer.fail());
	}
	hxfile reader(hxfile::open_mode_in | hxfile::open_mode_asserts, filename);
	char line[16];
	EXPECT_TRUE(reader.getline(line, 6));
	EXPECT_FALSE(reader.fail());
	EXPECT_EQ(line[0], 'a');
	EXPECT_EQ(line[4], 'e');
	EXPECT_EQ(line[5], '\0');
	EXPECT_TRUE(reader.getline(line, static_cast<int>(sizeof line)));
	EXPECT_EQ(line[0], 'f');
	EXPECT_EQ(line[4], 'j');
	EXPECT_EQ(line[5], '\0');
	EXPECT_FALSE(reader.getline(line, static_cast<int>(sizeof line)));
	EXPECT_TRUE(reader.eof());
}

TEST(hxfile_test, getline_exact_buffer_minus_one_chars) {
	const char filename[] = "hxfile_test_getline_exact.txt";
	{
		hxfile writer(hxfile::open_mode_out | hxfile::open_mode_asserts, filename);
		writer.write("abcde\nfg", 8u);
		EXPECT_FALSE(writer.fail());
	}
	hxfile reader(hxfile::open_mode_in | hxfile::open_mode_asserts, filename);
	char line[7];
	EXPECT_TRUE(reader.getline(line, static_cast<int>(sizeof line)));
	EXPECT_EQ(line[0], 'a');
	EXPECT_EQ(line[4], 'e');
	EXPECT_EQ(line[5], '\n');
	EXPECT_EQ(line[6], '\0');
	EXPECT_TRUE(reader.getline(line, static_cast<int>(sizeof line)));
	EXPECT_EQ(line[0], 'f');
	EXPECT_EQ(line[1], 'g');
	EXPECT_EQ(line[2], '\0');
}

TEST(hxfile_test, getline_array_reference_deduces_size) {
	const char filename[] = "hxfile_test_getline_array.txt";
	{
		hxfile writer(hxfile::open_mode_out | hxfile::open_mode_asserts, filename);
		writer.write("abc\nde", 6u);
		EXPECT_FALSE(writer.fail());
	}
	hxfile reader(hxfile::open_mode_in | hxfile::open_mode_asserts, filename);
	char line[4];
	EXPECT_TRUE(reader.getline(line));
	EXPECT_EQ(line[0], 'a');
	EXPECT_EQ(line[2], 'c');
	EXPECT_EQ(line[3], '\0');
	EXPECT_TRUE(reader.getline(line));
	EXPECT_EQ(line[0], '\n');
	EXPECT_EQ(line[1], '\0');
	EXPECT_TRUE(reader.getline(line));
	EXPECT_EQ(line[0], 'd');
	EXPECT_EQ(line[1], 'e');
	EXPECT_EQ(line[2], '\0');
	EXPECT_FALSE(reader.getline(line));
	EXPECT_TRUE(reader.eof());
}
#endif // (HX_USE_FILE_IO) != 2

#if defined __GNUC__
#pragma GCC diagnostic pop
#endif
#endif // HX_USE_FILE_IO
