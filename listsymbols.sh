#!/bin/sh
# SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
# SPDX-License-Identifier: MIT
# This file is licensed under the terms of the LICENSE.md file.

export POSIXLY_CORRECT=1

trap 'trap "" INT; pkill -9 -P $$ 2>/dev/null; wait 2>/dev/null; exit 1' INT
set -eu

if [ ! -f build/hxtest ]; then
	echo "build/hxtest not found!"
	exit 2; # File not found.
fi

# Capture output separately so a failure is not hidden by the pipeline.
HX_SYMBOLS_=$(nm --radix=d --print-size build/hxtest)

echo
echo "=========================================================================="
echo "= Largest elf symbols..."

# Duplicate names for the same function are ignored. The C++ ABI requires them.
HX_LINES_=200
echo "$HX_SYMBOLS_" | awk 'NF == 4 && !seen[$1]++ {print $2, $3, $4}' | sort -r 2>/dev/null | head -n "$HX_LINES_"      \
	| python3 -c 'import re,sys;[print(re.sub(r"\b0+(?=\d)",lambda m:" "*len(m.group()),l),end="") for l in sys.stdin]' \
	| c++filt

echo
echo "=========================================================================="
echo "= Non-test libhatchet symbols.."

echo "$HX_SYMBOLS_" | awk 'NF == 4 && $4 ~ /hx/ && $4 !~ /test/ && !seen[$1]++ {print $2, $3, $4}' | sort -r 2>/dev/null \
	| python3 -c 'import re,sys;[print(re.sub(r"\b0+(?=\d)",lambda m:" "*len(m.group()),l),end="") for l in sys.stdin]'  \
	| c++filt
