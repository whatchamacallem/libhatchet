// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.
//
// Hardware facts are from ESP-IDF v6.1.1.

#if !defined ESP_PLATFORM
#error hxboot_port_freertos.cpp implements ESP-IDF only.
#endif

#include "hxboot_port.hpp"

#include <hx/hxutility.h>

#include "driver/usb_serial_jtag.h"
#include "esp_log.h"
#include "esp_rom_sys.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"

#if !CONFIG_IDF_TARGET_ESP32C6
#error hxboot_port_freertos.cpp implements the ESP32-C6 only.
#endif
namespace {

// How long the host may leave bytes untaken before the port treats it as gone.
// The value of ESP-IDF's TX_FLUSH_TIMEOUT_US.
constexpr TickType_t c_port_drain_ticks = pdMS_TO_TICKS(50u);

StaticSemaphore_t s_port_mutex_storage;
SemaphoreHandle_t s_port_mutex;

} // namespace

bool hxboot_port_connected(void) {
	// True while the host sends start of frame packets. Its export from esp_sdk
	// keeps the monitor that watches for them in esp_sdk's link.
	return usb_serial_jtag_is_connected();
}

void hxboot_port_flush(void) {
	if(usb_serial_jtag_is_connected()) {
		usb_serial_jtag_wait_tx_done(c_port_drain_ticks);
	}
}

void hxboot_port_open(void) {
	// Silences the log and ROM printf as CONFIG_ESP_CONSOLE_NONE does.
	esp_log_level_set("*", ESP_LOG_NONE);
	esp_rom_install_channel_putc(1, hxnil);
	esp_rom_install_channel_putc(2, hxnil);

	s_port_mutex = xSemaphoreCreateMutexStatic(&s_port_mutex_storage);
	usb_serial_jtag_driver_config_t config = { HX_BOOT_PORT_BUFFER_SIZE, HX_BOOT_PORT_BUFFER_SIZE };
	const esp_err_t result = usb_serial_jtag_driver_install(&config);
	hxassert_hard(result == ESP_OK, "usb_serial_jtag_driver_install %d", result); (void)result;

	// The connection monitor starts out connected. Its tick hook clears that
	// after USJ_DISCONNECT_CONFIRM_PERIOD_MS, 3 ms, of ticks and one more
	// without a start of frame. The first tick may see one latched before the
	// wait. A tick runs the hook before it ends a delay.
	vTaskDelay(pdMS_TO_TICKS(3u) + 2u);

	// A reply that the previous program gave up on may have arrived. Its
	// sequence may match this program's first request. The driver's interrupt
	// took what the FIFO held during the wait.
	uint8_t stale[64];
	while(usb_serial_jtag_read_bytes(stale, sizeof stale, 0u) != 0) { }
}

size_t hxboot_port_read(void* bytes, size_t count, uint32_t timeout_ms) {
	TimeOut_t timeout;
	vTaskSetTimeOutState(&timeout);
	TickType_t remaining_ticks = pdMS_TO_TICKS(timeout_ms);
	size_t result = 0u;
	while(result < count) {
		uint8_t* const next = static_cast<uint8_t*>(bytes) + result;
		result += static_cast<size_t>(usb_serial_jtag_read_bytes(next,
			static_cast<uint32_t>(count - result), remaining_ticks));
		if(xTaskCheckForTimeOut(&timeout, &remaining_ticks) != pdFALSE) {
			break;
		}
	}
	return result;
}

void hxboot_port_write(const void* bytes, size_t count) {
	const uint8_t* next = static_cast<const uint8_t*>(bytes);
	xSemaphoreTake(s_port_mutex, portMAX_DELAY);
	// A ring buffer send larger than the buffer fails.
	while(count != 0u && usb_serial_jtag_is_connected()) {
		const size_t chunk = hxmin<size_t>(count, HX_BOOT_PORT_BUFFER_SIZE);
		if(usb_serial_jtag_write_bytes(next, chunk, c_port_drain_ticks) == 0) {
			break;
		}
		next += chunk;
		count -= chunk;
	}
	xSemaphoreGive(s_port_mutex);
}
