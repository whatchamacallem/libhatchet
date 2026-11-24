# SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
# SPDX-License-Identifier: MIT
# This file is licensed under the terms of the LICENSE.md file.
#
# The ESP-IDF project of esp_sdk, the factory app that holds ESP-IDF, and of
# the RAM images that run on it. A board lists its programs, each of which
# defines the target and the test named after it.

cmake_minimum_required(VERSION 3.27)

if(NOT HX_BOARD OR NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/boards/${HX_BOARD}/board.cmake")
	message(FATAL_ERROR "-DHX_BOARD=<board> names a directory in boards.")
endif()
set(HX_BOARD_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/boards/${HX_BOARD}")
include("${HX_BOARD_DIRECTORY}/board.cmake")

# The client region R and hxboot's slice B at its top.
set(ESP_SDK_CLIENT_REGION_SIZE 0x40000)
set(ESP_SDK_BOOT_SIZE 0x2000)

set(SDKCONFIG "${CMAKE_BINARY_DIR}/sdkconfig")
# esp_netif's bridge glue, which CONFIG_ESP_NETIF_BRIDGE_EN turns on, includes
# esp_eth.
set(COMPONENTS main esp_eth)
set(SDKCONFIG_DEFAULTS "${CMAKE_CURRENT_SOURCE_DIR}/sdkconfig.esp_sdk"
	"${HX_BOARD_DIRECTORY}/sdkconfig.board")

# A card larger than 32 GB comes formatted exFAT, which the ffconf.h of
# ESP-IDF's fatfs leaves out. A copy with exFAT overrides that component.
set(ESP_SDK_IDF_FATFS_DIRECTORY "$ENV{IDF_PATH}/components/fatfs")
set(ESP_SDK_FATFS_DIRECTORY "${CMAKE_BINARY_DIR}/components/fatfs")
file(COPY "${ESP_SDK_IDF_FATFS_DIRECTORY}" DESTINATION "${CMAKE_BINARY_DIR}/components"
	PATTERN ffconf.h EXCLUDE
	PATTERN host_test EXCLUDE
	PATTERN test_apps EXCLUDE
	PATTERN test_fatfsgen EXCLUDE)
file(READ "${ESP_SDK_IDF_FATFS_DIRECTORY}/src/ffconf.h" ESP_SDK_IDF_FATFS_SETTINGS)
string(REPLACE "#define FF_FS_EXFAT\t\t0\n" "#define FF_FS_EXFAT\t\t1\n" ESP_SDK_FATFS_SETTINGS
	"${ESP_SDK_IDF_FATFS_SETTINGS}")
if(ESP_SDK_FATFS_SETTINGS STREQUAL ESP_SDK_IDF_FATFS_SETTINGS)
	message(FATAL_ERROR "FF_FS_EXFAT not found in ${ESP_SDK_IDF_FATFS_DIRECTORY}/src/ffconf.h.")
endif()
# Rewritten only when changed, so fatfs is not rebuilt with every configure.
file(WRITE "${CMAKE_BINARY_DIR}/ffconf.h" "${ESP_SDK_FATFS_SETTINGS}")
configure_file("${CMAKE_BINARY_DIR}/ffconf.h" "${ESP_SDK_FATFS_DIRECTORY}/src/ffconf.h" COPYONLY)
set(EXTRA_COMPONENT_DIRS "${ESP_SDK_FATFS_DIRECTORY}")

# ESP-IDF's Python environment, which export.sh puts first on PATH, as idf.py
# passes it. ctest then runs the tests without export.sh.
find_program(PYTHON python REQUIRED)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
# Every component and every client sees R and B.
idf_build_set_property(COMPILE_DEFINITIONS
	"ESP_SDK_CLIENT_REGION_SIZE=${ESP_SDK_CLIENT_REGION_SIZE}u" APPEND)
idf_build_set_property(COMPILE_DEFINITIONS "ESP_SDK_BOOT_SIZE=${ESP_SDK_BOOT_SIZE}u" APPEND)
# For CONFIG_APP_REPRODUCIBLE_BUILD. Its prefix maps leave the build directory,
# where the exFAT copy of fatfs is compiled, in __FILE__. PROJECT_VER would
# come from git describe.
idf_build_set_property(COMPILE_OPTIONS "-fmacro-prefix-map=${CMAKE_BINARY_DIR}=." APPEND)
set(PROJECT_VER 1)
project(esp_sdk)

enable_testing()
include("${CMAKE_CURRENT_SOURCE_DIR}/esp_sdk_client.cmake")
foreach(HX_PROGRAM IN LISTS HX_BOARD_PROGRAMS)
	include("${CMAKE_CURRENT_SOURCE_DIR}/../${HX_PROGRAM}/esp_sdk.cmake")
endforeach()
