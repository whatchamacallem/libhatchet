#!/bin/sh
# SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
# SPDX-License-Identifier: MIT
# This file is licensed under the terms of the LICENSE.md file.
#
# Builds target/test_target.cpp with Meson and Ninja using clang and gcc, with
# and without the hx module, and runs its tests. It reports with the Test
# Anything Protocol (TAP) and "meson test" consumes the report. Then builds it
# with the Pico SDK when that is installed. See target/README.md.

trap 'trap "" INT PIPE TERM; pkill -P $$; wait; exit 1' INT PIPE TERM
set -eu

cd "$(dirname "$(readlink -f "$0")")"

export POSIXLY_CORRECT=1

HX_JOBS_="${HX_MAX_CORES:-$(getconf _NPROCESSORS_ONLN)}"

HX_VERBOSE_=""
if [ "${1:-}" = "--verbose" ]; then
	HX_VERBOSE_="--verbose"
elif [ -n "${1:-}" ]; then
	echo "usage_error: $0 [--verbose]"
	exit 1
fi

HX_CLANG_NATIVE_=meson/clang.ini
HX_GCC_NATIVE_=meson/gcc.ini
if [ "$(uname -s)" = Darwin ]; then
	PATH="/opt/homebrew/bin:/opt/homebrew/sbin${PATH+:$PATH}"
	if ! command -v brew > /dev/null 2>&1; then
		echo "error: Install brew and then run install_brew.sh."
		exit 1
	fi
	HX_CLANG_NATIVE_=meson/macos-clang.ini
	HX_GCC_NATIVE_=meson/macos-gcc.ini
elif [ -x /opt/rh/gcc-toolset-15/root/usr/bin/g++ ]; then
	HX_GCC_NATIVE_=meson/gcc-toolset-15.ini
fi

# Builds with the C library headers only and no C++ standard library.
HX_C_FLAGS_="-DHX_HARDENING_MODE=HX_HARDENING_MODE_DEBUG -ggdb3"
HX_CPP_FLAGS_="$HX_C_FLAGS_ -DHX_USE_STD_LIB=0 -DHX_USE_NAMESPACE=hx -DHX_USE_CONSOLE=2 \
	-std=c++23 -nostdinc++ -fno-exceptions -fno-rtti"
HX_LINK_FLAGS_="-nodefaultlibs -lc -lpthread -lm"

run_target() {
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

if [ -n "$HX_VERBOSE_" ]; then
	set -o xtrace
fi

# -- Meson ---------------------------------------------------------------------

echo "$HX_CLANG_NATIVE_ meson+ninja with textual includes..."
run_target --native-file="$HX_CLANG_NATIVE_" -Dbuild_module=disabled

echo "$HX_CLANG_NATIVE_ meson+ninja with named module..."
run_target --native-file="$HX_CLANG_NATIVE_"

echo "$HX_GCC_NATIVE_ meson+ninja with textual includes..."
run_target --native-file="$HX_GCC_NATIVE_" -Dbuild_module=disabled

# Textual includes are built with GCC 14, e.g. Debian 13.
echo "$HX_GCC_NATIVE_ meson+ninja with named module..."
run_target --native-file="$HX_GCC_NATIVE_"

# -- Pico SDK ------------------------------------------------------------------

HX_PICO_SDK_="$HOME/pico-sdk"
PICO_BOARD="${PICO_BOARD:-pico2}"
if [ -f "$HX_PICO_SDK_/pico_sdk_init.cmake" ] && command -v arm-none-eabi-gcc > /dev/null 2>&1; then
	echo "$PICO_BOARD cmake+ninja with the Pico SDK..."
	rm -rf "$(readlink -f build)" build; mkdir build
	if [ -n "$HX_VERBOSE_" ]; then
		cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -Wno-dev \
			-DPICO_BOARD="$PICO_BOARD" -DPICO_SDK_PATH="$HX_PICO_SDK_"
		ninja -v -C build -j "$HX_JOBS_"
	else
		if ! cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -Wno-dev \
				-DPICO_BOARD="$PICO_BOARD" -DPICO_SDK_PATH="$HX_PICO_SDK_"   \
				> build/cmake_out.txt 2>&1; then
			cat build/cmake_out.txt
			echo "error: cmake failed"
			exit 1
		fi
		if ! ninja -C build -j "$HX_JOBS_" >build/ninja_out.txt 2>&1; then
			cat build/ninja_out.txt
			echo "error: ninja build failed"
			exit 1
		fi
	fi
	ls build/hxtest_target.uf2
else
	echo "Pico SDK skipped. Requires arm-none-eabi-gcc and HX_PICO_SDK_=$HX_PICO_SDK_."
fi

echo 🪓🪓🪓
