#!/bin/sh
# SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
# SPDX-License-Identifier: MIT
# This file is licensed under the terms of the LICENSE.md file.

set -eu

cd "$(dirname "$(readlink -f "$0")")"

rm -rf "$(readlink -f build)" "$(readlink -f cache/ccache)"

# Nukes the files in .gitignore. The container test bundle has no .git.
[ ! -d .git ] || git clean -Xdf -e '!tmp/' -e '!tmp/**'
