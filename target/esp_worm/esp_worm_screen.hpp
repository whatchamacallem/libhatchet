#pragma once
// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

// The board's ST7789 LCD on the card's SPI bus, turned to 320x172 to show text
// in X11's misc-fixed 8x13 font at twice its size. Level 1. A task draws each
// row whose text or colors changed, 20 times a second.

#include <hx/libhatchet.h>

constexpr hxsize_t c_esp_worm_screen_columns = 20;

constexpr hxsize_t c_esp_worm_screen_rows = 6;

// Codes are below 128 and those that are not printable show as '?'. Starts as
// spaces.
extern char g_esp_worm_screen_text[c_esp_worm_screen_rows * c_esp_worm_screen_columns];

// Bits 4 to 6 are the background and bits 0 to 2 the foreground. In each, bit 0
// is red, bit 1 green and bit 2 blue at full intensity. Bits 3 and 7 are zero.
// Starts as 0x07, white on black.
extern uint8_t g_esp_worm_screen_colors[c_esp_worm_screen_rows * c_esp_worm_screen_columns];

// Called once on the main task after the card's SPI bus is initialized.
// `frame_function` is called before each frame is drawn.
hxattr_nodiscard bool esp_worm_screen_start(void (*frame_function)(void));
