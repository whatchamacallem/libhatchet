#!/bin/sh
# SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
# SPDX-License-Identifier: MIT
# This file is licensed under the terms of the LICENSE.md file.

# Nukes the files in .gitignore.
git clean -Xdf

# ccache is only for debugbuild.sh. Clearing it prevents stale binaries. It is
# also worthwhile resetting the cache stats to keep them from getting stale.
if command -v ccache > /dev/null 2>&1; then
	ccache --clear --zero-stats
fi
