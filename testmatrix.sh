#!/bin/sh
# SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
# SPDX-License-Identifier: MIT
# This file is licensed under the terms of the LICENSE.md file.

# Adds script arguments to the compiler command line using "$@".

# Prevent leaking background tasks.
trap '{ set +o xtrace; } 2> /dev/null
	trap - 1 2 3 6 15
	for pid in $(pgrep -g "$$" 2>/dev/null); do
		[ "$pid" = "$$" ] && continue
		kill -9 "$pid" 2>/dev/null
	done
	exit 1
' 1 2 3 6 15

set -eu

export POSIXLY_CORRECT=1

# Enables glibc heap integrity checking.
export MALLOC_CHECK_=3

# Fatal warning flags.
ERRORS="-Wall -Wextra -pedantic-errors -Werror -Wfatal-errors -Wcast-qual   \
	-Wdisabled-optimization -Wshadow -Wundef -Wconversion -Wdate-time       \
	-Wmissing-declarations -Wno-c2y-extensions -Wno-unknown-warning-option"

FLAGS="-DHX_USE_FILE_IO=2 -ffast-math -ggdb3 -D_FORTIFY_SOURCE=3"

SAN_UNDEF="-fsanitize=undefined,address"
SAN_THREAD="-fsanitize=thread"
SAN_MEMORY="-fsanitize=memory -fsanitize-memory-track-origins"

HX_DIR=$PWD

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
	OPT=$1; SAN=$2; shift 2
	echo "clang c17/c++23 -O$OPT $SAN $* ..."

	# compile C17
	clang -I"$HX_DIR/include" -DHX_HARDENING_MODE=3-$OPT -O$OPT $FLAGS $ERRORS $SAN   \
		-fdiagnostics-absolute-paths -pthread -std=c17 "$@" -c "$HX_DIR"/test/*.c

	# generate C++23 pch. clang does this automatically when a c++ header file
	# is the target. This is just a test.
	clang++ -I"$HX_DIR/include" -DHX_HARDENING_MODE=3-$OPT -O$OPT $FLAGS $ERRORS $SAN \
		-DHX_USE_THREADS=1 -pthread -std=c++23 -fno-exceptions                        \
		-fdiagnostics-absolute-paths "$@" "$HX_DIR"/include/hx/hxtest.hpp             \
		-o hxtest.hpp.pch

	# compile C++23 and link
	clang++ -I"$HX_DIR/include" -DHX_HARDENING_MODE=3-$OPT -O$OPT $FLAGS $ERRORS $SAN \
		-DHX_USE_THREADS=1 -pthread -std=c++23 -fno-exceptions                        \
		-fdiagnostics-absolute-paths "$@" -include-pch hxtest.hpp.pch                 \
		"$HX_DIR"/src/*.cpp "$HX_DIR"/test/*.cpp *.o -lpthread -lstdc++ -o hxtest

	run_hxtest
}

# Test undefined behavior/address use at all build levels with clang. Uses a
# pch.
for I in 0 1 2 3; do
	run_clang_build "$I" "$SAN_UNDEF" "$@"
done

# Run the thread sanitizer with all optimizations on.
run_clang_build 3 "$SAN_THREAD" "$@"

# Run the memory sanitizer in debug and with all optimizations on.
run_clang_build 3 "$SAN_MEMORY" "$@"
run_clang_build 0 "$SAN_MEMORY" "$@"

# -- GCC ----------------------------------------------------------------------

for I in 0 1 2 3; do
echo "gcc c99/c++11 -O$I $* ..."

gcc -I"$HX_DIR/include" -DHX_HARDENING_MODE=3-$I -O$I $FLAGS $ERRORS $SAN_UNDEF \
	-pthread -std=c99 -m32 "$@" -c "$HX_DIR"/test/*.c

gcc -I"$HX_DIR/include" -DHX_HARDENING_MODE=3-$I -O$I $FLAGS $ERRORS $SAN_UNDEF \
	-pthread -std=c++11 -fno-exceptions -fno-rtti "$@" "$HX_DIR"/src/*.cpp      \
	"$HX_DIR"/test/*.cpp *.o -lpthread -lstdc++ -m32 -o hxtest

run_hxtest
done

# Make sure the script returns 0.
echo "🪓🪓🪓 all tests passed."
