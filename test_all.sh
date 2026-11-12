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

HX_VERBOSE_=""
if [ "${1:-}" = "--verbose" ]; then
	HX_VERBOSE_="--verbose"
elif [ -n "${1:-}" ]; then
	echo "usage_error: $0 [--verbose]"
	exit 1
fi

# BSD grep does not support -P. Run install_brew.sh.
if [ "$(uname -s)" = Darwin ]; then
	PATH="/opt/homebrew/bin:/opt/homebrew/sbin${PATH+:$PATH}"
	if ! command -v brew > /dev/null 2>&1; then
		echo "error: Install brew and then run install_brew.sh."
		exit 1
	fi
	export PATH="$(brew --prefix grep)/libexec/gnubin:$PATH"
fi

# Delete files matching .gitignore and reset ccache. Required for following
# tests to avoid matches with intermediates.
./clean.sh

# The test directory should not use names ending with an underscore. Those names
# are reserved for internal symbols. Two underscores are allowed.
if grep -nE --exclude=hxtest_main.cpp                                             \
		'(^|[^[:alnum:]_])[[:alpha:]_][[:alnum:]_]*[[:alnum:]]_([^[:alnum:]_]|$)' \
		test/*.c test/*.cpp test/*.h test/*.hpp test/*.inl >&2; then
	echo "error: Alphanumeric sequences ending with '_' are not allowed in the test directory."
	exit 1
fi

# Require class and struct names in the test directory to contain "hx" and
# "test". This makes it clear in different messages whether a symbol is from the
# test suite or the library. Use a comment like "// hxtest" to disable this
# check.
if grep -nP '\b(class|struct)\b' test/*.c test/*.cpp test/*.h test/*.hpp test/*.inl \
		| grep -Pv '^[^:]*:[^:]*:.*(?=.*hx)(?=.*test)' >&2; then
	echo "error: Class/struct definitions in the test directory must contain both 'hx' and 'test'."
	exit 1
fi

# Some keywords are implemented when the standard library is not available.
# These are not.
HX_KEYWORDS_='(^|[^[:alnum:]_])(typeid|nullptr|co_await|co_yield|co_return|throw)([^[:alnum:]_]|$)'
if grep -nEHIR --include='*.cpp' --include='*.h' --include='*.hpp' --include='*.inl' \
		"$HX_KEYWORDS_" include src test >&2; then
	echo "error: C++ keywords must not depend on the C++ standard library."
	exit 1
fi

# Require tabs size 4 not spaces.
if grep -nE '^  ' */*.c */*.cpp */*.h */*.hpp */*.inl *.ps1 */*.ps1 *.sh >&2; then
	echo "error: Use tabs size 4 not spaces."
	exit 1
fi

# Check for stray CRLF.
if [ -d .git ] && git ls-files --eol | grep -E 'w/(crlf|mixed)'; then
	printf 'error: CRLF line ending found. Fix with: sed -i %ss/\\r//%s <file>\n' "'" "'"
	exit 1
fi

# Use an array to avoid globbing.
HX_TEXT_FILES_=(\(
	-name .clang-tidy -o -name .gdbinit -o -name .gitattributes
	-o -name .gitignore -o -name '*.bat' -o -name '*.c' -o -name '*.cpp'
	-o -name '*.h' -o -name '*.hpp' -o -name '*.inl' -o -name '*.json'
	-o -name '*.md' -o -name '*.ps1' -o -name '*.py' -o -name '*.sh'
	-o -name '*.txt' \))

# Check for trailing whitespace.
if find . -type f "${HX_TEXT_FILES_[@]}" -exec grep -nP '[ \t]+$' {} + >&2; then
	echo "error: Trailing whitespace found."
	exit 1
fi

# Check text files end with exactly one newline.
find . -type f "${HX_TEXT_FILES_[@]}" | while read -r HX_FILE_; do
	if [[ $(tail -c 2 "$HX_FILE_" | LC_ALL=C tr -dc '\n' | wc -c) -ne 1 ]]; then
		echo "error: Text files must end with exactly one newline: $HX_FILE_"
		exit 1
	fi
done

# Check for non-ASCII characters other than the allowed set.
HX_NON_ASCII_ALLOW_='©…²₁₂≤≥🗀❌🪓'
if find . -type f \( "${HX_TEXT_FILES_[@]}" \) -exec grep -nP "[^[:ascii:]$HX_NON_ASCII_ALLOW_]" {} + >&2; then
	echo "error: Non-ASCII characters other than '$HX_NON_ASCII_ALLOW_' found."
	exit 1
fi

# Reject ASCII control characters that are illegal in C/C++ source.
if find . -type f "${HX_TEXT_FILES_[@]}" \
		-exec grep -nP '[\x00-\x08\x0b-\x1f\x7f]' {} + >&2; then
	echo "error: Illegal control characters found in C/C++ source."
	exit 1
fi

# Smoke test a single debug build with a usage error first.
if HX_OUTPUT_=$(./debug_build.sh --run --gtest_filter=hxtest_no_such_suite.no_such_case 2>&1) \
		|| ! grep -q usage_error <<< "$HX_OUTPUT_"; then
	echo "error: debug_build.sh --gtest_filter with a bad filter should fail with usage_error."
	exit 1
fi

[ -d .git ] && git fsck

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
