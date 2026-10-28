#!/bin/bash
# SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
# SPDX-License-Identifier: MIT
# This file is licensed under the terms of the LICENSE.md file.
#
# Building and running the tests on an RPM based distribution requires these
# packages. Tested on Rocky Linux 10. EPEL and CRB are enabled for ccache,
# doxygen, gcovr, musl and ninja. meson is installed with pip.
#
# It should be safe to just run this script. Rocky Linux 10 does not package
# 32-bit glibc so the test scripts drop -m32 and build 64-bit there.
# google-chrome will be launched when a web page is generated, if available, but
# is not required or installed here. emcc is required for testwasm.sh but is not
# installed here.

trap 'trap "" INT; pkill -9 -P $$ 2>/dev/null; wait 2>/dev/null; exit 1' INT
set -euo pipefail

. /etc/os-release
case " $ID ${ID_LIKE:-} " in
	*" rhel "*|*" fedora "*|*" centos "*) ;;
	*)
		echo "error: $ID is not an RPM based distribution. See install_deb.sh." >&2
		exit 1 ;;
esac

HX_SUDO_=""
if [ "$(id -u)" -ne 0 ]; then
	HX_SUDO_="sudo"
fi

$HX_SUDO_ dnf install -y epel-release dnf-plugins-core
$HX_SUDO_ dnf config-manager --set-enabled crb

HX_CMD_=$(echo "$HX_SUDO_ dnf install -y                                    \
	ccache    clang       clang-tools-extra cmake   compiler-rt doxygen     \
	gcc       gcc-c++     gcovr             gdb     git         libasan     \
	gcc-toolset-15-gcc-c++                                                  \
	libubsan  llvm        musl-devel        musl-gcc musl-libc  ninja-build \
	procps-ng python3     python3-pip                                       \
" | tr -s '[:space:]' ' ')
echo "$HX_CMD_" | fold -s

$HX_CMD_

# The packaged meson is older than meson.build requires.
$HX_SUDO_ pip3 install --upgrade meson

for HX_CMD_ in clang cmake doxygen gcc gcovr meson musl-gcc ninja pgrep python3
do
	echo "$HX_CMD_ --version"
	$HX_CMD_ --version
done

if command -v emcc >/dev/null 2>&1; then
	emcc --version
else
	echo "error: emcc not found. It must be installed separately." >&2
fi
