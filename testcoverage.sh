#!/bin/sh
# SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
# SPDX-License-Identifier: MIT
# This file is licensed under the terms of the LICENSE.md file.

# POSIXLY_CORRECT=1 was breaking gcovr.
# Using a subdirectory was breaking gcovr. See "--root .. ."

# Prevent leaking background tasks.
trap '{ set +o xtrace; } 2> /dev/null
    trap - 1 2 3 6 15
    for pid in $(pgrep -g "$$" 2>/dev/null); do
        [ "$pid" = "$$" ] && continue
        kill -9 "$pid" 2>/dev/null
    done
    exit 1
' 1 2 3 6 15

set -eu

HX_DIR=$PWD
HX_GCOV=gcov-$(gcc -dumpversion)

# Args are: [--headless] [target-directory]
HX_HEADLESS=
HX_TARGET=
for ARG in "$@"; do
	if [ "$ARG" = "--headless" ]; then
		HX_HEADLESS=1
	else
		case "$ARG" in
			/*) HX_TARGET="$ARG" ;;
			*) HX_TARGET="$HX_DIR/$ARG" ;;
		esac
		case "$HX_TARGET" in
			*/) ;;
			*) HX_TARGET="$HX_TARGET/" ;;
		esac
	fi
done

[ -n "$HX_TARGET" ] || HX_TARGET="$HX_DIR/build/"

# Build artifacts are not retained.
rm -rf ./build; mkdir ./build && cd ./build

if [ -z "$HX_HEADLESS" ]; then
    set -o xtrace
fi

gcc -I"$HX_DIR"/include --coverage -O0 -g -DHX_HARDENING_MODE=HX_HARDENING_MODE_DEBUG \
    -std=c99 -Wall -Werror -Wfatal-errors -pthread -c "$HX_DIR"/test/*.c

g++ -I"$HX_DIR"/include --coverage -O0 -g -DHX_HARDENING_MODE=HX_HARDENING_MODE_DEBUG \
    -DHX_TEST_ERROR_HANDLING=1 -DHX_USE_CONSOLE=2 -DHX_USE_PROFILER=1 -std=c++23 \
    -Wall -Werror -Wextra \
    -Wfatal-errors -fno-exceptions -Wno-c2y-extensions -Wno-unknown-warning-option    \
    -pthread -lpthread -lstdc++ "$HX_DIR"/src/*.cpp "$HX_DIR"/test/*.cpp *.o -o hxtest

if [ -z "$HX_HEADLESS" ]; then
    echo runtests | ./hxtest help execstdin
else
    echo runtests | ./hxtest help execstdin > /dev/null 2>&1
fi

mkdir -p "$HX_TARGET"
gcovr --gcov-executable "$HX_GCOV" --exclude-lines-by-pattern '.*hxassert.*' --html-details "$HX_TARGET" --root .. .

{ set +o xtrace; } 2> /dev/null

# Launch Chrome if it is installed.
if [ -z "$HX_HEADLESS" ] && which google-chrome; then
	google-chrome "${HX_TARGET}coverage_details.html" >/dev/null 2>&1;
fi

# Make sure the script returns 0.
echo "🪓🪓🪓"
