#!/bin/sh
# SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
# SPDX-License-Identifier: MIT
# This file is licensed under the terms of the LICENSE.md file.

# Prevent leaking background tasks.
trap '{ set +o xtrace; } 2> /dev/null
	trap - 1 2 3 6 15
	for pid in $(pgrep -g "$$" 2>/dev/null); do
		[ "$pid" = "$$" ] && continue
		kill -9 "$pid" 2>/dev/null
	done
	exit 1
' 1 2 3 6 15

set -eu

export POSIXLY_CORRECT=1

VERBOSE=""
if [ "${1:-}" = "--verbose" ]; then
	VERBOSE="--verbose"
elif [ -n "${1:-}" ]; then
	echo "Usage: $0 [--verbose]"
	exit 1
fi

# Builds a module that uses MUSL lic headers only.
C_FLAGS="-DHX_HARDENING_MODE=HX_HARDENING_MODE_DEBUG -m32 -ggdb3"
CPP_FLAGS="$C_FLAGS -DHX_USE_LIBCXX=0 -DHX_USE_NAMESPACE=hx -DHX_USE_CONSOLE=2 \
	-std=c++23 -nostdinc++ -fno-exceptions -fno-rtti"
LINK_FLAGS="-m32 -nodefaultlibs -lc -lpthread -lm"

CORRECT="example/example_correct.txt"

run_example() {
	# Build artifacts are not retained.
	rm -rf "$(readlink -f build)" build
	if [ "$VERBOSE" = 1 ]; then
		meson setup build --buildtype=debug -Dc_args="$C_FLAGS" -Dcpp_args="$CPP_FLAGS" \
			-Dcpp_link_args="$LINK_FLAGS" -Dbuild_example=true "$@"
		ninja -v -C build
	else
		meson setup build --buildtype=debug -Dc_args="$C_FLAGS" -Dcpp_args="$CPP_FLAGS" \
			-Dcpp_link_args="$LINK_FLAGS" -Dbuild_example=true "$@" >/dev/null
		ninja -C build >/dev/null
	fi

	cp example/example.cfg build
	ln -sf example/hxexample build/hxtest

	if [ ! -f "$CORRECT" ]; then
		echo "WARNING: regenerating $CORRECT..."
		echo exit | (cd build && ./hxtest) 2>/dev/null > "$CORRECT"
	fi

	echo exit | (cd build && ./hxtest) 2>/dev/null > build/hxexample_out.txt
	if ! diff -u "$CORRECT" build/hxexample_out.txt; then
		echo "error: output differs from $CORRECT"
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
