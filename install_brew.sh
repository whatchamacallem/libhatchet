#!/bin/bash
# SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
# SPDX-License-Identifier: MIT
# This file is licensed under the terms of the LICENSE.md file.
#
# Building and running the tests on macOS requires these Homebrew packages.

HX_PACKAGES_="
	ccache cmake doxygen gcc gcovr git grep llvm meson ninja python3
"

trap 'trap "" INT PIPE TERM; pkill -P $$; wait; exit 1' INT PIPE TERM
set -euo pipefail

PATH="/opt/homebrew/bin:/opt/homebrew/sbin${PATH+:$PATH}"

if ! command -v brew >/dev/null 2>&1; then
	echo "error: brew not found. Install Homebrew from https://brew.sh first." >&2
	exit 1
fi

if command -v emcc >/dev/null 2>&1; then
	emcc --version
else
	echo "error: emcc not found. It must be installed separately." >&2
fi

# Network access is only required when packages are missing.
if brew list --formula $HX_PACKAGES_ >/dev/null 2>&1; then
	echo "packages_already_installed"
	exit 0
fi

brew update
HOMEBREW_NO_INSTALL_UPGRADE=1 brew install $HX_PACKAGES_
