#!/bin/sh
# SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
# SPDX-License-Identifier: MIT
# This file is licensed under the terms of the LICENSE.md file.
#
# Builds target/test_target.cpp with Meson and Ninja using clang and gcc, with
# and without the hx module, and runs its tests. It reports with the Test
# Anything Protocol (TAP) and "meson test" consumes the report. Then builds it
# for each board in HX_BOARDS_ whose SDK is installed and tests it on the board
# when the board is connected. Stops at the first board that fails. With
# --require-all every board has to be built and connected. --target runs only
# "meson" or one board, e.g. --target esp32c6. See target/README.md and
# target/iot.md.
#
# usage: test_target.sh [--verbose] [--require-all] [--target meson|<board>]

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

# The boards there is hardware for, as <sdk>:<target>. has_sdk_<sdk> checks
# that the SDK is installed, build_<sdk> builds the target in build/<target> and
# target/flash_<sdk>.sh tests it on the board. See target/iot.md.
HX_BOARDS_="pico:pico2 esp32:esp32c6"

HX_PICO_SDK_="$HOME/pico-sdk"
HX_ESP_IDF_="$HOME/esp-idf"

# Set by parse_arguments. "--verbose" or empty.
HX_VERBOSE_=""

# Set by parse_arguments. 1 when --require-all was passed.
HX_REQUIRE_ALL_=0

# Set by parse_arguments. "meson", a <target> from HX_BOARDS_ or empty for all.
HX_SELECTED_=""

# Set by select_native_files. The Meson native files for clang and gcc.
HX_CLANG_NATIVE_=""
HX_GCC_NATIVE_=""

usage_error() {
	echo "usage_error: $0 [--verbose] [--require-all] [--target meson|<board>]"
	echo "boards: $HX_BOARDS_"
	exit 1
}

parse_arguments() {
	while [ $# -gt 0 ]; do
		case "$1" in
			--verbose) HX_VERBOSE_="--verbose" ;;
			--require-all) HX_REQUIRE_ALL_=1 ;;
			--target)
				[ $# -ge 2 ] || usage_error
				HX_SELECTED_="$2"
				shift
				;;
			*) usage_error ;;
		esac
		shift
	done

	# The selection has to be meson or a board in HX_BOARDS_.
	if [ -n "$HX_SELECTED_" ] && [ "$HX_SELECTED_" != meson ]; then
		for HX_BOARD_ in $HX_BOARDS_; do
			[ "${HX_BOARD_#*:}" = "$HX_SELECTED_" ] && return 0
		done
		usage_error
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

# Builds and tests with clang and gcc, with and without the named module.
test_meson() {
	echo "$HX_CLANG_NATIVE_ meson+ninja with textual includes..."
	build_and_test_with_meson --native-file="$HX_CLANG_NATIVE_" -Dbuild_module=disabled

	echo "$HX_CLANG_NATIVE_ meson+ninja with named module..."
	build_and_test_with_meson --native-file="$HX_CLANG_NATIVE_"

	echo "$HX_GCC_NATIVE_ meson+ninja with textual includes..."
	build_and_test_with_meson --native-file="$HX_GCC_NATIVE_" -Dbuild_module=disabled

	# Textual includes are built with GCC 14, e.g. Debian 13.
	echo "$HX_GCC_NATIVE_ meson+ninja with named module..."
	build_and_test_with_meson --native-file="$HX_GCC_NATIVE_"
}

# -- Pico SDK ------------------------------------------------------------------

# Succeeds when the Pico SDK is installed. Otherwise prints what it requires.
has_sdk_pico() {
	if [ -f "$HX_PICO_SDK_/pico_sdk_init.cmake" ] && command -v arm-none-eabi-gcc > /dev/null 2>&1; then
		return 0
	fi
	echo "The Pico SDK requires arm-none-eabi-gcc and $HX_PICO_SDK_."
	return 1
}

# Builds board $1 in build/$1. The subdirectory leaves build/hxtest and keeps its
# CMakeCache.txt from being found by test_cmake.sh.
build_pico() {
	echo "$1 cmake+ninja with the Pico SDK..."
	if [ -n "$HX_VERBOSE_" ]; then
		cmake -S . -B "build/$1" -G Ninja -DCMAKE_BUILD_TYPE=Release -Wno-dev \
			-DPICO_BOARD="$1" -DPICO_SDK_PATH="$HX_PICO_SDK_"
		ninja -v -C "build/$1" -j "$HX_JOBS_"
	else
		if ! cmake -S . -B "build/$1" -G Ninja -DCMAKE_BUILD_TYPE=Release -Wno-dev \
				-DPICO_BOARD="$1" -DPICO_SDK_PATH="$HX_PICO_SDK_"                \
				> build/cmake_out.txt 2>&1; then
			cat build/cmake_out.txt
			echo "error: cmake failed"
			exit 1
		fi
		if ! ninja -C "build/$1" -j "$HX_JOBS_" >build/ninja_out.txt 2>&1; then
			cat build/ninja_out.txt
			echo "error: ninja build failed"
			exit 1
		fi
	fi
	ls "build/$1/hxtest_target.uf2"
}

