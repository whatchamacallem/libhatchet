# SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
# SPDX-License-Identifier: MIT
# This file is licensed under the terms of the LICENSE.md file.

cmake_minimum_required(VERSION 3.21)

if(NOT HX_BOARD OR NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/boards/${HX_BOARD}/board.cmake")
	message(FATAL_ERROR "-DHX_BOARD=<board> names a directory in boards.")
endif()
include("${CMAKE_CURRENT_SOURCE_DIR}/boards/${HX_BOARD}/board.cmake")

if(NOT PICO_SDK_PATH)
	if(DEFINED ENV{PICO_SDK_PATH})
		set(PICO_SDK_PATH "$ENV{PICO_SDK_PATH}")
	else()
		set(PICO_SDK_PATH "$ENV{HOME}/pico-sdk")
	endif()
endif()
# picotool is built once, next to the SDK, instead of in every build directory.
if(NOT PICOTOOL_FETCH_FROM_GIT_PATH AND NOT DEFINED ENV{PICOTOOL_FETCH_FROM_GIT_PATH})
	get_filename_component(PICOTOOL_FETCH_FROM_GIT_PATH "${PICO_SDK_PATH}/.." ABSOLUTE)
endif()

include(${PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(hxtarget_pico_sdk)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
enable_language(ASM)
pico_sdk_init()

enable_testing()
include("${CMAKE_CURRENT_SOURCE_DIR}/pico_sdk_program.cmake")
foreach(HX_PROGRAM IN LISTS HX_BOARD_PROGRAMS)
	include("${CMAKE_CURRENT_SOURCE_DIR}/../${HX_PROGRAM}/pico_sdk.cmake")
endforeach()
