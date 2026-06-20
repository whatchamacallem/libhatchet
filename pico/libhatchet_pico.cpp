// SPDX-FileCopyrightText: © 2026 Adrian Johnston.
// SPDX-License-Identifier: MIT

// Pico 2 entry point for the libhatchet test suite.

#include <hx/libhatchet.h>
#include <hx/hxconsole.hpp>
#include <hx/hxtest.hpp>

#include <stdio.h>

#include <pico/stdlib.h>
#include <pico/bootrom.h>

HX_NS_USE

static bool run_all_tests(void) {
    hxinit();
    hxlog_console("libhatchet 🪓🪓🪓 " LIBHATCHET_TAG "\n");

    const size_t tests_failing = static_cast<size_t>(RUN_ALL_TESTS());

    hxshutdown();
    return (tests_failing == 0u) ? EXIT_SUCCESS : EXIT_FAILURE;
}

int main() {
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
