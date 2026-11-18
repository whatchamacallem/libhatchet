// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

#include "../include/hx/hxutility.h"

#if HX_USE_FLOATING_POINT_TRAPS
#include <fenv.h>
#endif

extern "C" {

#if defined __GNUC__
__attribute__((no_sanitize("address")))
#endif
#if defined __clang__
__attribute__((no_sanitize("memory")))
#endif
void hxhex_view(const void* address, size_t bytes, bool pretty) {
	const int width = static_cast<int>(sizeof(size_t) * 2u);
	const volatile uint8_t* addr = reinterpret_cast<const volatile uint8_t*>(address);
	char text[17] = { '\0' };
	while(bytes != 0u) {
		const size_t step = bytes < 16u ? bytes : 16u;
		// Intentionally reads past the end of the array.
		if(pretty) {
			for(size_t i = 0u; i < step; ++i) {
				const uint8_t c = addr[i];
				text[i] = (c >= 0x20 && c <= 0x7e) ? static_cast<char>(c) : '.';
			}
			text[step] = '\0';
		}
		hxlog_handler(hxlog_level_console,
			"%0*zx: %02x%02x%02x%02x %02x%02x%02x%02x %02x%02x%02x%02x %02x%02x%02x%02x %s",
			width, reinterpret_cast<size_t>(addr),
			addr[0], addr[1],  addr[2],  addr[3],  addr[4],  addr[5],  addr[6],  addr[7],
			addr[8], addr[9], addr[10], addr[11], addr[12], addr[13], addr[14], addr[15], text);
		addr += step;
		bytes -= step;
	}
}

#if defined __GNUC__
__attribute__((no_sanitize("address")))
#endif
#if defined __clang__
__attribute__((no_sanitize("memory")))
#endif
void hxfloat_view(const float* address, size_t count) {
	static const char* const formats[4] = {
		"%0*zx: %a",
		"%0*zx: %a %a",
		"%0*zx: %a %a %a",
		"%0*zx: %a %a %a %a"
	};
	const int width = static_cast<int>(sizeof(size_t) * 2u);
#if HX_USE_FLOATING_POINT_TRAPS
	fenv_t fenv;
	::feholdexcept(&fenv);
#endif
	while(count != 0u) {
		const size_t step = count < 4u ? count : 4u;
		// Intentionally reads past the end of the array.
		hxlog_handler(hxlog_level_console, formats[step - 1u], width,
			reinterpret_cast<size_t>(address), address[0], address[1], address[2], address[3]);
		address += step;
		count -= step;
	}
#if HX_USE_FLOATING_POINT_TRAPS
	::feclearexcept(FE_ALL_EXCEPT);
	::fesetenv(&fenv);
#endif
}

} // extern "C" {
