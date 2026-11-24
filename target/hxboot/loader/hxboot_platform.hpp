#pragma once
// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

#include "hxboot_image.hpp"
// Returns the number of bytes read from the gold image slot at offset, which is
// less than count at the end of the slot.
size_t hxboot_flash_read(size_t offset_, void* bytes_, size_t count_);

// Restarts the chip through esp_sdk, which zeroes the image's bss and enters
// it. The payload stays at its load address across the restart. The header
// must be valid and loadable and its payload verified.
hxattr_noreturn void hxboot_launch(const hxboot_image_header& header_);

// Returns the board name, the load window and the esp_sdk hash that an image
// must match.
hxboot_memory hxboot_memory_map(void);
