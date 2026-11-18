#!/bin/sh
# SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
# SPDX-License-Identifier: MIT
# This file is licensed under the terms of the LICENSE.md file.
#
# Builds target/test_target.cpp with Meson and Ninja using clang and gcc, with
# and without the hx module, and runs its tests. It reports with the Test
# Anything Protocol (TAP) and "meson test" consumes the report. Then builds it
# with the Pico SDK and with ESP-IDF when they are installed. See
# target/README.md.

trap 'trap "" INT PIPE TERM; pkill -P $$; wait; exit 1' INT PIPE TERM
set -eu

cd "$(dirname "$(readlink -f "$0")")"

export POSIXLY_CORRECT=1

HX_JOBS_="${HX_MAX_JOBS:-$(getconf _NPROCESSORS_ONLN)}"

# Builds with the C library headers only and no C++ standard library.
HX_C_FLAGS_="-DHX_HARDENING_MODE=HX_HARDENING_MODE_DEBUG -ggdb3"
HX_CPP_FLAGS_="$HX_C_FLAGS_ -DHX_USE_STD_LIB=0 -DHX_USE_NAMESPACE=hx -DHX_USE_CONSOLE=2 \
	-std=c++23 -nostdinc++ -fno-exceptions -fno-rtti"
HX_LINK_FLAGS_="-nodefaultlibs -lc -lpthread -lm"

HX_PICO_SDK_="$HOME/pico-sdk"
PICO_BOARD="${PICO_BOARD:-pico2}"

# esp32 is the WT32-ETH01's chip.
HX_ESP_IDF_="$HOME/esp-idf"
IDF_TARGET="${IDF_TARGET:-esp32}"

# Set by parse_arguments. "--verbose" or empty.
HX_VERBOSE_=""

# Set by select_native_files. The Meson native files for clang and gcc.
HX_CLANG_NATIVE_=""
HX_GCC_NATIVE_=""

parse_arguments() {
	if [ "${1:-}" = "--verbose" ]; then
		HX_VERBOSE_="--verbose"
	elif [ -n "${1:-}" ]; then
		echo "usage_error: $0 [--verbose]"
		exit 1
	fi
}

# -- Meson ---------------------------------------------------------------------

# Adds the Homebrew directories to PATH on macOS. Run install_brew.sh.
add_homebrew_path() {
	PATH="/opt/homebrew/bin:/opt/homebrew/sbin${PATH+:$PATH}"
	if ! command -v brew > /dev/null 2>&1; then
		echo "error: Install brew and then run install_brew.sh."
		exit 1
	fi
}

select_native_files() {
	HX_CLANG_NATIVE_=meson/clang.ini
	HX_GCC_NATIVE_=meson/gcc.ini
	if [ "$(uname -s)" = Darwin ]; then
		HX_CLANG_NATIVE_=meson/macos-clang.ini
		HX_GCC_NATIVE_=meson/macos-gcc.ini
	elif [ -x /opt/rh/gcc-toolset-15/root/usr/bin/g++ ]; then
		HX_GCC_NATIVE_=meson/gcc-toolset-15.ini
	fi
}

# The arguments are passed to "meson setup".
build_and_test_with_meson() {
	# Build artifacts are not retained.
	rm -rf "$(readlink -f build)" build; mkdir build
	if [ -n "$HX_VERBOSE_" ]; then
		meson setup build --buildtype=debug -Dc_args="$HX_C_FLAGS_"       \
			-Dcpp_args="$HX_CPP_FLAGS_" -Dcpp_link_args="$HX_LINK_FLAGS_" "$@"
		ninja -v -C build -j "$HX_JOBS_"
	else
		if ! meson setup build --buildtype=debug -Dc_args="$HX_C_FLAGS_"      \
				-Dcpp_args="$HX_CPP_FLAGS_" -Dcpp_link_args="$HX_LINK_FLAGS_" \
				"$@" > build/meson_setup_out.txt 2>&1; then
			cat build/meson_setup_out.txt
			echo "error: meson setup failed"
			exit 1
		fi
		if ! ninja -C build -j "$HX_JOBS_" >build/ninja_out.txt 2>&1; then
			cat build/ninja_out.txt
			echo "error: ninja build failed"
			exit 1
		fi
	fi

	# The link lets the .vscode configuration debug it as build/hxtest.
	ln -sf hxtest_target build/hxtest

	meson test -C build --no-rebuild --print-errorlogs $HX_VERBOSE_
}

