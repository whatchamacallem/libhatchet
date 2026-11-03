#!/bin/sh
# SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
# SPDX-License-Identifier: MIT
# This file is licensed under the terms of the LICENSE.md file.

trap 'trap "" INT PIPE TERM; pkill -P $$; wait; exit 1' INT PIPE TERM
set -eu

cd "$(dirname "$(readlink -f "$0")")"

export POSIXLY_CORRECT=1

HX_JOBS_="${HX_MAX_CORES:-$(getconf _NPROCESSORS_ONLN)}"

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

# Builds a module that uses MUSL libc headers only.
HX_C_FLAGS_="-DHX_HARDENING_MODE=HX_HARDENING_MODE_DEBUG -ggdb3"
HX_CPP_FLAGS_="$HX_C_FLAGS_ -DHX_USE_STD_LIB=0 -DHX_USE_NAMESPACE=hx -DHX_USE_CONSOLE=2 \
	-std=c++23 -nostdinc++ -fno-exceptions -fno-rtti"
HX_LINK_FLAGS_="-nodefaultlibs -lc -lpthread -lm"

HX_CORRECT_="example/example_correct.txt"

run_example() {
	# Build artifacts are not retained.
	rm -rf "$(readlink -f build)" build; mkdir build
	if [ "${1:-}" = "--verbose" ]; then
		meson setup build --buildtype=debug -Dc_args="$HX_C_FLAGS_"       \
			-Dcpp_args="$HX_CPP_FLAGS_" -Dcpp_link_args="$HX_LINK_FLAGS_" \
			-Dbuild_example=true "$@"
		ninja -v -C build -j "$HX_JOBS_"
	else
		if ! meson setup build --buildtype=debug -Dc_args="$HX_C_FLAGS_"      \
				-Dcpp_args="$HX_CPP_FLAGS_" -Dcpp_link_args="$HX_LINK_FLAGS_" \
				-Dbuild_example=true "$@" > build/meson_setup_out.txt 2>&1; then
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

	cp example/example.cfg build
	ln -sf example/hxexample build/hxtest

	if [ ! -f "$HX_CORRECT_" ]; then
		echo "WARNING: regenerating $HX_CORRECT_..."
		echo exit | (cd build && ./hxtest) 2>/dev/null > "$HX_CORRECT_"
	fi

	echo exit | (cd build && ./hxtest) 2>/dev/null > build/hxexample_out.txt
	if ! git --no-pager diff --no-index "$HX_CORRECT_" build/hxexample_out.txt; then
		echo "error: output differs from $HX_CORRECT_"
		exit 1
	fi

	# Require that the test suite was not linked against.
	if grep -q "help_skipped" build/hxexample_out.txt; then
		echo "error: help_skipped found in output"
		exit 1
	fi
}

if [ "${1:-}" = "--verbose" ]; then
	set -o xtrace
fi

echo "$HX_CLANG_NATIVE_ meson+ninja with textual includes..."
run_example --native-file="$HX_CLANG_NATIVE_" -Dbuild_module=disabled

echo "$HX_CLANG_NATIVE_ meson+ninja with named module..."
run_example --native-file="$HX_CLANG_NATIVE_"

echo "$HX_GCC_NATIVE_ meson+ninja with textual includes..."
run_example --native-file="$HX_GCC_NATIVE_" -Dbuild_module=disabled

# Textual includes are built with GCC 14, e.g. Debian 13.
echo "$HX_GCC_NATIVE_ meson+ninja with named module..."
run_example --native-file="$HX_GCC_NATIVE_"

echo "🪓🪓🪓"
