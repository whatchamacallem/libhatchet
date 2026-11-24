/* SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
 * SPDX-License-Identifier: MIT
 * This file is licensed under the terms of the LICENSE.md file.
 *
 * The ROM loads the 2nd stage bootloader at every reset. Its iram_seg starts at
 * 0x4086B910, the end of its usable DRAM at 0x4087C610 less its stack overhead
 * and the lengths of its dram_seg, iram_loader_seg and iram_seg
 * (esp32c6/bootloader.memory.ld.in of ESP-IDF v6.1.1). .noinit holds R and
 * lies between .dram0.data and .dram0.bss. It must end below the bootloader or
 * a reset overwrites R.
 */

ASSERT(_noinit_end <= 0x4086B910, "esp_sdk.ld: .noinit overlaps the bootloader at 0x4086B910. Reduce ESP_SDK_CLIENT_REGION_SIZE.")
