// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

#include <hx/hxfile.hpp>
#include <hx/hxutility.h>
#include <hx/hxtest.hpp>
#include "test_trackers.hpp"

#if HX_USE_FILE_IO
#if defined __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-zero-length"
#endif

HX_NS_USE

namespace {
void hxfile_test_make_empty(const char* filename) {
	const hxfile writer(hxfile::out, "%s", filename);
	EXPECT_TRUE(writer.is_open());
}

void hxfile_test_make_bytes(const char* filename, const void* bytes, size_t count) {
	hxfile writer(hxfile::out, "%s", filename);
	EXPECT_TRUE(writer.is_open());
	EXPECT_EQ(writer.write(bytes, count), count);
}
} // namespace

TEST(hxfile_test, empty_name_rejects_empty_path) {
	const hxfile f(hxfile::in | hxfile::skip_asserts, "");
	EXPECT_EQ(f.fail(), true);
	EXPECT_EQ(f.is_open(), false);
	EXPECT_EQ(f, false);
}

#if defined __GNUC__
#pragma GCC diagnostic pop
#endif
TEST(hxfile_test, read_write_round_trip) {
	if(hxfile f = hxfile(hxfile::in | hxfile::out | hxfile::skip_asserts, "hxfile_test_read_write.txt")) {
		f << "hxfile_test_read_write.txt";
		EXPECT_EQ(f.fail(), false);
		EXPECT_EQ(f.is_open(), true);
		EXPECT_EQ(f, true);
		EXPECT_TRUE(f.flush());
		EXPECT_FALSE(f.fail());
	}
	else {
		ADD_FAILURE();
	}
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
}

TEST(hxfile_test, missing_file_reports_expectations) {
	const hxfile f(hxfile::in | hxfile::skip_asserts, "test-file-does-not-exist-%d", 123);
	EXPECT_EQ(f.fail(), true);
	EXPECT_EQ(f.is_open(), false);
	EXPECT_EQ((bool)(f.mode() & hxfile::skip_asserts), true);
}

TEST(hxfile_test, seek_and_read_maintain_state) {
	struct hxfile_test_arbitrary_t { uint32_t x; };
	const hxfile_test_arbitrary_t a { 0xefefefefu };
	const hxfile_test_arbitrary_t b { 0x01020304u };
	hxfile_test_arbitrary_t c { 0x0u };
	hxfile f(hxfile::in | hxfile::out, "hxfile_test_offset.bin");
	f.write1(a);
	f.write1(b);
	f.write1(a);
	EXPECT_EQ(f.get_pos(), 12u);
	f.set_pos(4);
	EXPECT_TRUE(!f.fail());
	EXPECT_FALSE(f.eof());
	f.read1(c);
	EXPECT_TRUE(!f.fail());
	EXPECT_FALSE(f.eof());
	EXPECT_EQ(b.x, c.x);
	EXPECT_EQ(f.get_pos(), 8u);
}

TEST(hxfile_test, move_copy_and_stream_operators) {
	class hxfile_test_record {
	public:
		uint32_t a;
		int16_t b;
		uint8_t c;
		int8_t d;
	};
	hxfile ft(hxfile::out | hxfile::skip_asserts, "hxfile_test_operators.bin");
	hxfile f(hxmove(ft));
	EXPECT_TRUE(f.is_open());
	const hxfile_test_record x { 77777u, -555, 77u, -55 };
	const int a = -3;
	f << x << a;
	f.print("(%d,%d)", 30, 70);
	EXPECT_TRUE(!f.fail());
	EXPECT_FALSE(f.eof());
	f.close();
	hxfile ftt(hxfile::in | hxfile::skip_asserts, "hxfile_test_operators.bin");
	f = hxmove(ftt);
	EXPECT_TRUE(f.is_open());
	hxfile_test_record y;
	::memset(&y, 0x00, sizeof y);
	int b = 0;
	int thirty = 0;
	int seventy = 0;
	f >> y >> b;
	f.scan("(%d,%d)", &thirty, &seventy);
	EXPECT_EQ(y.a, 77777u);
	EXPECT_EQ(y.b, -555);
	EXPECT_EQ(y.c, 77u);
	EXPECT_EQ(y.d, -55);
	EXPECT_EQ(b, -3);
	EXPECT_EQ(thirty, 30);
	EXPECT_EQ(seventy, 70);
	EXPECT_TRUE(!f.fail());
	EXPECT_FALSE(f.eof());
	char t = '\0';
	const size_t extra_byte = f.read(&t, sizeof t, sizeof t);
	EXPECT_TRUE(f.eof());
	EXPECT_EQ(extra_byte, 0u);
	EXPECT_TRUE(f.fail());
	f.clear();
	EXPECT_TRUE(!f.fail());
	f.close();
	EXPECT_TRUE(!f.is_open());
}

