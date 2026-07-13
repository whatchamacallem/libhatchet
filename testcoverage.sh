#!/bin/sh
# SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
# SPDX-License-Identifier: MIT
# This file is licensed under the terms of the LICENSE.md file.

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

HX_DIR=$PWD
HX_GCOV=gcov-$(gcc -dumpversion)

# Args are: [--verbose] [target-directory]
HX_VERBOSE=
HX_TARGET=
for ARG in "$@"; do
	if [ "$ARG" = "--verbose" ]; then
		HX_VERBOSE=1
	else
		case "$ARG" in
			/*) HX_TARGET="$ARG" ;;
			*) HX_TARGET="$HX_DIR/$ARG" ;;
		esac
		case "$HX_TARGET" in
			*/) ;;
			*) HX_TARGET="$HX_TARGET/" ;;
		esac
	fi
done

[ -n "$HX_TARGET" ] || HX_TARGET="$HX_DIR/build/"

# Build artifacts are not retained.
rm -rf "$(readlink -f build)" build; ln -s "$(mktemp -d)" build && cd build

# Uncalled functions are kept because dead code in a template library is missing
# line coverage waiting to bite.
HX_COVERAGE="--coverage -O0 -g -fprofile-update=atomic -fno-inline -fkeep-static-functions"
HX_COVERAGE_CXX="$HX_COVERAGE -fno-elide-constructors -fkeep-inline-functions"

if [ -n "$HX_VERBOSE" ]; then
	set -o xtrace
fi

gcc -I"$HX_DIR"/include $HX_COVERAGE -DHX_HARDENING_MODE=HX_HARDENING_MODE_DEBUG		\
	-DHX_USE_INLINING_ATTR=0 -std=c99 -Wall -Werror -Wfatal-errors -pthread -c          \
	"$HX_DIR"/test/*.c

g++ -I"$HX_DIR"/include $HX_COVERAGE_CXX -DHX_HARDENING_MODE=HX_HARDENING_MODE_DEBUG    \
	-DHX_TEST_ERROR_HANDLING=1 -DHX_USE_CONSOLE=2 -DHX_USE_PROFILER=1 -DHX_USE_LIBCXX=0 \
	-DHX_USE_INLINING_ATTR=0 -std=c++23 -Wall -Werror -Wextra -Wfatal-errors            \
	-fno-exceptions -Wno-c2y-extensions -Wno-unknown-warning-option -pthread -lpthread  \
	-nostdinc++ "$HX_DIR"/src/*.cpp "$HX_DIR"/test/*.cpp *.o -o hxtest

if [ -n "$HX_VERBOSE" ]; then
	echo runtests | ./hxtest help execstdin
else
	if ! echo runtests | ./hxtest help execstdin > console_output.txt 2>&1; then
		cat console_output.txt
		echo "error: hxtest non-zero exit."
		exit 1
	fi
fi

mkdir -p "$HX_TARGET"
HX_STATUS=0

# Line coverage.
gcovr --gcov-executable $HX_GCOV --gcov-object-directory . --exclude-throw-branches     \
	--exclude-unreachable-branches --exclude-noncode-lines --exclude-lines-by-pattern   \
	.*hxassert.* --exclude-branches-by-pattern .*hxassert.* --root $HX_DIR              \
	--html-details "${HX_TARGET}0coverage.html" --html-self-contained --json            \
	coverage.json --txt-metric branch --print-summary --fail-under-line 100 . || HX_STATUS=$?

{ set +o xtrace; } 2> /dev/null
# Branch coverage. Restricted to .gcda files produced by test/*.cpp and test/*.c
# translation units.
mkdir branches_only
for HX_SRC in "$HX_DIR"/test/*.cpp "$HX_DIR"/test/*.c; do
	HX_BASE=$(basename "$HX_SRC")
	HX_BASE=${HX_BASE%.*}
	for HX_GCDA in *"$HX_BASE".gcda; do
		[ -e "$HX_GCDA" ] && ln -s "../$HX_GCDA" "branches_only/$HX_GCDA"
		HX_GCNO=${HX_GCDA%.gcda}.gcno
		[ -e "$HX_GCNO" ] && ln -s "../$HX_GCNO" "branches_only/$HX_GCNO"
	done
done

gcovr --gcov-executable "$HX_GCOV" --gcov-object-directory branches_only                \
	--exclude-throw-branches --exclude-unreachable-branches --exclude-noncode-lines     \
	--exclude-lines-by-pattern .*hxassert.* --exclude-branches-by-pattern .*hxassert.*  \
	--root "$HX_DIR" --json coverage_test_branches.json . > /dev/null || HX_STATUS=$?

# Line coverage. A script is required because the gcovr text output includes
# remarks about suppressed lines.
if [ "$HX_STATUS" -ne 0 ]; then
python3 - coverage.json <<EOF
import json, sys
files = json.load(open(sys.argv[1]))["files"]
for f in sorted(files, key=lambda x: x["file"]):
	numbers = sorted(l["line_number"] for l in f["lines"]
				if l["count"] == 0 and not l.get("gcovr/excluded"))
	if not numbers:
		continue
	spans, start, prev = [], numbers[0], numbers[0]
	for n in numbers[1:] + [None]:
		if n == prev + 1:
			prev = n
			continue
		spans.append(str(start) if start == prev else "%d-%d" % (start, prev))
		start = prev = n
	print("%s:%s" % (f["file"], ",".join(spans)))
EOF
fi

# Branch coverage. Post-processing is required to merge multiple instantiations.
python3 - coverage_test_branches.json <<EOF || HX_STATUS=$?
import json, sys
files = json.load(open(sys.argv[1]))["files"]
missing = False
for f in sorted(files, key=lambda x: x["file"]):
	if "/include/" not in ("/" + f["file"]) and "/src/" not in ("/" + f["file"]):
		continue
	numbers = sorted(l["line_number"] for l in f["lines"]
				if not l.get("gcovr/excluded")
				and any(b["count"] == 0 for b in l.get("branches", [])))
	if not numbers:
		continue
	missing = True
	spans, start, prev = [], numbers[0], numbers[0]
	for n in numbers[1:] + [None]:
		if n == prev + 1:
			prev = n
			continue
		spans.append(str(start) if start == prev else "%d-%d" % (start, prev))
		start = prev = n
	print("%s:%s" % (f["file"], ",".join(spans)))
sys.exit(1 if missing else 0)
EOF

if [ "$HX_STATUS" -ne 0 ]; then
	echo "error: Missing coverage."
	exit 1
fi

# Launch Chrome if it is installed.
if [ -n "$HX_VERBOSE" ] && which google-chrome; then
	google-chrome "${HX_TARGET}coverage_details.html" >/dev/null 2>&1;
fi

# Make sure the script returns 0.
echo "🪓🪓🪓"
