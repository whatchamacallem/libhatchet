// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

#include "../include/hx/libhatchet.h"

// The global settings object. Constructed by hxinit after some global
// constructors may have run and before setting up memory management.
// Logging and asserts default to on until construction completes.

extern "C" {

struct hxsettings_t hxg_settings;

void hxsettings_construct_(void) {
	hxg_settings.log_level = hxlog_level_log;
	hxg_settings.deallocate_permanent = false;
	hxg_settings.test_break_on_failure = false;
	hxg_settings.test_filter = hxnull;
}

} // extern "C"
