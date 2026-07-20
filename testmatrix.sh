#!/bin/sh
# SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
# SPDX-License-Identifier: MIT
# This file is licensed under the terms of the LICENSE.md file.

# Adds script arguments to the compiler command line using "$@".

trap 'trap "" INT; pkill -9 -P $$ 2>/dev/null; wait 2>/dev/null; exit 1' INT
set -eu

export POSIXLY_CORRECT=1

# Enables glibc heap integrity checking.
export MALLOC_CHECK_=3

# Fatal warning flags.
HX_ERRORS_="-Wall -Wextra -pedantic-errors -Werror -Wfatal-errors -Wcast-qual \
	-Wdisabled-optimization -Wshadow -Wundef -Wconversion -Wdate-time         \
	-Winvalid-utf8 -Wmissing-declarations -Wno-c2y-extensions"

HX_FLAGS_="-DHX_USE_FILE_IO=2 -ffast-math -ggdb3 -D_FORTIFY_SOURCE=3"

HX_SAN_UNDEF_="-fsanitize=undefined,address"
HX_SAN_THREAD_="-fsanitize=thread"
HX_SAN_MEMORY_="-fsanitize=memory -fsanitize-memory-track-origins"

HX_DIR_=$PWD

run_hxtest() {
	if ./hxtest runtests > console_output.txt 2>&1; then
		grep -e '\[  PASSED  \]' -e '\[  FAILED  \]' -e 'FAILED TESTS' \
			console_output.txt || cat console_output.txt
		rm -f hxtest *.o *.txt *.bin *.json
	else
		cat console_output.txt
		echo "error: hxtest non-zero exit."
		exit 1
	fi
}

# Build artifacts are not retained.
rm -rf "$(readlink -f build)" build; ln -s "$(mktemp -d)" build && cd build

# -- Clang ---------------------------------------------------------------------

run_clang_build() {
	HX_OPT_=$1; HX_SAN_=$2; shift 2
	echo "clang c17/c++23 -O$HX_OPT_ $HX_SAN_ $* ..."

	# compile C17
	clang -I"$HX_DIR_/include" -DHX_HARDENING_MODE=3-$HX_OPT_ -O$HX_OPT_ $HX_FLAGS_ \
		$HX_ERRORS_ $HX_SAN_ -fdiagnostics-absolute-paths -pthread -std=c17 "$@"    \
		-c "$HX_DIR_"/test/*.c

	# generate C++23 pch. clang does this automatically when a c++ header file
	# is the target. This is just a test.
	clang++ -I"$HX_DIR_/include" -DHX_HARDENING_MODE=3-$HX_OPT_ -O$HX_OPT_ $HX_FLAGS_ \
		$HX_ERRORS_ $HX_SAN_ -DHX_USE_THREADS=1 -pthread -std=c++23 -fno-exceptions   \
		-fdiagnostics-absolute-paths "$@" "$HX_DIR_"/include/hx/hxtest.hpp            \
		-o hxtest.hpp.pch

	# compile C++23 and link
	clang++ -I"$HX_DIR_/include" -DHX_HARDENING_MODE=3-$HX_OPT_ -O$HX_OPT_ $HX_FLAGS_ \
		$HX_ERRORS_ $HX_SAN_ -DHX_USE_THREADS=1 -pthread -std=c++23 -fno-exceptions   \
		-fdiagnostics-absolute-paths "$@" -include-pch hxtest.hpp.pch                 \
		"$HX_DIR_"/src/*.cpp "$HX_DIR_"/test/*.cpp *.o -lpthread -lstdc++ -o hxtest

	run_hxtest
}

# Test undefined behavior/address use at all build levels with clang. Uses a
# pch.
for HX_I_ in 0 1 2 3; do
	run_clang_build "$HX_I_" "$HX_SAN_UNDEF_" "$@"
done

# Run the thread sanitizer with all optimizations on.
run_clang_build 3 "$HX_SAN_THREAD_" "$@"

# Run the memory sanitizer in debug and with all optimizations on.
run_clang_build 3 "$HX_SAN_MEMORY_" "$@"
run_clang_build 0 "$HX_SAN_MEMORY_" "$@"

# -- GCC ----------------------------------------------------------------------

for HX_I_ in 0 1 2 3; do
echo "gcc c99/c++11 -O$HX_I_ $* ..."

gcc -I"$HX_DIR_/include" -DHX_HARDENING_MODE=3-$HX_I_ -O$HX_I_ $HX_FLAGS_ $HX_ERRORS_ \
	$HX_SAN_UNDEF_ -pthread -std=c99 -m32 "$@" -c "$HX_DIR_"/test/*.c

gcc -I"$HX_DIR_/include" -DHX_HARDENING_MODE=3-$HX_I_ -O$HX_I_ $HX_FLAGS_ $HX_ERRORS_ \
	$HX_SAN_UNDEF_ -pthread -std=c++11 -fno-exceptions -fno-rtti "$@"                 \
	"$HX_DIR_"/src/*.cpp "$HX_DIR_"/test/*.cpp *.o -lpthread -lstdc++ -m32 -o hxtest

run_hxtest
done

# Make sure the script returns 0.
echo "🪓🪓🪓 all tests passed."
