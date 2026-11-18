# SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
# SPDX-License-Identifier: MIT
# This file is licensed under the terms of the LICENSE.md file.
#
# The ESP-IDF component that builds test_target.cpp with src/*.cpp. Its name is
# this directory's, target. The ESP-IDF section of the top level CMakeLists.txt
# adds it. See README.md.

file(GLOB HX_SOURCES "${CMAKE_CURRENT_LIST_DIR}/../src/*.cpp")

idf_component_register(
	SRCS test_target.cpp ${HX_SOURCES}
	INCLUDE_DIRS ../include
	PRIV_REQUIRES pthread
)

# ESP-IDF has pthreads. Its newlib has a <threads.h> without an implementation.
target_compile_definitions(${COMPONENT_LIB} PRIVATE
	HX_HARDENING_MODE=HX_HARDENING_MODE_VERBOSE
	HX_USE_FILE_IO=0
	HX_USE_THREADS=1
	HX_USE_STD_LIB=0
	HX_USE_PROFILER=0
	HX_USE_CONSOLE=0
)
target_compile_options(${COMPONENT_LIB} PRIVATE
	-fno-exceptions
	-fno-rtti
	-nostdinc++
)
