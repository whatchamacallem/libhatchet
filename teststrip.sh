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
        kill "$pid" 2>/dev/null
    done
    exit 1
' 1 2 3 6 15

set -eu

export POSIXLY_CORRECT=1

# Should detect no C++ standard library.
BUILD="-DHX_USE_LIBCXX=0 -DHX_HARDENING_MODE=HX_HARDENING_MODE_DEBUG -DHX_USE_THREADS=11"

ERRORS="-Wall -Wextra -pedantic-errors -Werror -Wfatal-errors -Wcast-qual \
	-Wdisabled-optimization -Wshadow -Wundef -Wconversion -Wdate-time \
	-Wmissing-declarations -Wno-assume"

FLAGS="-Os -static -g -ffunction-sections -fdata-sections -ffast-math"

HX_DIR=`pwd`

# Build artifacts are not retained.
rm -rf ./build; mkdir ./build && cd ./build

set -o xtrace

musl-gcc $BUILD $ERRORS $FLAGS -I$HX_DIR/include \
	-std=c17 -c $HX_DIR/src/*.c $HX_DIR/test/*.c

# Test every supported version of the standard without libc++.
for VERSION in 11 14 17 20; do

# Includes lld specific instruction to dead-strip. musl is the only library.
musl-gcc $BUILD $ERRORS $FLAGS -I$HX_DIR/include \
	-std=c++$VERSION -Wl,--gc-sections -fno-exceptions -fno-rtti \
	$HX_DIR/src/*.cpp $HX_DIR/test/*.cpp *.o -o hxtest

done

# Only process and display the c++20 version.
strip -o hxtest-strip --strip-unneeded hxtest

# Turn off tracing silently and make sure the command returns 0.
{ set +o xtrace; } 2> /dev/null

# Prints [  PASSED  ]
./hxtest-strip

cd ..

echo "=========================================================================="
echo "= Largest elf symbols..."
echo "=========================================================================="
./listsymbols.sh

echo "=========================================================================="
# Prints summary stats for the necessary components of the executable.
size build/hxtest-strip

echo "🪓🪓🪓"
