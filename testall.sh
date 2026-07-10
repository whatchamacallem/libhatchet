#!/bin/bash
# SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
# SPDX-License-Identifier: MIT
# This file is licensed under the terms of the LICENSE.md file.
#
# If build/hxtest fails, then the tests will stop at that point and it will be
# available to debug. Any shell command that fails should also stop the test
# process.

# Prevent leaking background tasks.
trap '{ set +o xtrace; } 2> /dev/null
	trap - 1 2 3 6 15
	for pid in $(pgrep -g "$$" 2>/dev/null); do
		[ "$pid" = "$$" ] && continue
		kill -9 "$pid" 2>/dev/null
	done
	exit 1
' 1 2 3 6 15

set -euo pipefail

VERBOSE=""
if [ "${1:-}" = "--verbose" ]; then
	VERBOSE="--verbose"
elif [ -n "${1:-}" ]; then
	echo "Usage: $0 [--verbose]"
	exit 1
fi

# Delete files matching .gitignore and reset ccache. Required for following
# tests to avoid matches with intermediates.
./clean.sh

# Tabs size 4 not spaces.
if grep -nE '^  ' */*.c */*.cpp */*.h */*.hpp */*.inl *.sh >&2; then
	echo "error: Lines starting with a space are not allowed in text files."
	exit 1
fi

# The test directory should not use names ending with an underscore. Those names
# are reserved for internal symbols. Two underscores are allowed.
if grep -nE --exclude=hxtest_main.cpp '(^|[^[:alnum:]_])[[:alpha:]_][[:alnum:]_]*[[:alnum:]]_([^[:alnum:]_]|$)' \
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
KEYWORDS='(^|[^[:alnum:]_])(typeid|nullptr|co_await|co_yield|co_return|throw)([^[:alnum:]_]|$)'
if grep -nEHIR --include='*.cpp' --include='*.h' --include='*.hpp' --include='*.inl' \
		"$KEYWORDS" include src test >&2; then
	echo "error: C++ keywords must not depend on the C++ standard library."
	exit 1
fi

# Check for stray CRLF.
if git ls-files -z | xargs -0 file | grep CRLF; then
	printf 'error: CRLF line ending found. Fix with: sed -i %ss/\\r//%s <file>\n' "'" "'"
	exit 1
fi

# Use an array to avoid globbing. example_correct.txt holds captured ANSI
# escapes and is exempted.
TEXT_FILES=(! -name example_correct.txt \(
	-name .clang-tidy -o -name .gdbinit -o -name .gitattributes
	-o -name .gitignore -o -name '*.bat' -o -name '*.c' -o -name '*.cpp'
	-o -name '*.h' -o -name '*.hpp' -o -name '*.inl' -o -name '*.json'
	-o -name '*.md' -o -name '*.py' -o -name '*.sh' -o -name '*.txt' \))

# Check for trailing whitespace.
if find . -type f "${TEXT_FILES[@]}" -exec grep -nP '[ \t]+$' {} + >&2; then
	echo "error: Trailing whitespace found."
	exit 1
fi

# Check for non-ASCII characters other than the allowed set. No Unicode BOM allowed.
NON_ASCII_ALLOW='©Θ…²₁₂≤≥❌📁🪓'
if find . -type f \( "${TEXT_FILES[@]}" \) -exec grep -nP "[^[:ascii:]$NON_ASCII_ALLOW]" {} + >&2; then
	echo "error: Non-ASCII characters other than '$NON_ASCII_ALLOW' found."
	exit 1
fi

# Reject ASCII control characters below 128 that are illegal in C/C++ source.
if find . -type f "${TEXT_FILES[@]}" \
		-exec grep -nP '[\x00-\x08\x0b-\x1f\x7f]' {} + >&2; then
	echo "error: Illegal control characters found in C/C++ source."
	exit 1
fi

# Check text files end with exactly one newline.
find . -type f "${TEXT_FILES[@]}" | while read -r FILE; do
	if [[ $(tail -c 2 "$FILE" | tr -dc '\n' | wc -c) -ne 1 ]]; then
		echo "error: Text files must end with exactly one newline: $FILE"
		exit 1
	fi
done

PS4='\e[38;5;208m[${SECONDS}s] ${BASH_SOURCE}:${LINENO}: \e[0m'
set -o xtrace

git fsck
./testcmake.sh $VERBOSE
./testcoverage.sh $VERBOSE
./testerrorhandling.sh
./testexample.sh $VERBOSE
./testmatrix.sh
./teststrip.sh $VERBOSE
./testwasm.sh

./debugbuild.sh $VERBOSE --grind --run
doxygen

./clean.sh

{ set +o xtrace; } 2> /dev/null
echo "testall.sh: All test scripts passed."
