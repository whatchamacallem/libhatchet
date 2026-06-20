#!/bin/sh
# SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
# SPDX-License-Identifier: MIT
# This file is licensed under the terms of the LICENSE.md file.

# Only rerun cmake manually as is customary. Build artifacts ARE retained.

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

if [ ! -f build/build.ninja ]; then
	rm -rf build
	cmake -S . -B build -G Ninja -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
else
	echo "found build/build.ninja..."
fi

ninja -C build

echo "run ./hxtest with GDB..."
cd build

set +o errexit

# Run the GDB smoke tests at the same time.
gdb -batch -x ../test/gdb_printer_test.gdb ./hxtest > testcmake.sh.txt 2>&1
CODE=$?

set -o errexit

if [ "$CODE" -ne 0 ]; then
	# Dump everything a second time with all spew enabled.
	cat testcmake.sh.txt
	echo "Stopping due to build/hxtest returning $CODE."
	exit "$CODE"
fi

# Filter out critical status messages only. This should stop the test if no
# output matches. The goal is to prevent AI from panicking over spew.
grep -E '\[  PASSED  \]|\[  FAILED  \]|FAILED TESTS' testcmake.sh.txt


cat gdb_printer_output.txt

cd ..

# Depends on -DCMAKE_EXPORT_COMPILE_COMMANDS=ON above. These two have to happen
# together.
echo 'run-clang-tidy... The "x warnings generated" messages are from system headers.'
run-clang-tidy -quiet -j 0 -p build src/*.cpp test/*.cpp

echo "🪓🪓🪓"
