# SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
# SPDX-License-Identifier: MIT
# This file is licensed under the terms of the LICENSE.md file.
#
# Compiles as a test suite when compiled as a top level project. Otherwise it
# compiles as a library.
#
# When built as a test suite Google Test is used instead of the <hx/hxtest.h>
# reimplementation. This is significantly slower than the test scripts when
# doing a clean build. Enabling exceptions is also tested.
#
# An embedded target instead compiles target/test_target.cpp with its SDK when
# this is the top level project. Each one is a section ending with return(). See
# target/README.md.

cmake_minimum_required(VERSION 3.21)

# -- Pico SDK ------------------------------------------------------------------

# -DPICO_BOARD=<board> selects this section.
if(PICO_BOARD AND CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
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

	# The SDK requires this before project().
	include(${PICO_SDK_PATH}/external/pico_sdk_import.cmake)

	project(libhatchet)

	set(CMAKE_C_STANDARD 17)
	set(CMAKE_C_STANDARD_REQUIRED ON)
	set(CMAKE_CXX_STANDARD 23)
	set(CMAKE_CXX_STANDARD_REQUIRED ON)
	enable_language(ASM)
	pico_sdk_init()

	file(GLOB SOURCES "src/*.cpp")
	add_library(hatchet STATIC ${SOURCES})
	target_include_directories(hatchet PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

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
		# Suppress pico-sdk's own operator new/delete; hatchet provides them.
		PICO_CXX_DISABLE_ALLOCATION_OVERRIDES=1
	)
	target_compile_options(hatchet PRIVATE
		-fno-exceptions
		-fno-rtti
		-nostdinc++
	)

	add_executable(hxtest_target target/test_target.cpp)

	target_compile_options(hxtest_target PRIVATE
		-fno-exceptions
		-Wall
		-Wextra
	)
	target_link_libraries(hxtest_target
		hatchet
		pico_stdlib
	)

	pico_enable_stdio_usb(hxtest_target 1)
	pico_enable_stdio_uart(hxtest_target 0)
	pico_add_extra_outputs(hxtest_target)

	return()
endif()

# -- Library and Test Suite ----------------------------------------------------

project(libhatchet)

if(NOT PROJECT_IS_TOP_LEVEL)
	# Compile as a library.
	file(GLOB SOURCES "src/*.cpp")
	add_library(hatchet STATIC ${SOURCES})
	target_include_directories(hatchet PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
else()
	# Compile as a test suite.
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}")
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}")

	set(CMAKE_C_STANDARD 17)
	set(CMAKE_C_STANDARD_REQUIRED ON)
	set(CMAKE_CXX_STANDARD 23)
	set(CMAKE_CXX_STANDARD_REQUIRED ON)
	set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE BOOL "Enable compile database generation")

	file(GLOB SOURCES "src/*.cpp")
	file(GLOB TEST_SOURCES "test/*.c" "test/*.cpp")

	# Add the executable for tests
	add_executable(
		hxtest
		${SOURCES}
		${TEST_SOURCES}
	)

	target_link_libraries(
		hxtest
		gtest
	)

	target_include_directories(hxtest PRIVATE ${CMAKE_SOURCE_DIR}/include)
	target_compile_definitions(hxtest PRIVATE HX_USE_TEST_FRAMEWORK=2)

	# Turn on everything so Clang-Tidy sees it.
	target_compile_definitions(hxtest PRIVATE HX_HARDENING_MODE=HX_HARDENING_MODE_DEBUG)
	target_compile_definitions(hxtest PRIVATE HX_USE_CONSOLE=2)
	target_compile_definitions(hxtest PRIVATE HX_USE_PROFILER=1)

	if(MSVC)
		target_compile_options(
			hxtest
			PRIVATE
				$<$<CONFIG:Debug>:/MTd>
				$<$<NOT:$<CONFIG:Debug>>:/MT>
				/permissive-
				/W4
				/wd4530
				/wd6326
				/WX
				/Zc:__cplusplus
				/Zc:preprocessor
		)
	else()
		target_compile_options(
			hxtest
			PRIVATE
				-ggdb3
				-Wall
				-Wextra
				-Werror
				-ftrivial-auto-var-init=pattern
		)
	endif()

	# Extract GoogleTest 1.15.2 into the cache directory.
	#
	# Uses a per-configuration sub-directory so that switching between Win32/x64,
	# Windows/Linux or Debug/Release builds does not cause conflicts between
	# incompatible cached GoogleTest binaries. CMAKE_GENERATOR_PLATFORM is set by
	# the Visual Studio multi-config generator (-A x64 / -A Win32). Otherwise the
	# system name and pointer size are used. CMAKE_BUILD_TYPE is set by
	# single-config generators (Ninja, Make) via -DCMAKE_BUILD_TYPE=.
	if(CMAKE_GENERATOR_PLATFORM)
		set(HX_CACHE_SUFFIX "${CMAKE_GENERATOR_PLATFORM}")
	elseif(CMAKE_SIZEOF_VOID_P EQUAL 8)
		set(HX_CACHE_SUFFIX "${CMAKE_SYSTEM_NAME}-x64")
	else()
		set(HX_CACHE_SUFFIX "${CMAKE_SYSTEM_NAME}-x86")
	endif()
	if(CMAKE_BUILD_TYPE)
		set(HX_CACHE_SUFFIX "${HX_CACHE_SUFFIX}-${CMAKE_BUILD_TYPE}")
	endif()
	set(
		HX_FETCHCONTENT_CACHE_DIR
		"${CMAKE_SOURCE_DIR}/cache/${HX_CACHE_SUFFIX}"
	)
	file(MAKE_DIRECTORY "${HX_FETCHCONTENT_CACHE_DIR}")
	set(
		FETCHCONTENT_BASE_DIR
		"${HX_FETCHCONTENT_CACHE_DIR}"
		CACHE PATH "FetchContent base directory"
		FORCE
	)
	include(FetchContent)
	set(HX_GOOGLETEST_ARCHIVE "${CMAKE_SOURCE_DIR}/googletest-1.15.2.txz")
	FetchContent_Declare(
		googletest
		URL "${HX_GOOGLETEST_ARCHIVE}"
		DOWNLOAD_EXTRACT_TIMESTAMP TRUE
	)
	set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
	FetchContent_MakeAvailable(googletest)
	if(MSVC)
		foreach(t gtest gtest_main gmock gmock_main)
			set_target_properties(${t} PROPERTIES MSVC_RUNTIME_LIBRARY
				"MultiThreaded$<$<CONFIG:Debug>:Debug>")
		endforeach()
	endif()

	# Use CMake to check that the tests are enumerable.
	enable_testing()
	include(GoogleTest)
	gtest_discover_tests(hxtest DISCOVERY_TIMEOUT 60)
endif()
