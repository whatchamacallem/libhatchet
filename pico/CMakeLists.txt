# SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
# SPDX-License-Identifier: MIT
# This file is licensed under the terms of the LICENSE.md file.

cmake_minimum_required(VERSION 3.21)

# The Pico SDK is at PICO_SDK_PATH, as a CMake or environment variable, or at
# ~/pico-sdk where "install_deb.sh --sdks" installs it. The SDK keeps picotool
# at PICOTOOL_FETCH_FROM_GIT_PATH, building it there when it is missing. That
# defaults to the directory containing the SDK so that it is built once.
if(NOT PICO_SDK_PATH)
    if(DEFINED ENV{PICO_SDK_PATH})
        set(PICO_SDK_PATH "$ENV{PICO_SDK_PATH}")
    else()
        set(PICO_SDK_PATH "$ENV{HOME}/pico-sdk")
    endif()
endif()

if(NOT PICOTOOL_FETCH_FROM_GIT_PATH AND NOT DEFINED ENV{PICOTOOL_FETCH_FROM_GIT_PATH})
    get_filename_component(PICOTOOL_FETCH_FROM_GIT_PATH "${PICO_SDK_PATH}/.." ABSOLUTE)
endif()

# libhatchet is the parent directory.
set(LIBHATCHET_PATH ${CMAKE_CURRENT_LIST_DIR}/..)

set(PICO_BOARD pico2 CACHE STRING "Board type")

include(${PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(libhatchet_test_pico C CXX ASM)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

pico_sdk_init()

# -- libhatchet -----------------------------------------------------------------

add_subdirectory(${LIBHATCHET_PATH} libhatchet)

# The pico sdk doesn't support disabling the C++ headers.
target_compile_definitions(hatchet PUBLIC
    HX_SKIP_LIBCXX_CHECK=1
    HX_PROVIDE_NEW_DELETE=1
    HX_HARDENING_MODE=HX_HARDENING_MODE_VERBOSE
    HX_USE_FILE_IO=0
    HX_USE_THREADS=0
    HX_USE_STD_LIB=0
    HX_USE_PROFILER=0
    HX_USE_CONSOLE=0
    HX_SLAB_BUDGET_TEMPORARY_STACK=200000
    # Suppress pico-sdk's own operator new/delete; hatchet provides them.
    PICO_CXX_DISABLE_ALLOCATION_OVERRIDES=1
)

target_compile_options(hatchet PRIVATE
    -fno-exceptions
    -fno-rtti
    -nostdinc++
)

# -- Test Executable ------------------------------------------------------------

get_filename_component(LIBHATCHET_ABS "${LIBHATCHET_PATH}" ABSOLUTE)
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
