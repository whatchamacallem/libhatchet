#!/bin/bash
# SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
# SPDX-License-Identifier: MIT
# This file is licensed under the terms of the LICENSE.md file.
#
# If build/hxtest fails, then the tests will stop at that point and it will be
# available to debug. Any shell command that fails should also stop the test
# process.

trap 'trap "" INT PIPE TERM; pkill -P $$; wait; exit 1' INT PIPE TERM
set -euo pipefail

cd "$(dirname "$(readlink -f "$0")")"

# Use an array to avoid globbing.
HX_TEXT_FILES_=(\(
	-name .clang-tidy -o -name .gdbinit -o -name .gitattributes
	-o -name .gitignore -o -name '*.bat' -o -name '*.c' -o -name '*.cpp'
	-o -name '*.h' -o -name '*.hpp' -o -name '*.inl' -o -name '*.json'
	-o -name '*.md' -o -name '*.ps1' -o -name '*.py' -o -name '*.sh'
	-o -name '*.txt' \))

# Set by parse_arguments. "--verbose" or empty.
HX_VERBOSE_=""

parse_arguments() {
	if [ "${1:-}" = "--verbose" ]; then
		HX_VERBOSE_="--verbose"
	elif [ -n "${1:-}" ]; then
		echo "usage_error: $0 [--verbose]"
		exit 1
	fi
}

# Adds the Homebrew directories to PATH on macOS. Run install_brew.sh.
add_homebrew_path() {
	PATH="/opt/homebrew/bin:/opt/homebrew/sbin${PATH+:$PATH}"
	if ! command -v brew > /dev/null 2>&1; then
		echo "error: Install brew and then run install_brew.sh."
		exit 1
	fi

	# BSD grep does not support -P.
	export PATH="$(brew --prefix grep)/libexec/gnubin:$PATH"
}

# A release will require the tests to pass without admin rights.
check_not_root() {
	if [ "$(id -u)" -eq 0 ]; then
		echo "error: Do not run test_all.sh as root. The tests must pass without admin rights."
		exit 1
	fi
}

check_compilers_installed() {
	local HX_CMD_
	for HX_CMD_ in gcc g++ clang clang++; do
		if ! command -v "$HX_CMD_" > /dev/null 2>&1; then
			echo "error: $HX_CMD_ not found. See install_*.sh."
			exit 1
		fi
	done
}

# -- Source Checks -------------------------------------------------------------

