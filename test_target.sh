#!/bin/sh
# SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
# SPDX-License-Identifier: MIT
# This file is licensed under the terms of the LICENSE.md file.
#
# Builds target/test_target/test_target.cpp with Meson and Ninja using clang and
# gcc, with and without the hx module, and runs it. "meson test" consumes its
# Test Anything Protocol (TAP) report. Then builds the programs of each board in
# target/<platform>/boards whose SDK is installed with CMake and Ninja in
# build/<board> and tests them with CTest, which flashes a connected board and
# judges its TAP report. The tests of a board that is not connected are
# skipped. Stops at the first failure.
#
# --board and --program select what is built and tested. A run that selects a
# board keeps build/<board>, so only what changed is rebuilt. A run that
# includes meson starts from an empty build directory. With --require-all every
# SDK has to be installed and every board connected and tested.
#
# usage: test_target.sh [--verbose] [--require-all] [--board meson|<board>]
#	[--program <program>]

trap 'trap "" INT PIPE TERM; pkill -P $$; wait; exit 1' INT PIPE TERM
set -eu

cd "$(dirname "$(readlink -f "$0")")"

export POSIXLY_CORRECT=1

HX_JOBS_="${HX_MAX_JOBS:-$(getconf _NPROCESSORS_ONLN)}"

# Builds with the C library headers only and no C++ standard library.
HX_C_FLAGS_="-DHX_HARDENING_MODE=HX_HARDENING_MODE_DEBUG -ggdb3"
HX_CPP_FLAGS_="$HX_C_FLAGS_ -DHX_USE_STD_LIB=0 -DHX_USE_NAMESPACE=hx -DHX_USE_CONSOLE=2 \
	-std=c++23 -nostdinc++ -fno-exceptions -fno-rtti"
HX_LINK_FLAGS_="-nodefaultlibs -lc -lpthread -lm"

# The boards as <platform>/boards/<board>. Each platform is a CMake project
# with a platform.sh.
HX_BOARDS_=$(cd target && ls -d */boards/*/ | sed 's|/$||')

# Set by parse_arguments. "--verbose" or empty.
HX_VERBOSE_=""

# Set by parse_arguments. 1 when --require-all was passed.
HX_REQUIRE_ALL_=0

# Set by parse_arguments. "meson", a <board> or empty for all.
HX_SELECTED_BOARD_=""

# Set by parse_arguments. A <program> or empty for all.
HX_SELECTED_PROGRAM_=""

# Set by select_native_files. The Meson native files for clang and gcc.
HX_CLANG_NATIVE_=""
HX_GCC_NATIVE_=""

usage_error() {
	echo "usage_error: $0 [--verbose] [--require-all] [--board meson|<board>] [--program <program>]"
	echo "boards: meson $(echo $HX_BOARDS_ | sed 's|[^ ]*/||g')"
	exit 1
}

parse_arguments() {
	while [ $# -gt 0 ]; do
		case "$1" in
			--verbose) HX_VERBOSE_="--verbose" ;;
			--require-all) HX_REQUIRE_ALL_=1 ;;
			--board)
				[ $# -ge 2 ] || usage_error
				HX_SELECTED_BOARD_="$2"
				shift
				;;
			--program)
				[ $# -ge 2 ] || usage_error
				HX_SELECTED_PROGRAM_="$2"
				shift
				;;
			*) usage_error ;;
		esac
		shift
	done

	# A board is meson or in HX_BOARDS_. A program has a <platform>.cmake.
	HX_BOARD_VALID_=0
	HX_PROGRAM_VALID_=0
	if [ -z "$HX_SELECTED_BOARD_" ] || [ "$HX_SELECTED_BOARD_" = meson ]; then
		HX_BOARD_VALID_=1
	fi
	if [ -z "$HX_SELECTED_PROGRAM_" ]; then
		HX_PROGRAM_VALID_=1
	fi
	for HX_BOARD_ in $HX_BOARDS_; do
		if [ "${HX_BOARD_##*/}" = "$HX_SELECTED_BOARD_" ]; then
			HX_BOARD_VALID_=1
		fi
		if [ -f "target/$HX_SELECTED_PROGRAM_/${HX_BOARD_%%/*}.cmake" ]; then
			HX_PROGRAM_VALID_=1
		fi
	done
	[ "$HX_BOARD_VALID_$HX_PROGRAM_VALID_" = 11 ] || usage_error
}

# Runs a command. Without --verbose its output is only shown when it fails.
run_quietly() {
	if [ -n "$HX_VERBOSE_" ]; then
		"$@"
	elif ! "$@" > build/command_out.txt 2>&1; then
		cat build/command_out.txt
		echo "error: $1 failed"
		exit 1
	fi
}

# -- Meson ---------------------------------------------------------------------

# Adds the Homebrew directories to PATH on macOS. Run install_brew.sh.
add_homebrew_path() {
	PATH="/opt/homebrew/bin:/opt/homebrew/sbin${PATH+:$PATH}"
	if ! command -v brew > /dev/null 2>&1; then
		echo "error: Install brew and then run install_brew.sh."
		exit 1
	fi
}

