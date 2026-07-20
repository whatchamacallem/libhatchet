#!/bin/bash
# SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
# SPDX-License-Identifier: MIT
# This file is licensed under the terms of the LICENSE.md file.

set -euo pipefail

if [ -n "$(declare -Fx)" ]; then
	echo "error: Exported shell functions interfere with reproducibility." >&2
	declare -Fx >&2
	exit 1
fi

export LANG=C.UTF-8
export LC_ALL=C.UTF-8

[ "$(locale charmap)" = "UTF-8" ] || { echo "error: Locale is not UTF-8." >&2; exit 1; }

HX_TAG_=$(grep 'define LIBHATCHET_TAG' include/hx/libhatchet.h | cut -d'"' -f2)
HX_BRANCH_=$(git branch --show-current)
HX_TARGET_="main"

if [[ "$HX_TAG_" != v* ]]; then
	echo "error: Invalid tag. TAG=\"$HX_TAG_\" does not start with a v."
	exit 1
fi

if git tag --list | grep -qxF "$HX_TAG_"; then
	echo "error: Tag $HX_TAG_ already exists. Update libhatchet.h."
	exit 1
fi

clear

echo -e "\e[38;5;208mThis will release $HX_TAG_ from $HX_BRANCH_ onto $HX_TARGET_.\e[0m"
read -ep "Do you want to proceed? [y/n] " -n 1 HX_ANSWER_
if [ "$HX_ANSWER_" != "y" ] && [ "$HX_ANSWER_" != "Y" ]; then
	exit 1
fi

./testall.sh

if ! git diff --quiet || ! git diff --cached --quiet \
		|| [ -n "$(git ls-files --others --exclude-standard)" ]; then
	git add .
	git commit -q -m "$HX_BRANCH_ $HX_TAG_"
fi
git checkout -q "$HX_TARGET_"
git merge --squash "$HX_BRANCH_"
git commit -q -m "$HX_TAG_"
HX_DESCRIBE_=$(git describe --tags "$HX_TARGET_^{commit}")
git tag -a -m "$HX_DESCRIBE_" "$HX_TAG_"
git push -q
git push -q --tags
git checkout -q "$HX_BRANCH_"
git reset -q --hard "$HX_TARGET_"
git push -q --force

# Publish docs to the gh-pages branch served by GitHub Pages. The commit
# has no parent and the branch is purged afterwards.
doxygen
touch docs/.nojekyll
GIT_INDEX_FILE=$(mktemp -u)
export GIT_INDEX_FILE
GIT_WORK_TREE="docs" git add -A
HX_DOCS_TREE_OBJECT_=$(git write-tree)
HX_DOCS_COMMIT_=$(git commit-tree "$HX_DOCS_TREE_OBJECT_" -m "docs $HX_TAG_")
git push -q --force origin "$HX_DOCS_COMMIT_:refs/heads/gh-pages"
rm -f "$GIT_INDEX_FILE"
unset GIT_INDEX_FILE
git update-ref -d "refs/remotes/origin/gh-pages" 2> /dev/null || true
git prune --expire=now

echo -e "\e[38;5;208mDone releasing $HX_TAG_ ($HX_DESCRIBE_).\e[0m"
