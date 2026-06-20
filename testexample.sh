#!/bin/sh
# SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
# SPDX-License-Identifier: MIT
# This file is licensed under the terms of the LICENSE.md file.

# Prevent leaking background tasks.
trap '{ set +o xtrace; } 2> /dev/null
	trap - 1 2 3 6 15
	for pid in $(pgrep -g "$$" 2>/dev/null); do
		[ "$pid" = "$$" ] && continue
		kill "$pid" 2>/dev/null
	done
	exit 1
' 1 2 3 6 15

set -eu

export POSIXLY_CORRECT=1

BUILD_FLAGS="-DHX_HARDENING_MODE=HX_HARDENING_MODE_DEBUG -m32 -ggdb3"
LINK_FLAGS="-m32 -lstdc++ -lm"

HX_DIR=`pwd`

# Build artifacts are not retained.
rm -rf ./build
meson setup build --buildtype=debug -Dc_args="$BUILD_FLAGS" -Dcpp_args="$BUILD_FLAGS" \
	-Dcpp_link_args="$LINK_FLAGS" -Dbuild_example=true && ninja -C build

cp "$HX_DIR/example/example.cfg" ./build/example

CORRECT="$HX_DIR/example/example_correct.txt"

if [ ! -f "$CORRECT" ]; then
	echo "WARNING: regenerating $CORRECT..."
	echo exit | (cd ./build/example && ./hxexample) 2>/dev/null > "$CORRECT"
fi

echo exit | (cd ./build/example && ./hxexample) 2>/dev/null > build/example/hxexample_out.txt
if ! diff -u "$CORRECT" build/example/hxexample_out.txt; then
	echo "error: output differs from $CORRECT"
	exit 1
fi

cat "$CORRECT"
printf "\nOutput matches\nRun hxexample from the build/example directory to test interactively."

echo "🪓🪓🪓"
