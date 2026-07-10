#!/bin/sh
# SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
# SPDX-License-Identifier: MIT
# This file is licensed under the terms of the LICENSE.md file.

set -eu

export POSIXLY_CORRECT=1

if [ ! -f build/hxtest ]; then
	echo "build/hxtest not found!"
	exit 2; # File not found.
fi

# Capture output separately so a failure is not hidden by the pipeline.
SYMBOLS=$(nm --radix=d --print-size build/hxtest)

echo
echo "=========================================================================="
echo "= Largest elf symbols..."

# Duplicate names for the same function are ignored. The C++ ABI requires them.
LINES=200
echo "$SYMBOLS" | awk 'NF == 4 && !seen[$1]++ {print $2, $3, $4}' | sort -r 2>/dev/null | head -n "$LINES"              \
	| python3 -c 'import re,sys;[print(re.sub(r"\b0+(?=\d)",lambda m:" "*len(m.group()),l),end="") for l in sys.stdin]' \
	| c++filt

echo
echo "=========================================================================="
echo "= Non-test libhatchet symbols.."

echo "$SYMBOLS" | awk 'NF == 4 && $4 ~ /hx/ && $4 !~ /test/ && !seen[$1]++ {print $2, $3, $4}' | sort -r 2>/dev/null    \
	| python3 -c 'import re,sys;[print(re.sub(r"\b0+(?=\d)",lambda m:" "*len(m.group()),l),end="") for l in sys.stdin]' \
	| c++filt
