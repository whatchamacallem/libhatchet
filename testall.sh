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
        kill "$pid" 2>/dev/null
    done
    exit 1
' 1 2 3 6 15

set -euo pipefail

HEADLESS=""
if [ "${1:-}" = "--headless" ]; then
	HEADLESS="--headless"
fi

if [ -z "$HEADLESS" ]; then
	clear

	echo "
  '||   ||  '||      '||                .           '||                .
   ||  ...   || ...   || ..    ....   .||.    ....   || ..     ....  .||.
   ||   ||   ||'  ||  ||' ||  '' .||   ||   .|   ''  ||' ||  .|...||  ||
   ||   ||   ||    |  ||  ||  .|' ||   ||   ||       ||  ||  ||       ||
  .||. .||.  '|...'  .||. ||. '|..'|'  '|.'  '|...' .||. ||.  '|...'  '|.'
"
fi

# Delete files matching .gitignore and reset ccache.
./clean.sh

# The test directory should not use names ending with an underscore. Those names
# are reserved for internal symbols. Two underscores are allowed. Coverage
# testing should be possible without using internal symbols. Symbols ending with
# an underscore are not intended to be used externally and may change without
# notice.
if grep -nE '(^|[^[:alnum:]_])[[:alpha:]_][[:alnum:]_]*[[:alnum:]]_([^[:alnum:]_]|$)' test/*.cpp >&2; then
	echo "error: Alphanumeric sequences ending with '_' are not allowed in test/*.cpp."
	exit 1
fi

# Require class and struct names in the test directory to contain "hx" and
# "test". This makes it clear in different messages whether a symbol is from the
# test suite or the library. Use a comment like "// hxtest" to disable this
# check.
if grep -nP '\b(class|struct)\b' test/*.cpp | grep -Pv '^[^:]*:[^:]*:.*(?=.*hx)(?=.*test)' >&2; then
	echo "error: Class/struct definitions in test/*.cpp must contain both 'hx' and 'test'."
	exit 1
fi

# Some keywords are implemented when the standard library is not available.
# These are not.
KEYWORDS='(^|[^[:alnum:]_])(typeid|nullptr|co_await|co_yield|co_return|throw)([^[:alnum:]_]|$)'
if grep -nEHIR --include='*.cpp' --include='*.hpp' "$KEYWORDS" include src test >&2; then
	echo "error: C++ keywords must not depend on the C++ standard library."
	exit 1
fi

# Check for stray CRLF.
if file $(git ls-files) | grep CRLF; then
	printf 'error: CRLF line ending found. Fix with: sed -i %ss/\\r//%s <file>\n' "'" "'"
	exit 1
fi

# Check for trailing whitespace.
if grep -rnP '[ \t]+$' --include='*.h' --include='*.hpp' --include='*.inl' --include='*.c' \
		--include='*.cpp' --include='*.md' --include='*.sh' . >&2; then
	echo "error: Trailing whitespace found."
	exit 1
fi

# Check text files end with exactly one newline.
find . -type f \( -name "*.hpp" -o -name "*.cpp" -o -name "*.h" -o -name "*.c" -o -name "*.inl" \
				-o -name "*.sh" -o -name "*.py" \) | while read -r FILE; do
	if [[ $(tail -c 2 "$FILE" | tr -dc '\n' | wc -c) -ne 1 ]]; then
		echo "error: Text files must end with exactly one newline: $FILE"
		exit 1
	fi
done

PS4='\e[38;5;208m[${SECONDS}s] ${BASH_SOURCE}:${LINENO}: \e[0m'
set -o xtrace

./testcmake.sh $HEADLESS
./testcoverage.sh $HEADLESS
./testerrorhandling.sh
./testexample.sh $HEADLESS
./testmatrix.sh
./teststrip.sh $HEADLESS
./testwasm.sh --headless
./debugbuild.sh --grind --run
doxygen

./clean.sh

{ set +o xtrace; } 2> /dev/null
echo "testall.sh: All test scripts passed."
