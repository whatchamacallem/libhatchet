#pragma once
// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

// A request to the host is one TAP comment line, so the tty log stays a TAP
// stream, and its reply is one binary frame. One request is outstanding at a
// time and is repeated when no reply arrives, so every operation is idempotent
// and the port buffer is the only flow control.

#include "hxboot_port.hpp"

#if !defined HX_BOOT_LINK_ATTEMPTS
#define HX_BOOT_LINK_ATTEMPTS 3u
#endif

// The largest reply payload in bytes. A port buffer less the 8 byte head and
// 4 byte CRC of a frame.
#define HX_BOOT_LINK_CHUNK (HX_BOOT_PORT_BUFFER_SIZE - 12u)

#define HX_BOOT_LINK_NAME_MAX 63u

#if !defined HX_BOOT_LINK_TIMEOUT_MS
#define HX_BOOT_LINK_TIMEOUT_MS 250u
#endif

enum hxboot_link_status : uint8_t {
	hxboot_link_status_ok,
	hxboot_link_status_not_found,
	hxboot_link_status_bad_request,
	hxboot_link_status_io_error,
	// No valid reply arrived. Never sent by the host.
	hxboot_link_status_no_server
};

// Returns the status of one request after its retries. Sets reply_size to the
// payload size when the status is ok.
hxattr_nodiscard hxboot_link_status hxboot_link_call(void* reply_, size_t reply_capacity_,
	size_t& reply_size_, const char* format_, ...) hxattr_printf(4, 5);
