// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

// This is the POSIX file descriptor version using open/read/write/close.
// Use an alternate .cpp file for alternate implementations.

#include "../include/hx/hxfile.hpp"

#if HX_USE_POSIX_FILE_IO

// These are only dependencies of the POSIX implementation. This is to allow
// easy reimplementation.
#include <fcntl.h>
#include <unistd.h>

#include <stdio.h> // ::vsnprintf

// In this version the file is a POSIX fd stored directly as intptr_t. -1
// represents closed or hxdev_null; valid fds are >= 0.

hxfile hxin(hxfile::in, static_cast<intptr_t>(STDIN_FILENO));
hxfile hxout(hxfile::out, static_cast<intptr_t>(STDOUT_FILENO));
#ifndef __wasm__
hxfile hxerr(hxfile::out, static_cast<intptr_t>(STDERR_FILENO));
#else
// Don't use stdout with the default index.js provided by the emsdk.
hxfile hxerr(hxfile::out, static_cast<intptr_t>(STDOUT_FILENO));
#endif
hxfile hxdev_null(hxfile::out, static_cast<intptr_t>(-1));

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

hxfile::hxfile(hxfile&& file) {
	::memcpy(static_cast<void*>(this), static_cast<const void*>(&file), sizeof file);
	::memset(static_cast<void*>(&file), 0x00, sizeof file);
	file.m_file_pimpl_ = static_cast<intptr_t>(-1);
}

hxfile::~hxfile(void) {
	close();
}

