#!/bin/bash
# SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
# SPDX-License-Identifier: MIT
# This file is licensed under the terms of the LICENSE.md file.
#
# Building and running the tests on an RPM based distribution requires these
# packages. Tested on Rocky Linux 10. EPEL and CRB are enabled for ccache,
# doxygen, gcovr, meson, musl and ninja.
#
# It should be safe to just run this script. Rocky Linux 10 does not package
# 32-bit glibc so the test scripts drop -m32 and build 64-bit there.
# google-chrome will be launched when a web page is generated, if available,
# but is not required or installed here. emcc is required for testwasm.sh but
# is not installed here.

trap 'trap "" INT PIPE TERM; pkill -P $$; wait; exit 1' INT PIPE TERM
set -euo pipefail

HX_PACKAGES_="
	ccache                  clang       clang-tools-extra   cmake
	compiler-rt             doxygen     gcc                 gcc-c++
	gcc-toolset-15-gcc-c++  gcovr       gdb                 git
	libasan                 libubsan    llvm                meson
	musl-devel              musl-gcc    musl-libc           musl-libc-static
	ninja-build             procps-ng   python3
"

if command -v emcc >/dev/null 2>&1; then
	emcc --version
else
	echo "error: emcc not found. It must be installed separately." >&2
fi

# Network access is only required when packages are missing.
if rpm -q $HX_PACKAGES_ >/dev/null; then
	echo "packages_already_installed"
	exit 0
fi

HX_SUDO_=""
if [ "$(id -u)" -ne 0 ]; then
	HX_SUDO_="sudo"
fi

# Weak dependencies are not required and metadata is always refreshed.
HX_DNF_OPTIONS_="-y --refresh --setopt=install_weak_deps=False"

$HX_SUDO_ dnf install $HX_DNF_OPTIONS_ epel-release dnf-plugins-core
$HX_SUDO_ dnf config-manager --set-enabled crb

HX_CMD_=$(echo "$HX_SUDO_ dnf install $HX_DNF_OPTIONS_ $HX_PACKAGES_" \
	| tr -s '[:space:]' ' ')
echo "$HX_CMD_" | fold -s

$HX_CMD_

for HX_CMD_ in clang cmake doxygen gcc gcovr meson musl-gcc ninja pgrep python3
do
	echo "$HX_CMD_ --version"
	$HX_CMD_ --version
done
