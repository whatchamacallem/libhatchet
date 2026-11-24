#pragma once
// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

// The WireGuard interface, its keys and its addresses. Level 1. The device
// only answers handshakes and the host is its one peer. The tunnel listens on
// every interface and does not refer to a link. No link is trusted, so every
// link gets the same encryption.

#include <hx/libhatchet.h>

#include "lwip/udp.h"

#if !defined ESP_WORM_TUNNEL_ADDRESS
#define ESP_WORM_TUNNEL_ADDRESS "10.9.0.2"
#endif

#if !defined ESP_WORM_TUNNEL_NETMASK
#define ESP_WORM_TUNNEL_NETMASK "255.255.255.0"
#endif

#if !defined ESP_WORM_TUNNEL_PEER_ADDRESS
// The host's address in the tunnel, the only one allowed.
#define ESP_WORM_TUNNEL_PEER_ADDRESS "10.9.0.1"
#endif

#if !defined ESP_WORM_TUNNEL_PEER_KEY
#error ESP_WORM_TUNNEL_PEER_KEY is not set.
#endif

#if !defined ESP_WORM_TUNNEL_PORT
#define ESP_WORM_TUNNEL_PORT 51820
#endif

#if !defined ESP_WORM_TUNNEL_PRIVATE_KEY
#error ESP_WORM_TUNNEL_PRIVATE_KEY is not set.
#endif
// Has the PCB send and take packets only through the tunnel, whatever lwIP's
// routes are. Called on lwIP's thread after `esp_worm_tunnel_start`.
void esp_worm_tunnel_bind(udp_pcb& udp_control_);

// Called once on lwIP's thread.
hxattr_nodiscard bool esp_worm_tunnel_start(void);
