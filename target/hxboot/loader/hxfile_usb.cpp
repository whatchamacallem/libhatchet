// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

// This is the hxboot version. Named files are read only and are served by the
// host over the link.

#include <hx/hxfile.hpp>

//#if (HX_USE_FILE_IO) == 3

#include <hx/hxutility.h>

#include "hxboot_crc.hpp"
#include "hxboot_link.hpp"
#include "hxboot_port.hpp"

#include <stdio.h>

#if !defined HX_BOOT_FILE_MAX
#define HX_BOOT_FILE_MAX 4u
#endif

HX_NS_BEGIN_

namespace {

constexpr uint32_t c_boot_crc_table[16] = {
	0x00000000u, 0x1db71064u, 0x3b6e20c8u, 0x26d930acu,
	0x76dc4190u, 0x6b6b51f4u, 0x4db26158u, 0x5005713cu,
	0xedb88320u, 0xf00f9344u, 0xd6d6a3e8u, 0xcb61b38cu,
	0x9b64c2b0u, 0x86d3d2d4u, 0xa00ae278u, 0xbdbdf21cu
};

constexpr uint16_t c_boot_link_magic = 0x4c48u;

class hxboot_link_header {
public:
	uint16_t magic;
	uint16_t sequence;
	uint16_t status;
	uint16_t payload_size;
};

static_assert(sizeof(hxboot_link_header) == 8u, "hxboot_link_header is sent");

class hxboot_open_file {
public:
	char name[HX_BOOT_LINK_NAME_MAX + 1u];
	size_t size;
	size_t position;
};

uint16_t s_link_sequence;

hxboot_open_file s_open_files[HX_BOOT_FILE_MAX];

// It is empty, so reading hxin fails at its end.
hxboot_open_file s_port_file;

} // namespace {

hxfile hxin(hxfile::open_mode_in, reinterpret_cast<intptr_t>(&s_port_file));
hxfile hxout(hxfile::open_mode_out, reinterpret_cast<intptr_t>(&s_port_file));
hxfile hxerr(hxfile::open_mode_out, reinterpret_cast<intptr_t>(&s_port_file));
hxfile hxdev_null(hxfile::open_mode_out, static_cast<intptr_t>(0));

uint32_t hxboot_crc32(const void* bytes, size_t count, uint32_t crc) {
	hxassertf(bytes != hxnull, "hxboot_crc32 null");
	const uint8_t* hxrestrict src = static_cast<const uint8_t*>(bytes);
	const uint8_t* const end = src + count;
	crc = ~crc;
	while(src != end) {
		crc ^= static_cast<uint32_t>(*src++);
		crc = (crc >> 4) ^ c_boot_crc_table[crc & 15u];
		crc = (crc >> 4) ^ c_boot_crc_table[crc & 15u];
	}
	return ~crc;
}

hxboot_link_status hxboot_link_call(void* reply, size_t reply_capacity, size_t& reply_size,
		const char* format, ...) {
	hxassertf(reply != hxnull && format != hxnull, "hxboot_link_call null");

	// The line is "# hxlink ssss operation cccccccc\n". The operation is
	// formatted once at offset 14. Each attempt writes the text before it with
	// a new sequence and then the CRC of the text from the sequence on. The
	// longest operation is "read", a name and two size_t in hex.
	static_assert(14u + 5u + HX_BOOT_LINK_NAME_MAX + 2u * (1u + 2u * sizeof(size_t)) + 11u
		<= HX_MAX_LINE, "HX_MAX_LINE is shorter than a hxlink request");
	char line[HX_MAX_LINE];
	va_list args; // NOLINT(cppcoreguidelines-init-variables)
	va_start(args, format);
	const int operation_size = ::vsnprintf(line + 14, HX_MAX_LINE - 24, format, args);
	va_end(args);
	hxassertf(operation_size > 0 && operation_size < HX_MAX_LINE - 24, "hxlink %s", format);
	char* const crc_text = line + 14 + operation_size;

	for(uint32_t attempt = 0u; attempt < HX_BOOT_LINK_ATTEMPTS && hxboot_port_connected(); ++attempt) {
		const uint16_t sequence = s_link_sequence++;
		::snprintf(line, 14u, "# hxlink %04x", static_cast<unsigned int>(sequence));
		line[13] = ' ';
		const uint32_t request_crc = hxboot_crc32(line + 9,
			static_cast<size_t>(crc_text - (line + 9)));
		::snprintf(crc_text, 11u, " %08x\n", static_cast<unsigned int>(request_crc));
		hxboot_port_write(line, static_cast<size_t>(crc_text - line) + 10u);

		hxboot_link_header header;
		uint8_t* const window = reinterpret_cast<uint8_t*>(&header);
		size_t window_size = 0u;
		for(;;) {
			const size_t wanted = sizeof header - window_size;
			if(hxboot_port_read(window + window_size, wanted, HX_BOOT_LINK_TIMEOUT_MS) != wanted) {
				break;
			}
			const size_t payload_size = header.payload_size;
			if(header.magic != c_boot_link_magic || header.sequence != sequence
					|| payload_size > reply_capacity || payload_size > HX_BOOT_LINK_CHUNK) {
				::memmove(window, window + 1, sizeof header - 1u);
				window_size = sizeof header - 1u;
				continue;
			}
			uint32_t reply_crc = 0u;
			if(hxboot_port_read(reply, payload_size, HX_BOOT_LINK_TIMEOUT_MS) != payload_size
					|| hxboot_port_read(&reply_crc, sizeof reply_crc, HX_BOOT_LINK_TIMEOUT_MS)
						!= sizeof reply_crc) {
				break;
			}
			if(reply_crc == hxboot_crc32(reply, payload_size, hxboot_crc32(&header, sizeof header))) {
				hxassertf(header.status < hxboot_link_status_no_server, "hxlink status %u",
					static_cast<unsigned int>(header.status));
				reply_size = payload_size;
				return static_cast<hxboot_link_status>(header.status);
			}
			window_size = 0u;
		}
	}
	return hxboot_link_status_no_server;
}

