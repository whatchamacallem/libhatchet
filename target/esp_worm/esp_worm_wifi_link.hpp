#pragma once
// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

// The IP link under the tunnel, as a Wi-Fi station. Level 1.

#include <hx/libhatchet.h>

#if !defined ESP_WORM_WIFI_PASSWORD
#error ESP_WORM_WIFI_PASSWORD is not set.
#endif

#if !defined ESP_WORM_WIFI_SSID
#error ESP_WORM_WIFI_SSID is not set.
#endif
// Returns whether the device joined the network and was given an address
// within 20 seconds. It joins again whenever it is disconnected. Called once on
// the main task after `esp_netif_init`.
hxattr_nodiscard bool esp_worm_wifi_link_start(void);
