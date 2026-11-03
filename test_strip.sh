#!/bin/sh
# SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
# SPDX-License-Identifier: MIT
# This file is licensed under the terms of the LICENSE.md file.

trap 'trap "" INT PIPE TERM; pkill -P $$; wait; exit 1' INT PIPE TERM
set -eu

cd "$(dirname "$(readlink -f "$0")")"

export POSIXLY_CORRECT=1

HX_DIR_=$PWD
HX_JOBS_="${HX_MAX_CORES:-$(getconf _NPROCESSORS_ONLN)}"

HX_BUILD_="-DHX_HARDENING_MODE=HX_HARDENING_MODE_NONE -DHX_PROVIDE_NEW_DELETE=0 \
	-DHX_USE_STD_LIB=0 -DHX_USE_LOGGING=1 -DHX_USE_THREADS=11 -DHX_SLAB_BUDGET_PERMANENT=0"

HX_ERRORS_="-Wfatal-errors -Wall -Wextra -pedantic-errors -Werror -Wcast-qual \
	-Wdisabled-optimization -Wshadow -Wundef -Wconversion -Wdate-time         \
	-Wmissing-declarations -Wno-c2y-extensions -Wno-unknown-warning-option"

HX_FLAGS_="-Os -march=native -static -g0 -ffunction-sections -fdata-sections -ffast-math \
	-fno-asynchronous-unwind-tables -fno-stack-protector -ffile-prefix-map=$HX_DIR_/="

# Build artifacts are not retained.
rm -rf "$(readlink -f build)" build
if [ "${HX_RAMDISK:-0}" = 1 ]; then mkdir build; else ln -s "$(mktemp -d)" build; fi
cd build

if [ "${1:-}" = "--verbose" ]; then
	set -o xtrace
fi

HX_CXX23_="23"
if [ -z "$(echo | musl-gcc -std=c++23 -dM -E -x c++ - 2>/dev/null | grep __cplusplus)" ]; then
	HX_CXX23_=""
	echo "C++23 not supported..."
fi

musl-gcc $HX_BUILD_ $HX_ERRORS_ $HX_FLAGS_ -I"$HX_DIR_/include" -std=c17 -c \
	"$HX_DIR_"/test/*.c

# Test every supported version of the standard without libc++.
for HX_VERSION_ in 11 14 17 20 $HX_CXX23_; do
printf '%s ' "$HX_VERSION_"

printf '%s\0' "$HX_DIR_"/src/*.cpp "$HX_DIR_"/test/*.cpp |
	xargs -0 -n 1 -P "$HX_JOBS_" sh -c '"$0" "$@" || exit 255'                           \
	musl-gcc $HX_BUILD_ $HX_ERRORS_ $HX_FLAGS_ -I"$HX_DIR_/include" -std=c++$HX_VERSION_ \
		-nostdinc++ -fno-exceptions -fno-rtti -flto=auto -c

# -Wno-maybe-uninitialized is passed to the link time optimizer because it has
# false positives. -Wl,--gc-sections and -flto=12 should reduce size.
musl-gcc $HX_FLAGS_ -Wno-maybe-uninitialized -Wl,--gc-sections -nodefaultlibs -flto=auto \
	*.o -lc -lpthread -lm -o hxtest

done
printf '\n'

# Only process and display the c++23 version.
strip -o hxtest-strip --strip-unneeded hxtest

{ set +o xtrace; } 2> /dev/null

if [ "${1:-}" = "--verbose" ]; then
	./hxtest-strip "check_stats 1" runtests
else
	if ./hxtest-strip "check_stats 1" runtests > console_output.txt 2>&1; then
		grep -E '\[  PASSED  \]|\[  FAILED  \]|FAILED TESTS' console_output.txt
	else
		cat console_output.txt
		exit 1
	fi
fi

cd ..
./list_symbols.sh "${1:-}"

echo
size build/hxtest-strip

echo "🪓🪓🪓"
