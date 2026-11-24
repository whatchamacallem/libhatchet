#pragma once
// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

// Test suite 1 of the TAP stream, where hxboot reports loading the image served
// over USB. The launched program writes test suite 2 and on.

#include <hx/libhatchet.h>
// The testcases of the suite hxboot in plan order.
enum hxboot_stage : uint8_t {
	// The constructors, the port and hxinit are done.
	hxboot_stage_init,
	hxboot_stage_usb_header,
	// The image is at its load address and is about to be launched.
	hxboot_stage_usb_payload,
	hxboot_stage_count
};

// One subtest. Every stage is reported once in order as an indented testcase.
class hxboot_report {
public:
	hxboot_report(void);

	~hxboot_report();

	// Reports the stage as not ok after a diagnostic comment.
	void fail(hxboot_stage stage_, const char* diagnostic_);

	void pass(hxboot_stage stage_);

private:
	hxboot_report(const hxboot_report&) = delete;
	void operator=(const hxboot_report&) = delete;

	uint8_t m_next_;
	uint8_t m_fail_count_;
};
