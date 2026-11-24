// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

// The portable part of hxboot. The host is reached through hxfile alone.

#include <hx/hxfile.hpp>

#include "hxboot_crc.hpp"
#include "hxboot_main.hpp"
#include "hxboot_platform.hpp"
#include "hxboot_report.hpp"
constexpr const char* c_boot_stage_names[hxboot_stage_count] = {
	"init", "usb_header", "usb_payload"
};

bool hxboot_image_header::loadable(const hxboot_memory& memory) const {
	hxassertf(memory.ram_begin <= memory.ram_end, "hxboot_memory window");

	// The names are NUL padded.
	const bool target_matches = ::memcmp(target, memory.target, sizeof target) == 0;
	const bool image_fits = payload_size != 0u && payload_size <= image_size
		&& load_address >= memory.ram_begin && load_address <= memory.ram_end
		&& image_size <= memory.ram_end - load_address;
	const bool entry_inside = entry_address >= load_address
		&& entry_address - load_address < payload_size;
	const bool esp_sdk_matches = ::memcmp(esp_sdk_hash, memory.esp_sdk_hash,
		sizeof esp_sdk_hash) == 0;
	return target_matches && image_fits && entry_inside && esp_sdk_matches;
}

bool hxboot_image_header::valid(void) const {
	// header_crc covers header bytes 8 to 51, which start at target.
	const size_t offset = offsetof(hxboot_image_header, target);
	return magic == c_boot_image_magic
		&& header_crc == hxboot_crc32(reinterpret_cast<const char*>(this) + offset,
			sizeof *this - offset);
}

bool hxboot_image_header::verified(void) const {
	return payload_crc == hxboot_crc32(reinterpret_cast<const void*>(load_address),
		payload_size);
}

hxboot_report::hxboot_report(void) : m_next_(hxboot_stage_init), m_fail_count_(0u) {
	hxout << "TAP version 14\n# Subtest: hxboot\n";
}

hxboot_report::~hxboot_report() {
	hxassertf(m_next_ == hxboot_stage_count, "hxboot_report stage %d", static_cast<int>(m_next_));
	hxout.print("    1..%d\n%s 1 - hxboot\n", static_cast<int>(hxboot_stage_count),
		m_fail_count_ == 0u ? "ok" : "not ok");
}

void hxboot_report::fail(hxboot_stage stage, const char* diagnostic) {
	hxassertf(stage == m_next_ && stage < hxboot_stage_count, "hxboot_report stage %d",
		static_cast<int>(stage));
	hxout.print("    # %s\n    not ok %d - %s\n", diagnostic, stage + 1,
		c_boot_stage_names[stage]);
	++m_next_;
	++m_fail_count_;
}

void hxboot_report::pass(hxboot_stage stage) {
	hxassertf(stage == m_next_ && stage < hxboot_stage_count, "hxboot_report stage %d",
		static_cast<int>(stage));
	hxout.print("    ok %d - %s\n", stage + 1, c_boot_stage_names[stage]);
	++m_next_;
}

// These are the rules for automated verification runs of hxboot and the
// programs it launches. hxboot is a debug tool. A run passes if and only if
// the image served over USB is loaded and run and every test it reports
// passes. Test suite 1 is loading that image. When it fails the run has failed,
// but the image in the gold slot is still launched to show that it can be. That
// is not a testcase. A gold slot that does not load bails out. A gold image
// that passes all of its tests ends a complete document whose one failure is
// test suite 1.
void hxboot_main(void) {
	hxinit();

	const hxboot_memory memory = hxboot_memory_map();
	hxboot_image_header header;
	bool usb_loaded = false;
	{
		hxboot_report report;
		report.pass(hxboot_stage_init);

		hxfile file(hxfile::open_mode_in, HX_BOOT_IMAGE_NAME);
		bool usb_header = false;
		if(!file) {
			report.fail(hxboot_stage_usb_header, HX_BOOT_IMAGE_NAME " not opened");
		}
		else if(!file.read1(header)) {
			report.fail(hxboot_stage_usb_header, HX_BOOT_IMAGE_NAME " header not read");
		}
		else if(!header.valid()) {
			report.fail(hxboot_stage_usb_header, HX_BOOT_IMAGE_NAME " header not valid");
		}
		else if(!header.loadable(memory)) {
			report.fail(hxboot_stage_usb_header, HX_BOOT_IMAGE_NAME " header not loadable");
		}
		else {
			report.pass(hxboot_stage_usb_header);
			usb_header = true;
		}

		if(!usb_header) {
			report.fail(hxboot_stage_usb_payload, "no usb header");
		}
		else if(file.read(reinterpret_cast<void*>(header.load_address),
				memory.ram_end - header.load_address, header.payload_size)
				!= header.payload_size) {
			report.fail(hxboot_stage_usb_payload, HX_BOOT_IMAGE_NAME " payload not read");
		}
		else if(!header.verified()) {
			report.fail(hxboot_stage_usb_payload, HX_BOOT_IMAGE_NAME " payload not verified");
		}
		else {
			report.pass(hxboot_stage_usb_payload);
			usb_loaded = true;
		}
	}

	// The gold slot is loaded without testcases and holds a header at offset 0
	// and the payload after it. The launched gold image continues the document
	// after test suite 1 failed. Bailing out ends it instead.
	const char* bail_reason = hxnull;
	if(!usb_loaded) {
		if(hxboot_flash_read(0u, &header, sizeof header) != sizeof header) {
			bail_reason = "gold slot header not read";
		}
		else if(!header.valid()) {
			bail_reason = "gold slot header not valid";
		}
		else if(!header.loadable(memory)) {
			bail_reason = "gold slot header not loadable";
		}
		else if(hxboot_flash_read(sizeof header, reinterpret_cast<void*>(header.load_address),
				header.payload_size) != header.payload_size) {
			bail_reason = "gold slot payload not read";
		}
		else if(!header.verified()) {
			bail_reason = "gold slot payload not verified";
		}
	}
	if(bail_reason != hxnull) {
		hxout.print("Bail out! %s\n", bail_reason);
	}

	// The output is on its way to the host before the restart because nothing
	// is reported after it.
	hxout.flush();
	if(bail_reason == hxnull) {
		hxboot_launch(header);
	}
}