hxfile::hxfile(void) noexcept {
	::memset(static_cast<void*>(this), 0x00, sizeof *this);
}

hxfile::hxfile(uint8_t mode, intptr_t file) noexcept : hxfile() {
	m_file_pimpl_ = file;
	m_open_mode_ = mode;
}

hxfile::hxfile(uint8_t mode, const char* filename, ...) : hxfile() {
	va_list args; // NOLINT(cppcoreguidelines-init-variables)
	va_start(args, filename);
	openv_(mode, filename, args);
	va_end(args);
}

hxfile::hxfile(hxfile&& file) noexcept {
	hxassertf(this != &file, "self_copy");
	::memcpy(static_cast<void*>(this), static_cast<const void*>(&file), sizeof file);
	::memset(static_cast<void*>(&file), 0x00, sizeof file);
}

hxfile::~hxfile() {
	close();
}

void hxfile::operator=(hxfile&& file) noexcept {
	hxassertf(this != &file, "self_copy");
	close();
	::memcpy(static_cast<void*>(this), static_cast<const void*>(&file), sizeof file);
	::memset(static_cast<void*>(&file), 0x00, sizeof file);
}

hxfile::operator bool(void) const {
	return (m_file_pimpl_ != 0) && !m_fail_;
}

bool hxfile::open(uint8_t mode, const char* filename, ...) {
	close();

	va_list args; // NOLINT(cppcoreguidelines-init-variables)
	va_start(args, filename);
	const bool result = openv_(mode, filename, args);
	va_end(args);
	return result;
}

bool hxfile::openv_(uint8_t mode, const char* filename, va_list args) {
	hxassertf(m_file_pimpl_ == 0, "sys_err");

	m_open_mode_ = mode;

	const int access = mode & (hxfile::open_mode_in | hxfile::open_mode_out);
	if(access != hxfile::open_mode_in) {
		hxassert_hard(access == hxfile::open_mode_none || (mode & hxfile::open_mode_asserts) == 0u,
			"read_only %s", filename);
		m_fail_ = true;
		return false;
	}

	hxboot_open_file* file = s_open_files;
	hxboot_open_file* const files_end = s_open_files + HX_BOOT_FILE_MAX;
	while(file != files_end && file->name[0] != '\0') {
		++file;
	}
	hxassert_hard(file != files_end || (mode & hxfile::open_mode_asserts) == 0u,
		"HX_BOOT_FILE_MAX %s", filename);
	if(file == files_end) {
		m_fail_ = true;
		return false;
	}

	const int name_size = ::vsnprintf(file->name, sizeof file->name, filename, args);
	hxassertf(name_size > 0 && name_size <= static_cast<int>(HX_BOOT_LINK_NAME_MAX),
		"hxlink name %s", file->name); (void)name_size;
	for(const char* it = file->name; *it != '\0'; ++it) {
		hxassertf(*it > ' ' && *it <= '~', "hxlink name %s", file->name);
	}

	uint32_t size = 0u;
	size_t reply_size = 0u;
	const hxboot_link_status status = hxboot_link_call(&size, sizeof size, reply_size,
		"size %s", file->name);
	const bool found = (status == hxboot_link_status_ok) && (reply_size == sizeof size);
	hxassert_hard(found || (mode & hxfile::open_mode_asserts) == 0u,
		"hxlink size %s status %u", file->name, static_cast<unsigned int>(status));
	if(!found) {
		file->name[0] = '\0';
		m_fail_ = true;
		return false;
	}

	file->size = size;
	file->position = 0u;
	m_file_pimpl_ = reinterpret_cast<intptr_t>(file);
	m_owns_ = true;
	return true;
}

