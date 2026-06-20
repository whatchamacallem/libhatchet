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

set -o errexit

# Test configuration options in ways that are not varied by the release level
# during normal testing.
echo "NOTA BENE: These tests will spew errors and still return successfully."
./testmatrix.sh \
	'-DHX_MEMORY_MANAGER_DISABLE=(HX_RELEASE==2)' \
	'-DHX_TEST_ERROR_HANDLING=1'
