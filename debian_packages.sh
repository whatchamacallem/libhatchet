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

# Prevent leaking background tasks.
trap '{ set +o xtrace; } 2> /dev/null
    trap - 1 2 3 6 15
    for pid in $(pgrep -g "$$" 2>/dev/null); do
        [ "$pid" = "$$" ] && continue
        kill -9 "$pid" 2>/dev/null
    done
    exit 1
' 1 2 3 6 15

set -euo pipefail

# bash only line art.
colors=(208 214 220 226 190 154 118 82 83 84 85 86 87 45 39 33
		27 33 39 45 87 86 85 84 83 82 118 154 190 226 220 214)
total_colors=${#colors[@]}
color_offset=0

separator() {
	for((i=40; i--;)); do
		local color_index=$(((color_offset + i) % total_colors))
		echo -ne "\033[38;5;${colors[color_index]}m--"
	done
	echo -e "\033[0m"
	((color_offset += 3))
}

separator ---------------------------------------------------------------------
CMD=$(echo "sudo apt install                                         \
	ccache        clang        clang-tidy   cmake    doxygen         \
	g++           g++-multilib gcc-multilib gcovr    gdb             \
	gdb-multiarch libc++-dev   llvm         llvm-dev musl            \
	musl-dev      musl-tools   ninja-build  procps   universal-ctags \
" | tr -s '[:space:]' ' ')
echo "$CMD" | fold -s

separator ---------------------------------------------------------------------
$CMD

separator ---------------------------------------------------------------------
for CMD in clang cmake ctags doxygen gcc gcovr musl-gcc ninja pgrep python3
do
	echo "$CMD --version"
    $CMD --version
	separator -----------------------------------------------------------------
done

echo "emcc must be installed separately:"
which emcc || echo "error: emcc not found"
