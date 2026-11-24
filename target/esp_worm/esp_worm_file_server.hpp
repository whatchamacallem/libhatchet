#pragma once
// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

// The TFTP server that keeps files on the card. Level 2. It serves one
// transfer at a time in `octet` mode. A name is written once and read many
// times. It sends and takes packets only through the tunnel.

#include <hx/libhatchet.h>

#if !defined ESP_WORM_CARD_ROOT
#define ESP_WORM_CARD_ROOT "/sdcard"
#endif
// Called once on lwIP's thread after `esp_worm_tunnel_start`.
hxattr_nodiscard bool esp_worm_file_server_start(void);
