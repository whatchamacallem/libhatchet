#!/bin/sh
# SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
# SPDX-License-Identifier: MIT
# This file is licensed under the terms of the LICENSE.md file.

# Adds script arguments to the compiler command line using "$@".

trap 'trap "" INT PIPE TERM; pkill -P $$; wait; exit 1' INT PIPE TERM
set -eu

cd "$(dirname "$(readlink -f "$0")")"

export POSIXLY_CORRECT=1
export MALLOC_CHECK_=3

# Fatal warning flags.
HX_ERRORS_="-Wall -Wextra -pedantic-errors -Werror -Wfatal-errors -Wcast-qual \
	-Wdisabled-optimization -Wshadow -Wundef -Wconversion -Wdate-time         \
	-Winvalid-utf8 -Wmissing-declarations -Wno-c2y-extensions                 \
	-Wno-unknown-warning-option"

# Show non-trivial use of HX_HARDENING_MODE_*.
HX_FLAGS_="-DHX_USE_SLAB_ALLOCATOR=(HX_HARDENING_MODE!=HX_HARDENING_MODE_VERBOSE) \
	-DHX_USE_FILE_IO=2 -ffast-math -ggdb"

HX_SAN_UNDEF_="-fsanitize=undefined,address"
HX_SAN_THREAD_="-fsanitize=thread"
HX_SAN_MEMORY_="-fsanitize=memory -fsanitize-memory-track-origins"

HX_DIR_=$PWD
HX_JOBS_="${HX_MAX_CORES:-$(getconf _NPROCESSORS_ONLN)}"

HX_GCC_=gcc
HX_GXX_=g++
if [ "$(uname -s)" = Darwin ]; then
	PATH="/opt/homebrew/bin:/opt/homebrew/sbin${PATH+:$PATH}"
	if ! command -v brew > /dev/null 2>&1; then
		echo "error: Install brew and then run install_brew.sh."
		exit 1
	fi
	HX_GCC_=$(basename "$(brew --prefix gcc)"/bin/gcc-[0-9]*)
	HX_GXX_="g++-${HX_GCC_#gcc-}"
fi

HX_M32_="-m32"
if [ "$($HX_GCC_ -m32 -print-file-name=libc.so 2>/dev/null)" = libc.so ]; then
	HX_M32_=""
	echo "32-bit libc not installed. Building 64-bit..."
fi

run_hxtest() {
	if ./hxtest runtests > console_output.txt 2>&1; then
		# The summary of a TAP report or of Google Test style reporting.
		grep -E '^(not ok|Bail out!|# (passed|failed) )|\[  PASSED  \]|FAILED TESTS' \
			console_output.txt || cat console_output.txt
		rm -f hxtest *.o *.txt *.bin *.json
	else
		cat console_output.txt
		echo "error: hxtest non-zero exit."
		exit 1
	fi
}

# Build artifacts are not retained.
rm -rf "$(readlink -f build)" build
if [ "${HX_RAMDISK:-0}" = 1 ]; then mkdir build; else ln -s "$(mktemp -d)" build; fi
cd build

# -- Clang ---------------------------------------------------------------------
# Run different sanitizers at all build levels with gcc.

