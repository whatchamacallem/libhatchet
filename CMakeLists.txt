# SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
# SPDX-License-Identifier: MIT
# This file is licensed under the terms of the LICENSE.md file.
#
# This is a CMake build that uses Google Test instead of the <hx/hxtest.h>
# reimplementation. It is significantly slower than the test scripts when doing
# a clean build.
#
# This file is for testing and is not intended for building a library for your
# application. To add libhatchet to your programs just add the src and include
# directories directly.

cmake_minimum_required(VERSION 3.10)
project(libhatchet)

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
target_compile_definitions(hxtest PRIVATE HX_USE_GOOGLE_TEST)

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
      -fno-exceptions
      -ggdb3
      -Wall
      -Wextra
      -Werror
  )
endif()

# Download or un-tar GoogleTest into a local workspace cache.
#
# Uses a per-configuration sub-directory so that switching between Win32/x64 or
# Debug/Release builds does not cause conflicts between incompatible cached
# GoogleTest binaries. CMAKE_GENERATOR_PLATFORM is set by the Visual Studio
# multi-config generator (-A x64 / -A Win32). CMAKE_BUILD_TYPE is set by
# single-config generators (Ninja, Make) via -DCMAKE_BUILD_TYPE=. When neither
# is set the cache lives directly under ./cache.
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
if(WIN32)
  set(HX_GOOGLETEST_ARCHIVE "https://github.com/google/googletest/releases/download/v1.15.2/googletest-1.15.2.tar.gz")
else()
  set(HX_GOOGLETEST_ARCHIVE "${CMAKE_SOURCE_DIR}/googletest-1.15.2.txz")
endif()
FetchContent_Declare(
  googletest
  URL "${HX_GOOGLETEST_ARCHIVE}"
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
set(gtest_force_shared_crt OFF CACHE BOOL "Link GoogleTest with static CRT on MSVC" FORCE)
FetchContent_MakeAvailable(googletest)

# Use CMake to check that the tests are enumerable.
include(GoogleTest)
gtest_discover_tests(hxtest DISCOVERY_TIMEOUT 60)
