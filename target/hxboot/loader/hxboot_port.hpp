#pragma once
// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

// The port is 8 bit clean in both directions, without newline translation or
// echo.

#include <hx/libhatchet.h>

// Bytes the port holds in each direction. One whole reply frame.
#define HX_BOOT_PORT_BUFFER_SIZE 1036u
bool hxboot_port_connected(void);

// Blocks until the host has every byte, disconnects or stops reading.
void hxboot_port_flush(void);

// Turns off the SDK's own console output and discards the bytes already
// received. Called once before any other port function.
void hxboot_port_open(void);

// Returns the bytes read, fewer than count on timeout.
size_t hxboot_port_read(void* bytes_, size_t count_, uint32_t timeout_ms_);

// Writes atomically. Discards when there is no host or it stops reading.
void hxboot_port_write(const void* bytes_, size_t count_);
