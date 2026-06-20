#!/bin/sh
# SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
# SPDX-License-Identifier: MIT
# This file is licensed under the terms of the LICENSE.md file.
#
# Measures the preprocessed LOC cost of including all public libhatchet headers
# vs. equivalent GNU (libstdc++) and Clang (libc++) standard library headers.

set -eu

FLAGS="-std=c++23 -x c++ -E -P -w"

# Build artifacts are not retained.
rm -rf ./build; mkdir ./build && cd ./build

# <hx/*>
{
	for h in ../include/hx/*.h ../include/hx/*.hpp; do
		# These have no equivalent in the standard library or are not to be
		# included directly.
		case "${h##*/}" in hxconsole.hpp|hxmemory_manager.h|hxprofiler.hpp|hxsettings.h|hxtest.hpp)
			continue;; esac

		printf '#include "%s"\n' "$h"
	done
} > hx_all.cpp

# Equivalent standard library headers
{
	for h in \
		algorithm array assert.h bitset chrono condition_variable deque fstream \
		functional future initializer_list list memory mutex optional random    \
		thread type_traits unordered_map unordered_set utility
	do
		printf '#include <%s>\n' "$h"
	done
} > std_all.cpp

# Average timing over 10 runs to reduce noise.
printf '// Empty File\n' > empty_file.cpp
GNU_BASE_TOTAL=0; LLVM_BASE_TOTAL=0; HX_TOTAL=0; GNU_TOTAL=0; LLVM_TOTAL=0
for i in $(seq 10); do
	t0=$(date +%s%N); g++ $FLAGS empty_file.cpp > /dev/null;
	GNU_BASE_TOTAL=$(( GNU_BASE_TOTAL + $(date +%s%N) - t0 ))
	t0=$(date +%s%N); clang++ $FLAGS empty_file.cpp > /dev/null;
	LLVM_BASE_TOTAL=$(( LLVM_BASE_TOTAL + $(date +%s%N) - t0 ))
	t0=$(date +%s%N); clang++ $FLAGS -DHX_USE_LIBCXX=0 -I../include hx_all.cpp > hx_pp.txt;
	HX_TOTAL=$(( HX_TOTAL + $(date +%s%N) - t0 ))
	t0=$(date +%s%N); g++ $FLAGS std_all.cpp > gnu_pp.txt;
	GNU_TOTAL=$(( GNU_TOTAL + $(date +%s%N) - t0 ))
	t0=$(date +%s%N); clang++ $FLAGS std_all.cpp > llvm_pp.txt;
	LLVM_TOTAL=$(( LLVM_TOTAL + $(date +%s%N) - t0 ))
done
GNU_BASE=$(( GNU_BASE_TOTAL / 10000000 ))
LLVM_BASE=$(( LLVM_BASE_TOTAL / 10000000 ))
HX_MS=$(( HX_TOTAL / 10000000 - LLVM_BASE ))
GNU_MS=$(( GNU_TOTAL / 10000000 - GNU_BASE ))
LLVM_MS=$(( LLVM_TOTAL / 10000000 - LLVM_BASE ))

HX_LOC=$(wc -l < hx_pp.txt)
GNU_LOC=$(wc -l < gnu_pp.txt)
LLVM_LOC=$(wc -l < llvm_pp.txt)
GNU_RATIO=$(awk "BEGIN { printf \"%.1f\", $GNU_LOC / $HX_LOC }")
LLVM_RATIO=$(awk "BEGIN { printf \"%.1f\", $LLVM_LOC / $HX_LOC }")
GNU_MS_RATIO=$(awk "BEGIN { printf \"%.1f\", $GNU_MS / $HX_MS }")
LLVM_MS_RATIO=$(awk "BEGIN { printf \"%.1f\", $LLVM_MS / $HX_MS }")

printf '| %-30s | %6s | %8s | %6s | %8s |\n' "Library" "LOC" "LOC mult" "ms" "ms mult"
printf '|:-------------------------------|-------:|---------:|-------:|---------:|\n'
printf '| %-30s | %6d | %8s | %6d | %8s |\n'   "libhatchet  (clang)"          "$HX_LOC"   "-"           "$HX_MS"   "-"
printf '| %-30s | %6d | %7sx | %6d | %7sx |\n' "libstdc++   (g++/libstdc++)"  "$GNU_LOC"  "$GNU_RATIO"  "$GNU_MS"  "$GNU_MS_RATIO"
printf '| %-30s | %6d | %7sx | %6d | %7sx |\n' "libc++      (clang++/libc++)" "$LLVM_LOC" "$LLVM_RATIO" "$LLVM_MS" "$LLVM_MS_RATIO"
