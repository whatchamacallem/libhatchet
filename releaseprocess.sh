#!/bin/bash
# SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
# SPDX-License-Identifier: MIT
# This file is licensed under the terms of the LICENSE.md file.
#
# In include/hx/libhatchet.h bump LIBHATCHET_VER and LIBHATCHET_TAG and then
# leave the changes unstaged before running. The script will exit early if
# this is not done, there is a merge conflict or the tag is not unique.

set -eu pipefail

TAG=$(grep 'define LIBHATCHET_TAG' include/hx/libhatchet.h | cut -d'"' -f2)
BRANCH=$(git branch --show-current)
TARGET="main"

if [[ "$TAG" != v* ]]; then
    echo "error: Invalid tag. TAG=\"$TAG\" does not start with a v."
    exit 1
fi

if git tag --list | grep -qxF "$TAG"; then
    echo "error: Tag $TAG already exists. Update libhatchet.h."
    exit 1
fi

echo -e "\e[38;5;208mThis will release $TAG from $BRANCH onto $TARGET.\e[0m"
read -ep "Do you want to proceed? [y/n] " -n 1 answer
if [ "$answer" != "y" ] && [ "$answer" != "Y" ]; then
    exit 1
fi

./testall.sh

set -o xtrace

if ! git diff --quiet || ! git diff --cached --quiet; then
    git add .
    git commit -m "$BRANCH $TAG"
fi
git checkout "$TARGET"
git merge --squash "$BRANCH"
git commit -m "$TAG"
git tag "$TAG"
git push
git push --tags
git checkout "$BRANCH"
git reset --hard "$TARGET"
git push --force

{ set +o xtrace; } 2> /dev/null

echo -e "\e[38;5;208mDone releasing $TAG.\e[0m"
