// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.
//
// esp_worm is a TFTP server that keeps files on the TF card of an
// ESP32-C6-LCD-1.47. It is a demo of tftp and wireguard-tools on Ubuntu
// reaching the device. TFTP runs in a WireGuard tunnel carried by Wi-Fi. hxboot
// launches it as a client of esp_sdk. Each step of its start is a testcase in
// hxboot's TAP document on the debug USB. Level 3.

#if !defined ESP_PLATFORM
#error esp_worm.cpp implements ESP-IDF only.
#endif

#include "hxboot_port.hpp"
#include "esp_sdk.hpp"
#include "esp_worm_file_server.hpp"
#include "esp_worm_screen.hpp"
#include "esp_worm_tunnel.hpp"
#include "esp_worm_wifi_link.hpp"
#include "hxtap_producer.hpp"

#include <hx/hxutility.h>

#include <string.h>

#include "esp_netif.h"
#include "esp_random.h"
#include "esp_system.h"
#include "esp_vfs_fat.h"
#include "esp_wifi.h"
#include "lwip/tcpip.h"

extern "C" {
#include "wireguard-platform.h"
}

#if !CONFIG_IDF_TARGET_ESP32C6
#error esp_worm.cpp implements the ESP32-C6 only.
#endif

// With core locking tcpip_callback_wait runs its function on the calling task.
#if LWIP_TCPIP_CORE_LOCKING
#error esp_worm requires LWIP_TCPIP_CORE_LOCKING=0.
#endif

#if !defined ESP_WORM_CARD_CLOCK_PIN
#define ESP_WORM_CARD_CLOCK_PIN 7
#endif

#if !defined ESP_WORM_CARD_DATA_IN_PIN
// MOSI.
#define ESP_WORM_CARD_DATA_IN_PIN 6
#endif

#if !defined ESP_WORM_CARD_DATA_OUT_PIN
// MISO.
#define ESP_WORM_CARD_DATA_OUT_PIN 5
#endif

#if !defined ESP_WORM_CARD_SELECT_PIN
#define ESP_WORM_CARD_SELECT_PIN 4
#endif

