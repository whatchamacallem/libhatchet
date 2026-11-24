// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.
//
// The entry of every client of esp_sdk. esp_sdk has zeroed the client's bss.

#include "esp_sdk.hpp"

// The linker script places .preinit_array and .init_array between these.
extern "C" void (* const esp_sdk_client_init_array_begin[])(void);
extern "C" void (* const esp_sdk_client_init_array_end[])(void);

void esp_sdk_client_start(void) {
	for(void (* const* init_function)(void) = esp_sdk_client_init_array_begin;
			init_function != esp_sdk_client_init_array_end; ++init_function) {
		(*init_function)();
	}
	esp_sdk_client_main();
}
