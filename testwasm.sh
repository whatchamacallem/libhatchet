#!/bin/sh
# SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
# SPDX-License-Identifier: MIT
# This file is licensed under the terms of the LICENSE.md file.
#
# Requires the the emsdk. After installing the emsdk it is recommended to add it
# to your .bashrc.

trap 'trap "" INT; pkill -9 -P $$ 2>/dev/null; wait 2>/dev/null; exit 1' INT
set -eu

export POSIXLY_CORRECT=1

HX_DIR_=$PWD

# Build artifacts are not retained.
rm -rf "$(readlink -f build)" build; ln -s "$(mktemp -d)" build && cd build

emcc -I"$HX_DIR_/include" -O2 -pthread -fdiagnostics-absolute-paths -c "$HX_DIR_"/test/*.c

# Dump the memory manager because a web browser doesn't need that. -pthread
# requires SharedArrayBuffer which needs COOP/COEP headers from the server.
HX_PIDS_=""
for HX_FILE_ in "$HX_DIR_"/src/*.cpp "$HX_DIR_"/test/*.cpp; do
	emcc -O2 -fno-exceptions -fno-rtti -fdiagnostics-absolute-paths          \
		-Werror -Wfatal-errors -DHX_USE_FILE_IO=0 -DHX_USE_MEMORY_MANAGER=0  \
		-DHX_USE_THREADS=1 -DHX_USE_CONSOLE=1 -Wno-c2y-extensions -pthread   \
		-std=c++23 -flto=auto -I"$HX_DIR_/include" -c "$HX_FILE_"            \
		-o "$(basename "$HX_FILE_" .cpp).o" & HX_PIDS_="$HX_PIDS_ $!"
done
for HX_PID_ in $HX_PIDS_; do wait "$HX_PID_" || exit 1; done

emcc -O2 -fno-exceptions -fno-rtti -DHX_USE_THREADS=1 -pthread -sEXIT_RUNTIME=1 \
	-sPTHREAD_POOL_SIZE=4 -sPROXY_TO_PTHREAD -std=c++23 -flto=auto *.o          \
	-o index.html

if [ "${1:-}" = "--verbose" ]; then

	echo "Serving http://0.0.0.0:9876/"

	# Start a web server with COOP/COEP headers required for SharedArrayBuffer
	# (pthreads).
	python3 -c "
import http.server, sys
class Handler(http.server.SimpleHTTPRequestHandler):
	def end_headers(self):
		self.send_header('Cross-Origin-Opener-Policy', 'same-origin')
		self.send_header('Cross-Origin-Embedder-Policy', 'require-corp')
		super().end_headers()
http.server.HTTPServer(('', 9876), Handler).serve_forever()
" &
	HX_SERVER_PID_=$!
	set +e

	# Launch Chrome if it is installed.
	if which google-chrome; then
		google-chrome http://0.0.0.0:9876/ >/dev/null 2>&1 &
	fi

	# Wait for the web server. Kill its process group when interrupted (Ctrl-C).
	echo "Press ctrl-c to kill the web server."
	wait "$HX_SERVER_PID_"
fi

# Say goodbye and make sure the script returns 0.
echo 🪓🪓🪓
