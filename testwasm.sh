#!/bin/sh
# SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
# SPDX-License-Identifier: MIT
# This file is licensed under the terms of the LICENSE.md file.
#
# Requires the the emsdk. After installing the emsdk it is recommended to add it
# to your .bashrc.

# Prevent leaking background tasks.
trap '{ set +o xtrace; } 2> /dev/null
    trap - 1 2 3 6 15
    for pid in $(pgrep -g "$$" 2>/dev/null); do
        [ "$pid" = "$$" ] && continue
        kill -9 "$pid" 2>/dev/null
    done
    exit 1
' 1 2 3 6 15

set -eu

export POSIXLY_CORRECT=1

HX_DIR=$PWD

# Build artifacts are not retained.
rm -rf "$(readlink -f build)" build; ln -s "$(mktemp -d)" build && cd build

emcc -I"$HX_DIR/include" -O2 -pthread -fdiagnostics-absolute-paths -c "$HX_DIR"/test/*.c

# Dump the memory manager because a web browser doesn't need that. -pthread
# requires SharedArrayBuffer which needs COOP/COEP headers from the server.
emcc -O2 -fno-exceptions -fno-rtti -fdiagnostics-absolute-paths          \
	-Werror -Wfatal-errors -DHX_USE_FILE_IO=0 -DHX_USE_MEMORY_MANAGER=0  \
	-DHX_USE_THREADS=1 -DHX_USE_CONSOLE=1 -Wno-c2y-extensions -pthread   \
	-sEXIT_RUNTIME=1 -sPTHREAD_POOL_SIZE=4 -sPROXY_TO_PTHREAD -std=c++23 \
	-flto=auto -I"$HX_DIR/include" *.o "$HX_DIR"/src/*.cpp               \
	"$HX_DIR"/test/*.cpp -o index.html

if [ "${1:-}" != "--headless" ]; then

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
	SERVER_PID=$!
	set +e

	# Launch Chrome if it is installed.
	if which google-chrome; then
		google-chrome http://0.0.0.0:9876/ >/dev/null 2>&1 &
	fi

	# Wait for the web server. Kill its process group when interrupted (Ctrl-C).
	echo "Press ctrl-c to kill the web server."
	wait "$SERVER_PID"
fi

# Say goodbye and make sure the script returns 0.
echo 🪓🪓🪓
