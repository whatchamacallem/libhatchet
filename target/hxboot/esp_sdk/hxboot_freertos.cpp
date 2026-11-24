// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

#if !defined ESP_PLATFORM
#error hxboot_freertos.cpp implements ESP-IDF only.
#endif

#include "hxboot_main.hpp"
#include "hxboot_platform.hpp"
#include "hxboot_port.hpp"
#include "esp_sdk.hpp"

#include <hx/hxutility.h>

#include "esp_app_desc.h"
#include "esp_partition.h"

#if !CONFIG_IDF_TARGET_ESP32C6
#error hxboot_freertos.cpp implements the ESP32-C6 only.
#endif

size_t hxboot_flash_read(size_t offset, void* bytes, size_t count) {
	const esp_partition_t* const gold_partition = esp_partition_find_first(
		static_cast<esp_partition_type_t>(c_esp_sdk_partition_type),
		static_cast<esp_partition_subtype_t>(c_esp_sdk_partition_subtype_gold), hxnil);
	hxassertf(gold_partition != hxnull, "hxboot_flash_read: No gold partition.");
	hxassertf(offset <= gold_partition->size, "hxboot_flash_read: Offset %u past the slot.",
		static_cast<unsigned int>(offset));
	count = hxmin<size_t>(count, gold_partition->size - offset);
	const esp_err_t result = esp_partition_read(gold_partition, offset, bytes, count);
	return result == ESP_OK ? count : 0u;
}

void hxboot_launch(const hxboot_image_header& header) {
	esp_sdk_launch(header);
}

hxboot_memory hxboot_memory_map(void) {
	// hxboot runs in the slice at the top of the client region and an image may
	// use the rest of it.
	const uintptr_t client_region = reinterpret_cast<uintptr_t>(esp_sdk_client_region);
	hxboot_memory boot_memory = {
		"esp32c6",
		static_cast<uint32_t>(client_region),
		static_cast<uint32_t>(client_region + ESP_SDK_CLIENT_REGION_SIZE - ESP_SDK_BOOT_SIZE),
		{ }
	};

	::memcpy(boot_memory.esp_sdk_hash, esp_app_get_description()->app_elf_sha256,
		sizeof boot_memory.esp_sdk_hash);
	return boot_memory;
}

extern "C" void esp_sdk_client_main(void) {
	hxboot_port_open();
	hxboot_main();

	// No image was launched.
	hxboot_port_flush();
}
