#!/bin/sh
# SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
# SPDX-License-Identifier: MIT
# This file is licensed under the terms of the LICENSE.md file.
#
# Tests libhatchet with gcc and clang in a variety of configurations. Tests C99,
# C17, C++11, and C++17.
#
# The -m32 switch enables 32-bit compilation. You will need these packages on
# Ubuntu:
#
#   sudo apt-get install gcc-multilib g++-multilib
#
# Adds script arguments to the compiler command line using "$@" so, for example,
# calling:
#
#   ./testmatrix.sh -DHX_TEST_ERROR_HANDLING=1
#
# will run the tests with HX_TEST_ERROR_HANDLING defined as 1.

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

# Enables glibc heap integrity checking.
export MALLOC_CHECK_=3

# Fatal warning flags.
ERRORS="-Wall -Wextra -pedantic-errors -Werror -Wfatal-errors -Wcast-qual \
	-Wdisabled-optimization -Wshadow -Wundef -Wconversion -Wdate-time \
	-Wmissing-declarations -Wno-c2y-extensions"

FLAGS="-DHX_USE_POSIX_FILE_IO=1 -ffast-math -ggdb3 -D_FORTIFY_SOURCE=3"

SANITIZE_UNDEF="-fsanitize=undefined,address"
SANITIZE_THREAD="-fsanitize=thread"
SANITIZE_MEMORY="-fsanitize=memory -fsanitize-memory-track-origins"

HX_DIR=`pwd`

run_hxtest() {
	if ./hxtest runtests > console_output.txt 2>&1; then
		grep -e '\[  PASSED  \]' -e '\[  FAILED  \]' -e 'FAILED TESTS' \
			console_output.txt || cat console_output.txt
	else
		cat console_output.txt
		echo "error: hxtest non-zero exit."
		exit 1
	fi
}

# Build artifacts are not retained.
rm -rf ./build; mkdir ./build && cd ./build

# -----------------------------------------------------------------------------
# clang

run_clang_build() {
	echo "clang c17/c++20 -O$1 ..."

	# compile C17
	clang -I../include -DHX_HARDENING_MODE=3-$1 -O$1 $FLAGS $ERRORS \
		-fdiagnostics-absolute-paths -pthread -std=c17 -c ../test/*.c

	# generate C++20 pch. clang does this automatically when a c++ header file
	# is the target. This is just a test.
	clang++ -I../include -DHX_HARDENING_MODE=3-$1 -O$1 $FLAGS $ERRORS \
		-DHX_USE_THREADS=1 -pthread -std=c++23 \
		-fno-exceptions -fdiagnostics-absolute-paths \
		../include/hx/hxtest.hpp -o hxtest.hpp.pch

	# compile C++23 and link
	clang++ -I../include -DHX_HARDENING_MODE=3-$1 -O$1 $FLAGS $ERRORS \
		-DHX_USE_THREADS=1 -pthread -std=c++23 \
		-fno-exceptions -fdiagnostics-absolute-paths \
		-include-pch hxtest.hpp.pch ../src/*.cpp ../test/*.cpp *.o \
		-lpthread -lstdc++ -o hxtest

	run_hxtest

	rm -f hxtest *.o *.txt *.bin *.json *.pch
}

# Test undefined behavior/address use at all build levels with clang. Uses pch
# and allows exceptions just to make sure there are none.
for I in 0 1 2 3; do
	run_clang_build "$I" "$SANITIZE_UNDEF" "$@"
done

# Run the thread sanitizer with all optimizations on.
run_clang_build 3 "$SANITIZE_THREAD" "$@"

# Run the memory sanitizer in debug and with all optimizations on.
run_clang_build 3 "$SANITIZE_MEMORY" "$@"
run_clang_build 0 "$SANITIZE_MEMORY" "$@"

# -----------------------------------------------------------------------------
# gcc

for I in 0 1 2 3; do
echo "gcc c99/c++11 -O$I $@ ..."

gcc -I$HX_DIR/include -DHX_HARDENING_MODE=3-$I -O$I $FLAGS $ERRORS \
	-pthread -std=c99 -m32 "$@" -c $HX_DIR/test/*.c

gcc -I$HX_DIR/include -DHX_HARDENING_MODE=3-$I -O$I $FLAGS $ERRORS \
	-pthread -std=c++11 -fno-exceptions -fno-rtti "$@" $HX_DIR/src/*.cpp \
	$HX_DIR/test/*.cpp *.o -lpthread -lstdc++ -m32 -o hxtest

run_hxtest

rm -f hxtest *.o *.txt *.bin *.json
done

# -----------------------------------------------------------------------------

# Make sure the script returns 0.
echo "🪓🪓🪓 all tests passed."
