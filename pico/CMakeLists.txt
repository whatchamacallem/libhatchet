# SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
# SPDX-License-Identifier: MIT
# This file is licensed under the terms of the LICENSE.md file.

cmake_minimum_required(VERSION 3.21)

if(NOT DEFINED ENV{PICO_SDK_PATH})
    message(FATAL_ERROR "PICO_SDK_PATH environment variable is not set")
endif()

if(NOT DEFINED ENV{LIBHATCHET_PATH})
    message(FATAL_ERROR "LIBHATCHET_PATH environment variable is not set")
endif()

include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(libhatchet_test_pico C CXX ASM)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

pico_sdk_init()

# ── libhatchet ─────────────────────────────────────────────────────────────────

add_subdirectory($ENV{LIBHATCHET_PATH} libhatchet)

# The pico sdk doesn't support disabling the C++ headers.
target_compile_definitions(hatchet PUBLIC
    HX_SKIP_LIBCXX_CHECK=1
    HX_PROVIDE_NEW_DELETE=1
    HX_HARDENING_MODE=HX_HARDENING_MODE_VERBOSE
    HX_USE_FILE_IO=0
    HX_USE_THREADS=0
    HX_USE_LIBCXX=0
    HX_USE_PROFILER=0
    HX_USE_CONSOLE=0
    HX_MEMORY_BUDGET_TEMPORARY_STACK=200000
    # Suppress pico-sdk's own operator new/delete; hatchet provides them.
    PICO_CXX_DISABLE_ALLOCATION_OVERRIDES=1
)

target_compile_options(hatchet PRIVATE
    -fno-exceptions
    -fno-rtti
    -nostdinc++
)

# ── test executable ────────────────────────────────────────────────────────────

get_filename_component(LIBHATCHET_ABS "$ENV{LIBHATCHET_PATH}" ABSOLUTE)
file(GLOB TEST_SOURCES "${LIBHATCHET_ABS}/test/*.c" "${LIBHATCHET_ABS}/test/*.cpp")
list(REMOVE_ITEM TEST_SOURCES "${LIBHATCHET_ABS}/test/hxtest_main.cpp")

add_executable(libhatchet_test_pico
    libhatchet_pico.cpp
    ${TEST_SOURCES}
)

target_link_libraries(libhatchet_test_pico
    hatchet
    pico_stdlib
)

target_compile_options(libhatchet_test_pico PRIVATE
    -fno-exceptions
    -Wall
    -Wextra
)

pico_enable_stdio_usb(libhatchet_test_pico 1)
pico_enable_stdio_uart(libhatchet_test_pico 0)

pico_add_extra_outputs(libhatchet_test_pico)