static_assert(__COUNTER__ == 0);
namespace {

// The file server has at most one file open.
constexpr int c_card_file_count = 1;

class esp_worm_lwip_start {
public:
	bool (*start_function)(void);
	bool started;
};

bool s_card_mounted;

uint16_t s_screen_frame_count;

void s_screen_frame(void) {
	char frame_line[c_esp_worm_screen_columns + 1];
	const int line_length = ::snprintf(frame_line, sizeof frame_line, "esp_worm %-5u",
		static_cast<unsigned int>(s_screen_frame_count));
	hxassertf(line_length > 0 && static_cast<size_t>(line_length) < sizeof frame_line,
		"s_screen_frame");
	::memcpy(g_esp_worm_screen_text, frame_line, static_cast<size_t>(line_length));
	++s_screen_frame_count;
}

bool s_lwip_start(bool (*start_function)(void)) {
	esp_worm_lwip_start lwip_start = { start_function, false };
	const err_t wait_error = tcpip_callback_wait([](void* callback_context) {
		esp_worm_lwip_start& waited_start = *static_cast<esp_worm_lwip_start*>(callback_context);
		waited_start.started = waited_start.start_function();
	}, &lwip_start);
	return wait_error == ERR_OK && lwip_start.started;
}

// Starts the SPI bus that the screen shares with the card. The second row shows
// red, blue and green, each on its complement, color ^ 7.
bool s_screen_start(void) {
	spi_bus_config_t bus_settings = {};
	bus_settings.mosi_io_num = ESP_WORM_CARD_DATA_IN_PIN;
	bus_settings.miso_io_num = ESP_WORM_CARD_DATA_OUT_PIN;
	bus_settings.sclk_io_num = ESP_WORM_CARD_CLOCK_PIN;
	bus_settings.quadwp_io_num = -1;
	bus_settings.quadhd_io_num = -1;
	TAP_CHECK(spi_bus_initialize(SDSPI_DEFAULT_HOST, &bus_settings, SDSPI_DEFAULT_DMA) == ESP_OK);
	TAP_CHECK(esp_worm_screen_start(s_screen_frame));
	const char color_words[] = "red blue green";
	char* const words_text = g_esp_worm_screen_text + c_esp_worm_screen_columns;
	uint8_t* const words_colors = g_esp_worm_screen_colors + c_esp_worm_screen_columns;
	::memcpy(words_text, color_words, sizeof color_words - 1u);
	::memset(words_colors, 0x61, 3u);
	::memset(words_colors + 4, 0x34, 4u);
	::memset(words_colors + 9, 0x52, 5u);
	return true;
}

bool s_card_mount(void) {
	const sdmmc_host_t host_settings = SDSPI_HOST_DEFAULT();
	sdspi_device_config_t device_settings = SDSPI_DEVICE_CONFIG_DEFAULT();
	device_settings.gpio_cs = static_cast<gpio_num_t>(ESP_WORM_CARD_SELECT_PIN);

	esp_vfs_fat_mount_config_t mount_settings = VFS_FAT_MOUNT_DEFAULT_CONFIG();
	mount_settings.max_files = c_card_file_count;

	sdmmc_card_t* mounted_card = hxnil;
	TAP_CHECK(esp_vfs_fat_sdspi_mount(ESP_WORM_CARD_ROOT, &host_settings, &device_settings,
		&mount_settings, &mounted_card) == ESP_OK);
	s_card_mounted = true;
	return true;
}

// esp_netif takes its index into each netif's client_data in the callback of
// the tcpip_init it calls, so lwIP is started only through it.
bool s_network_start(void) {
	return esp_netif_init() == ESP_OK;
}

bool s_wifi_link_start(void) {
	TAP_CHECK(esp_worm_wifi_link_start());
	esp_netif_ip_info_t address_info;
	wifi_ap_record_t network_record;
	wifi_phy_mode_t physical_mode;
	TAP_CHECK(esp_netif_get_ip_info(esp_netif_get_default_netif(), &address_info) == ESP_OK);
	TAP_CHECK(esp_wifi_sta_get_ap_info(&network_record) == ESP_OK);
	TAP_CHECK(esp_wifi_sta_get_negotiated_phymode(&physical_mode) == ESP_OK);
	char comment_line[HX_MAX_LINE];
	const int line_length = ::snprintf(comment_line, sizeof comment_line,
		"# " IPSTR " authmode %d phymode %d channel %d rssi %d\n", IP2STR(&address_info.ip),
		static_cast<int>(network_record.authmode), static_cast<int>(physical_mode),
		network_record.primary, network_record.rssi);
	hxassertf(line_length > 0 && static_cast<size_t>(line_length) < sizeof comment_line,
		"s_wifi_link_start");
	hxboot_port_write(comment_line, static_cast<size_t>(line_length));
	return true;
}

bool s_tunnel_start(void) {
	return s_lwip_start(esp_worm_tunnel_start);
}

bool s_file_server_start(void) {
	TAP_CHECK(s_card_mounted);
	return s_lwip_start(esp_worm_file_server_start);
}

} // namespace

extern "C" void wireguard_random_bytes(void* random_bytes, size_t byte_count) {
	esp_fill_random(random_bytes, byte_count);
}

extern "C" void esp_sdk_client_main(void) {
	hxboot_port_open();
	hxinit();

	bool passed = true;
	TAP_TEST_CASE(s_screen_start);
	TAP_TEST_CASE(s_card_mount);
	TAP_TEST_CASE(s_network_start);
	TAP_TEST_CASE(s_wifi_link_start);
	TAP_TEST_CASE(s_tunnel_start);
	TAP_TEST_CASE(s_file_server_start);

	char comment_line[HX_MAX_LINE];
	const int line_length = ::snprintf(comment_line, sizeof comment_line,
		"# free heap %u\n", static_cast<unsigned int>(esp_get_free_heap_size()));
	hxassertf(line_length > 0 && static_cast<size_t>(line_length) < sizeof comment_line,
		"esp_sdk_client_main");
	hxboot_port_write(comment_line, static_cast<size_t>(line_length));

	// hxboot leaves its TAP document open for the program it launches. The plan
	// ends the document and has hxboot's server exit and free the tty.
	TAP_TEST_PLAN();
	hxboot_port_flush();
}

void hxtap::test_write_(const char* text, size_t count) {
	hxboot_port_write(text, count);
}
