#!/bin/sh
# SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
# SPDX-License-Identifier: MIT
# This file is licensed under the terms of the LICENSE.md file.

# Prevent leaking background tasks.
trap '{ set +o xtrace; } 2> /dev/null
    trap - 1 2 3 6 15
    for pid in $(pgrep -g "$$" 2>/dev/null); do
        [ "$pid" = "$$" ] && continue
        kill "$pid" 2>/dev/null
    done
    exit 1
' 1 2 3 6 15

set -eu

# Tests additional configuration options as well.
echo "WARNING: These tests will spew errors and still return successfully."
./testmatrix.sh -DHX_TEST_ERROR_HANDLING=1 -DHX_USE_THREADS=0 \
	'-DHX_USE_MEMORY_MANAGER=(HX_HARDENING_MODE!=HX_HARDENING_MODE_STANDARD)'
