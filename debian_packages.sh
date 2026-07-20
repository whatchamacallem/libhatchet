#!/bin/bash
# SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
# SPDX-License-Identifier: MIT
# This file is licensed under the terms of the LICENSE.md file.
#
# Building and running all the tests on Ubuntu 24.04 LTS requires these packages.
#
# It should be safe to just run this script on Ubuntu. gdb-multiarch is now
# required for GDB to set 32-bit breakpoints reliably. google-chrome will be
# launched when a web page is generated, if available, but is not required or
# installed here. emcc is required for testwasm.sh but is not installed here.

trap 'trap "" INT; pkill -9 -P $$ 2>/dev/null; wait 2>/dev/null; exit 1' INT
set -euo pipefail

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

separator ---------------------------------------------------------------------
HX_CMD_=$(echo "sudo apt install                                     \
	ccache        clang        clang-tidy   cmake    doxygen         \
	g++           g++-multilib gcc-multilib gcovr    gdb             \
	gdb-multiarch libc++-dev   llvm         llvm-dev musl            \
	musl-dev      musl-tools   ninja-build  procps   universal-ctags \
" | tr -s '[:space:]' ' ')
echo "$HX_CMD_" | fold -s

separator ---------------------------------------------------------------------
$HX_CMD_

separator ---------------------------------------------------------------------
for HX_CMD_ in clang cmake ctags doxygen gcc gcovr musl-gcc ninja pgrep python3
do
	echo "$HX_CMD_ --version"
	$HX_CMD_ --version
	separator -----------------------------------------------------------------
done

if command -v emcc >/dev/null 2>&1; then
	emcc --version
else
	echo "error: emcc not found. It must be installed separately." >&2
fi
