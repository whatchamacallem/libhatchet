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

# Build artifacts are not retained.
clean_build() {
	rm -rf "$(readlink -f build)" build; mkdir build
}

# Shows the output of a build step only when it fails or with --verbose.
build_step() {
	if [ -n "$HX_VERBOSE_" ]; then
		"$@"
	elif ! "$@" > build/build_step_out.txt 2>&1; then
		cat build/build_step_out.txt
		echo "error: $1 failed"
		exit 1
	fi
}

if [ -n "$HX_VERBOSE_" ]; then
	set -o xtrace
fi

# -- Meson ---------------------------------------------------------------------

# build_module=auto uses textual includes when the compiler has no modules, e.g.
# GCC 14 on Debian 13.
for HX_NATIVE_ in "$HX_CLANG_NATIVE_" "$HX_GCC_NATIVE_"; do
	for HX_MODULE_ in disabled auto; do
		echo "$HX_NATIVE_ meson+ninja with build_module=$HX_MODULE_..."
		clean_build
		build_step meson setup build --buildtype=debug --native-file="$HX_NATIVE_" \
			-Dbuild_module="$HX_MODULE_" -Dc_args="$HX_C_FLAGS_"                 \
			-Dcpp_args="$HX_CPP_FLAGS_" -Dcpp_link_args="$HX_LINK_FLAGS_"
		build_step meson compile -C build -j "$HX_JOBS_" $HX_VERBOSE_

		# The link lets the .vscode configuration debug it as build/hxtest.
		ln -sf hxtest_target build/hxtest

		meson test -C build --no-rebuild --print-errorlogs $HX_VERBOSE_
	done
done

# -- Pico SDK ------------------------------------------------------------------

# The SDK is a CMake project and CMakeLists.txt builds target/test_target.cpp
# with it when PICO_BOARD is set. It is at PICO_SDK_PATH or at ~/pico-sdk where
# "install_deb.sh --sdks" installs it. Running it takes a board. See
# target/libhatchet_flash_pico.sh.
PICO_SDK_PATH="${PICO_SDK_PATH:-$HOME/pico-sdk}"
PICO_BOARD="${PICO_BOARD:-pico2}"
if [ -f "$PICO_SDK_PATH/pico_sdk_init.cmake" ] && command -v arm-none-eabi-gcc > /dev/null 2>&1; then
	echo "$PICO_BOARD cmake+ninja with the Pico SDK..."
	clean_build
	build_step cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -Wno-dev \
		-DPICO_BOARD="$PICO_BOARD" -DPICO_SDK_PATH="$PICO_SDK_PATH"
	build_step cmake --build build -j "$HX_JOBS_" $HX_VERBOSE_
	ls build/hxtest_target.uf2
else
	echo "Pico SDK skipped. Requires arm-none-eabi-gcc and PICO_SDK_PATH=$PICO_SDK_PATH."
fi

echo 🪓🪓🪓
