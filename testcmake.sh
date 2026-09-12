#!/bin/sh
# SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
# SPDX-License-Identifier: MIT
# This file is licensed under the terms of the LICENSE.md file.

# Does not rerun cmake as is customary. Build artifacts ARE retained.

trap 'trap "" INT; pkill -9 -P $$ 2>/dev/null; wait 2>/dev/null; exit 1' INT
set -eu

export POSIXLY_CORRECT=1

HX_OPT_NO_TIDY_=0
HX_OPT_VERBOSE_=0
for HX_ARG_ in "$@"; do
	case "$HX_ARG_" in
		"")          ;;
		--no-tidy)   HX_OPT_NO_TIDY_=1 ;;
		--verbose)   HX_OPT_VERBOSE_=1 ;;
		*)
			echo "usage_error: $0 [--no-tidy] [--verbose]"
			echo "  --no-tidy   Skip clang-tidy."
			echo "  --verbose   Full output."
			exit 1 ;;
	esac
done

if [ ! -f build/CMakeCache.txt ]; then
	rm -rf "$(readlink -f build)" build; mkdir build
	if [ "$HX_OPT_VERBOSE_" = "1" ]; then
		cmake -S . -B build -G Ninja -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
	else
		cmake -S . -B build -G Ninja -DCMAKE_EXPORT_COMPILE_COMMANDS=ON >build/testcmake.sh.log 2>&1 \
			|| { echo "error: cmake failed:"; cat build/testcmake.sh.log; exit 1; }
	fi
else
	echo "Found build/CMakeCache.txt..."
fi

if [ "$HX_OPT_VERBOSE_" = "1" ]; then
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
if [ "$HX_OPT_VERBOSE_" = "1" ]; then
	cat gdb_printer_output.txt
fi

