#pragma once
// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

// What esp_sdk shares with its clients, hxboot and esp_worm. esp_sdk is the
// factory app. A client is a RAM image linked against esp_sdk's addresses that
// runs in the client region R.

#include "hxboot_image.hpp"

#if !defined ESP_SDK_CLIENT_REGION_SIZE || !defined ESP_SDK_BOOT_SIZE
#error ESP_SDK_CLIENT_REGION_SIZE and ESP_SDK_BOOT_SIZE come from esp_sdk/CMakeLists.txt.
#endif

// The custom partition type of the slots in each board's partitions.csv.
hxinline_constexpr uint8_t c_esp_sdk_partition_type = 0x40u;

// The slot esp_sdk loads hxboot from.
hxinline_constexpr uint8_t c_esp_sdk_partition_subtype_boot = 0x00u;

// The slot hxboot launches when no image is served over USB.
hxinline_constexpr uint8_t c_esp_sdk_partition_subtype_gold = 0x01u;

extern "C" {

// R. Nothing initializes it, so it survives esp_restart. hxboot runs in its
// last ESP_SDK_BOOT_SIZE bytes and an image is loaded below them.
extern uint8_t esp_sdk_client_region[ESP_SDK_CLIENT_REGION_SIZE];

// Saves a valid header of an image below hxboot in R whose payload is at its
// load address and restarts. esp_sdk then zeroes the image's bss, adds the rest
// of R to the heap and enters the image once.
hxattr_noreturn void esp_sdk_launch(const hxboot_image_header& header_);

// Each client defines it. It is called on esp_sdk's main task after the
// client's constructors.
void esp_sdk_client_main(void);

// The entry of a client.
void esp_sdk_client_start(void);

} // extern "C"
