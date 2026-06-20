#!/bin/sh
# SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
# SPDX-License-Identifier: MIT
# This file is licensed under the terms of the LICENSE.md file.

set -eu

LINES=200

export POSIXLY_CORRECT=1

if [ ! -f build/hxtest ]; then
	echo "build/hxtest not found!"
	return 2; # File not found.
fi

nm --radix=d --print-size build/hxtest | awk 'NF == 4 {print $2, $3, $4}' | sort -r 2>/dev/null | head -n "$LINES"   \
 | python3 -c 'import re,sys;[print(re.sub(r"\b0+(?=\d)",lambda m:" "*len(m.group()),l),end="") for l in sys.stdin]' \
 | c++filt
