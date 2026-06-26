#!/bin/sh
# SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
# SPDX-License-Identifier: MIT
# This file is licensed under the terms of the LICENSE.md file.
#
# sudo apt install musl musl-dev musl-tools

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

BUILD="-DHX_USE_LIBCXX=0 -DHX_HARDENING_MODE=HX_HARDENING_MODE_NONE \
    -DHX_USE_LOGGING=1 -DHX_USE_THREADS=11"

ERRORS="-Wall -Wextra -pedantic-errors -Werror -Wfatal-errors -Wcast-qual   \
	-Wdisabled-optimization -Wshadow -Wundef -Wconversion -Wdate-time       \
	-Wmissing-declarations -Wno-c2y-extensions -Wno-unknown-warning-option"

# 32-bit MUSL is not tested as it is unsupported on Ubuntu.
FLAGS="-Os -static -g -ffunction-sections -fdata-sections -ffast-math"

HX_DIR=$PWD

# Build artifacts are not retained.
rm -rf ./build; mkdir ./build && cd ./build

if [ "${1:-}" != "--headless" ]; then
    set -o xtrace
fi

musl-gcc $BUILD $ERRORS $FLAGS -I"$HX_DIR/include" \
	-std=c17 -c "$HX_DIR"/test/*.c

# Test every supported version of the standard without libc++.
for VERSION in 11 14 17 20; do

# -Wl,--gc-sections and -flto=12 should reduce size.
musl-gcc $BUILD $ERRORS $FLAGS -I"$HX_DIR/include" -std=c++$VERSION -nostdinc++ \
    -fno-exceptions -fno-rtti -Wl,--gc-sections -nodefaultlibs -flto=12 \
	"$HX_DIR"/src/*.cpp "$HX_DIR"/test/*.cpp *.o -lc -lpthread -lm -o hxtest

done

# Only process and display the c++20 version.
strip -o hxtest-strip --strip-unneeded hxtest

# Turn off tracing silently and make sure the command returns 0.
{ set +o xtrace; } 2> /dev/null

if [ "${1:-}" != "--headless" ]; then
    ./hxtest-strip
else
    ./hxtest-strip 2>&1 | grep -E '\[  PASSED  \]|\[  FAILED  \]|FAILED TESTS'
fi

cd ..

# --headless skips all output.
if [ "${1:-}" != "--headless" ]; then
    ./listsymbols.sh
fi

SYMBOLS=$(nm --radix=d --print-size build/hxtest)
echo "$SYMBOLS" | awk 'NF == 4 && $4 ~ /hx/ && $4 !~ /test/ && !seen[$1]++ {total += $2} \
 END {printf "= Total non-test libhatchet bytes: %d\n", total}'

size build/hxtest-strip

echo "🪓🪓🪓"
