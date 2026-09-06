// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

// This is the POSIX file descriptor version using open/read/write/close.
// Use an alternate .cpp file for alternate implementations.

#include "../include/hx/hxfile.hpp"

#if (HX_USE_FILE_IO) == 2

// These are only dependencies of the POSIX implementation. This is to allow
// easy reimplementation.
#include <fcntl.h>
#include <unistd.h>

#include <stdio.h> // ::vsnprintf

HX_NS_BEGIN_

// In this version the file is a POSIX fd stored directly as intptr_t. -1
// represents closed or hxdev_null; valid fds are ≥ 0.

hxfile hxin(hxfile::open_mode_in, static_cast<intptr_t>(STDIN_FILENO));
hxfile hxout(hxfile::open_mode_out, static_cast<intptr_t>(STDOUT_FILENO));
#ifndef __wasm__
hxfile hxerr(hxfile::open_mode_out, static_cast<intptr_t>(STDERR_FILENO));
#else
// Don't use stdout with the default index.js provided by the emsdk.
hxfile hxerr(hxfile::open_mode_out, static_cast<intptr_t>(STDOUT_FILENO));
#endif
hxfile hxdev_null(hxfile::open_mode_out, static_cast<intptr_t>(-1));

hxfile::hxfile(void) {
	::memset(static_cast<void*>(this), 0x00, sizeof *this);
	m_file_pimpl_ = -1;
}

hxfile::hxfile(uint8_t mode, intptr_t file) {
	::memset(static_cast<void*>(this), 0x00, sizeof *this);
	m_open_mode_ = mode;
	m_file_pimpl_ = file;
}

hxfile::hxfile(uint8_t mode, const char* filename, ...) : hxfile() {
	va_list args;
	va_start(args, filename);
	openv_(mode, filename, args);
	va_end(args);
}

hxfile::hxfile(hxfile&& file) noexcept {
	hxassertf(this != &file, "self_copy");
	::memcpy(static_cast<void*>(this), static_cast<const void*>(&file), sizeof file);
	::memset(static_cast<void*>(&file), 0x00, sizeof file);
	file.m_file_pimpl_ = static_cast<intptr_t>(-1);
}

hxfile::~hxfile(void) {
	close();
}

void hxfile::operator=(hxfile&& file) noexcept {
	hxassertf(this != &file, "self_copy");
	close();
	::memcpy(static_cast<void*>(this), static_cast<const void*>(&file), sizeof file);
	::memset(static_cast<void*>(&file), 0x00, sizeof file);
	file.m_file_pimpl_ = static_cast<intptr_t>(-1);
}

hxfile::operator bool(void) const {
	return (m_file_pimpl_ >= 0) && !m_fail_;
}

bool hxfile::open(uint8_t mode, const char* filename, ...) {
	close();
	va_list args;
	va_start(args, filename);
	const bool rv = openv_(mode, filename, args);
	va_end(args);
	return rv;
}

bool hxfile::openv_(uint8_t mode, const char* filename, va_list args) {
	hxassertf(m_file_pimpl_ < 0, "sys_err");
	m_open_mode_ = mode;

	int flags = 0;
	switch (static_cast<int>(mode) & (hxfile::open_mode_in | hxfile::open_mode_out)) {
	case hxfile::open_mode_none:
		m_fail_ = true;
		return false;
	case hxfile::open_mode_in:
		flags = O_RDONLY;
		break;
	case hxfile::open_mode_out:
		flags = O_WRONLY | O_CREAT | O_TRUNC;
		break;
	default:
		flags = O_RDWR | O_CREAT | O_TRUNC;
	}

	char line_buf[HX_MAX_LINE];
	const int len = ::vsnprintf(line_buf, HX_MAX_LINE, filename, args);
	hxassertf(len >= 0 && len < HX_MAX_LINE, "vsnprintf %d", len); (void)len;

	const int fd = ::open(line_buf, flags, 0666u);
	hxassert_hard((fd >= 0) || ((mode & hxfile::open_mode_asserts) == 0u),
		"open %s: %s", line_buf, ::strerror(errno));

	m_fail_ = (fd < 0);
	m_owns_ = !m_fail_;
	if(!m_fail_) {
		m_file_pimpl_ = static_cast<intptr_t>(fd);
	}
	return !m_fail_;
}

void hxfile::close(void) {
	if(m_owns_) {
		const int code = ::close(static_cast<int>(m_file_pimpl_));
		hxassertf(code == 0, "close %d", code); (void)code;
	}
	::memset(static_cast<void*>(this), 0x00, sizeof *this);
	m_file_pimpl_ = static_cast<intptr_t>(-1);
}

hxattr_nodiscard bool hxfile::is_open(void) const {
	return m_file_pimpl_ >= 0;
}

void hxfile::clear(void) {
	m_fail_ = false;
	m_eof_ = false;
}

size_t hxfile::get_pos(void) const {
	hxassertf(m_file_pimpl_ >= 0, "bad_file %zd", static_cast<hxsize_t>(m_file_pimpl_));
	const off_t result = ::lseek(static_cast<int>(m_file_pimpl_), 0, SEEK_CUR);
	hxassertf(result >= 0 || ((m_open_mode_ & hxfile::open_mode_asserts) == 0u),
		"bad_seek %zd", static_cast<hxsize_t>(result));
	if(result < 0) {
		// m_fail_ = true; // Files that do not support lseek do not set m_fail_.
		return 0u;
	}
	return static_cast<size_t>(result);
}

