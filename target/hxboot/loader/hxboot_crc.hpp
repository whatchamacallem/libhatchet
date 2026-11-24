#pragma once
// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

#include <hx/libhatchet.h>
// Returns zlib.crc32 of the bytes continued from crc.
hxattr_nodiscard uint32_t hxboot_crc32(const void* bytes_, size_t count_, uint32_t crc_=0u);
