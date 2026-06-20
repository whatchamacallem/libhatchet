# SPDX-FileCopyrightText: © 2026 Adrian Johnston.
# SPDX-License-Identifier: MIT

cmake_minimum_required(VERSION 3.21)

include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(libhatchet_test_pico C CXX ASM)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

pico_sdk_init()

# ── libhatchet ─────────────────────────────────────────────────────────────────
add_subdirectory($ENV{LIBHATCHET_PATH} libhatchet)

target_compile_definitions(hatchet PUBLIC
    HX_HARDENING_MODE=HX_HARDENING_MODE_VERBOSE
    HX_USE_FILE_IO=0
    HX_USE_THREADS=0
    HX_USE_LIBCXX=0
    HX_PROVIDE_NEW_DELETE=1
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
file(GLOB TEST_SOURCES "../libhatchet/test/*.c" "../libhatchet/test/*.cpp")
# hxtest_main.cpp provides the desktop main(); we supply our own in libhatchet_pico.cpp.
list(REMOVE_ITEM TEST_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/../libhatchet/test/hxtest_main.cpp")

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