void hxfile::close(void) {
	if(m_owns_) {
		reinterpret_cast<hxboot_open_file*>(m_file_pimpl_)->name[0] = '\0';
	}
	::memset(static_cast<void*>(this), 0x00, sizeof *this);
}

bool hxfile::is_open(void) const {
	return m_file_pimpl_ != 0;
}

void hxfile::clear(void) {
	m_fail_ = false;
	m_eof_ = false;
}

size_t hxfile::get_pos(void) const {
	hxassertf(m_file_pimpl_ != 0, "bad_file");
	return reinterpret_cast<const hxboot_open_file*>(m_file_pimpl_)->position;
}

bool hxfile::set_pos(size_t position) {
	hxassertf(m_file_pimpl_ != 0, "bad_file");
	hxboot_open_file& file = *reinterpret_cast<hxboot_open_file*>(m_file_pimpl_);
	hxassertf(position <= file.size || (m_open_mode_ & hxfile::open_mode_asserts) == 0u,
		"bad_seek %zu size %zu", position, file.size);
	m_fail_ = position > file.size;
	if(!m_fail_) {
		file.position = position;
		m_eof_ = false;
	}
	return !m_fail_;
}

// Requests stop at the end of the file, so the end costs no request.
size_t hxfile::read(void* bytes, size_t buffer_size, size_t byte_count) {
	hxassertf(((m_open_mode_ & hxfile::open_mode_in) != 0u) && (m_file_pimpl_ != 0), "bad_file");
	hxassert_hard(byte_count <= buffer_size || ((m_open_mode_ & hxfile::open_mode_asserts) == 0u),
		"read want %zu cap %zu", byte_count, buffer_size);

	if(byte_count > buffer_size) {
		m_fail_ = true;
		return 0u;
	}

	hxboot_open_file& file = *reinterpret_cast<hxboot_open_file*>(m_file_pimpl_);
	uint8_t* const dst = static_cast<uint8_t*>(bytes);
	const size_t wanted = hxmin(byte_count, file.size - file.position);
	size_t total = 0u;
	while(total < wanted) {
		const size_t count = hxmin<size_t>(wanted - total, HX_BOOT_LINK_CHUNK);
		size_t reply_size = 0u;
		if(hxboot_link_call(dst + total, count, reply_size, "read %s %zx %zx",
				file.name, file.position, count) != hxboot_link_status_ok) {
			break;
		}
		total += reply_size;
		file.position += reply_size;
		if(reply_size != count) {
			break;
		}
	}

	hxassertf((total == byte_count) || ((m_open_mode_ & hxfile::open_mode_asserts) == 0u),
		"hxlink read %s want %zu got %zu", file.name, byte_count, total);

	if(total != byte_count) {
		m_fail_ = true;
		m_eof_ = (file.position == file.size);
	}
	return total;
}

size_t hxfile::write(const void* bytes, size_t byte_count) {
	hxassertf((m_open_mode_ & hxfile::open_mode_out) != 0u, "bad_file");

	if(m_file_pimpl_ == 0) {
		return byte_count;
	}
	hxboot_port_write(bytes, byte_count);

	// hxout and hxerr are shared by concurrent logging. Only a change is stored.
	if(m_fail_) {
		m_fail_ = false;
	}
	return byte_count;
}

bool hxfile::flush(void) { // NOLINT(readability-make-member-function-const)
	hxassertf((m_open_mode_ & hxfile::open_mode_out) != 0u, "bad_file");
	if(m_file_pimpl_ != 0) {
		hxboot_port_flush();
	}
	return true;
}

// One write keeps a line whole.
bool hxfile::print(const char* format, ...) {
	hxassertf((m_open_mode_ & hxfile::open_mode_out) != 0u, "bad_file");

	if(m_file_pimpl_ == 0) {
		return true;
	}

	char line[HX_MAX_LINE];
	va_list args; // NOLINT(cppcoreguidelines-init-variables)
	va_start(args, format);
	const int len = ::vsnprintf(line, HX_MAX_LINE, format, args);
	va_end(args);

	hxassert_hard(len >= 0 || ((m_open_mode_ & hxfile::open_mode_asserts) == 0u),
		"vsnprintf %d", len);
	if(len < 0) {
		m_fail_ = true;
		return false;
	}
	hxboot_port_write(line, hxmin(static_cast<size_t>(len), sizeof line - 1u));
	return true;
}

HX_NS_END_
//#endif // (HX_USE_FILE_IO) == 3
