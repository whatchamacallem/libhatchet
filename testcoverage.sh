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
        kill "$pid" 2>/dev/null
    done
    exit 1
' 1 2 3 6 15

set -eu

HX_DIR=`pwd`

# Build artifacts are not retained.
rm -rf ./build; mkdir ./build && cd ./build

set -o xtrace

gcc -I$HX_DIR/include --coverage -O0 -g -DHX_HARDENING_MODE=HX_HARDENING_MODE_DEBUG \
    -std=c99 -Wall -Werror -Wfatal-errors -pthread -c $HX_DIR/test/*.c

g++ -I$HX_DIR/include --coverage -O0 -g -DHX_HARDENING_MODE=HX_HARDENING_MODE_DEBUG \
    -DHX_TEST_ERROR_HANDLING=1 -DHX_USE_PROFILER=1 -std=c++23 -Wall -Werror         \
    -Wfatal-errors -fno-exceptions -pthread -lpthread -lstdc++                      \
    $HX_DIR/src/*.cpp $HX_DIR/test/*.cpp *.o -o hxtest

echo runtests | ./hxtest help execstdin
if [ $? -ne 0 ]; then
  echo "error: hxtest exit: $?"
fi

gcovr --exclude-lines-by-pattern '.*hxassert.*' --html-details coverage.html --root .. .

{ set +o xtrace; } 2> /dev/null

# Launch Chrome if it is installed.
if [ "${1:-}" != "--headless" ] && which google-chrome; then
	google-chrome coverage.html >/dev/null 2>&1;
fi

# Make sure the script returns 0.
echo "🪓🪓🪓"
