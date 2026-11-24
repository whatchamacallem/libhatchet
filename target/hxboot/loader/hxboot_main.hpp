#pragma once
// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

#include <hx/libhatchet.h>

#if !defined HX_BOOT_IMAGE_NAME
// The file hxboot asks the host for.
#define HX_BOOT_IMAGE_NAME "hxboot.hxb"
#endif
// Calls hxinit, reports test suite 1 and launches an image. Returns only after
// it bails out because the host and the gold slot both fail. The port must
// already be open.
void hxboot_main(void);
