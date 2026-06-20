#!/bin/sh
# SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
# SPDX-License-Identifier: MIT
# This file is licensed under the terms of the LICENSE.md file.

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

BUILD="-DHX_HARDENING_MODE=HX_HARDENING_MODE_DEBUG -O0"

ERRORS="-Wall -Wextra -pedantic-errors -Werror -Wfatal-errors -Wcast-qual \
	-Wdisabled-optimization -Wshadow -Wundef -Wconversion -Wdate-time \
	-Wmissing-declarations -Wno-unused-variable"

FLAGS="-m32 -ggdb3 -fdiagnostics-absolute-paths -fdiagnostics-color=always"

HX_DIR=`pwd`

# Build artifacts are not retained.
rm -rf ./build; mkdir ./build && cd ./build

PIDS=""
for FILE in $HX_DIR/src/*.c; do
	ccache clang $BUILD $ERRORS $FLAGS -I$HX_DIR/include \
		-std=c17 -pthread -c $FILE & PIDS="$PIDS $!"
done

for FILE in $HX_DIR/src/*.cpp $HX_DIR/example/example.cpp; do
	ccache clang++ $BUILD $ERRORS $FLAGS -I$HX_DIR/include \
		-std=c++20 -pthread -fno-exceptions -fno-rtti \
		-c $FILE & PIDS="$PIDS $!"
done

for PID in $PIDS; do
	if ! wait "$PID"; then
		exit 1
	fi
done

ccache clang++ $BUILD $FLAGS *.o -lpthread -lstdc++ -lm -o hxexample

cp $HX_DIR/example/example.cfg .

CORRECT=$HX_DIR/example/example_correct.txt

if [ ! -f "$CORRECT" ]; then
	echo "WARNING: regenerating $CORRECT..."
	echo exit | ./hxexample 2>/dev/null > "$CORRECT"
fi

echo exit | ./hxexample 2>/dev/null > hxexample_out.txt
if ! diff -u "$CORRECT" hxexample_out.txt; then
	echo "error: output differs from $CORRECT"
	exit 1
fi

cat "$CORRECT"
echo "output matches example_correct.txt"
echo "\nRun hxexample from the build directory to test interactively."

# Run clang-tidy directly without cmake or compile_commands.json.
echo "run clang-tidy on example.cpp..."
clang-tidy -quiet $HX_DIR/example/example.cpp -- \
	-std=c++20 -pthread -fno-exceptions -fno-rtti \
	-I$HX_DIR/include $BUILD $FLAGS

echo "🪓🪓🪓"
