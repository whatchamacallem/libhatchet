#!/bin/bash
# SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
# SPDX-License-Identifier: MIT
# This file is licensed under the terms of the LICENSE.md file.
#
# Building and running all the tests on a Debian derivative requires these
# packages.
#
# usage: install_deb.sh [--sdks]
#
# It should be safe to just run this script. --sdks also installs the emsdk,
# which test_wasm.sh requires, and the Pico SDK with picotool, which the target
# directory requires. They go in ~/emsdk and ~/pico-sdk.

trap 'trap "" INT PIPE TERM; pkill -P $$; wait; exit 1' INT PIPE TERM
set -euo pipefail

export DEBIAN_FRONTEND=noninteractive

# -- Packages ------------------------------------------------------------------
HX_PACKAGES_="
	ccache       clang        clang-tidy      cmake    doxygen    g++
	g++-multilib gcc-multilib gcovr           gdb      git        llvm
	make         meson        musl            musl-dev musl-tools ninja-build
	procps       python3      libclang-rt-dev"

# --sdks:
HX_SDK_PACKAGES_="
	ca-certificates                gcc-arm-none-eabi libnewlib-arm-none-eabi
	libstdc++-arm-none-eabi-newlib libusb-1.0-0-dev  pkg-config
	xz-utils"

HX_EMSDK_="$HOME/emsdk"

HX_PICO_SDK_="$HOME/pico-sdk"
HX_PICO_SDK_VERSION_=2.3.1
HX_PICOTOOL_DIR_="$HOME/picotool"

# -- Line Art ------------------------------------------------------------------
HX_COLORS_=(208 214 220 226 190 154 118 82 83 84 85 86 87 45 39 33
			27 33 39 45 87 86 85 84 83 82 118 154 190 226 220 214)
HX_TOTAL_COLORS_=${#HX_COLORS_[@]}
HX_COLOR_OFFSET_=0

line() {
	for((HX_I_=40; HX_I_--;)); do
		local HX_COLOR_INDEX_=$(((HX_COLOR_OFFSET_ + HX_I_) % HX_TOTAL_COLORS_))
		echo -ne "\033[38;5;${HX_COLORS_[HX_COLOR_INDEX_]}m--"
	done
	echo -e "\033[0m"
	((HX_COLOR_OFFSET_ += 3))
}

# -- Check Packages Already Installed ------------------------------------------

HX_MISSING_=0
if [ "${1:-}" = "--sdks" ]; then
	HX_PACKAGES_="$HX_PACKAGES_ $HX_SDK_PACKAGES_"
	[ -f "$HX_EMSDK_/.emscripten" ] || HX_MISSING_=1
	[ -x "$HX_PICOTOOL_DIR_/picotool" ] || HX_MISSING_=1
fi
grep -qv '^ii' < <(dpkg-query -W -f='${db:Status-Abbrev}\n' \
	$HX_PACKAGES_ 2>&1) && HX_MISSING_=1
if [ "$HX_MISSING_" = "0" ]; then
	echo "packages_already_installed"
	exit 0
fi

# -- Debian Shared -------------------------------------------------------------

line
HX_SUDO_=""
if [ "$(id -u)" -ne 0 ]; then
	HX_SUDO_="sudo"
fi

$HX_SUDO_ apt-get update

line
HX_CMD_=$(echo "$HX_SUDO_ apt-get install -y --no-install-recommends \
	--no-upgrade $HX_PACKAGES_" | tr -s '[:space:]' ' ')
echo "$HX_CMD_" | fold -s

line
$HX_CMD_

for HX_CMD_ in clang cmake doxygen gcc gcovr meson musl-gcc ninja pgrep python3
do
	line
	echo "$HX_CMD_ --version"
	$HX_CMD_ --version
done

if [ ! "${1:-}" = "--sdks" ]; then
	echo "warning: Some SDKs omitted as --sdks was not used."
	exit 0
fi

# -- Emscripten ----------------------------------------------------------------

if [ ! -f "$HX_EMSDK_/.emscripten" ]; then
	line
	git clone --depth 1 https://github.com/emscripten-core/emsdk.git "$HX_EMSDK_"
	"$HX_EMSDK_/emsdk" install latest
	"$HX_EMSDK_/emsdk" activate latest
	EMSDK_QUIET=1 . "$HX_EMSDK_/emsdk_env.sh"
	emcc --version
fi

# -- Pico ----------------------------------------------------------------------

if [ ! -x "$HX_PICOTOOL_DIR_/picotool" ]; then
	line
	[ -d "$HX_PICO_SDK_" ] || git -c advice.detachedHead=false clone \
		--depth 1 --branch "$HX_PICO_SDK_VERSION_"                   \
		--recurse-submodules --shallow-submodules                    \
		https://github.com/raspberrypi/pico-sdk.git "$HX_PICO_SDK_"
	HX_TMP_DIR_=$(mktemp -d)
	git -c advice.detachedHead=false clone --depth 1                        \
		--branch "$HX_PICO_SDK_VERSION_"                                    \
		https://github.com/raspberrypi/picotool.git "$HX_TMP_DIR_/picotool"
	cmake -S "$HX_TMP_DIR_/picotool" -B "$HX_TMP_DIR_/build" -G Ninja \
		-DCMAKE_BUILD_TYPE=Release -DPICO_SDK_PATH="$HX_PICO_SDK_"    \
		-DPICOTOOL_FLAT_INSTALL=1                                     \
		-DCMAKE_INSTALL_PREFIX="$(dirname "$HX_PICOTOOL_DIR_")"
	cmake --build "$HX_TMP_DIR_/build"
	cmake --install "$HX_TMP_DIR_/build"
	rm -rf "$HX_TMP_DIR_"
fi

# ------------------------------------------------------------------------------
echo 🪓🪓🪓
