// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

// This is the C <stdio.h> version using FILE*. Use an alternate .cpp file for
// alternate implementations. Annex K of the C11 standard was never widely
// implemented. Therefore no *_s functions are used here.  See the NOLINT
// markers.

#include "../include/hx/hxfile.hpp"

#if (HX_USE_FILE_IO) == 1

// These are only dependencies of the libhatchet here. This is to allow
// easy reimplementation.
#include <stdio.h>

#if defined _MSC_VER
// Allow use of fopen as fopen_s is not portable.
#pragma warning(disable: 4996)
#endif

HX_NS_BEGIN_

// In this version targets require an implementation of fopen(), fclose(),
// fread(), fwrite(), fgets(), and feof().

hxfile hxin(hxfile::in, reinterpret_cast<intptr_t>(stdin));
hxfile hxout(hxfile::out, reinterpret_cast<intptr_t>(stdout));
#ifndef __wasm__
hxfile hxerr(hxfile::out, reinterpret_cast<intptr_t>(stderr));
#else
// Don't use stdout with the default index.js provided by the emsdk.
hxfile hxerr(hxfile::out, reinterpret_cast<intptr_t>(stdout));
#endif
hxfile hxdev_null(hxfile::out, static_cast<intptr_t>(0));

hxfile::hxfile(void) {
	::memset(static_cast<void*>(this), 0x00, sizeof *this);
}

// In this version the file is a FILE* reinterpreted as intptr_t.
hxfile::hxfile(uint8_t mode, intptr_t file) : hxfile() {
	m_file_pimpl_ = file; // does not own.
	m_open_mode_ = mode;
}

hxfile::hxfile(uint8_t mode, const char* filename, ...) : hxfile() {
	va_list args;
	va_start(args, filename);
	openv_(mode, filename, args);
	va_end(args);
}

hxfile::hxfile(hxfile&& file) noexcept {
	hxassertmsg(this != &file, "self_assignment");
	::memcpy(static_cast<void*>(this), static_cast<const void*>(&file), sizeof file);
	::memset(static_cast<void*>(&file), 0x00, sizeof file);
}

hxfile::~hxfile(void) {
	close();
}

void hxfile::operator=(hxfile&& file) noexcept {
	hxassertmsg(this != &file, "self_assignment");
	close();
	::memcpy(static_cast<void*>(this), static_cast<const void*>(&file), sizeof file);
	::memset(static_cast<void*>(&file), 0x00, sizeof file);
}

hxfile::operator bool(void) const {
	return (m_file_pimpl_ != 0) && !m_fail_;
}

bool hxfile::open(uint8_t mode, const char* filename, ...) {
	close(); // openv_ assumes the file is closed.

	va_list args;
	va_start(args, filename);
	const bool rv = openv_(mode, filename, args);
	va_end(args);
	return rv;
}

bool hxfile::openv_(uint8_t mode, const char* filename, va_list args) {
	hxassert(m_file_pimpl_ == 0);

	m_open_mode_ = mode; // Record mode regardless.

	const char* m = hxnull;
	switch (static_cast<int>(mode) & (hxfile::in | hxfile::out)) {
	case hxfile::none:
		m_fail_ = true;
		return false;
	case hxfile::in:
		m = "rb";
		break;
	case hxfile::out:
		m = "wb";
		break;
	default:
		m = "w+b";
	}

	char line_buf[HX_MAX_LINE];
	const int len = ::vsnprintf(line_buf, HX_MAX_LINE, filename, args);
	hxassertmsg(len >= 0 && len < HX_MAX_LINE, "vsnprintf"); (void)len;

	m_file_pimpl_ = reinterpret_cast<intptr_t>(::fopen(line_buf, m));
	hxassert_hard((m_file_pimpl_ != 0) || ((mode & hxfile::skip_asserts) != 0u),
		"fopen %s %s: %s", line_buf, m, ::strerror(errno));

	m_fail_ = (m_file_pimpl_ == 0);
	m_owns_ = !m_fail_;
	return !m_fail_;
}

void hxfile::close(void) {
	if(m_owns_) {
		const int code = ::fclose(reinterpret_cast<FILE*>(m_file_pimpl_));
		hxassertmsg(code == 0, "fclose"); (void)code;
	}
	::memset(static_cast<void*>(this), 0x00, sizeof *this);
}

hxattr_nodiscard bool hxfile::is_open(void) const {
	return m_file_pimpl_ != 0;
}

void hxfile::clear(void) {
	m_fail_ = false;
	m_eof_ = false;
	if(m_file_pimpl_ != 0) {
		::clearerr(reinterpret_cast<FILE*>(m_file_pimpl_));
	}
}

size_t hxfile::get_pos(void) const {
	hxassertmsg(m_file_pimpl_ != 0, "invalid_file");
	// Requires a 64-bit long to support 64-bit files.
	return static_cast<size_t>(::ftell(reinterpret_cast<FILE*>(m_file_pimpl_)));
}

