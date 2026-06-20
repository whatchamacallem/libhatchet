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
	echo "Found build/build.ninja..."
fi

ninja -C build

echo "Run build/hxtest with GDB..."
cd build

# Run the GDB smoke tests at the same time.
gdb -batch -x ../test/gdb_printer_test.gdb ./hxtest > testcmake.sh.txt 2>&1

# GDB exits 0 even after SIGTRAP so check for the summary line explicitly.
if ! grep -qE '\[  PASSED  \]' testcmake.sh.txt; then
	echo "GDB exited before test suite completed."
	tail -5 testcmake.sh.txt
	exit 1
fi

grep -E '\[  PASSED  \]|\[  FAILED  \]|FAILED TESTS' testcmake.sh.txt

echo "Listing GDB output:"
cat gdb_printer_output.txt

cd ..

# Depends on -DCMAKE_EXPORT_COMPILE_COMMANDS=ON above. These two have to happen
# together.
echo "Run clang-tidy..."
run-clang-tidy -quiet -j 0 -p build src/*.cpp test/*.cpp 2>&1 \
	| grep -vE '^[0-9]+ warnings generated\.$|^$'

echo "🪓🪓🪓"
