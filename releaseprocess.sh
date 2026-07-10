#!/bin/bash
# SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
# SPDX-License-Identifier: MIT
# This file is licensed under the terms of the LICENSE.md file.

set -euo pipefail

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

./testall.sh

set -o xtrace

if ! git diff --quiet || ! git diff --cached --quiet \
		|| [ -n "$(git ls-files --others --exclude-standard)" ]; then
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

# Publish docs to the gh-pages branch served by GitHub Pages. The commit
# has no parent and the branch is purged afterwards.
doxygen
touch docs/.nojekyll
GIT_INDEX_FILE=$(mktemp -u)
export GIT_INDEX_FILE
GIT_WORK_TREE="docs" git add -A
DOCS_TREE_OBJECT=$(git write-tree)
DOCS_COMMIT=$(git commit-tree "$DOCS_TREE_OBJECT" -m "docs $TAG")
git push --force origin "$DOCS_COMMIT:refs/heads/gh-pages"
rm -f "$GIT_INDEX_FILE"
unset GIT_INDEX_FILE
git update-ref -d "refs/remotes/origin/gh-pages" 2> /dev/null || true
git prune --expire=now

{ set +o xtrace; } 2> /dev/null

echo -e "\e[38;5;208mDone releasing $TAG ($DESCRIBE).\e[0m"