void hxfile::operator=(hxfile&& file) {
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
	hxassert(m_file_pimpl_ < 0);
	m_open_mode_ = mode;

	int flags = 0;
	switch (static_cast<int>(mode) & (hxfile::in | hxfile::out)) {
	case hxfile::none:
		return false;
	case hxfile::in:
		flags = O_RDONLY;
		break;
	case hxfile::out:
		flags = O_WRONLY | O_CREAT | O_TRUNC;
		break;
	default:
		flags = O_RDWR | O_CREAT | O_TRUNC;
	}

	char line_buf[HX_MAX_LINE];
	const int len = ::vsnprintf(line_buf, HX_MAX_LINE, filename, args);
	hxassertmsg(len >= 0 && len < HX_MAX_LINE, "vsnprintf"); (void)len;

	const int fd = ::open(line_buf, flags, 0666u); // NOLINT
	hxassert_always((fd >= 0) || ((mode & hxfile::skip_asserts) != 0u),
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
		hxassertmsg(code == 0, "close"); (void)code;
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
	hxassertmsg(m_file_pimpl_ >= 0, "invalid_file");
	return static_cast<size_t>(::lseek(static_cast<int>(m_file_pimpl_), 0, SEEK_CUR));
}

bool hxfile::set_pos(size_t position) {
	hxassertmsg(m_file_pimpl_ >= 0, "invalid_file");
	const off_t result = ::lseek(static_cast<int>(m_file_pimpl_), static_cast<off_t>(position), SEEK_SET);
	m_fail_ = (result < 0);
	m_eof_ = m_fail_;
	return !m_fail_;
}

size_t hxfile::read(void* bytes, size_t buffer_size, size_t byte_count) {
	hxassertmsg(((m_open_mode_ & hxfile::in) != 0u) && (m_file_pimpl_ >= 0), "invalid_file");
	hxassert_always(byte_count <= buffer_size || ((m_open_mode_ & hxfile::skip_asserts) != 0u),
		"read %zu overflows %zu", byte_count, buffer_size);

	if(byte_count > buffer_size) {
		m_fail_ = true;
		return 0u;
	}

	uint8_t* dst = static_cast<uint8_t*>(bytes);
	size_t total = 0u;
	ssize_t last_n = 0;
	while(total < byte_count) {
		last_n = ::read(static_cast<int>(m_file_pimpl_), dst + total, byte_count - total);
		if(last_n <= 0) {
			break;
		}
		total += static_cast<size_t>(last_n);
	}

	hxassertmsg(total == byte_count || ((m_open_mode_ & hxfile::skip_asserts) != 0u),
		"read expected %zu != actual %zu: %s", byte_count, total, ::strerror(errno));

	if(total != byte_count) {
		m_fail_ = true;
		m_eof_ = (last_n == 0);
	}
	return total;
}

size_t hxfile::write(const void* bytes, size_t byte_count) {
	hxassertmsg((m_open_mode_ & hxfile::out) != 0u, "invalid_file");

	if(m_file_pimpl_ < 0) {
		// Writing to hxdev_null (-1) is a no-op.
		return byte_count;
	}

	const uint8_t* src = static_cast<const uint8_t*>(bytes);
	size_t total = 0u;
	while(total < byte_count) {
		const ssize_t n = ::write(static_cast<int>(m_file_pimpl_), src + total, byte_count - total);
		if(n <= 0) {
			break;
		}
		total += static_cast<size_t>(n);
	}

	hxassertmsg(total == byte_count || ((m_open_mode_ & hxfile::skip_asserts) != 0u),
		"write expected %zu != actual %zu: %s", byte_count, total, ::strerror(errno));

	m_fail_ = (total != byte_count);
	return total;
}

bool hxfile::flush(void) { // NOLINT
	hxassertmsg((m_open_mode_ & hxfile::out) != 0u, "invalid_file");
	// POSIX bypasses userspace buffering so there is nothing to flush.
	return true;
}

bool hxfile::getline(char* buffer, int buffer_size) {
	hxassertmsg(((m_open_mode_ & hxfile::in) != 0u) && (m_file_pimpl_ >= 0), "invalid_file");

	int written = 0;
	char buf[HX_MAX_LINE];

	for(;;) {
		const ssize_t bytes_read = ::read(static_cast<int>(m_file_pimpl_), buf, HX_MAX_LINE - 1u);

		if(bytes_read <= 0) {
			buffer[written] = '\0';
			if(written > 0 && bytes_read == 0) {
				return true;
			}
			m_fail_ = true;
			m_eof_ = (bytes_read == 0);
			return false;
		}

		const char* const nl = static_cast<const char*>(::memchr(buf, '\n', static_cast<size_t>(bytes_read)));
		const ssize_t line_end = nl ? static_cast<ssize_t>(nl - buf + 1) : bytes_read;
		const int copy_len = (line_end < static_cast<ssize_t>(buffer_size - 1 - written)) ?
			static_cast<int>(line_end) : buffer_size - 1 - written;
		::memcpy(buffer + written, buf, static_cast<size_t>(copy_len));
		written += copy_len;

		if(nl || written == buffer_size - 1) {
			if(static_cast<ssize_t>(copy_len) < bytes_read) {
				::lseek(static_cast<int>(m_file_pimpl_),
					static_cast<off_t>(copy_len) - static_cast<off_t>(bytes_read), SEEK_CUR);
			}
			buffer[written] = '\0';
			return true;
		}
	}
}

// See vsnprintf for the rationale for truncating output at HX_MAX_LINE.
bool hxfile::print(const char* format, ...) { // NOLINT
	hxassertmsg((m_open_mode_ & hxfile::out) != 0u, "invalid_file");

	if(m_file_pimpl_ < 0) {
		return true;
	}

	char buf[HX_MAX_LINE + 1u];
	va_list args;
	va_start(args, format);
	const int len = ::vsnprintf(buf, HX_MAX_LINE + 1u, format, args);
	va_end(args);

	hxassert_always(len >= 0, "vsnprintf %s", ::strerror(errno));
	if(len < 0) {
		m_fail_ = true;
		return false;
	}

	const size_t to_write = (static_cast<size_t>(len) <= HX_MAX_LINE) ?
		static_cast<size_t>(len) : static_cast<size_t>(HX_MAX_LINE);
	size_t written = 0u;
	while(written < to_write) {
		const ssize_t n = ::write(static_cast<int>(m_file_pimpl_), buf + written, to_write - written);
		if(n <= 0) {
			break;
		}
		written += static_cast<size_t>(n);
	}
	hxassert_always(written == to_write, "write %s", ::strerror(errno));

	if(written != to_write) {
		m_fail_ = true;
		return false;
	}
	return true;
}

// See vsscanf for the rationale. scan() consumes up to HX_MAX_LINE-1 bytes from
// the fd to provide a buffer for vsscanf. Unconsumed bytes are not returned to
// the fd; interleaved read()/scan() calls must account for this.
int hxfile::scan(const char* format, ...) {
	hxassertmsg(((m_open_mode_ & hxfile::in) != 0u) && (m_file_pimpl_ >= 0), "invalid_file");

	char buf[HX_MAX_LINE];
	const ssize_t bytes_read = ::read(static_cast<int>(m_file_pimpl_), buf, HX_MAX_LINE - 1u);
	if(bytes_read > 0) {
		buf[static_cast<size_t>(bytes_read)] = '\0';
	} else {
		buf[0] = '\0';
	}

	va_list args;
	va_start(args, format);
	const int items_scanned = ::vsscanf(buf, format, args);
	va_end(args);

	hxassert_always(items_scanned != EOF || ((m_open_mode_ & hxfile::skip_asserts) != 0u),
		"vsscanf %s", ::strerror(errno));

	if(items_scanned == EOF) {
		m_fail_ = true;
		m_eof_ = (bytes_read == 0);
	}
	return items_scanned;
}

#endif // HX_USE_POSIX_FILE_IO
