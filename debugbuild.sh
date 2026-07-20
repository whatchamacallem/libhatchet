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
# -Wno-ambiguous-reversed-operator is for legacy clang++.
HX_ERRORS_="-Wall -Wextra -pedantic-errors -Werror -Wfatal-errors -Wcast-qual \
	-Wdisabled-optimization -Wshadow -Wundef -Wconversion -Wdate-time         \
	-Wmissing-declarations -Wno-ambiguous-reversed-operator                   \
	-Wno-unused-variable -Wno-c2y-extensions -Wno-unknown-warning-option"

HX_FLAGS_="-O0 -m32 -ggdb3 -fdiagnostics-absolute-paths -fdiagnostics-color=always"

export POSIXLY_CORRECT=1

# Prevent leaking background tasks.
trap '{ set +o xtrace; } 2> /dev/null
	trap - 1 2 3 6 15
	for HX_PID_ in $(pgrep -g "$$" 2>/dev/null); do
		[ "$HX_PID_" = "$$" ] && continue
		kill -9 "$HX_PID_" 2>/dev/null
	done
	exit 1
' 1 2 3 6 15

set -eu

HX_OPT_CLEAR_=0
HX_OPT_GRIND_=0
HX_OPT_VERBOSE_=0
HX_OPT_RUN_=0
for HX_ARG_ in "$@"; do
	case "$HX_ARG_" in
		--clear)   HX_OPT_CLEAR_=1 ;;
		--grind)   HX_OPT_GRIND_=1 ;;
		--verbose) HX_OPT_VERBOSE_=1 ;;
		--run)     HX_OPT_RUN_=1 ;;
		*)
			echo "Usage: $0 [--clear] [--grind] [--verbose] [--run]"
			echo "  --clear   Clear the terminal before building."
			echo "  --grind   Build all configuration combinations."
			echo "  --verbose Full hxtest output."
			echo "  --run     Run hxtest after building."
			exit 1
			;;
	esac
done

# Clear the output window when building from the editor.
if [ "$HX_OPT_CLEAR_" = "1" ]; then
	export TERM=xterm-256color
	clear
fi

HX_CXX_="-std=c++23"
if [ -z "$(echo | clang++ -std=c++23 -dM -E -x c++ - 2>/dev/null | grep __cplusplus)" ]; then
	HX_CXX_="-std=c++20"
	echo "C++23 not supported..."
fi

build_hxtest() {
	# Build artifacts are not retained. Only ccache the normal build.
	HX_CCACHE_="${1:-}"
	rm -rf "$(readlink -f build)" build; ln -s "$(mktemp -d)" build

	HX_PIDS_=""
	for HX_FILE_ in test/*.c; do
		$HX_CCACHE_ clang $HX_BUILD_ $HX_ERRORS_ $HX_FLAGS_ -Iinclude -std=c17 -c $HX_FILE_ \
			-o build/$(basename "$HX_FILE_" .c).o & HX_PIDS_="$HX_PIDS_ $!"
	done

	for HX_FILE_ in src/*.cpp test/*.cpp; do
		$HX_CCACHE_ clang++ $HX_BUILD_ $HX_ERRORS_ $HX_FLAGS_ -Iinclude $HX_CXX_ \
			-fno-exceptions -fno-rtti -c $HX_FILE_                               \
			-o build/$(basename "$HX_FILE_" .cpp).o & HX_PIDS_="$HX_PIDS_ $!"
	done

	for HX_PID_ in $HX_PIDS_; do
		if ! wait "$HX_PID_"; then
			exit 1
		fi
	done

	$HX_CCACHE_ clang++ $HX_FLAGS_ build/*.o -lpthread -lstdc++ -lm -o build/hxtest
}

if [ "$HX_OPT_GRIND_" = "1" ]; then
	HX_COUNT_=1
	HX_SPINNER_STATE_=0
	set -- "[    ]" "[#   ]" "[##  ]" "[### ]" "[####]" "[ ###]" "[  ##]" "[   #]"

	for HX_NAMESPACE_ in "" -DHX_USE_NAMESPACE=hx; do
	for HX_CONSOLE_   in 0 1 2;  do
	for HX_FILE_IO_   in 0 1 2;  do
	for HX_LIBCXX_    in 0 1;    do
	for HX_LOGGING_   in 0 1 2;  do
	for HX_MEMORY_    in 0 1;    do
	for HX_PROFILER_  in 0 1;    do
	for HX_THREADS_   in 0 1 11; do

		# Only test every 7th permutation. It is important to use a prime number.
		HX_COUNT_=$((HX_COUNT_ + 1))
		[ $(($HX_COUNT_ % 7)) -eq 0 ] || continue

		HX_BUILD_="$HX_NAMESPACE_ -DHX_HARDENING_MODE=HX_HARDENING_MODE_DEBUG"
		HX_BUILD_="$HX_BUILD_ -DHX_USE_CONSOLE=$HX_CONSOLE_"
		HX_BUILD_="$HX_BUILD_ -DHX_USE_FILE_IO=$HX_FILE_IO_"
		HX_BUILD_="$HX_BUILD_ -DHX_USE_LIBCXX=$HX_LIBCXX_"
		[ "$HX_LIBCXX_" = "0" ] && HX_BUILD_="$HX_BUILD_ -nostdinc++"
		HX_BUILD_="$HX_BUILD_ -DHX_USE_LOGGING=$HX_LOGGING_"
		HX_BUILD_="$HX_BUILD_ -DHX_USE_MEMORY_MANAGER=$HX_MEMORY_"
		HX_BUILD_="$HX_BUILD_ -DHX_USE_PROFILER=$HX_PROFILER_"
		HX_BUILD_="$HX_BUILD_ -DHX_USE_THREADS=$HX_THREADS_"

		if [ "$HX_OPT_VERBOSE_" = "1" ]; then
			echo "[$HX_COUNT_] $HX_BUILD_"
		else
			HX_SPINNER_STATE_=$(((HX_SPINNER_STATE_ % 8) + 1))
			eval "printf '\\r%s' \"\${$HX_SPINNER_STATE_}\""
		fi

		build_hxtest
	done; done; done; done; done; done; done; done

	if [ "$HX_OPT_VERBOSE_" = "0" ]; then
		printf "\r      \r"
	fi
fi

# Enable as much code as possible for the default build and run.
HX_BUILD_="-DHX_HARDENING_MODE=HX_HARDENING_MODE_DEBUG -DHX_USE_CONSOLE=2 -DHX_USE_PROFILER=1"
build_hxtest ccache

# Show stats or save tokens.
if [ "$HX_OPT_VERBOSE_" = "1" ] && [ -z "${CLAUDECODE:-}" ]; then
	ccache --show-stats
fi

if [ "$HX_OPT_RUN_" = "1" ]; then
	cd build
	if [ "$HX_OPT_VERBOSE_" = "1" ]; then
		./hxtest
	else
		if ./hxtest > console_output.txt 2>&1; then
			grep -E '\[  PASSED  \]|\[  FAILED  \]|FAILED TESTS' console_output.txt
		else
			cat console_output.txt
			exit 1
		fi
	fi
fi

echo 🪓🪓🪓