bool hxfile::set_pos(size_t position) {
	hxassertmsg(m_file_pimpl_ != 0, "invalid_file");
	// Requires a 64-bit long to support 64-bit files.
	m_fail_ = ::fseek(reinterpret_cast<FILE*>(m_file_pimpl_), static_cast<long>(position), 0) != 0;
	if(!m_fail_) {
		m_eof_ = false;
	}
	return !m_fail_;
}

size_t hxfile::read(void* bytes, size_t buffer_size, size_t byte_count) {
	hxassertmsg(((m_open_mode_ & hxfile::in) != 0u) && (m_file_pimpl_ != 0), "invalid_file");
	hxassert_hard(byte_count <= buffer_size || ((m_open_mode_ & hxfile::skip_asserts) != 0u),
		"read %zu overflows %zu", byte_count, buffer_size);

	if(byte_count > buffer_size) {
		m_fail_ = true;
		return 0u;
	}

	const size_t bytes_read = ::fread(bytes, 1, byte_count, reinterpret_cast<FILE*>(m_file_pimpl_)); // NOLINT(clang-analyzer-core.NonNullParamChecker)

	hxassertmsg((byte_count == bytes_read) || ((m_open_mode_ & hxfile::skip_asserts) != 0u),
		"fread expected %zu != actual %zu: %s", byte_count, bytes_read, ::strerror(errno));

	if(byte_count != bytes_read) {
		m_fail_ = true;
		m_eof_ = (::feof(reinterpret_cast<FILE*>(m_file_pimpl_)) != 0);
	}
	return bytes_read;
}

size_t hxfile::write(const void* bytes, size_t byte_count) {
	hxassertmsg((m_open_mode_ & hxfile::out) != 0u, "invalid_file");

	if(m_file_pimpl_ == 0) {
		// Writing to null emulates /dev/null support.
		return byte_count;
	}
	const size_t bytes_written = ::fwrite(bytes, 1, byte_count, reinterpret_cast<FILE*>(m_file_pimpl_));

	hxassertmsg((byte_count == bytes_written) || ((m_open_mode_ & hxfile::skip_asserts) != 0u),
		"fwrite expected %zu != actual %zu: %s", byte_count, bytes_written, ::strerror(errno));

	// Can restore goodness.
	m_fail_ = byte_count != bytes_written;
	return bytes_written;
}

bool hxfile::flush(void) { // NOLINT(readability-make-member-function-const)
	hxassertmsg((m_open_mode_ & hxfile::out) != 0u, "invalid_file");
	if(m_file_pimpl_ == 0) {
		return true;
	}

	const int result = ::fflush(reinterpret_cast<FILE*>(m_file_pimpl_));
	hxassertmsg((result == 0) || ((m_open_mode_ & hxfile::skip_asserts) != 0u),
		"fflush %s", ::strerror(errno));
	return result == 0;
}

bool hxfile::getline(char* buffer, int buffer_size) {
	hxassertmsg(((m_open_mode_ & hxfile::in) != 0u) && (m_file_pimpl_ != 0), "invalid_file");

	char* result = ::fgets(buffer, buffer_size, reinterpret_cast<FILE*>(m_file_pimpl_)); // NOLINT(clang-analyzer-core.NonNullParamChecker)

	hxassertmsg(!::ferror(reinterpret_cast<FILE*>(m_file_pimpl_)), "fgets %s", ::strerror(errno));

	if(result == hxnull) {
		m_fail_ = true;
		m_eof_ = (::feof(reinterpret_cast<FILE*>(m_file_pimpl_)) != 0); // 0: not past end.
		return false; // EOF or error.
	}
	return true;
}

// See vsnprintf to reimplement this without FILE* support.
bool hxfile::print(const char* format, ...) {
	hxassertmsg((m_open_mode_ & hxfile::out) != 0u, "invalid_file");

	if(m_file_pimpl_ == 0) {
		// Writing to null emulates /dev/null support.
		return true;
	}

	va_list args;
	va_start(args, format);
	const int len = ::vfprintf(reinterpret_cast<FILE*>(m_file_pimpl_), format, args);
	va_end(args);

	hxassert_hard(len >= 0, "vfprintf %s", ::strerror(errno));
	if(len < 0) {
		m_fail_ = true;
		return false;
	}
	return true;
}

// See vscanf to reimplement this without FILE* support.
int hxfile::scan(const char* format, ...) {
	hxassertmsg(((m_open_mode_ & hxfile::in) != 0u) && (m_file_pimpl_ != 0), "invalid_file");
	va_list args;
	va_start(args, format);
	const int items_scanned = ::vfscanf(reinterpret_cast<FILE*>(m_file_pimpl_), format, args); // NOLINT(clang-analyzer-core.NonNullParamChecker)
	va_end(args);

	hxassert_hard(items_scanned != EOF || ((m_open_mode_ & hxfile::skip_asserts) != 0u), "vfscanf %s", ::strerror(errno));

	if(items_scanned == EOF) {
		m_fail_ = true;
		m_eof_ = (::feof(reinterpret_cast<FILE*>(m_file_pimpl_)) != 0);
	}
	return items_scanned;
}

HX_NS_END_
#endif // HX_USE_FILE_IO == 1
