# SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
# SPDX-License-Identifier: MIT
# This file is licensed under the terms of the LICENSE.md file.
#
# The test writes the board's gold image to the gold slot and refuses to serve
# an image, so hxboot fails test suite 1 and launches the gold slot.

if(NOT ESP_SDK_GOLD_PROGRAM)
	message(FATAL_ERROR "hxboot requires ESP_SDK_GOLD_PROGRAM in ${HX_BOARD}/board.cmake.")
endif()

file(GLOB HX_BOOT_SOURCES "${CMAKE_CURRENT_LIST_DIR}/loader/*.cpp"
	"${CMAKE_CURRENT_LIST_DIR}/esp_sdk/*.cpp")
esp_sdk_client(hxboot MEMORY_REGION boot
	SOURCE_FILES ${HX_BOOT_SOURCES} ${ESP_SDK_HATCHET_SOURCES}
	COMPILE_DEFINITIONS
		HX_HARDENING_MODE=HX_HARDENING_MODE_NONE
		HX_MAX_LINE=256
		HX_USE_CONSOLE=0
		HX_USE_FILE_IO=3
		HX_USE_LOGGING=1
		HX_USE_PROFILER=0
		HX_USE_SLAB_ALLOCATOR=0
		HX_USE_STD_LIB=0
		HX_USE_THREADS=0
	REQUIRED_COMPONENTS esp_app_format esp_driver_usb_serial_jtag esp_partition esp_rom
		esp_system freertos log)

esp_sdk_program(hxboot CLIENTS ${ESP_SDK_GOLD_PROGRAM}
	COMMAND ${ESP_SDK_RUN_COMMAND} --flash ${ESP_SDK_GOLD_OFFSET} ${ESP_SDK_GOLD_PROGRAM}.hxb
		--gold-run --report "${CMAKE_BINARY_DIR}/hxboot_report.txt")
