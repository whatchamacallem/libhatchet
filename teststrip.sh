#!/bin/sh
# SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
# SPDX-License-Identifier: MIT
# This file is licensed under the terms of the LICENSE.md file.

trap 'trap "" INT; pkill -9 -P $$ 2>/dev/null; wait 2>/dev/null; exit 1' INT
set -eu

export POSIXLY_CORRECT=1

# Proves test suite can run without non-placement ::new() and ::delete. This is
# because there may be no general purpose allocator at all.
HX_BUILD_="-DHX_HARDENING_MODE=HX_HARDENING_MODE_NONE -DHX_PROVIDE_NEW_DELETE=0 \
	-DHX_USE_LIBCXX=0 -DHX_USE_LOGGING=1 -DHX_USE_THREADS=11"

HX_ERRORS_="-Wfatal-errors -Wall -Wextra -pedantic-errors -Werror -Wcast-qual \
	-Wdisabled-optimization -Wshadow -Wundef -Wconversion -Wdate-time         \
	-Wmissing-declarations -Wno-c2y-extensions -Wno-unknown-warning-option"

# 32-bit MUSL is not tested as it is unsupported on Ubuntu.
HX_FLAGS_="-Os -march=native -static -g0 -ffunction-sections -fdata-sections -ffast-math \
	-fno-asynchronous-unwind-tables"

HX_DIR_=$PWD

# Build artifacts are not retained.
rm -rf "$(readlink -f build)" build; ln -s "$(mktemp -d)" build && cd build

if [ "${1:-}" = "--verbose" ]; then
	set -o xtrace
fi

HX_CXX23_="23"
if [ -z "$(echo | musl-gcc -std=c++23 -dM -E -x c++ - 2>/dev/null | grep __cplusplus)" ]; then
	HX_CXX23_=""
	echo "C++23 not supported..."
fi

musl-gcc $HX_BUILD_ $HX_ERRORS_ $HX_FLAGS_ -I"$HX_DIR_/include" -std=c17 -c \
	"$HX_DIR_"/test/*.c & HX_PIDS_="$!"

# Test every supported version of the standard without libc++.
for HX_VERSION_ in 11 14 17 20 $HX_CXX23_; do
printf '%s ' "$HX_VERSION_"

for HX_FILE_ in "$HX_DIR_"/src/*.cpp "$HX_DIR_"/test/*.cpp; do
	musl-gcc $HX_BUILD_ $HX_ERRORS_ $HX_FLAGS_ -I"$HX_DIR_/include" -std=c++$HX_VERSION_ \
		-nostdinc++ -fno-exceptions -fno-rtti -flto=auto -c "$HX_FILE_"                  \
		-o "$(basename "$HX_FILE_" .cpp).o" & HX_PIDS_="$HX_PIDS_ $!"
done
for HX_PID_ in $HX_PIDS_; do wait "$HX_PID_" || exit 1; done
HX_PIDS_=""

# -Wno-maybe-uninitialized is passed to the link time optimizer because it has
# false positives. -Wl,--gc-sections and -flto=12 should reduce size.
musl-gcc $HX_BUILD_ $HX_ERRORS_ -Wno-maybe-uninitialized $HX_FLAGS_ -std=c++$HX_VERSION_  \
	-nostdinc++ -fno-exceptions -fno-rtti -Wl,--gc-sections -nodefaultlibs -flto=auto *.o \
	-lc -lpthread -lm -o hxtest

done
printf '\n'

# Only process and display the c++23 version.
strip -o hxtest-strip --strip-unneeded hxtest

{ set +o xtrace; } 2> /dev/null

if [ "${1:-}" = "--verbose" ]; then
	./hxtest-strip
else
	if ./hxtest-strip > console_output.txt 2>&1; then
		grep -E '\[  PASSED  \]|\[  FAILED  \]|FAILED TESTS' console_output.txt
	else
		cat console_output.txt
		exit 1
	fi
fi

cd ..

if [ "${1:-}" = "--verbose" ]; then
	./listsymbols.sh
fi

HX_SYMBOLS_=$(nm --radix=d --print-size build/hxtest)
echo "$HX_SYMBOLS_" | awk 'NF == 4 && $4 ~ /hx/ && $4 !~ /test/ && !seen[$1]++ {total += $2} \
	END {printf "= Total non-test libhatchet bytes: %d\n", total}'

size build/hxtest-strip

echo "🪓🪓🪓"
