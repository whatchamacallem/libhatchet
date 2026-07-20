#!/bin/sh
# SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
# SPDX-License-Identifier: MIT
# This file is licensed under the terms of the LICENSE.md file.

trap 'trap "" INT; pkill -9 -P $$ 2>/dev/null; wait 2>/dev/null; exit 1' INT
set -eu

# Test disabling a few other things as well.
echo "WARNING: These tests will spew errors and still return successfully."
./testmatrix.sh -DHX_TEST_ERROR_HANDLING=1 -DHX_USE_CONSOLE=0 \
	-DHX_USE_LOGGING=0 -DHX_MEMORY_BUDGET_PERMANENT=0         \
	'-DHX_USE_MEMORY_MANAGER=(HX_HARDENING_MODE!=HX_HARDENING_MODE_STANDARD)'
