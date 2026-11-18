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
# but is not required or installed here. emcc is required for test_wasm.sh but
# is not installed here. Network access is only required when packages are
# missing.

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

HX_DNF_OPTIONS_="-y --refresh --setopt=install_weak_deps=False"
HX_SUDO_=""

install_packages() {
	local HX_CMD_
	HX_CMD_=$(echo "$HX_SUDO_ dnf install $HX_DNF_OPTIONS_ $HX_PACKAGES_" \
		| tr -s '[:space:]' ' ')
	echo "$HX_CMD_" | fold -s

	$HX_CMD_
}

print_versions() {
	local HX_CMD_
	for HX_CMD_ in clang cmake doxygen gcc gcovr meson musl-gcc ninja pgrep \
		python3
	do
		echo "$HX_CMD_ --version"
		$HX_CMD_ --version
	done
}

main() {
	if rpm -q $HX_PACKAGES_ >/dev/null; then
		echo "packages_already_installed"
		return
	fi

	[ "$(id -u)" -ne 0 ] && HX_SUDO_="sudo"
	$HX_SUDO_ dnf install $HX_DNF_OPTIONS_ epel-release dnf-plugins-core
	$HX_SUDO_ dnf config-manager --set-enabled crb
	install_packages
	print_versions

	echo 🪓🪓🪓
}

main "$@"
