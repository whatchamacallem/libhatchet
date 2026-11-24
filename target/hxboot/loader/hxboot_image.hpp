#pragma once
// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

#include <hx/libhatchet.h>
// "HXB2" as a little endian word.
hxinline_constexpr uint32_t c_boot_image_magic = 0x32425848u;

class hxboot_memory {
public:
	// The board name, NUL padded.
	char target[16];
	uint32_t ram_begin;
	// One past the last byte an image may occupy, bss included.
	uint32_t ram_end;
	// The first 8 bytes of the SHA256 of the running esp_sdk ELF.
	uint8_t esp_sdk_hash[8];
};

// The first 52 bytes of a .hxb file as stored.
class hxboot_image_header {
public:
	hxattr_nodiscard bool loadable(const hxboot_memory& memory_) const;

	// Returns true when magic and header_crc are correct.
	hxattr_nodiscard bool valid(void) const;

	// Returns true when payload_crc is the CRC32 of the payload at
	// load_address.
	hxattr_nodiscard bool verified(void) const;

	uint32_t magic;
	// CRC32 of the fields that follow it.
	uint32_t header_crc;
	// The board name, NUL padded.
	char target[16];
	uint32_t load_address;
	uint32_t entry_address;
	// Bytes stored after the header.
	uint32_t payload_size;
	// The payload and the bss after it.
	uint32_t image_size;
	uint32_t payload_crc;
	// The first 8 bytes of the SHA256 of the esp_sdk ELF the image links
	// against.
	uint8_t esp_sdk_hash[8];
};

static_assert(sizeof(hxboot_image_header) == 52u, "hxboot_image_header is stored");
