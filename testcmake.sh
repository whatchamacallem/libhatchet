#!/bin/sh
# SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
# SPDX-License-Identifier: MIT
# This file is licensed under the terms of the LICENSE.md file.

# Does not rerun cmake as is customary. Build artifacts ARE retained.

trap 'trap "" INT; pkill -9 -P $$ 2>/dev/null; wait 2>/dev/null; exit 1' INT
set -eu

export POSIXLY_CORRECT=1

if [ ! -f build/CMakeCache.txt ]; then
	rm -rf "$(readlink -f build)" build; mkdir build
	if [ "${1:-}" = "--verbose" ]; then
		cmake -S . -B build -G Ninja -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
	else
		cmake -S . -B build -G Ninja -DCMAKE_EXPORT_COMPILE_COMMANDS=ON >build/testcmake.sh.log 2>&1 \
			|| { echo "error: cmake failed:"; cat build/testcmake.sh.log; exit 1; }
	fi
else
	echo "Found build/CMakeCache.txt..."
fi

if [ "${1:-}" = "--verbose" ]; then
	ninja -C build
else
	ninja -C build >build/testcmake.sh.log 2>&1 \
		|| { echo "error: ninja failed:"; cat build/testcmake.sh.log; exit 1; }
fi

echo "Run build/hxtest with GDB..."
cd build

# Run the GDB smoke tests at the same time.
gdb -batch -x ../test/gdb_printer_test.gdb ./hxtest > testcmake.sh.txt 2>&1

# GDB exits 0 even after SIGTRAP so check for the summary line explicitly.
if ! grep -qE '\[  PASSED  \]' testcmake.sh.txt; then
	echo "error: GDB exited prematurely or test suite failed."
	tail -5 testcmake.sh.txt
	exit 1
fi

echo "Check GDB pretty printer output..."
grep -E '^\$[0-9]+ = ' gdb_printer_output.txt > actual_pretty_print.txt
if [ "${1:-}" = "--verbose" ]; then
	cat actual_pretty_print.txt
fi

cat > expected_pretty_print.txt <<'EOF'
$1 = [3] hxtest_object = {{value = 71, moved_from = 0}, {value = 72, moved_from = 0}, {value = 73, moved_from = 0}}
$2 = [4] int = {7, 14, 21, 28}
$3 = 1010000000000000000000000000000000000000000000000000000000000000001 = {m_data_[0] = 1, m_data_[1] = 5}
$4 = 1010000000000000000000000000000000000000000000000000000000000000001 = {m_data_[0] = 1, m_data_[1] = 5}
$5 = [3] hxtest_const_list_node_t = {1, 2, 3}
$6 = [4/4] int = {3, 4, 5, 6}
$7 = [8/8] int = {4, 5, 6, 7, 8, 9, 10, 11}
$8 = [3/4] int->hxtest_object = {[1] = {value = 10, moved_from = 0}, [2] = {value = 20, moved_from = 0}, [3] = {value = 30, moved_from = 0}}
$9 = [3/8] int->hxtest_object = {[1] = {value = 10, moved_from = 0}, [2] = {value = 20, moved_from = 0}, [3] = {value = 30, moved_from = 0}}
$10 = [3/4] hxtest_object = {{value = 10, moved_from = 0}, {value = 20, moved_from = 0}, {value = 30, moved_from = 0}}
$11 = [3/8] hxtest_object = {{value = 10, moved_from = 0}, {value = 20, moved_from = 0}, {value = 30, moved_from = 0}}
$12 = [39/32 buckets] hxtest_integer = {57, 61, 39, 67, 55, 75, 60, 48, 45, 72, 64, 56, 59, 66, 42, 63, 62, 44, 53, 73, 58, 71, 52, 40, 46, 68, 65, 41, 51, 76, 70, 77, 49, 69, 43, 54, 47, 74, 50}
$13 = [3] hxtest_list_node_t = {1, 2, 3}
$14 = [5/5] hxtest_object = {{value = 91, moved_from = 0}, {value = 0, moved_from = 0}, {value = 97, moved_from = 0}, {value = 97, moved_from = 0}, {value = 99, moved_from = 0}}
$15 = [7/8] int = {10, 9, 6, 8, 7, 4, 5}
EOF

if ! diff -u expected_pretty_print.txt actual_pretty_print.txt; then
	cat gdb_printer_output.txt
	echo "error: GDB pretty printer output does not match expected output."
	exit 1
fi

cd ..

# Depends on -DCMAKE_EXPORT_COMPILE_COMMANDS=ON above. These two have to happen
# together.
echo "Run clang-tidy..."
run-clang-tidy -quiet -j 0 -p build src/*.cpp test/*.c test/*.cpp 2>&1        \
	| grep -vE '^Running clang-tidy for|^[0-9]+ warnings generated\.$|^\[|^$' \
	&& { echo "clang-tidy errors."; exit 1; }

echo "🪓🪓🪓"
