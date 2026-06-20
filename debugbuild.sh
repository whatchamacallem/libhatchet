#!/bin/sh
# SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
# SPDX-License-Identifier: MIT
# This file is licensed under the terms of the LICENSE.md file.
#
# This build uses 32-bit pointers because they are easier to read.
#
# The -m32 switch enables 32-bit compilation. See ubuntu_packages.sh.
#
# Do not use a .pch with ccache. It won't work as expected.

# Prevent leaking background tasks.
trap '{ set +o xtrace; } 2> /dev/null
    trap - 1 2 3 6 15
    for pid in $(pgrep -g "$$" 2>/dev/null); do
        [ "$pid" = "$$" ] && continue
        kill "$pid" 2>/dev/null
    done
    exit 1
' 1 2 3 6 15

set -o errexit

export POSIXLY_CORRECT=1

# Clear the output window when used interactively.
if [ "$1" != "--headless" ] && [ -z "$CLAUDE_CODE" ]; then
	export TERM=xterm-256color
	clear
fi

# Should detect threading and the standard library.
BUILD="-DHX_RELEASE=0 -O0"

# -Wdate-time is for ccache. -Wno-unused-variable is only for debugging.
ERRORS="-Wall -Wextra -pedantic-errors -Werror -Wfatal-errors -Wcast-qual \
	-Wdisabled-optimization -Wshadow -Wundef -Wconversion -Wdate-time \
	-Wmissing-declarations -Wno-unused-variable"

FLAGS="-m32 -ggdb3 -fdiagnostics-absolute-paths -fdiagnostics-color=always"

# Build artifacts are not retained.
rm -rf ./build; mkdir ./build && cd ./build

for FILE in ../src/*.c ../test/*.c; do
	ccache clang $BUILD $ERRORS $FLAGS -I../include \
		-std=c17 -pthread -c $FILE & PIDS="$PIDS $!"
done

for FILE in ../src/*.cpp ../test/*.cpp; do
	ccache clang++ $BUILD $ERRORS $FLAGS -I../include \
		-std=c++23 -pthread -fno-exceptions -fno-rtti  \
		-c $FILE & PIDS="$PIDS $!"
done

for PID in $PIDS; do
	if ! wait "$PID"; then
		exit 1
	fi
done

ccache clang++ $BUILD $FLAGS *.o -lpthread -lstdc++ -lm -o hxtest

if [ -z "$CLAUDE_CODE" ]; then
	# save tokens
	ccache --show-stats
fi

if [ "$1" = "--run" ]; then
	 ./hxtest
fi

echo 🪓🪓🪓
