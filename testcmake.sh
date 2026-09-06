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
rm -f gdb_printer_output.txt
gdb -batch -x ../test/gdb_printer_test.gdb ./hxtest > testcmake.sh.txt 2>&1

# GDB exits 0 even after SIGTRAP so check for the summary line explicitly.
if ! grep -qE '\[  PASSED  \]' testcmake.sh.txt; then
	echo "error: GDB exited prematurely or test suite failed."
	tail -5 testcmake.sh.txt
	exit 1
fi

echo "Check GDB pretty printer output..."
if [ "${1:-}" = "--verbose" ]; then
	cat gdb_printer_output.txt
fi

cat > expected_pretty_print.txt <<'EOF'
$1 = [4] hxarray<hxtest_object, 4> = {[32B static], { state=valid, value=7 }, { state=valid, value=14 }, { state=valid, value=21 }, { state=valid, value=28 }}
$2 = [32B static]
$3 = [3] hxarray<hxtest_object, 0> = {[24B dynamic], { state=valid, value=71 }, { state=valid, value=72 }, { state=valid, value=73 }}
$4 = [24B dynamic]
$5 = [0] const hxarray<int, 0>
$6 = [0B dynamic]
$7 = [16B] const hxbitset<67> = {01000000 00000000 05000000 00000000}
$8 = [16B] const hxbitset<67> = {01000000 00000000 05000000 00000000}
$9 = [40B] const hxbitset<259> = {1f000000 00000000 00000000 00000000, 00000000 00000000 00000000 00000000, 01000000 00000000}
$10 = [0] const hxconstexpr_list<hxtest_constexpr_list_pair_node_t, hxdo_not_delete>
$11 = [1] hxconstexpr_list<hxtest_constexpr_list_pair_node_t, hxdo_not_delete> = {31, 32}
$12 = [3] hxconstexpr_list<hxtest_constexpr_list_object_node_t, hxdo_not_delete> = {{ state=valid, value=1 }, { state=valid, value=2 }, { state=valid, value=3 }}
$13 = [4/4] hxdeque<hxtest_object, 4> = {[32B static], { state=valid, value=3 }, { state=valid, value=4 }, { state=valid, value=5 }, { state=valid, value=6 }}
$14 = [0/0] const hxdeque<int, 0>
$15 = [0/4] hxdeque<int, 0> = {[16B dynamic]}
$16 = [8/8] hxdeque<hxtest_object, 0> = {[64B dynamic], { state=valid, value=4 }, { state=valid, value=5 }, { state=valid, value=6 }, { state=valid, value=7 }, { state=valid, value=8 }, { state=valid, value=9 }, { state=valid, value=10 }, { state=valid, value=11 }}
$17 = [1] const hxexpected<int, bool> = {hxallocator = [4B static], value = 34, error = false}
$18 = [0] const hxexpected<int, bool> = {hxallocator = [4B static], error = true}
$19 = [0] const hxexpected<int, int> = {hxallocator = [4B static], error = 31}
$20 = [3/4] const hxflat_map<hxtest_object, hxtest_object, hxkey_less_t<hxtest_object, void>, false, 4> = {[32B static], [32B static], { state=valid, value=10 }, { state=valid, value=20 }, { state=valid, value=30 }}
$21 = [3/3] const hxflat_map<hxtest_object, hxtest_object, hxkey_less_t<hxtest_object, void>, true, 0> = {[24B dynamic], [24B dynamic], { state=valid, value=10 }, { state=valid, value=20 }, { state=valid, value=30 }}
$22 = [0/0] const hxflat_map<hxtest_object, hxtest_object, hxkey_less_t<hxtest_object, void>, true, 0>
$23 = [0/4] hxflat_map<hxtest_object, hxtest_object, hxkey_less_t<hxtest_object, void>, true, 0> = {[32B dynamic], [32B dynamic]}
$24 = [3/4] const hxflat_set<hxtest_object, hxkey_less_t<hxtest_object, void>, false, 4> = {[32B static], { state=valid, value=10 }, { state=valid, value=20 }, { state=valid, value=30 }}
$25 = [3/3] const hxflat_set<hxtest_object, hxkey_less_t<hxtest_object, void>, true, 0> = {[24B dynamic], { state=valid, value=10 }, { state=valid, value=20 }, { state=valid, value=30 }}
$26 = [0/0] const hxflat_set<hxtest_object, hxkey_less_t<hxtest_object, void>, true, 0>
$27 = [0/8] hxflat_set<hxtest_object, hxkey_less_t<hxtest_object, void>, true, 0> = {[64B dynamic]}
$28 = [2/3] hxhandle_table<hxtest_object, hxdefault_delete, 2> = {[1] = { state=valid, value=0 }, [3] = { state=valid, value=2 }}
$29 = [0/0] const hxhandle_table<hxtest_object, hxdefault_delete, 0>
$30 = [39/32 buckets] hxhash_table<hxtest_object_node, hxdefault_delete, true, 0> = {[256B dynamic], { state=valid, value=57 }, { state=valid, value=39 }, { state=valid, value=61 }, { state=valid, value=67 }, { state=valid, value=55 }, { state=valid, value=48 }, { state=valid, value=45 }, { state=valid, value=75 }, { state=valid, value=60 }, { state=valid, value=64 }, { state=valid, value=72 }, { state=valid, value=56 }, { state=valid, value=59 }, { state=valid, value=66 }, { state=valid, value=42 }, { state=valid, value=63 }, { state=valid, value=44 }, { state=valid, value=62 }, { state=valid, value=53 }, { state=valid, value=58 }, { state=valid, value=73 }, { state=valid, value=71 }, { state=valid, value=40 }, { state=valid, value=52 }, { state=valid, value=46 }, { state=valid, value=68 }, { state=valid, value=41 }, { state=valid, value=65 }, { state=valid, value=51 }, { state=valid, value=70 }, { state=valid, value=76 }, { state=valid, value=77 }, { state=valid, value=49 }, { state=valid, value=69 }, { state=valid, value=43 }, { state=valid, value=54 }, { state=valid, value=47 }, { state=valid, value=50 }, { state=valid, value=74 }}
$31 = [0/0 buckets] const hxhash_table<hxtest_set_node_t, hxdo_not_delete, false, 0>
$32 = [1/2 buckets] hxhash_table<hxtest_set_node_t, hxdo_not_delete, false, 0> = {[16B dynamic], 31}
$33 = [1/2 buckets] hxhash_table<hxtest_map_node_t, hxdo_not_delete, false, 1> = {["hxallocator"] = [16B static], [31] = 32}
$34 = [0] const hxlist<hxtest_list_pair_node_t, hxdo_not_delete>
$35 = [1] hxlist<hxtest_list_pair_node_t, hxdo_not_delete> = {31, 32}
$36 = [3] hxlist<hxtest_list_object_node_t, hxdo_not_delete> = {{ state=valid, value=1 }, { state=valid, value=2 }, { state=valid, value=3 }}
$37 = [1] const hxptr<hxtest_object, hxdefault_delete> = {value = { state=valid, value=7 }}
$38 = [0] const hxptr<hxtest_object, hxdefault_delete> = null
$39 = [1] const hxref<hxtest_object> = {value = { state=valid, value=7 }}
$40 = [0] const hxref<hxtest_object> = null
$41 = [3] hxslot_map<hxtest_object, 2> = {{ state=valid, value=0 }, { state=valid, value=1 }, { state=valid, value=2 }}
$42 = [0] const hxslot_map<hxtest_object, 0>
$43 = [5/5] hxvector<hxtest_object, 0> = {[40B dynamic], { state=valid, value=91 }, { state=valid, value=0 }, { state=valid, value=97 }, { state=valid, value=97 }, { state=valid, value=99 }}
$44 = [0/0] const hxvector<hxtest_object, 0>
$45 = [0/3] hxvector<hxtest_object, 0> = {[24B dynamic]}
$46 = [7/8] hxvector<hxtest_object, 8> = {[64B static], { state=valid, value=10 }, { state=valid, value=9 }, { state=valid, value=6 }, { state=valid, value=8 }, { state=valid, value=7 }, { state=valid, value=5 }, { state=valid, value=4 }}
EOF

if ! diff -u expected_pretty_print.txt gdb_printer_output.txt; then
	cat gdb_printer_output.txt
	echo "error: GDB pretty printer output does not match expected output."
	exit 1
fi

cd ..

# Depends on -DCMAKE_EXPORT_COMPILE_COMMANDS=ON above. These two have to happen
# together.
echo "Run clang-tidy..."
run-clang-tidy -quiet -j 0 -p build src/*.cpp test/*.c test/*.cpp 2>&1   \
	| grep -vE '^Running clang-tidy for|^[0-9]+ warnings generated\.$|^\[|^$' \
	&& { echo "clang-tidy errors."; exit 1; }

echo "🪓🪓🪓"