# -- ESP-IDF -------------------------------------------------------------------

# Succeeds when ESP-IDF is installed. Otherwise prints what it requires.
has_sdk_esp32() {
	if [ -f "$HX_ESP_IDF_/export.sh" ]; then
		return 0
	fi
	echo "ESP-IDF requires $HX_ESP_IDF_."
	return 1
}

# Builds chip $1 in build/$1 for the same reasons as the Pico SDK. The ESP-IDF
# environment is only entered in a subshell. A different IDF_PATH is not used.
# export.sh requires IDF_PATH when sourced by sh and does not support set -u.
build_esp32() {
	echo "$1 idf.py+ninja with ESP-IDF..."
	(
		export IDF_PATH="$HX_ESP_IDF_"
		set +u
		. "$HX_ESP_IDF_/export.sh" > build/idf_export_out.txt 2>&1 || {
			cat build/idf_export_out.txt
			echo "error: ESP-IDF export.sh failed"
			exit 1
		}
		if [ -n "$HX_VERBOSE_" ]; then
			idf.py -v -B "build/$1" -DIDF_TARGET="$1" build
		elif ! idf.py -B "build/$1" -DIDF_TARGET="$1" build \
				> build/idf_out.txt 2>&1; then
			cat build/idf_out.txt
			echo "error: idf.py build failed"
			exit 1
		fi
	)
	ls "build/$1/hxtest_target.bin"
}

# -- Boards --------------------------------------------------------------------

# Builds an <sdk>:<target> from HX_BOARDS_ when the SDK is installed and tests
# it when the board is connected. A connected board has to pass. With
# --require-all the SDK has to be installed and the board connected. A board
# that is not connected is built and passes with a warning. Exits at the first
# failure. The flash script is run with bash because the executable bits are not
# tested.
test_board() {
	HX_SDK_="${1%%:*}"
	HX_TARGET_="${1#*:}"
	HX_FLASH_="target/flash_$HX_SDK_.sh"

	if ! "has_sdk_$HX_SDK_"; then
		if [ "$HX_REQUIRE_ALL_" = 1 ]; then
			echo "error: --require-all: $HX_TARGET_ can't be built."
			exit 1
		fi
		echo "$HX_TARGET_ skipped."
		return 0
	fi

	# --require-all stops before the build when the board is missing.
	HX_CONNECTED_=1
	bash "$HX_FLASH_" --target "$HX_TARGET_" --connected || HX_CONNECTED_=0
	if [ "$HX_CONNECTED_" = 0 ] && [ "$HX_REQUIRE_ALL_" = 1 ]; then
		echo "error: --require-all: No $HX_TARGET_ is connected."
		exit 1
	fi

	# build exists when test_meson did not run. The board is built from scratch.
	mkdir -p build
	rm -rf "build/$HX_TARGET_"
	"build_$HX_SDK_" "$HX_TARGET_"

	if [ "$HX_CONNECTED_" = 0 ]; then
		echo "warning: $HX_TARGET_ built but not tested. None is connected."
		return 0
	fi

	echo "$HX_TARGET_ testing on the board with $HX_FLASH_..."
	if ! bash "$HX_FLASH_" --target "$HX_TARGET_" --no-build; then
		echo "error: $HX_TARGET_ failed on the board."
		exit 1
	fi
}

# -- Main ----------------------------------------------------------------------

main() {
	parse_arguments "$@"

	[ "$(uname -s)" = Darwin ] && add_homebrew_path
	select_native_files

	[ -n "$HX_VERBOSE_" ] && set -o xtrace

	if [ -z "$HX_SELECTED_" ] || [ "$HX_SELECTED_" = meson ]; then
		test_meson
	fi

	for HX_BOARD_ in $HX_BOARDS_; do
		if [ -z "$HX_SELECTED_" ] || [ "$HX_SELECTED_" = "${HX_BOARD_#*:}" ]; then
			test_board "$HX_BOARD_"
		fi
	done

	echo 🪓🪓🪓
}

main "$@"
