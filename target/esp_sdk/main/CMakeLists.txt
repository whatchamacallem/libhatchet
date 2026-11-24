# SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
# SPDX-License-Identifier: MIT
# This file is licensed under the terms of the LICENSE.md file.
#
# esp_sdk's one component. It requires every component that a client calls, so
# that esp_sdk links them.

idf_component_register(
	SRCS esp_sdk.cpp
	PRIV_INCLUDE_DIRS ../../hxboot/loader ../../../include
	PRIV_REQUIRES esp_app_format esp_driver_gpio esp_driver_sdspi
		esp_driver_spi esp_driver_usb_serial_jtag esp_event esp_hw_support esp_lcd esp_netif
		esp_partition esp_rom esp_system esp_wifi fatfs freertos heap log lwip
)

# Keeps every symbol that only a client calls through --gc-sections.
set(ESP_SDK_EXPORTS_FILE "${CMAKE_CURRENT_LIST_DIR}/esp_sdk_exports.txt")
file(STRINGS "${ESP_SDK_EXPORTS_FILE}" ESP_SDK_EXPORTS)
list(TRANSFORM ESP_SDK_EXPORTS PREPEND "-Wl,--require-defined=")
target_link_options(${COMPONENT_LIB} INTERFACE ${ESP_SDK_EXPORTS})
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${ESP_SDK_EXPORTS_FILE}")

target_linker_script(${COMPONENT_LIB} INTERFACE "${CMAKE_CURRENT_LIST_DIR}/esp_sdk.ld")
