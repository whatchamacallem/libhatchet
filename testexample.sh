#!/bin/sh
# SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
# SPDX-License-Identifier: MIT
# This file is licensed under the terms of the LICENSE.md file.

trap 'trap "" INT; pkill -9 -P $$ 2>/dev/null; wait 2>/dev/null; exit 1' INT
set -eu

export POSIXLY_CORRECT=1

HX_VERBOSE_=""
if [ "${1:-}" = "--verbose" ]; then
	HX_VERBOSE_="--verbose"
elif [ -n "${1:-}" ]; then
	echo "Usage: $0 [--verbose]"
	exit 1
fi

# Builds a module that uses MUSL lic headers only.
HX_C_FLAGS_="-DHX_HARDENING_MODE=HX_HARDENING_MODE_DEBUG -m32 -ggdb3"
HX_CPP_FLAGS_="$HX_C_FLAGS_ -DHX_USE_LIBCXX=0 -DHX_USE_NAMESPACE=hx -DHX_USE_CONSOLE=2 \
	-std=c++23 -nostdinc++ -fno-exceptions -fno-rtti"
HX_LINK_FLAGS_="-m32 -nodefaultlibs -lc -lpthread -lm"

HX_CORRECT_="example/example_correct.txt"

run_example() {
	# Build artifacts are not retained.
	rm -rf "$(readlink -f build)" build; mkdir build
	if [ "$HX_VERBOSE_" = 1 ]; then
		meson setup build --buildtype=debug -Dc_args="$HX_C_FLAGS_"       \
			-Dcpp_args="$HX_CPP_FLAGS_" -Dcpp_link_args="$HX_LINK_FLAGS_" \
			-Dbuild_example=true "$@"
		ninja -v -C build
	else
		if ! meson setup build --buildtype=debug -Dc_args="$HX_C_FLAGS_"      \
				-Dcpp_args="$HX_CPP_FLAGS_" -Dcpp_link_args="$HX_LINK_FLAGS_" \
				-Dbuild_example=true "$@" > build/meson_setup_out.txt 2>&1; then
			cat build/meson_setup_out.txt
			echo "error: meson setup failed"
			exit 1
		fi
		if ! ninja -C build >build/ninja_out.txt 2>&1; then
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
	if ! diff -u "$HX_CORRECT_" build/hxexample_out.txt; then
		echo "error: output differs from $HX_CORRECT_"
		exit 1
	fi
}

export CC=clang
export CXX=clang++
echo "$CXX meson+ninja with textual includes..."
run_example
echo "$CXX meson+ninja with named module..."
run_example -Dbuild_module=true

export CC=gcc
export CXX=g++
echo "$CXX meson+ninja with textual includes..."
run_example
echo "$CXX meson+ninja with named module..."
run_example -Dbuild_module=true

echo "Output matches"
echo "Run hxtest from the build directory to test interactively."

echo "🪓🪓🪓"
