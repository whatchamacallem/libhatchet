#!/bin/sh
# SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
# SPDX-License-Identifier: MIT
# This file is licensed under the terms of the LICENSE.md file.

trap 'trap "" INT; pkill -9 -P $$ 2>/dev/null; wait 2>/dev/null; exit 1' INT
set -eu

if ! ./debugbuild.sh --run --gtest_filter=hxtest_no_such_suite.no_such_case 2>&1 \
		| grep -q usage_error; then
	echo "error: debugbuild.sh --gtest_filter with a bad filter should mention usage_error."
	exit 1
fi

if ! ./debugbuild.sh -DHX_USE_SLAB_ALLOCATOR= 2>&1 | grep -q bad_define; then
	echo "error: debugbuild.sh with an empty define should mention bad_define."
	exit 1
fi

# Test disabling a few other things as well.
echo "WARNING: These tests will spew errors and still return successfully."
./testmatrix.sh -DHX_TEST_ERROR_HANDLING=1 -DHX_USE_CONSOLE=0 \
	-DHX_USE_LOGGING=0 -DHX_SLAB_BUDGET_PERMANENT=0         \
	'-DHX_USE_SLAB_ALLOCATOR=(HX_HARDENING_MODE!=HX_HARDENING_MODE_STANDARD)'
