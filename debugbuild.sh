#!/bin/sh
# SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
# SPDX-License-Identifier: MIT
# This file is licensed under the terms of the LICENSE.md file.
#
# Intended for used with GDB during iterative development. Also checks
# permutations of build flags.
#
# This build uses 32-bit pointers because they are easier to read. The -m32
# switch enables 32-bit compilation. See debian_packages.sh. Do not use a .pch
# with ccache. It won't work as expected.

# -Wno-unused-variable is only for debugging.
ERRORS="-Wall -Wextra -pedantic-errors -Werror -Wfatal-errors -Wcast-qual \
	-Wdisabled-optimization -Wshadow -Wundef -Wconversion -Wdate-time     \
	-Wmissing-declarations -Wno-unused-variable -Wno-c2y-extensions       \
	-Wno-unknown-warning-option"

FLAGS="-O0 -m32 -ggdb3 -fdiagnostics-absolute-paths -fdiagnostics-color=always"

export POSIXLY_CORRECT=1

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

OPT_GRIND=0
OPT_HEADLESS=0
OPT_RUN=0
for ARG in "$@"; do
	case "$ARG" in
		--grind)    OPT_GRIND=1 ;;
		--headless) OPT_HEADLESS=1 ;;
		--run)      OPT_RUN=1 ;;
		*)
			echo "Usage: $0 [--grind] [--headless] [--run]"
			echo "  --grind    Build all configuration combinations."
			echo "  --headless Reduce hxtest output to pass and fail lines."
			echo "  --run      Run hxtest after building."
			exit 1
			;;
	esac
done

# Clear the output window when building from the editor.
if [ "$OPT_HEADLESS" = "0" ] && [ "$OPT_RUN" = "0" ] && [ -z "${CLAUDECODE:-}" ]; then
	export TERM=xterm-256color
	clear
fi

build_hxtest() {
	# Build artifacts are not retained.
	rm -rf build; mkdir build

	PIDS=""
	for FILE in test/*.c; do
		ccache clang $BUILD $ERRORS $FLAGS -Iinclude -std=c17 -c $FILE \
			-o build/$(basename "$FILE" .c).o & PIDS="$PIDS $!"
	done

	for FILE in src/*.cpp test/*.cpp; do
		ccache clang++ $BUILD $ERRORS $FLAGS -Iinclude -std=c++23 -fno-exceptions -fno-rtti \
			-c $FILE -o build/$(basename "$FILE" .cpp).o & PIDS="$PIDS $!"
	done

	for PID in $PIDS; do
		if ! wait "$PID"; then
			exit 1
		fi
	done

	ccache clang++ $FLAGS build/*.o -lpthread -lstdc++ -lm -o build/hxtest
}

if [ "$OPT_GRIND" = "1" ]; then
	COUNT=1
	SPINNER_STATE=0
	set -- "[    ]" "[#   ]" "[##  ]" "[### ]" "[####]" "[ ###]" "[  ##]" "[   #]"

	for NAMESPACE in "" -DHX_USE_NAMESPACE=hx; do
	for CONSOLE   in 0 1 2;  do
	for FILE_IO   in 0 1 2;  do
	for LIBCXX    in 0 1;    do
	for LOGGING   in 0 1 2;  do
	for MEMORY    in 0 1;    do
	for PROFILER  in 0 1;    do
	for THREADS   in 0 1 11; do

		# Only test every 7th permutation. It is important to use a prime number.
		COUNT=$((COUNT + 1))
		[ $(($COUNT % 7)) -eq 0 ] || continue

		BUILD="$NAMESPACE -DHX_HARDENING_MODE=HX_HARDENING_MODE_DEBUG"
		[ -n "$CONSOLE"   ] && BUILD="$BUILD -DHX_USE_CONSOLE=$CONSOLE"
		[ -n "$FILE_IO"   ] && BUILD="$BUILD -DHX_USE_FILE_IO=$FILE_IO"
		[ -n "$LIBCXX"    ] && BUILD="$BUILD -DHX_USE_LIBCXX=$LIBCXX"
		[ "$LIBCXX" = "0" ] && BUILD="$BUILD -nostdinc++"
		[ -n "$LOGGING"   ] && BUILD="$BUILD -DHX_USE_LOGGING=$LOGGING"
		[ -n "$MEMORY"    ] && BUILD="$BUILD -DHX_USE_MEMORY_MANAGER=$MEMORY"
		[ -n "$PROFILER"  ] && BUILD="$BUILD -DHX_USE_PROFILER=$PROFILER"
		[ -n "$THREADS"   ] && BUILD="$BUILD -DHX_USE_THREADS=$THREADS"

		if [ "$OPT_HEADLESS" = "1" ]; then
			SPINNER_STATE=$(((SPINNER_STATE % 8) + 1))
			eval "printf '\\r%s' \"\${$SPINNER_STATE}\""
		else
			echo "[$COUNT] $BUILD"
		fi

		build_hxtest
	done; done; done; done; done; done; done; done

	if [ "$OPT_HEADLESS" = "1" ]; then
		printf "\r      \r"
	fi
fi

# Enable as much code as possible for the default build and run.
BUILD="-DHX_HARDENING_MODE=HX_HARDENING_MODE_DEBUG -DHX_USE_CONSOLE=2 -DHX_USE_PROFILER=1"
build_hxtest

# Show stats or save tokens.
if [ "$OPT_HEADLESS" = "0" ] && [ -z "${CLAUDECODE:-}" ]; then
	ccache --show-stats
fi

if [ "$OPT_RUN" = "1" ]; then
	cd build
	if [ "$OPT_HEADLESS" = "1" ]; then
		./hxtest 2>&1 | grep -E '\[  PASSED  \]|\[  FAILED  \]|FAILED TESTS'
	else
		./hxtest
	fi
fi

echo 🪓🪓🪓