# -- Pico SDK ------------------------------------------------------------------

# Built in a subdirectory to leave build/hxtest and to keep its CMakeCache.txt
# from being found by test_cmake.sh.
build_with_pico_sdk() {
	if [ -f "$HX_PICO_SDK_/pico_sdk_init.cmake" ] && command -v arm-none-eabi-gcc > /dev/null 2>&1; then
		echo "$PICO_BOARD cmake+ninja with the Pico SDK..."
		if [ -n "$HX_VERBOSE_" ]; then
			cmake -S . -B build/pico -G Ninja -DCMAKE_BUILD_TYPE=Release -Wno-dev \
				-DPICO_BOARD="$PICO_BOARD" -DPICO_SDK_PATH="$HX_PICO_SDK_"
			ninja -v -C build/pico -j "$HX_JOBS_"
		else
			if ! cmake -S . -B build/pico -G Ninja -DCMAKE_BUILD_TYPE=Release -Wno-dev \
					-DPICO_BOARD="$PICO_BOARD" -DPICO_SDK_PATH="$HX_PICO_SDK_"        \
					> build/cmake_out.txt 2>&1; then
				cat build/cmake_out.txt
				echo "error: cmake failed"
				exit 1
			fi
			if ! ninja -C build/pico -j "$HX_JOBS_" >build/ninja_out.txt 2>&1; then
				cat build/ninja_out.txt
				echo "error: ninja build failed"
				exit 1
			fi
		fi
		ls build/pico/hxtest_target.uf2
	else
		echo "Pico SDK skipped. Requires arm-none-eabi-gcc and HX_PICO_SDK_=$HX_PICO_SDK_."
	fi
}

# -- ESP-IDF -------------------------------------------------------------------

# Built in a subdirectory for the same reasons as the Pico SDK. The ESP-IDF
# environment is only entered in a subshell. A different IDF_PATH is not used.
# export.sh requires IDF_PATH when sourced by sh and does not support set -u.
build_with_esp_idf() {
	if [ -f "$HX_ESP_IDF_/export.sh" ]; then
		echo "$IDF_TARGET idf.py+ninja with ESP-IDF..."
		(
			export IDF_PATH="$HX_ESP_IDF_"
			set +u
			. "$HX_ESP_IDF_/export.sh" > build/idf_export_out.txt 2>&1 || {
				cat build/idf_export_out.txt
				echo "error: ESP-IDF export.sh failed"
				exit 1
			}
			if [ -n "$HX_VERBOSE_" ]; then
				idf.py -v -B "build/$IDF_TARGET" -DIDF_TARGET="$IDF_TARGET" build
			elif ! idf.py -B "build/$IDF_TARGET" -DIDF_TARGET="$IDF_TARGET" build \
					> build/idf_out.txt 2>&1; then
				cat build/idf_out.txt
				echo "error: idf.py build failed"
				exit 1
			fi
		)
		ls "build/$IDF_TARGET/hxtest_target.bin"
	else
		echo "ESP-IDF skipped. Requires HX_ESP_IDF_=$HX_ESP_IDF_."
	fi
}

# -- Main ----------------------------------------------------------------------

main() {
	parse_arguments "$@"

	[ "$(uname -s)" = Darwin ] && add_homebrew_path
	select_native_files

	[ -n "$HX_VERBOSE_" ] && set -o xtrace

	echo "$HX_CLANG_NATIVE_ meson+ninja with textual includes..."
	build_and_test_with_meson --native-file="$HX_CLANG_NATIVE_" -Dbuild_module=disabled

	echo "$HX_CLANG_NATIVE_ meson+ninja with named module..."
	build_and_test_with_meson --native-file="$HX_CLANG_NATIVE_"

	echo "$HX_GCC_NATIVE_ meson+ninja with textual includes..."
	build_and_test_with_meson --native-file="$HX_GCC_NATIVE_" -Dbuild_module=disabled

	# Textual includes are built with GCC 14, e.g. Debian 13.
	echo "$HX_GCC_NATIVE_ meson+ninja with named module..."
	build_and_test_with_meson --native-file="$HX_GCC_NATIVE_"

	build_with_pico_sdk

	build_with_esp_idf

	echo 🪓🪓🪓
}

main "$@"