run_clang_build() {
	HX_OPT_=$1; HX_SAN_=$2; shift 2
	echo "clang c17/c++23 -O$HX_OPT_ $HX_SAN_ $* ..."

	# _FORTIFY_SOURCE is predefined by some toolchains and glibc warns when it is
	# used without optimization.
	if [ "$HX_OPT_" = 0 ]; then HX_FORTIFY_="-U_FORTIFY_SOURCE"; else HX_FORTIFY_=""; fi

	# compile C17
	clang -I"$HX_DIR_/include" "-DHX_HARDENING_MODE=(3-$HX_OPT_)" -O$HX_OPT_ \
		$HX_FLAGS_ $HX_FORTIFY_ $HX_ERRORS_ $HX_SAN_                         \
		-fdiagnostics-absolute-paths -pthread -std=c17 "$@"                  \
		-c "$HX_DIR_"/test/*.c

	# generate C++23 pch. clang does this automatically when a c++ header file
	# is the target. This is just a test.
	clang++ -I"$HX_DIR_/include" "-DHX_HARDENING_MODE=(3-$HX_OPT_)" -O$HX_OPT_ \
		$HX_FLAGS_ $HX_FORTIFY_ $HX_ERRORS_ $HX_SAN_ -DHX_USE_THREADS=1        \
		-pthread -std=c++23 -fno-exceptions -fdiagnostics-absolute-paths "$@"  \
		"$HX_DIR_"/include/hx/hxtest.hpp -o hxtest.hpp.pch

	# compile C++23
	printf '%s\0' "$HX_DIR_"/src/*.cpp "$HX_DIR_"/test/*.cpp |
		xargs -0 -n 1 -P "$HX_JOBS_" sh -c '"$0" "$@" || exit 255'           \
		clang++ -I"$HX_DIR_/include" "-DHX_HARDENING_MODE=(3-$HX_OPT_)"      \
			-O$HX_OPT_ $HX_FLAGS_ $HX_FORTIFY_ $HX_ERRORS_ $HX_SAN_          \
			-DHX_USE_THREADS=1 -pthread -std=c++23 -fno-exceptions           \
			-fdiagnostics-absolute-paths "$@" -include-pch hxtest.hpp.pch -c

	# clang++ links its own C++ standard library.
	clang++ $HX_SAN_ *.o -lpthread -o hxtest

	run_hxtest
}

# Undefined behavior and address sanitization at all build levels with clang.
for HX_I_ in 0 1 2 3; do
	run_clang_build "$HX_I_" "$HX_SAN_UNDEF_" "$@"
done

# Run the thread sanitizer with all optimizations on.
run_clang_build 3 "$HX_SAN_THREAD_" "$@"

# Run the memory sanitizer in debug and with all optimizations on.
if [ "$(uname -s)" != Darwin ]; then
	run_clang_build 3 "$HX_SAN_MEMORY_" "$@"
	run_clang_build 0 "$HX_SAN_MEMORY_" "$@"
fi

# -- GCC ----------------------------------------------------------------------
# Undefined behavior and address sanitization at all build levels with gcc.
# These also test Google Test style reporting with HX_USE_TEST_FRAMEWORK=1.

for HX_I_ in 0 1 2 3; do
	echo "$HX_GCC_ c99/c++11 -O$HX_I_ $* ..."

	if [ "$HX_I_" = 0 ]; then HX_FORTIFY_="-U_FORTIFY_SOURCE"; else HX_FORTIFY_=""; fi

	$HX_GCC_ -I"$HX_DIR_/include" "-DHX_HARDENING_MODE=(3-$HX_I_)" -O$HX_I_  \
		$HX_FLAGS_ $HX_ERRORS_ $HX_FORTIFY_ $HX_SAN_UNDEF_ -pthread -std=c99 \
		$HX_M32_ "$@" -c "$HX_DIR_"/test/*.c

	printf '%s\0' "$HX_DIR_"/src/*.cpp "$HX_DIR_"/test/*.cpp |
		xargs -0 -n 1 -P "$HX_JOBS_" sh -c '"$0" "$@" || exit 255'              \
		$HX_GCC_ -I"$HX_DIR_/include" "-DHX_HARDENING_MODE=(3-$HX_I_)" -O$HX_I_ \
			$HX_FLAGS_ $HX_ERRORS_ $HX_FORTIFY_ $HX_SAN_UNDEF_ -pthread         \
			-DHX_USE_TEST_FRAMEWORK=1 -std=c++11 -fno-exceptions -fno-rtti      \
			$HX_M32_ "$@" -c

	$HX_GCC_ $HX_SAN_UNDEF_ *.o -lpthread -lstdc++ $HX_M32_ -o hxtest

	run_hxtest
done

echo 🪓🪓🪓
