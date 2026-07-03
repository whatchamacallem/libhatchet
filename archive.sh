#!/bin/sh
# SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
# SPDX-License-Identifier: MIT
# This file is licensed under the terms of the LICENSE.md file.

set -eu

SCRIPT_NAME=$(basename "$0")
PROJECT="$(basename "$PWD")"
DATE="$(date +%Y-%m-%d)"
ARCHIVE="$PROJECT-$DATE.git.txz"

# Print help if the first arg starts with a - or there is more than one.
if [ "$#" -gt 1 ] || { [ "$#" -eq 1 ] && [ "${1#-}" != "$1" ]; }; then
	echo "$SCRIPT_NAME [destination-directory]"
	echo "Will create $ARCHIVE in the destination-directory if"
	echo "provided, otherwise ~/Backups/ if it exists and in ~/ otherwise. Restores all"
	echo "files if $0 is the only file in the directory."
	exit 1
fi

# Check for the .git file.
if [ ! -d ".git" ]; then
	echo "error: .git not found" >&2
	exit 1
fi

# Extract archive if this script is the only non-hidden file.
if [ "$(command ls)" = "$SCRIPT_NAME" ]; then
	git fsck
	git restore .
	echo "Extracted all files in $PROJECT."

	FS_TYPE=$(stat -f -c "%T" . 2>/dev/null) || FS_TYPE=""
	if [ "$FS_TYPE" = "v9fs" ] || [ "$FS_TYPE" = "fuseblk" ] || [ "$FS_TYPE" = "ntfs" ]; then
		echo "Windows detected. Setting config core.fileMode false."
		git config core.fileMode false
	fi
	exit 0
fi

# Create archive.
if [ "$#" -eq 1 ]; then
	DESTINATION="$1"
elif [ -d "$HOME/Backups" ]; then
	DESTINATION="$HOME/Backups"
else
	DESTINATION="$HOME"
fi
if [ ! -d "$DESTINATION" ]; then
	echo "Destination directory not found: $DESTINATION" >&2
	exit 1
fi

git fsck

# Delete the gh-pages GitHub Pages branch. Should not be required.
git update-ref -d "refs/remotes/origin/gh-pages" 2> /dev/null || true

git reflog expire --expire=24.hours.ago --expire-unreachable=24.hours.ago --all
git gc --prune=now --aggressive

# Save everything including local config.
tar -cJf "$DESTINATION/$ARCHIVE" -C ".." "$PROJECT/$SCRIPT_NAME" "$PROJECT/.git"

printf "Wrote: "
ls -h1s "$DESTINATION/$ARCHIVE"
