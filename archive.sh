#!/bin/sh
# SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
# SPDX-License-Identifier: MIT
# This file is licensed under the terms of the LICENSE.md file.

set -eu

HX_SCRIPT_NAME_=$(basename "$0")
HX_PROJECT_="$(basename "$PWD")"
HX_DATE_="$(date +%Y-%m-%d)"
HX_ARCHIVE_="$HX_PROJECT_-$HX_DATE_.git.txz"

# Print help if there is more than one arg or the first arg starts with a -.
if [ "$#" -gt 1 ] || { [ "$#" -eq 1 ] && [ "${1#-}" != "$1" ]; }; then
	echo "usage: $0 [destination-directory]"
	echo "Will create $HX_ARCHIVE_ in the destination-directory if"
	echo "provided, otherwise ~/Backups/ if it exists and in ~/ otherwise. Restores all"
	echo "files if $HX_SCRIPT_NAME_ is the only file in the directory."
	exit 1
fi

# Check for the .git file.
if [ ! -d ".git" ]; then
	echo "error: .git not found" >&2
	exit 1
fi

# Extract archive if this script is the only non-hidden file.
if [ "$(command ls)" = "$HX_SCRIPT_NAME_" ]; then
	git fsck
	git restore .
	echo "Extracted all files in $HX_PROJECT_."

	HX_FS_TYPE_=$(stat -f -c "%T" . 2>/dev/null) || HX_FS_TYPE_=""
	if [ "$HX_FS_TYPE_" = "v9fs" ] || [ "$HX_FS_TYPE_" = "fuseblk" ] || [ "$HX_FS_TYPE_" = "ntfs" ]; then
		echo "Windows detected. Setting config core.fileMode false."
		git config core.fileMode false
	fi
	exit 0
fi

# Create archive.
if [ "$#" -eq 1 ]; then
	HX_DEST_DIR_="$1"
elif [ -d "$HOME/Backups" ]; then
	HX_DEST_DIR_="$HOME/Backups"
else
	HX_DEST_DIR_="$HOME"
fi
if [ ! -d "$HX_DEST_DIR_" ]; then
	echo "Destination directory not found: $HX_DEST_DIR_" >&2
	exit 1
fi

git fsck

# Delete the gh-pages GitHub Pages branch. Should not be required.
git update-ref -d "refs/remotes/origin/gh-pages" 2> /dev/null || true

git reflog expire --expire=24.hours.ago --expire-unreachable=24.hours.ago --all
git gc --prune=now --aggressive

# Save everything including local config.
tar -cJf "$HX_DEST_DIR_/$HX_ARCHIVE_" -C ".." "$HX_PROJECT_/$HX_SCRIPT_NAME_" "$HX_PROJECT_/.git"

printf "Wrote: "
ls -h1s "$HX_DEST_DIR_/$HX_ARCHIVE_"
