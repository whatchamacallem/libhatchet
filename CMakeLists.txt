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

cmake_minimum_required(VERSION 3.21)
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
	target_compile_definitions(hxtest PRIVATE HX_USE_GOOGLE_TEST=1)

	# Turn on everything so Clang-Tidy sees it.
	target_compile_definitions(hxtest PRIVATE HX_HARDENING_MODE=HX_HARDENING_MODE_DEBUG)
	target_compile_definitions(hxtest PRIVATE HX_USE_PROFILER=1)

	if(MSVC)
		target_compile_options(
			hxtest
			PRIVATE
				$<$<CONFIG:Debug>:/MTd>
				$<$<NOT:$<CONFIG:Debug>>:/MT>
				/MP
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
		)
	endif()

	# Extract GoogleTest 1.15.2 into the cache directory.
	#
	# Uses a per-configuration sub-directory so that switching between Win32/x64
	# or Debug/Release builds does not cause conflicts between incompatible
	# cached GoogleTest binaries. CMAKE_GENERATOR_PLATFORM is set by the Visual
	# Studio multi-config generator (-A x64 / -A Win32). CMAKE_BUILD_TYPE is set
	# by single-config generators (Ninja, Make) via -DCMAKE_BUILD_TYPE=. When
	# neither is set the cache lives directly under ./cache.
	if(CMAKE_GENERATOR_PLATFORM)
		set(HX_CACHE_SUFFIX "${CMAKE_GENERATOR_PLATFORM}")
	elseif(CMAKE_SIZEOF_VOID_P EQUAL 8)
		set(HX_CACHE_SUFFIX "x64")
	else()
		set(HX_CACHE_SUFFIX "x86")
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
	include(GoogleTest)
	gtest_discover_tests(hxtest DISCOVERY_TIMEOUT 60)
endif()