# The test directory should not use names ending with an underscore. Those names
# are reserved for internal symbols. Two underscores are allowed.
check_reserved_names() {
	if grep -nE --exclude=hxtest_main.cpp                                             \
			'(^|[^[:alnum:]_])[[:alpha:]_][[:alnum:]_]*[[:alnum:]]_([^[:alnum:]_]|$)' \
			test/*.c test/*.cpp test/*.h test/*.hpp test/*.inl >&2; then
		echo "error: Alphanumeric sequences ending with '_' are not allowed in the test directory."
		exit 1
	fi
}

# Require class and struct names in the test directory to contain "hx" and
# "test". This makes it clear in different messages whether a symbol is from the
# test suite or the library. Use a comment like "// hxtest" to disable this
# check.
check_test_class_names() {
	if grep -nP '\b(class|struct)\b' test/*.c test/*.cpp test/*.h test/*.hpp test/*.inl \
			| grep -Pv '^[^:]*:[^:]*:.*(?=.*hx)(?=.*test)' >&2; then
		echo "error: Class/struct definitions in the test directory must contain both 'hx' and 'test'."
		exit 1
	fi
}

# Some keywords are implemented when the standard library is not available.
# These are not.
check_unsupported_keywords() {
	local HX_KEYWORDS_='(^|[^[:alnum:]_])(typeid|nullptr|co_await|co_yield|co_return|throw)([^[:alnum:]_]|$)'
	if grep -nEHIR --include='*.cpp' --include='*.h' --include='*.hpp' --include='*.inl' \
			"$HX_KEYWORDS_" include src test >&2; then
		echo "error: C++ keywords must not depend on the C++ standard library."
		exit 1
	fi
}

# Require tabs size 4 not spaces.
check_tab_indentation() {
	if grep -nE '^  ' */*.c */*.cpp */*.h */*.hpp */*.inl *.ps1 */*.ps1 *.sh >&2; then
		echo "error: Use tabs size 4 not spaces."
		exit 1
	fi
}

# Check for stray CRLF.
check_line_endings() {
	if [ -d .git ] && git ls-files --eol | grep -E 'w/(crlf|mixed)'; then
		printf 'error: CRLF line ending found. Fix with: sed -i %ss/\\r//%s <file>\n' "'" "'"
		exit 1
	fi
}

check_trailing_whitespace() {
	if find . -type f "${HX_TEXT_FILES_[@]}" -exec grep -nP '[ \t]+$' {} + >&2; then
		echo "error: Trailing whitespace found."
		exit 1
	fi
}

# Check text files end with exactly one newline.
check_final_newlines() {
	find . -type f "${HX_TEXT_FILES_[@]}" | while read -r HX_FILE_; do
		if [[ $(tail -c 2 "$HX_FILE_" | LC_ALL=C tr -dc '\n' | wc -c) -ne 1 ]]; then
			echo "error: Text files must end with exactly one newline: $HX_FILE_"
			exit 1
		fi
	done
}

# Check for non-ASCII characters other than the allowed set. The escapes are
# the UTF-8 BOM.
check_non_ascii_characters() {
	local HX_NON_ASCII_ALLOW_=$'©…²₁₂≤≥🗀❌🪓\xEF\xBB\xBF'
	if find . -type f \( "${HX_TEXT_FILES_[@]}" \) -exec grep -nP "[^[:ascii:]$HX_NON_ASCII_ALLOW_]" {} + >&2; then
		echo "error: Non-ASCII characters other than '$HX_NON_ASCII_ALLOW_' found."
		exit 1
	fi
}

# Reject ASCII control characters that are illegal in C/C++ source.
check_control_characters() {
	if find . -type f "${HX_TEXT_FILES_[@]}" \
			-exec grep -nP '[\x00-\x08\x0b-\x1f\x7f]' {} + >&2; then
		echo "error: Illegal control characters found in C/C++ source."
		exit 1
	fi
}

# -- Test Scripts --------------------------------------------------------------

# Smoke test a single debug build with a usage error first.
smoke_test_debug_build() {
	local HX_OUTPUT_
	if HX_OUTPUT_=$(./debug_build.sh --run --gtest_filter=hxtest_no_such_suite.no_such_case 2>&1) \
			|| ! grep -q usage_error <<< "$HX_OUTPUT_"; then
		echo "$HX_OUTPUT_" >&2
		echo "error: debug_build.sh --gtest_filter with a bad filter should fail with usage_error."
		exit 1
	fi
}

# -- Main ----------------------------------------------------------------------

main() {
	parse_arguments "$@"
	check_not_root

	./clean.sh
	[ -d .git ] && git fsck

	[ "$(uname -s)" = Darwin ] && add_homebrew_path
	check_compilers_installed

	check_reserved_names
	check_test_class_names
	check_unsupported_keywords
	check_tab_indentation
	check_line_endings
	check_trailing_whitespace
	check_final_newlines
	check_non_ascii_characters
	check_control_characters

	smoke_test_debug_build

	PS4='\e[38;5;208m[${SECONDS}s] ${BASH_SOURCE}:${LINENO}: \e[0m'
	set -o xtrace

	./test_cmake.sh $HX_VERBOSE_
	./test_coverage.sh $HX_VERBOSE_
	./test_target.sh $HX_VERBOSE_
	./test_matrix.sh
	if command -v musl-gcc > /dev/null 2>&1; then
		./test_strip.sh $HX_VERBOSE_
	else
		echo "test_strip.sh skipped. Requires musl-gcc."
	fi
	if [ -d "$HOME/emsdk" ]; then
		./test_wasm.sh $HX_VERBOSE_
	else
		echo "test_wasm.sh skipped. Requires emsdk."
	fi

	./debug_build.sh $HX_VERBOSE_ --grind --run
	doxygen
	./clean.sh
	{ set +o xtrace; } 2> /dev/null

	echo "test_all.sh: All test scripts passed."
}

main "$@"
