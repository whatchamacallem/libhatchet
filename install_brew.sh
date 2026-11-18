#!/bin/bash
# SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
# SPDX-License-Identifier: MIT
# This file is licensed under the terms of the LICENSE.md file.
#
# Building and running the tests on macOS requires these Homebrew packages.
# Network access is only required when packages are missing.

trap 'trap "" INT PIPE TERM; pkill -P $$; wait; exit 1' INT PIPE TERM
set -euo pipefail

PATH="/opt/homebrew/bin:/opt/homebrew/sbin${PATH+:$PATH}"

HX_PACKAGES_="ccache cmake doxygen gcc gcovr git grep llvm meson ninja python3"

check_brew() {
	if ! command -v brew >/dev/null 2>&1; then
		echo "error: brew not found on PATH. See https://brew.sh" >&2
		exit 1
	fi
	brew list --formula $HX_PACKAGES_ >/dev/null 2>&1
}

main() {
	if check_brew; then
		echo "packages_already_installed"
		return
	fi

	brew update
	HOMEBREW_NO_INSTALL_UPGRADE=1 brew install $HX_PACKAGES_

	echo 🪓🪓🪓
}

main "$@"
