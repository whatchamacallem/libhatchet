# SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
# SPDX-License-Identifier: MIT
# This file is licensed under the terms of the LICENSE.md file.

# The Pico SDK doesn't support disabling the C++ headers.
file(GLOB PICO_SDK_HATCHET_SOURCES "${CMAKE_CURRENT_LIST_DIR}/../../src/*.cpp")
add_library(hatchet STATIC ${PICO_SDK_HATCHET_SOURCES})
target_include_directories(hatchet PUBLIC "${CMAKE_CURRENT_LIST_DIR}/../../include")
target_compile_definitions(hatchet PUBLIC
	HX_SKIP_LIBCXX_CHECK=1
	HX_PROVIDE_NEW_DELETE=1
	HX_HARDENING_MODE=HX_HARDENING_MODE_VERBOSE
	HX_USE_FILE_IO=0
	HX_USE_THREADS=0
	HX_USE_STD_LIB=0
	HX_USE_PROFILER=0
	HX_USE_CONSOLE=0
	PICO_CXX_DISABLE_ALLOCATION_OVERRIDES=1
)
target_compile_options(hatchet PRIVATE
	-fno-exceptions
	-fno-rtti
	-nostdinc++
)

# pico_sdk_program(<name> SOURCE_FILES <file>...)
function(pico_sdk_program PICO_SDK_PROGRAM_NAME)
	cmake_parse_arguments(PARSE_ARGV 1 PICO_SDK_PROGRAM "" "" "SOURCE_FILES")
	add_executable(${PICO_SDK_PROGRAM_NAME} ${PICO_SDK_PROGRAM_SOURCE_FILES}
		"${CMAKE_CURRENT_FUNCTION_LIST_DIR}/pico_sdk_hooks.cpp")
	target_compile_definitions(${PICO_SDK_PROGRAM_NAME} PRIVATE HX_TARGET_HOOKS=1)
	target_compile_options(${PICO_SDK_PROGRAM_NAME} PRIVATE
		-fno-exceptions
		-Wall
		-Wextra
	)
	target_link_libraries(${PICO_SDK_PROGRAM_NAME}
		hatchet
		pico_stdlib
	)
	pico_enable_stdio_usb(${PICO_SDK_PROGRAM_NAME} 1)
	pico_enable_stdio_uart(${PICO_SDK_PROGRAM_NAME} 0)
	pico_add_extra_outputs(${PICO_SDK_PROGRAM_NAME})

	add_test(NAME ${PICO_SDK_PROGRAM_NAME}
		COMMAND bash "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/run_pico_sdk.sh"
			--picotool "$<TARGET_FILE:picotool>" --vendor ${HX_BOARD_USB_VENDOR}
			--uf2 "${CMAKE_CURRENT_BINARY_DIR}/${PICO_SDK_PROGRAM_NAME}.uf2"
			--report "${CMAKE_CURRENT_BINARY_DIR}/${PICO_SDK_PROGRAM_NAME}_report.txt")
	set_tests_properties(${PICO_SDK_PROGRAM_NAME} PROPERTIES SKIP_RETURN_CODE 77 RUN_SERIAL TRUE)
endfunction()
