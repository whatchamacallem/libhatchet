// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.
//
// Hardware facts are from ESP-IDF v6.1.1.
//
// esp_sdk does not link libhatchet. Its headers provide types and macros only.

#include "esp_sdk.hpp"

#include "esp32c6/rom/rtc.h"
#include "esp_app_desc.h"
#include "esp_heap_caps_init.h"
#include "esp_log.h"
#include "esp_partition.h"
#include "esp_rom_crc.h"
#include "esp_system.h"

// ESP-IDF locks the PMP at boot. With memory protection the RX part of HP SRAM
// can't be written and the RW part can't execute until reset, so no client can
// run from R, which is in the RW part. ESP-TEE runs the application in user
// mode.
#if CONFIG_ESP_SYSTEM_MEMPROT || CONFIG_SECURE_ENABLE_TEE
#error esp_sdk requires CONFIG_ESP_SYSTEM_MEMPROT=n and CONFIG_SECURE_ENABLE_TEE=n.
#endif
namespace {

constexpr const char* c_esp_sdk_log_tag = "esp_sdk";

// header_crc covers the header from target on.
constexpr size_t c_esp_sdk_header_fields_offset = offsetof(hxboot_image_header, target);
constexpr uint32_t c_esp_sdk_header_fields_size
	= static_cast<uint32_t>(sizeof(hxboot_image_header) - c_esp_sdk_header_fields_offset);

constexpr char c_esp_sdk_target[sizeof(hxboot_image_header::target)] = "esp32c6";

// esp_restart on the C6 resets CPU 0 alone, which clears no SRAM. The
// bootloader then loads esp_sdk's sections again, but not .noinit, which is
// NOLOAD.
__NOINIT_ATTR hxboot_image_header s_launch_record;

} // namespace

// The ROM loads the bootloader at every reset. A link time assert keeps R
// below it.
alignas(16) __NOINIT_ATTR uint8_t esp_sdk_client_region[ESP_SDK_CLIENT_REGION_SIZE];

void esp_sdk_launch(const hxboot_image_header& header) {
	assert(header.load_address >= reinterpret_cast<uintptr_t>(esp_sdk_client_region)
		&& header.load_address + header.image_size <= reinterpret_cast<uintptr_t>(
			esp_sdk_client_region + ESP_SDK_CLIENT_REGION_SIZE - ESP_SDK_BOOT_SIZE));
	s_launch_record = header;

	// Keeps the ROM's boot messages out of the TAP document. LP_AON_STORE4
	// keeps the setting until power-on, so app_main clears it.
	rtc_suppress_rom_log();
	esp_restart();
}

extern "C" void app_main(void) {
	// The ROM logs the reset after a crash. Both halves of LP_AON_STORE4 hold
	// the XTAL frequency, so bits 0 and 16 are cleared together.
	REG_CLR_BIT(LP_AON_STORE4_REG, RTC_DISABLE_ROM_LOG);

	// A launch record is used once. A crash or reset of the image it launched
	// returns to hxboot.
	hxboot_image_header header = s_launch_record;
	s_launch_record.magic = 0u;

	const uint8_t* const header_fields = reinterpret_cast<const uint8_t*>(&header)
		+ c_esp_sdk_header_fields_offset;
	const bool launched = header.magic == c_boot_image_magic
		&& header.header_crc == esp_rom_crc32_le(0u, header_fields, c_esp_sdk_header_fields_size);
	if(!launched) {
		// hxboot's slot holds a header at offset 0 and the payload after it.
		uint8_t* const boot = esp_sdk_client_region + ESP_SDK_CLIENT_REGION_SIZE
			- ESP_SDK_BOOT_SIZE;
		const esp_partition_t* const slot = esp_partition_find_first(
			static_cast<esp_partition_type_t>(c_esp_sdk_partition_type),
			static_cast<esp_partition_subtype_t>(c_esp_sdk_partition_subtype_boot), hxnull);
		const char* failure = hxnull;
		if(slot == hxnull) {
			failure = "no slot";
		}
		else if(esp_partition_read(slot, 0u, &header, sizeof header) != ESP_OK) {
			failure = "header not read";
		}
		else if(header.magic != c_boot_image_magic || header.header_crc
				!= esp_rom_crc32_le(0u, header_fields, c_esp_sdk_header_fields_size)) {
			failure = "header not valid";
		}
		else if(::memcmp(header.target, c_esp_sdk_target, sizeof header.target) != 0) {
			failure = "target not esp32c6";
		}
		else if(header.load_address != reinterpret_cast<uintptr_t>(boot)) {
			failure = "load address not hxboot's";
		}
		else if(header.payload_size > header.image_size
				|| header.image_size > ESP_SDK_BOOT_SIZE) {
			failure = "size not hxboot's";
		}
		else if(header.entry_address - header.load_address >= header.payload_size) {
			failure = "entry outside the payload";
		}
		else if(::memcmp(header.esp_sdk_hash, esp_app_get_description()->app_elf_sha256,
				sizeof header.esp_sdk_hash) != 0) {
			failure = "linked against another esp_sdk";
		}
		else if(esp_partition_read(slot, sizeof header, boot, header.payload_size) != ESP_OK) {
			failure = "payload not read";
		}
		else if(esp_rom_crc32_le(0u, boot, header.payload_size) != header.payload_crc) {
			failure = "payload not verified";
		}
		if(failure != hxnull) {
			// The log goes to UART0. Returning idles instead of resetting into
			// the same failure.
			ESP_LOGE(c_esp_sdk_log_tag, "hxboot slot: %s. R is at %p.", failure,
				static_cast<void*>(esp_sdk_client_region));
			return;
		}
	}

	uint8_t* const load = reinterpret_cast<uint8_t*>(header.load_address);
	::memset(load + header.payload_size, 0, header.image_size - header.payload_size);

	if(launched) {
		// The rest of R is heap while the image runs. heap_caps_add_region
		// takes the caps of the region the start is in.
		const uintptr_t image_end = header.load_address + header.image_size;
		const intptr_t heap_begin = static_cast<intptr_t>((image_end + 15u) & ~uintptr_t{15u});
		const intptr_t region_end = reinterpret_cast<intptr_t>(esp_sdk_client_region
			+ ESP_SDK_CLIENT_REGION_SIZE);
		ESP_ERROR_CHECK(heap_caps_add_region(heap_begin, region_end));
	}

	// The C6 caches flash only, so HP SRAM needs no cache maintenance.
	// Zifencei is in the C6 -march.
	asm volatile("fence.i" : : : "memory");

	void (*const entry)(void) = reinterpret_cast<void (*)(void)>(header.entry_address);
	entry();
}
