#!/bin/sh
# SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
# SPDX-License-Identifier: MIT
# This file is licensed under the terms of the LICENSE.md file.
#
# Builds the example with Meson and Ninja using clang and gcc, with and without
# the hx module, and runs its tests. The example reports them with the Test
# Anything Protocol (TAP) and the Python below consumes the report.

trap 'trap "" INT PIPE TERM; pkill -P $$; wait; exit 1' INT PIPE TERM
set -eu

cd "$(dirname "$(readlink -f "$0")")"

export POSIXLY_CORRECT=1

HX_JOBS_="${HX_MAX_CORES:-$(getconf _NPROCESSORS_ONLN)}"

HX_VERBOSE_=""
if [ "${1:-}" = "--verbose" ]; then
	HX_VERBOSE_="--verbose"
elif [ -n "${1:-}" ]; then
	echo "usage_error: $0 [--verbose]"
	exit 1
fi

HX_CLANG_NATIVE_=meson/clang.ini
HX_GCC_NATIVE_=meson/gcc.ini
if [ "$(uname -s)" = Darwin ]; then
	PATH="/opt/homebrew/bin:/opt/homebrew/sbin${PATH+:$PATH}"
	if ! command -v brew > /dev/null 2>&1; then
		echo "error: Install brew and then run install_brew.sh."
		exit 1
	fi
	HX_CLANG_NATIVE_=meson/macos-clang.ini
	HX_GCC_NATIVE_=meson/macos-gcc.ini
elif [ -x /opt/rh/gcc-toolset-15/root/usr/bin/g++ ]; then
	HX_GCC_NATIVE_=meson/gcc-toolset-15.ini
fi

# Builds with the C library headers only and no C++ standard library.
HX_C_FLAGS_="-DHX_HARDENING_MODE=HX_HARDENING_MODE_DEBUG -ggdb3"
HX_CPP_FLAGS_="$HX_C_FLAGS_ -DHX_USE_STD_LIB=0 -DHX_USE_NAMESPACE=hx -DHX_USE_CONSOLE=2 \
	-std=c++23 -nostdinc++ -fno-exceptions -fno-rtti"
HX_LINK_FLAGS_="-nodefaultlibs -lc -lpthread -lm"

run_target() {
	# Build artifacts are not retained.
	rm -rf "$(readlink -f build)" build; mkdir build
	if [ -n "$HX_VERBOSE_" ]; then
		meson setup build --buildtype=debug -Dc_args="$HX_C_FLAGS_"       \
			-Dcpp_args="$HX_CPP_FLAGS_" -Dcpp_link_args="$HX_LINK_FLAGS_" \
			-Dbuild_example=true "$@"
		ninja -v -C build -j "$HX_JOBS_"
	else
		if ! meson setup build --buildtype=debug -Dc_args="$HX_C_FLAGS_"      \
				-Dcpp_args="$HX_CPP_FLAGS_" -Dcpp_link_args="$HX_LINK_FLAGS_" \
				-Dbuild_example=true "$@" > build/meson_setup_out.txt 2>&1; then
			cat build/meson_setup_out.txt
			echo "error: meson setup failed"
			exit 1
		fi
		if ! ninja -C build -j "$HX_JOBS_" >build/ninja_out.txt 2>&1; then
			cat build/ninja_out.txt
			echo "error: ninja build failed"
			exit 1
		fi
	fi

	# The example reads example.cfg from the working directory. The link lets
	# the .vscode configuration debug it as build/hxtest.
	cp example/example.cfg build
	ln -sf example/hxexample build/hxtest

	# "hxexample runtests" is the TAP producer and Python is the consumer. It
	# reads the report from a pipe and requires every planned test to pass.
	# --verbose shows the report, which is also shown when it fails.
	(cd build && exec python3 - "$HX_VERBOSE_" ./hxtest runtests) <<'HX_EOF_'
import re, subprocess, sys
verbose = sys.argv[1] == "--verbose"
producer = subprocess.Popen(sys.argv[2:], stdin=subprocess.DEVNULL,
	stdout=subprocess.PIPE, text=True)
report, plan, tests, failures = [], None, 0, []
for line in producer.stdout:
	report.append(line)
	if verbose:
		sys.stdout.write(line)
	test = re.match(r"(not )?ok\b(.*)", line)
	if test:
		tests += 1
		if test.group(1) and not re.search(r"#\s*TODO\b", test.group(2), re.I):
			failures.append(line)
	elif line.startswith("1.."):
		plan = int(line[3:].split()[0])
	elif line.startswith("Bail out!"):
		failures.append(line)
status = producer.wait()
if plan != tests or tests == 0 or failures or status != 0:
	if not verbose:
		sys.stdout.writelines(report)
	sys.exit("error: planned %s tests, ran %d, failed %d, exit status %d"
		% (plan, tests, len(failures), status))
print("passed %d tests" % tests)
HX_EOF_
}

if [ -n "$HX_VERBOSE_" ]; then
	set -o xtrace
fi

echo "$HX_CLANG_NATIVE_ meson+ninja with textual includes..."
run_target --native-file="$HX_CLANG_NATIVE_" -Dbuild_module=disabled

echo "$HX_CLANG_NATIVE_ meson+ninja with named module..."
run_target --native-file="$HX_CLANG_NATIVE_"

echo "$HX_GCC_NATIVE_ meson+ninja with textual includes..."
run_target --native-file="$HX_GCC_NATIVE_" -Dbuild_module=disabled

# Textual includes are built with GCC 14, e.g. Debian 13.
echo "$HX_GCC_NATIVE_ meson+ninja with named module..."
run_target --native-file="$HX_GCC_NATIVE_"

echo 🪓🪓🪓
