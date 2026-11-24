// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

#include "esp_worm_wifi_link.hpp"

#include <hx/hxutility.h>

#include "esp_event.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
namespace {

// Joining takes a few seconds and DHCP a few more. hxboot's server takes 60
// seconds of silence for a crash.
constexpr uint32_t c_wifi_link_wait_milliseconds = 20000u;

StaticSemaphore_t s_address_semaphore_storage;

void s_wifi_link_event(void* callback_context, esp_event_base_t event_base,
		int32_t event_identifier, void* event_data) {
	(void)event_data;
	if(event_base == IP_EVENT) {
		xSemaphoreGive(static_cast<SemaphoreHandle_t>(callback_context));
		return;
	}
	(void)event_identifier;
	const esp_err_t connect_result = esp_wifi_connect();
	hxassertf(connect_result == ESP_OK, "esp_wifi_connect %d", connect_result);
	(void)connect_result;
}

} // namespace

bool esp_worm_wifi_link_start(void) {
	const SemaphoreHandle_t address_semaphore
		= xSemaphoreCreateBinaryStatic(&s_address_semaphore_storage);
	if(esp_event_loop_create_default() != ESP_OK
			|| esp_netif_create_default_wifi_sta() == hxnil) {
		return false;
	}

	const wifi_init_config_t wifi_settings = WIFI_INIT_CONFIG_DEFAULT();
	if(esp_wifi_init(&wifi_settings) != ESP_OK
			|| esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_START, s_wifi_link_event,
				address_semaphore) != ESP_OK
			|| esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED,
				s_wifi_link_event, address_semaphore) != ESP_OK
			|| esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, s_wifi_link_event,
				address_semaphore) != ESP_OK) {
		return false;
	}

	wifi_config_t station_settings = {};
	static_assert(sizeof ESP_WORM_WIFI_SSID <= sizeof station_settings.sta.ssid + 1u,
		"ESP_WORM_WIFI_SSID is too long.");
	static_assert(sizeof ESP_WORM_WIFI_PASSWORD <= sizeof station_settings.sta.password,
		"ESP_WORM_WIFI_PASSWORD is too long.");
	::memcpy(station_settings.sta.ssid, ESP_WORM_WIFI_SSID, sizeof ESP_WORM_WIFI_SSID - 1u);
	::memcpy(station_settings.sta.password, ESP_WORM_WIFI_PASSWORD,
		sizeof ESP_WORM_WIFI_PASSWORD - 1u);
	if(esp_wifi_set_mode(WIFI_MODE_STA) != ESP_OK
			|| esp_wifi_set_config(WIFI_IF_STA, &station_settings) != ESP_OK
			|| esp_wifi_start() != ESP_OK) {
		return false;
	}
	return xSemaphoreTake(address_semaphore, pdMS_TO_TICKS(c_wifi_link_wait_milliseconds))
		== pdTRUE;
}