TEST(hxfile_test, getline_multi_chunk) {
	const char filename[] = "hxfile_test_getline_long.txt";
	{
		hxfile writer(hxfile::out, filename);
		char content[HX_MAX_LINE + 1];
		::memset(content, 'x', HX_MAX_LINE);
		content[HX_MAX_LINE] = '\n';
		writer.write(content, static_cast<size_t>(HX_MAX_LINE + 1));
		EXPECT_FALSE(writer.fail());
	}
	hxfile reader(hxfile::in, filename);
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
		hxfile writer(hxfile::out, filename);
		writer.write("hello\nworld\n", 12u);
		EXPECT_FALSE(writer.fail());
	}
	hxfile reader(hxfile::in, filename);
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
		hxfile writer(hxfile::out, filename);
		writer.write("abcdefghij", 10u);
		EXPECT_FALSE(writer.fail());
	}
	hxfile reader(hxfile::in, filename);
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

TEST(hxfile_test, print_max_line_boundary) {
	char str[HX_MAX_LINE + 1];
	::memset(str, 'a', sizeof str);
	str[HX_MAX_LINE - 1] = '\0';
	{
		hxfile f(hxfile::out, "hxfile_test_print_limit.txt");
		EXPECT_TRUE(f.print("%s", str));
		EXPECT_FALSE(f.fail());
		EXPECT_EQ(f.get_pos(), static_cast<size_t>(HX_MAX_LINE - 1));
	}
	str[HX_MAX_LINE - 1] = 'a';
	str[HX_MAX_LINE] = '\0';
	{
		hxfile f(hxfile::out, "hxfile_test_print_limit.txt");
		EXPECT_TRUE(f.print("%s", str));
		EXPECT_FALSE(f.fail());
		EXPECT_EQ(f.get_pos(), static_cast<size_t>(HX_MAX_LINE));
	}
}

TEST(hxfile_test, read_byte_count_equals_buffer_size) {
	const char filename[] = "hxfile_test_read_eq.bin";
	uint8_t data[4] = { 0x01u, 0x02u, 0x03u, 0x04u };
	{
		hxfile writer(hxfile::out, filename);
		writer.write(data, sizeof data);
		EXPECT_FALSE(writer.fail());
	}
	hxfile reader(hxfile::in | hxfile::skip_asserts, filename);
	uint8_t buf[4] = { 0u, 0u, 0u, 0u };
	EXPECT_EQ(reader.read(buf, sizeof buf, sizeof buf), sizeof buf);
	EXPECT_FALSE(reader.fail());
	EXPECT_EQ(buf[0], 0x01u);
	EXPECT_EQ(buf[3], 0x04u);
}

