// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

#include <stdio.h>

#include <pico/stdlib.h>
#include <pico/bootrom.h>

void target_start(void) {
	stdio_init_all();
	while(!stdio_usb_connected()) {
		sleep_ms(100);
	}
	// Terminals can lose what is sent right after connecting and the TAP
	// version line comes first.
	sleep_ms(250);
}

// Reboots into BOOTSEL so that the next build can be flashed without the
// button.
void target_finish(void) {
	::puts("# Rebooting into BOOTSEL...");
	stdio_flush();
	reset_usb_boot(0, 0);
}
