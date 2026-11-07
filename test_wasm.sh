#!/bin/sh
# SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
# SPDX-License-Identifier: MIT
# This file is licensed under the terms of the LICENSE.md file.
#
# Requires the emsdk, which provides emcc and node. "install_deb.sh --sdks"
# installs it to $HOME/emsdk.

trap 'trap "" INT PIPE TERM; pkill -P $$; wait; exit 1' INT PIPE TERM
set -eu

cd "$(dirname "$(readlink -f "$0")")"

export POSIXLY_CORRECT=1

HX_DIR_=$PWD
HX_JOBS_="${HX_MAX_CORES:-$(getconf _NPROCESSORS_ONLN)}"

HX_EMSDK_="$HOME/emsdk"
if [ ! -d "$HX_EMSDK_" ]; then
	echo "error: $HX_EMSDK_ not found. Run install_deb.sh --sdks."
	exit 1
fi

# sh can only source emsdk_env.sh from its own directory.
export EMSDK_QUIET=1
cd "$HX_EMSDK_"
. ./emsdk_env.sh
cd "$HX_DIR_"

if [ "${1:-}" = "--verbose" ]; then
	set -o xtrace
fi

# Build artifacts are not retained.
rm -rf "$(readlink -f build)" build
if [ "${HX_RAMDISK:-0}" = 1 ]; then mkdir build; else ln -s "$(mktemp -d)" build; fi
cd build

emcc -I"$HX_DIR_/include" -O2 -pthread -fdiagnostics-absolute-paths -c \
	"$HX_DIR_"/test/*.c

# -pthread requires SharedArrayBuffer which needs COOP/COEP headers from the
# server.
printf '%s\0' "$HX_DIR_"/src/*.cpp "$HX_DIR_"/test/*.cpp |
	xargs -0 -n 1 -P "$HX_JOBS_" sh -c '"$0" "$@" || exit 255'               \
	emcc -O2 -fno-exceptions -fno-rtti -fdiagnostics-absolute-paths          \
		-Werror -Wfatal-errors -DHX_USE_FILE_IO=0 -DHX_USE_SLAB_ALLOCATOR=0  \
		-DHX_USE_THREADS=1 -DHX_USE_CONSOLE=1 -Wno-c2y-extensions -pthread   \
		-std=c++23 -flto=auto -I"$HX_DIR_/include" -c

em++ -O2 -pthread -sEXIT_RUNTIME=1 -sPTHREAD_POOL_SIZE=4 -sPROXY_TO_PTHREAD \
	-flto=auto *.o -o index.html

if [ "${1:-}" = "--verbose" ]; then
	"$EMSDK_NODE" index.js
else
	if "$EMSDK_NODE" index.js > console_output.txt 2>&1; then
		grep -E '^(not ok|Bail out!|# )' console_output.txt
	else
		cat console_output.txt
		exit 1
	fi
fi

if [ "${1:-}" = "--verbose" ]; then

	echo "Serving http://0.0.0.0:9876/"

	# Start a web server with COOP/COEP headers required for SharedArrayBuffer
	# (pthreads).
	python3 - <<'HX_EOF_' &
import http.server, sys
class Handler(http.server.SimpleHTTPRequestHandler):
	def end_headers(self):
		self.send_header('Cross-Origin-Opener-Policy', 'same-origin')
		self.send_header('Cross-Origin-Embedder-Policy', 'require-corp')
		super().end_headers()
http.server.HTTPServer(('', 9876), Handler).serve_forever()
HX_EOF_
	HX_SERVER_PID_=$!
	set +e

	# Launch Google Chrome if it is installed.
	if command -v google-chrome >/dev/null 2>&1; then
		google-chrome http://0.0.0.0:9876/ >/dev/null 2>&1 &
	fi

	# Wait for the web server. Kill its process group when interrupted (Ctrl-C).
	echo "Press ctrl-c to kill the web server."
	wait "$HX_SERVER_PID_"
fi

echo 🪓🪓🪓
