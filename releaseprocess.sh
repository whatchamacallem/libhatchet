#!/bin/bash
# SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
# SPDX-License-Identifier: MIT
# This file is licensed under the terms of the LICENSE.md file.

set -eu pipefail

export LANG=C.UTF-8
export LC_ALL=C.UTF-8

[ "$(locale charmap)" = "UTF-8" ] || { echo "error: Locale is not UTF-8." >&2; exit 1; }

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

clear

echo -e "\e[38;5;208mThis will release $TAG from $BRANCH onto $TARGET.\e[0m"
read -ep "Do you want to proceed? [y/n] " -n 1 answer
if [ "$answer" != "y" ] && [ "$answer" != "Y" ]; then
    exit 1
fi

./testall.sh --headless

set -o xtrace

if ! git diff --quiet || ! git diff --cached --quiet; then
    git add .
    git commit -m "$BRANCH $TAG"
fi
git checkout "$TARGET"
git merge --squash "$BRANCH"
git commit -m "$TAG"
DESCRIBE=$(git describe --tags "$TARGET^{commit}")
git tag -a -m "$DESCRIBE" "$TAG"
git push
git push --tags
git checkout "$BRANCH"
git reset --hard "$TARGET"
git push --force

# Regenerate Doxygen HTML and publish it to the gh-pages branch served by
# GitHub Pages. The commit is built with a throwaway index and pushed by hash so
# HEAD, the working tree and the reflog are never touched.
PAGES_BRANCH="gh-pages"
eval "$(grep OUTPUT_DIRECTORY Doxyfile | tr -d ' ')"
doxygen
touch "$OUTPUT_DIRECTORY/.nojekyll"
GIT_INDEX_FILE=$(mktemp -u)
export GIT_INDEX_FILE
GIT_WORK_TREE="$OUTPUT_DIRECTORY" git add -A
TREE=$(git write-tree)
PAGES_PARENT=$(git ls-remote origin "refs/heads/$PAGES_BRANCH" | cut -f1)
DOCS_COMMIT=$(git commit-tree "$TREE" ${PAGES_PARENT:+-p "$PAGES_PARENT"} -m "docs $TAG")
git push --force origin "$DOCS_COMMIT:refs/heads/$PAGES_BRANCH"
rm -f "$GIT_INDEX_FILE"
unset GIT_INDEX_FILE
git update-ref -d "refs/remotes/origin/$PAGES_BRANCH" 2> /dev/null || true
git prune --expire=now

./clean.sh

{ set +o xtrace; } 2> /dev/null

echo -e "\e[38;5;208mDone releasing $TAG ($DESCRIBE).\e[0m"