TEST(hxfile_test, getline_exact_buffer_minus_one_chars) {
	const char filename[] = "hxfile_test_getline_exact.txt";
	{
		hxfile writer(hxfile::out, filename);
		writer.write("abcde\nfg", 8u);
		EXPECT_FALSE(writer.fail());
	}
	hxfile reader(hxfile::in, filename);
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

TEST(hxfile_test, eof_variants) {
	const char filename[] = "hxfile_test_empty.bin";
	{
		const hxfile writer(hxfile::out | hxfile::skip_asserts, filename);
		EXPECT_TRUE(writer.is_open());
		EXPECT_FALSE(writer.fail());
	}
	{
		hxfile reader(hxfile::in | hxfile::skip_asserts, filename);
		uint8_t buffer[4];
		EXPECT_TRUE(reader.is_open());
		EXPECT_FALSE(reader.fail());
		EXPECT_EQ(reader.read(buffer, sizeof buffer, sizeof buffer), 0u);
		EXPECT_TRUE(reader.fail());
		EXPECT_TRUE(reader.eof());
	}
	{
		hxfile reader(hxfile::in | hxfile::skip_asserts, filename);
		uint8_t buffer[4];
		EXPECT_TRUE(reader.is_open());
		EXPECT_FALSE(reader.fail());
		EXPECT_EQ(reader.read(buffer, sizeof buffer, sizeof buffer + 1u), 0u);
		EXPECT_TRUE(reader.fail());
		EXPECT_FALSE(reader.eof());
	}
	{
		hxfile reader(hxfile::in | hxfile::skip_asserts, filename);
		char line[8];
		EXPECT_TRUE(reader.is_open());
		EXPECT_FALSE(reader.fail());
		EXPECT_FALSE(reader.getline(line));
		EXPECT_TRUE(reader.fail());
		EXPECT_TRUE(reader.eof());
	}
	{
		hxfile reader(hxfile::in | hxfile::skip_asserts, filename);
		int scanned = 0;
		EXPECT_TRUE(reader.is_open());
		EXPECT_FALSE(reader.fail());
		const int result = reader.scan("%d", &scanned);
		EXPECT_LT(result, 0);
		EXPECT_TRUE(reader.fail());
		EXPECT_TRUE(reader.eof());
	}
}

TEST(hxfile_test, dev_null_writes_are_skipped) {
	const char bytes[4] = { 'a', 'b', 'c', 'd' };
	EXPECT_EQ(hxdev_null.write(bytes, sizeof bytes), sizeof bytes);
	EXPECT_FALSE(hxdev_null.fail());
	EXPECT_TRUE(hxdev_null.flush());
	EXPECT_TRUE(hxdev_null.print("ignored %d", 5));
	EXPECT_FALSE(hxdev_null.fail());
}

TEST(hxfile_test, open_method_reopens_file) {
	hxfile f;
	EXPECT_FALSE(f.is_open());
	EXPECT_TRUE(f.open(hxfile::out | hxfile::skip_asserts, "hxfile_test_open%d.txt", 1));
	EXPECT_TRUE(f.is_open());
	EXPECT_FALSE(f.fail());
}

TEST(hxfile_test, none_mode_fails_without_opening) {
	const hxfile f(hxfile::none | hxfile::skip_asserts, "hxfile_test_none.txt");
	EXPECT_FALSE(f.is_open());
	EXPECT_TRUE(f.fail());
}

TEST(hxfile_test, assert_open_fails_fires_when_not_skipped) {
	hxlog_warning("EXPECTING_ASSERT_FAILURE");
	const hxtest_skip_asserts skip(1);
	const hxfile f(hxfile::in, "test-file-does-not-exist-assert");
#if (HX_HARDENING_MODE) != HX_HARDENING_MODE_NONE
	EXPECT_EQ(skip.remaining(), 0);
#endif
	EXPECT_TRUE(f.fail());
	EXPECT_FALSE(f.is_open());
}

TEST(hxfile_test, assert_open_fails_honors_skip) {
	const hxtest_skip_asserts skip(1);
	const hxfile f(hxfile::in | hxfile::skip_asserts, "test-file-does-not-exist-assert");
	EXPECT_EQ(skip.remaining(), 1);
	EXPECT_TRUE(f.fail());
	EXPECT_FALSE(f.is_open());
}

TEST(hxfile_test, assert_read_overflow_fires_when_not_skipped) {
	hxlog_warning("EXPECTING_ASSERT_FAILURE");
	const char filename[] = "hxfile_test_assert_read_overflow.bin";
	const uint8_t data[4] = { 1u, 2u, 3u, 4u };
	hxfile_test_make_bytes(filename, data, sizeof data);

	hxfile reader(hxfile::in, filename);
	uint8_t buf[2] = { 0u, 0u };
	const hxtest_skip_asserts skip(1);
	EXPECT_EQ(reader.read(buf, sizeof buf, sizeof buf + 1u), 0u);
#if (HX_HARDENING_MODE) != HX_HARDENING_MODE_NONE
	EXPECT_EQ(skip.remaining(), 0);
#endif
	EXPECT_TRUE(reader.fail());
	EXPECT_FALSE(reader.eof());
}

TEST(hxfile_test, assert_read_overflow_honors_skip) {
	const char filename[] = "hxfile_test_assert_read_overflow.bin";
	const uint8_t data[4] = { 1u, 2u, 3u, 4u };
	hxfile_test_make_bytes(filename, data, sizeof data);

	hxfile reader(hxfile::in | hxfile::skip_asserts, filename);
	uint8_t buf[2] = { 0u, 0u };
	const hxtest_skip_asserts skip(1);
	EXPECT_EQ(reader.read(buf, sizeof buf, sizeof buf + 1u), 0u);
	EXPECT_EQ(skip.remaining(), 1);
	EXPECT_TRUE(reader.fail());
	EXPECT_FALSE(reader.eof());
}

TEST(hxfile_test, assert_read_short_fires_when_not_skipped) {
	hxlog_warning("EXPECTING_ASSERT_FAILURE");
	const char filename[] = "hxfile_test_assert_read_short.bin";
	hxfile_test_make_empty(filename);

	hxfile reader(hxfile::in, filename);
	uint8_t buf[4] = { 0u, 0u, 0u, 0u };
	const hxtest_skip_asserts skip(1);
	EXPECT_EQ(reader.read(buf, sizeof buf, sizeof buf), 0u);
#if (HX_HARDENING_MODE) == HX_HARDENING_MODE_DEBUG
	EXPECT_EQ(skip.remaining(), 0);
#endif
	EXPECT_TRUE(reader.fail());
	EXPECT_TRUE(reader.eof());
}

TEST(hxfile_test, assert_read_short_honors_skip) {
	const char filename[] = "hxfile_test_assert_read_short.bin";
	hxfile_test_make_empty(filename);

	hxfile reader(hxfile::in | hxfile::skip_asserts, filename);
	uint8_t buf[4] = { 0u, 0u, 0u, 0u };
	const hxtest_skip_asserts skip(1);
	EXPECT_EQ(reader.read(buf, sizeof buf, sizeof buf), 0u);
	EXPECT_EQ(skip.remaining(), 1);
	EXPECT_TRUE(reader.fail());
	EXPECT_TRUE(reader.eof());
}

TEST(hxfile_test, assert_scan_eof_fires_when_not_skipped) {
	hxlog_warning("EXPECTING_ASSERT_FAILURE");
	const char filename[] = "hxfile_test_assert_scan_eof.txt";
	hxfile_test_make_empty(filename);

	hxfile reader(hxfile::in, filename);
	int scanned = 0;
	const hxtest_skip_asserts skip(1);
	EXPECT_LT(reader.scan("%d", &scanned), 0);
#if (HX_HARDENING_MODE) != HX_HARDENING_MODE_NONE
	EXPECT_EQ(skip.remaining(), 0);
#endif
	EXPECT_TRUE(reader.fail());
	EXPECT_TRUE(reader.eof());
}

TEST(hxfile_test, assert_scan_eof_honors_skip) {
	const char filename[] = "hxfile_test_assert_scan_eof.txt";
	hxfile_test_make_empty(filename);

	hxfile reader(hxfile::in | hxfile::skip_asserts, filename);
	int scanned = 0;
	const hxtest_skip_asserts skip(1);
	EXPECT_LT(reader.scan("%d", &scanned), 0);
	EXPECT_EQ(skip.remaining(), 1);
	EXPECT_TRUE(reader.fail());
	EXPECT_TRUE(reader.eof());
}

TEST(hxfile_test, getline_single_byte_buffer_reads_nothing) {
	const char filename[] = "hxfile_test_getline_single.txt";
	{
		hxfile writer(hxfile::out, filename);
		writer.write("ab\n", 3u);
	}
	hxfile reader(hxfile::in, filename);
	char buf[2] = { '\xff', '\xff' };
	EXPECT_TRUE(reader.getline(buf, 1));
	EXPECT_EQ(buf[0], '\0');
	EXPECT_EQ(buf[1], '\xff');
	EXPECT_FALSE(reader.fail());
	EXPECT_FALSE(reader.eof());
	EXPECT_TRUE(reader.getline(buf + 1, 1));
	EXPECT_EQ(buf[1], '\0');
}
#endif // HX_USE_FILE_IO
