// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

// Pico 2 entry point for the libhatchet test suite.

#include <hx/libhatchet.h>
#include <hx/hxconsole.hpp>
#include <hx/hxtest.hpp>

#include <stdio.h>

#include <pico/stdlib.h>
#include <pico/bootrom.h>

HX_NS_USE

static int run_all_tests(void) {
    hxinit();
    hxlog_console("libhatchet 🪓🪓🪓 " LIBHATCHET_TAG "\n");

    const size_t tests_failing = static_cast<size_t>(RUN_ALL_TESTS());

    hxshutdown();
    return (tests_failing == 0u) ? EXIT_SUCCESS : EXIT_FAILURE;
}

int main(void) {
    stdio_init_all();

    while (!stdio_usb_connected()) {
        sleep_ms(100);
    }

    int result = run_all_tests();

    puts("Rebooting into BOOTSEL...");
    stdio_flush();
    reset_usb_boot(0, 0);
    return result;
}