cat > expected_pretty_print.txt <<'EOF'
$1 = [4] hxarray<hxtest_object, 4> = {[32B static], { value=7, ticket=100 }, { value=14, ticket=101 }, { value=21, ticket=102 }, { value=28, ticket=103 }}
$2 = [32B static]
$3 = [3] hxarray<hxtest_object, 0> = {[24B dynamic], { value=71, ticket=100 }, { value=72, ticket=101 }, { value=73, ticket=102 }}
$4 = [24B dynamic]
$5 = [0] const hxarray<int, 0>
$6 = [0B dynamic]
$7 = [16B] const hxbitset<67> = {01000000 00000000 05000000 00000000}
$8 = [16B] const hxbitset<67> = {01000000 00000000 05000000 00000000}
$9 = [40B] const hxbitset<259> = {1f000000 00000000 00000000 00000000, 00000000 00000000 00000000 00000000, 01000000 00000000}
$10 = [4/4] hxdeque<hxtest_object, 4> = {[32B static], { value=3, ticket=102 }, { value=4, ticket=103 }, { value=5, ticket=104 }, { value=6, ticket=105 }}
$11 = [0/0] const hxdeque<int, 0>
$12 = [0/4] hxdeque<int, 0> = {[16B dynamic]}
$13 = [8/8] hxdeque<hxtest_object, 0> = {[64B dynamic], { value=4, ticket=104 }, { value=5, ticket=105 }, { value=6, ticket=106 }, { value=7, ticket=107 }, { value=8, ticket=108 }, { value=9, ticket=109 }, { value=10, ticket=110 }, { value=11, ticket=111 }}
$14 = [1] const hxexpected<int, bool> = {hxallocator = [4B static], value = 34, error = false}
$15 = [0] const hxexpected<int, bool> = {hxallocator = [4B static], error = true}
$16 = [0] const hxexpected<int, int> = {hxallocator = [4B static], error = 31}
$17 = [3/4] const hxflat_map<hxtest_object, hxtest_object, 4, hxkey_three_way_t<hxtest_object, void>, 2> = {[32B static], [32B static], { value=10, ticket=103 }, { value=20, ticket=104 }, { value=30, ticket=105 }}
$18 = [3/3] const hxflat_map<hxtest_object, hxtest_object, 0, hxkey_three_way_t<hxtest_object, void>, 3> = {[24B dynamic], [24B dynamic], { value=10, ticket=103 }, { value=20, ticket=104 }, { value=30, ticket=105 }}
$19 = [0/0] const hxflat_map<hxtest_object, hxtest_object, 0, hxkey_three_way_t<hxtest_object, void>, 3>
$20 = [0/4] hxflat_map<hxtest_object, hxtest_object, 0, hxkey_three_way_t<hxtest_object, void>, 3> = {[32B dynamic], [32B dynamic]}
$21 = [3/4] const hxflat_set<hxtest_object, 4, hxkey_three_way_t<hxtest_object, void>, 2> = {[32B static], { value=10, ticket=100 }, { value=20, ticket=101 }, { value=30, ticket=102 }}
$22 = [3/3] const hxflat_set<hxtest_object, 0, hxkey_three_way_t<hxtest_object, void>, 3> = {[24B dynamic], { value=10, ticket=100 }, { value=20, ticket=101 }, { value=30, ticket=102 }}
$23 = [0/0] const hxflat_set<hxtest_object, 0, hxkey_three_way_t<hxtest_object, void>, 3>
$24 = [0/8] hxflat_set<hxtest_object, 0, hxkey_three_way_t<hxtest_object, void>, 3> = {[64B dynamic]}
$25 = [2/3] hxhandle_table<hxtest_object, hxdefault_delete, 2> = {[1] = { value=0, ticket=100 }, [3] = { value=2, ticket=102 }}
$26 = [0/0] const hxhandle_table<hxtest_object, hxdefault_delete, 0>
$27 = [39/32 buckets] hxhash_table<hxtest_object_node, hxdefault_delete, 0, 1> = {[256B dynamic], { value=57, ticket=215 }, { value=39, ticket=179 }, { value=61, ticket=223 }, { value=67, ticket=235 }, { value=55, ticket=211 }, { value=48, ticket=197 }, { value=45, ticket=191 }, { value=75, ticket=251 }, { value=60, ticket=221 }, { value=64, ticket=229 }, { value=72, ticket=245 }, { value=56, ticket=213 }, { value=59, ticket=219 }, { value=66, ticket=233 }, { value=42, ticket=185 }, { value=63, ticket=227 }, { value=44, ticket=189 }, { value=62, ticket=225 }, { value=53, ticket=207 }, { value=58, ticket=217 }, { value=73, ticket=247 }, { value=71, ticket=243 }, { value=40, ticket=181 }, { value=52, ticket=205 }, { value=46, ticket=193 }, { value=68, ticket=237 }, { value=41, ticket=183 }, { value=65, ticket=231 }, { value=51, ticket=203 }, { value=70, ticket=241 }, { value=76, ticket=253 }, { value=77, ticket=255 }, { value=49, ticket=199 }, { value=69, ticket=239 }, { value=43, ticket=187 }, { value=54, ticket=209 }, { value=47, ticket=195 }, { value=50, ticket=201 }, { value=74, ticket=249 }}
$28 = [0/0 buckets] const hxhash_table<hxtest_set_node_t, hxdo_not_delete, 0, 0>
$29 = [1/2 buckets] hxhash_table<hxtest_set_node_t, hxdo_not_delete, 0, 0> = {[16B dynamic], 31}
$30 = [1/2 buckets] hxhash_table<hxtest_map_node_t, hxdo_not_delete, 1, 0> = {["hxallocator"] = [16B static], [31] = 32}
$31 = [0] const hxlist_constexpr<hxtest_constexpr_list_pair_node_t, hxdo_not_delete>
$32 = [1] hxlist_constexpr<hxtest_constexpr_list_pair_node_t, hxdo_not_delete> = {31, 32}
$33 = [3] hxlist_constexpr<hxtest_constexpr_list_object_node_t, hxdo_not_delete> = {{ value=1, ticket=100 }, { value=2, ticket=101 }, { value=3, ticket=102 }}
$34 = [0] const hxlist<hxtest_list_pair_node_t, hxdo_not_delete>
$35 = [1] hxlist<hxtest_list_pair_node_t, hxdo_not_delete> = {31, 32}
$36 = [3] hxlist<hxtest_list_object_node_t, hxdo_not_delete> = {{ value=1, ticket=100 }, { value=2, ticket=101 }, { value=3, ticket=102 }}
$37 = [1] const hxptr<hxtest_object, hxdefault_delete> = {value = { value=7, ticket=100 }}
$38 = [0] const hxptr<hxtest_object, hxdefault_delete> = null
$39 = [1] const hxref<hxtest_object> = {value = { value=7, ticket=100 }}
$40 = [0] const hxref<hxtest_object> = null
$41 = [3] hxslot_map<hxtest_object, 2> = {{ value=0, ticket=100 }, { value=1, ticket=101 }, { value=2, ticket=102 }}
$42 = [0] const hxslot_map<hxtest_object, 0>
$43 = [5/5] hxvector<hxtest_object, 0> = {[40B dynamic], { value=91, ticket=100 }, { value=0, ticket=106 }, { value=97, ticket=105 }, { value=97, ticket=105 }, { value=99, ticket=107 }}
$44 = [0/0] const hxvector<hxtest_object, 0>
$45 = [0/3] hxvector<hxtest_object, 0> = {[24B dynamic]}
$46 = [7/8] hxvector<hxtest_object, 8> = {[64B static], { value=10, ticket=100 }, { value=9, ticket=101 }, { value=6, ticket=107 }, { value=8, ticket=103 }, { value=7, ticket=104 }, { value=5, ticket=102 }, { value=4, ticket=106 }}
EOF

if ! diff -u expected_pretty_print.txt gdb_printer_output.txt; then
	cat gdb_printer_output.txt
	echo "error: GDB pretty printer output does not match expected output."
	exit 1
fi

cd ..

if [ "$HX_OPT_NO_TIDY_" = "1" ]; then
	echo "Skipping clang-tidy..."
else
	# Depends on -DCMAKE_EXPORT_COMPILE_COMMANDS=ON above. These two have to
	# happen together.
	echo "Run clang-tidy..."
	run-clang-tidy -quiet -j 0 -p build src/*.cpp test/*.c test/*.cpp 2>&1   \
		| grep -vE '^Running clang-tidy for|^[0-9]+ warnings generated\.$|^\[|^$' \
		&& { echo "clang-tidy errors."; exit 1; }
fi

echo "🪓🪓🪓"
