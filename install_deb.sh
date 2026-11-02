#!/bin/bash
# SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
# SPDX-License-Identifier: MIT
# This file is licensed under the terms of the LICENSE.md file.
#
# Building and running all the tests on a Debian derivative requires these
# packages.
#
# It should be safe to just run this script. google-chrome will be launched
# when a web page is generated, if available, but is not required or installed
# here. emcc is required for test_wasm.sh but is not installed here.

HX_PACKAGES_="
	ccache       clang        clang-tidy      cmake    doxygen    g++
	g++-multilib gcc-multilib gcovr           gdb      git        llvm
	make         meson        musl            musl-dev musl-tools ninja-build
	procps       python3      libclang-rt-dev
"

trap 'trap "" INT PIPE TERM; pkill -P $$; wait; exit 1' INT PIPE TERM
set -euo pipefail

export DEBIAN_FRONTEND=noninteractive

# bash only line art.
HX_COLORS_=(208 214 220 226 190 154 118 82 83 84 85 86 87 45 39 33
			27 33 39 45 87 86 85 84 83 82 118 154 190 226 220 214)
HX_TOTAL_COLORS_=${#HX_COLORS_[@]}
HX_COLOR_OFFSET_=0

separator() {
	for((HX_I_=40; HX_I_--;)); do
		local HX_COLOR_INDEX_=$(((HX_COLOR_OFFSET_ + HX_I_) % HX_TOTAL_COLORS_))
		echo -ne "\033[38;5;${HX_COLORS_[HX_COLOR_INDEX_]}m--"
	done
	echo -e "\033[0m"
	((HX_COLOR_OFFSET_ += 3))
}

if command -v emcc >/dev/null 2>&1; then
	emcc --version
else
	echo "error: emcc not found. It must be installed separately." >&2
fi

HX_STATUS_=$(dpkg-query -W -f='${db:Status-Abbrev}\n' $HX_PACKAGES_ 2>&1 || true)
if ! grep -qv '^ii' <<<"$HX_STATUS_"; then
	echo "packages_already_installed"
	exit 0
fi

HX_SUDO_=""
if [ "$(id -u)" -ne 0 ]; then
	HX_SUDO_="sudo"
fi

separator ---------------------------------------------------------------------
$HX_SUDO_ apt-get update

separator ---------------------------------------------------------------------
HX_CMD_=$(echo "$HX_SUDO_ apt-get install -y --no-install-recommends \
	--no-upgrade $HX_PACKAGES_" | tr -s '[:space:]' ' ')
echo "$HX_CMD_" | fold -s

separator ---------------------------------------------------------------------
$HX_CMD_

separator ---------------------------------------------------------------------
for HX_CMD_ in clang cmake doxygen gcc gcovr meson musl-gcc ninja pgrep python3
do
	echo "$HX_CMD_ --version"
	$HX_CMD_ --version
	separator -----------------------------------------------------------------
done