select_native_files() {
	HX_CLANG_NATIVE_=meson/clang.ini
	HX_GCC_NATIVE_=meson/gcc.ini
	if [ "$(uname -s)" = Darwin ]; then
		HX_CLANG_NATIVE_=meson/macos-clang.ini
		HX_GCC_NATIVE_=meson/macos-gcc.ini
	elif [ -x /opt/rh/gcc-toolset-15/root/usr/bin/g++ ]; then
		HX_GCC_NATIVE_=meson/gcc-toolset-15.ini
	fi
}

# The arguments are passed to "meson setup".
build_and_test_with_meson() {
	# Build artifacts are not retained.
	rm -rf "$(readlink -f build)" build; mkdir build
	run_quietly meson setup build --buildtype=debug -Dc_args="$HX_C_FLAGS_" \
		-Dcpp_args="$HX_CPP_FLAGS_" -Dcpp_link_args="$HX_LINK_FLAGS_" "$@"
	run_quietly ninja -v -C build -j "$HX_JOBS_"

	# The link lets the .vscode configuration debug it as build/hxtest.
	ln -sf hxtest_target build/hxtest

	meson test -C build --no-rebuild --print-errorlogs $HX_VERBOSE_
}

# Builds and tests with clang and gcc, with and without the named module.
test_meson() {
	echo "$HX_CLANG_NATIVE_ meson+ninja with textual includes..."
	build_and_test_with_meson --native-file="$HX_CLANG_NATIVE_" -Dbuild_module=disabled

	echo "$HX_CLANG_NATIVE_ meson+ninja with named module..."
	build_and_test_with_meson --native-file="$HX_CLANG_NATIVE_"

	echo "$HX_GCC_NATIVE_ meson+ninja with textual includes..."
	build_and_test_with_meson --native-file="$HX_GCC_NATIVE_" -Dbuild_module=disabled

	# Textual includes are built with GCC 14, e.g. Debian 13.
	echo "$HX_GCC_NATIVE_ meson+ninja with named module..."
	build_and_test_with_meson --native-file="$HX_GCC_NATIVE_"
}

# -- Boards --------------------------------------------------------------------

# Builds and tests the selected programs of board $2 of platform $1 when the
# SDK is installed. The SDK's environment stays in a subshell. The build
# directory is configured once and Ninja configures it again when its CMake
# files change. A subdirectory of build leaves build/hxtest and keeps its
# CMakeCache.txt from being found by test_cmake.sh. Exits at the first failure.
test_board() {
	(
		. "target/$1/platform.sh"
		if ! platform_sdk; then
			if [ "$HX_REQUIRE_ALL_" = 1 ]; then
				echo "error: --require-all: $2 can't be built."
				exit 1
			fi
			echo "$2 skipped."
			exit 0
		fi

		mkdir -p "build/$2"
		run_quietly platform_environment
		if [ ! -f "build/$2/CMakeCache.txt" ]; then
			echo "$2 cmake with target/$1..."
			run_quietly cmake -S "target/$1" -B "build/$2" -G Ninja -DHX_BOARD="$2"
		fi

		# The programs are the tests of the board.
		for HX_PROGRAM_ in $(ctest --test-dir "build/$2" -N | sed -n 's/^ *Test *#[0-9]*: //p'); do
			if [ -n "$HX_SELECTED_PROGRAM_" ] && [ "$HX_SELECTED_PROGRAM_" != "$HX_PROGRAM_" ]; then
				continue
			fi
			echo "$2 $HX_PROGRAM_ cmake+ninja..."
			run_quietly cmake --build "build/$2" --target "$HX_PROGRAM_" -j "$HX_JOBS_"
			echo "$2 $HX_PROGRAM_ ctest..."
			HX_REQUIRE_BOARD="$HX_REQUIRE_ALL_" ctest --test-dir "build/$2" \
				-R "^$HX_PROGRAM_\$" --no-tests=error --output-on-failure $HX_VERBOSE_
		done
	)
}

# -- Main ----------------------------------------------------------------------

main() {
	parse_arguments "$@"

	[ "$(uname -s)" = Darwin ] && add_homebrew_path
	select_native_files

	[ -n "$HX_VERBOSE_" ] && set -o xtrace

	# Meson builds test_target alone.
	if { [ -z "$HX_SELECTED_BOARD_" ] || [ "$HX_SELECTED_BOARD_" = meson ]; } &&
			{ [ -z "$HX_SELECTED_PROGRAM_" ] || [ "$HX_SELECTED_PROGRAM_" = test_target ]; }; then
		test_meson
	fi

	for HX_BOARD_ in $HX_BOARDS_; do
		if [ -z "$HX_SELECTED_BOARD_" ] || [ "$HX_SELECTED_BOARD_" = "${HX_BOARD_##*/}" ]; then
			test_board "${HX_BOARD_%%/*}" "${HX_BOARD_##*/}"
		fi
	done

	echo 🪓🪓🪓
}

main "$@"
