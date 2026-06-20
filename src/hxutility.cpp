// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

#include "../include/hx/hxutility.h"

extern "C" {

// The non-inline utility functions are written in plain C. That provides an important test.

#if defined __clang__
__attribute__((no_sanitize("address")))
__attribute__((no_sanitize("memory")))
#endif
void hxhex_dump(const void* address, size_t bytes, bool pretty) {
	(void)address; (void)bytes; (void)pretty;
#if (HX_HARDENING_MODE) > HX_HARDENING_MODE_STANDARD
		bytes = (bytes + 15u) & ~(size_t)15; // round up to 16 bytes.
		const volatile uint8_t* addr = reinterpret_cast<const volatile uint8_t*>(address);
		for(size_t i = 0; i < bytes;) {
			if(pretty) {
				// Adjust the number of leading zeros for pointers to match uintptr_t.
				hxlog_console("%0*zx: ", (int)sizeof(uintptr_t), reinterpret_cast<uintptr_t>(addr));
			}
			const volatile uint8_t* str = addr;
			for(size_t maximum = 4u; i < bytes && maximum-- != 0u; i += 4) {
				hxlog_console("%02x%02x%02x%02x ", addr[0], addr[1], addr[2], addr[3]);
				addr += 4;
			}
			if(pretty) {
				while(str < addr) {
					hxlog_console("%c", (*str >= 0x20 && *str <= 0x7e) ? *str : '.');
					++str;
				}
			}
			hxlog_console("\n");
		}
#endif
}

#if defined __clang__
__attribute__((no_sanitize("address")))
__attribute__((no_sanitize("memory")))
#endif
void hxfloat_dump(const float* address, size_t count) {
	(void)address; (void)count;
#if (HX_HARDENING_MODE) > HX_HARDENING_MODE_STANDARD
	for(size_t i = 0; i < count;) {
		hxlog_console("%08x: ", static_cast<unsigned int>(reinterpret_cast<uintptr_t>(address)));
		for(size_t maximum = 4u; i < count && maximum-- != 0u; i++) {
			hxlog_console("%8f ", *address++);
		}
		hxlog_console("\n");
	}
#endif
}

const char* hxbasename(const char* path) {
	for(const char* it = path; *it != '\0'; ++it) {
		if(*it == '/' || *it == '\\') {
			path = it + 1;
		}
	}
	return path;
}

hxattr_noexcept char* hxstring_duplicate(const char* string, enum hxsystem_allocator_t id) {
	const size_t len = strlen(string);
	char* temp = static_cast<char*>(hxmalloc_ext(len + 1, id, 1u));
	::memcpy(temp, string, len + 1);
	return temp;
}

} // extern "C" {