bool hxfile::set_pos(size_t position) {
	hxassertf(m_file_pimpl_ >= 0, "bad_file %zd", static_cast<hxsize_t>(m_file_pimpl_));
	const off_t result = ::lseek(static_cast<int>(m_file_pimpl_), static_cast<off_t>(position), SEEK_SET);
	hxassertf(result >= 0 || ((m_open_mode_ & hxfile::open_mode_asserts) == 0u),
		"bad_seek %zd position %zu", static_cast<hxsize_t>(result), position);
	m_fail_ = (result < 0);
	if(!m_fail_) {
		m_eof_ = false;
	}
	return !m_fail_;
}

size_t hxfile::read(void* bytes, size_t buffer_size, size_t byte_count) {
	hxassertf(((m_open_mode_ & hxfile::open_mode_in) != 0u) && (m_file_pimpl_ >= 0),
		"bad_file %zd", static_cast<hxsize_t>(m_file_pimpl_));
	hxassert_hard(byte_count <= buffer_size || ((m_open_mode_ & hxfile::open_mode_asserts) == 0u),
		"read want %zu cap %zu", byte_count, buffer_size);

	if(byte_count > buffer_size) {
		m_fail_ = true;
		return 0u;
	}

	uint8_t* dst = static_cast<uint8_t*>(bytes);
	size_t total = 0u;
	ssize_t last_n = 0;
	while(total < byte_count) {
		do {
			last_n = ::read(static_cast<int>(m_file_pimpl_), dst + total, byte_count - total);
		} while(last_n < 0 && errno == EINTR);
		if(last_n <= 0) {
			break;
		}
		total += static_cast<size_t>(last_n);
	}

	hxassertf(total == byte_count || ((m_open_mode_ & hxfile::open_mode_asserts) == 0u),
		"read want %zu got %zu %s", byte_count, total,
		(last_n < 0) ? ::strerror(errno) : "");

	if(total != byte_count) {
		m_fail_ = true;
		m_eof_ = (last_n == 0);
	}
	return total;
}

size_t hxfile::write(const void* bytes, size_t byte_count) {
	hxassertf((m_open_mode_ & hxfile::open_mode_out) != 0u, "bad_file mode %#zx", static_cast<size_t>(m_open_mode_));

	if(m_file_pimpl_ < 0) {
		// Writing to hxdev_null (-1) is a no-op.
		return byte_count;
	}

	const uint8_t* src = static_cast<const uint8_t*>(bytes);
	size_t total = 0u;
	while(total < byte_count) {
		ssize_t n = 0;
		do {
			n = ::write(static_cast<int>(m_file_pimpl_), src + total, byte_count - total);
		} while(n < 0 && errno == EINTR);
		if(n <= 0) {
			break;
		}
		total += static_cast<size_t>(n);
	}

	hxassertf(total == byte_count || ((m_open_mode_ & hxfile::open_mode_asserts) == 0u),
		"write want %zu got %zu %s", byte_count, total, ::strerror(errno));

	m_fail_ = (total != byte_count);
	return total;
}

bool hxfile::flush(void) {
	hxassertf((m_open_mode_ & hxfile::open_mode_out) != 0u, "bad_file mode %#zx", static_cast<size_t>(m_open_mode_));
	// POSIX bypasses userspace buffering so there is nothing to flush.
	return true;
}

// See vsnprintf for the rationale for truncating output at HX_MAX_LINE.
bool hxfile::print(const char* format, ...) {
	hxassertf((m_open_mode_ & hxfile::open_mode_out) != 0u, "bad_file mode %#zx", static_cast<size_t>(m_open_mode_));

	if(m_file_pimpl_ < 0) {
		return true;
	}

	char line_buf[HX_MAX_LINE + 1u];
	va_list args;
	va_start(args, format);
	const int len = ::vsnprintf(line_buf, HX_MAX_LINE + 1u, format, args);
	va_end(args);

	hxassert_hard(len >= 0 || ((m_open_mode_ & hxfile::open_mode_asserts) == 0u),
		"vsnprintf %zd %s", static_cast<hxsize_t>(len), ::strerror(errno));
	if(len < 0) {
		m_fail_ = true;
		return false;
	}

	const size_t to_write = (static_cast<size_t>(len) <= HX_MAX_LINE) ?
		static_cast<size_t>(len) : static_cast<size_t>(HX_MAX_LINE);
	size_t written = 0u;
	while(written < to_write) {
		ssize_t n = 0;
		do {
			n = ::write(static_cast<int>(m_file_pimpl_), line_buf + written, to_write - written);
		} while(n < 0 && errno == EINTR);
		if(n <= 0) {
			break;
		}
		written += static_cast<size_t>(n);
	}
	hxassert_hard(written == to_write || ((m_open_mode_ & hxfile::open_mode_asserts) == 0u),
		"write want %zu got %zu %s", to_write, written, ::strerror(errno));

	if(written != to_write) {
		m_fail_ = true;
		return false;
	}
	return true;
}

HX_NS_END_
#endif // (HX_USE_FILE_IO) == 2
