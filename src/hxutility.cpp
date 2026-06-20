// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

#include "../include/hx/hxutility.h"

extern "C" {

const char* hxbasename(const char* path) {
	for(const char* it = path; *it != '\0'; ++it) {
		if(*it == '/' || *it == '\\') {
			path = it + 1;
		}
	}
	return path;
}

#if defined __clang__
__attribute__((no_sanitize("address")))
__attribute__((no_sanitize("memory")))
#endif
void hxhex_dump(const void* address, size_t bytes, bool pretty) {
	bytes = (bytes + 15u) & ~(size_t)15; // round up to 16 bytes.
	const volatile uint8_t* addr = reinterpret_cast<const volatile uint8_t*>(address);
	for(size_t i = 0; i < bytes;) {
		if(pretty) {
			// Adjust the number of leading zeros for pointers to match uintptr_t.
			hxlog_handler(hxlog_level_console, "%0*zx: ", (int)sizeof(uintptr_t), reinterpret_cast<uintptr_t>(addr));
		}
		const volatile uint8_t* str = addr;
		for(size_t maximum = 4u; i < bytes && maximum-- != 0u; i += 4) {
			hxlog_handler(hxlog_level_console, "%02x%02x%02x%02x ", addr[0], addr[1], addr[2], addr[3]);
			addr += 4;
		}
		if(pretty) {
			while(str < addr) {
				hxlog_handler(hxlog_level_console, "%c", (*str >= 0x20 && *str <= 0x7e) ? *str : '.');
				++str;
			}
		}
		hxlog_handler(hxlog_level_console, "\n");
	}
}

#if defined __clang__
__attribute__((no_sanitize("address")))
__attribute__((no_sanitize("memory")))
#endif
void hxfloat_dump(const float* address, size_t count) {
	for(size_t i = 0; i < count;) {
		hxlog_handler(hxlog_level_console, "%08x: ", static_cast<unsigned int>(reinterpret_cast<uintptr_t>(address)));
		for(size_t maximum = 4u; i < count && maximum-- != 0u; i++) {
			hxlog_handler(hxlog_level_console, "%8f ", *address++);
		}
		hxlog_handler(hxlog_level_console, "\n");
	}
}

} // extern "C" {
