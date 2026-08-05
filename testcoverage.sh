#!/bin/sh
# SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
# SPDX-License-Identifier: MIT
# This file is licensed under the terms of the LICENSE.md file.

trap 'trap "" INT; pkill -9 -P $$ 2>/dev/null; wait 2>/dev/null; exit 1' INT
set -eu

HX_DIR_=$PWD
HX_GCOV_=gcov-$(gcc -dumpversion)

# Uncalled functions are kept because dead code in a template library is missing
# line coverage waiting to bite.
HX_COVERAGE_="--coverage -O0 -g -fprofile-update=atomic -fno-inline -fkeep-static-functions"
HX_COVERAGE_CXX_="$HX_COVERAGE_ -fno-elide-constructors -fkeep-inline-functions"

HXUSAGE_="usage: $0 [--verbose] [destination-directory]"

HX_VERBOSE_=
HX_DEST_DIR_=
for HX_ARG_ in "$@"; do
	if [ "$HX_ARG_" = "--verbose" ]; then
		HX_VERBOSE_=1
	elif [ -n "$HX_ARG_" ]; then
		case "$HX_ARG_" in
			-*)
				echo "$HXUSAGE_" >&2
				exit 1 ;;
			/*) HX_DEST_DIR_="$HX_ARG_" ;;
			*) HX_DEST_DIR_="$HX_DIR_/$HX_ARG_" ;;
		esac
		case "$HX_DEST_DIR_" in
			*/) ;;
			*) HX_DEST_DIR_="$HX_DEST_DIR_/" ;;
		esac
	fi
done

[ -n "$HX_DEST_DIR_" ] || HX_DEST_DIR_="$HX_DIR_/build/"

if [ -n "$HX_VERBOSE_" ]; then
	set -o xtrace
fi

# Build artifacts are not retained.
rm -rf "$(readlink -f build)" build; ln -s "$(mktemp -d)" build && cd build

gcc -I"$HX_DIR_"/include $HX_COVERAGE_ -DHX_HARDENING_MODE=HX_HARDENING_MODE_DEBUG \
	-DHX_USE_INLINING_ATTR=0 -std=c99 -Wall -Werror -Wfatal-errors -pthread -c     \
	"$HX_DIR_"/test/*.c & HX_PIDS_="$!"

for HX_FILE_ in "$HX_DIR_"/src/*.cpp "$HX_DIR_"/test/*.cpp; do
	g++ -I"$HX_DIR_"/include $HX_COVERAGE_CXX_ -DHX_HARDENING_MODE=HX_HARDENING_MODE_DEBUG   \
		-DHX_TEST_ERROR_HANDLING=1 -DHX_USE_CONSOLE=2 -DHX_USE_PROFILER=1 -DHX_USE_LIBCXX=0  \
		-DHX_USE_INLINING_ATTR=0 -std=c++23 -Wall -Werror -Wextra -Wfatal-errors             \
		-fno-exceptions -Wno-c2y-extensions -Wno-unknown-warning-option -pthread -nostdinc++ \
		-c "$HX_FILE_" -o "$(basename "$HX_FILE_" .cpp).o" & HX_PIDS_="$HX_PIDS_ $!"
done
for HX_PID_ in $HX_PIDS_; do wait "$HX_PID_" || exit 1; done

g++ --coverage *.o -lpthread -o hxtest

if [ -n "$HX_VERBOSE_" ]; then
	echo runtests | ./hxtest help execstdin
else
	if ! echo runtests | ./hxtest help execstdin > console_output.txt 2>&1; then
		cat console_output.txt
		echo "error: hxtest non-zero exit."
		exit 1
	fi
fi

mkdir -p "$HX_DEST_DIR_"

# Line coverage.
HX_STATUS_=0
gcovr --gcov-executable $HX_GCOV_ --gcov-object-directory . --exclude-throw-branches  \
	--exclude-unreachable-branches --exclude-noncode-lines --exclude-lines-by-pattern \
	.*hxassert.* --exclude-branches-by-pattern .*hxassert.* --root $HX_DIR_           \
	--html-details "${HX_DEST_DIR_}0coverage.html" --html-self-contained --json       \
	coverage.json --txt-metric branch --print-summary --fail-under-line 100 . || HX_STATUS_=$?

{ set +o xtrace; } 2> /dev/null

# Branch coverage. Restricted to .gcda files produced by test/*.cpp and test/*.c
# translation units.
mkdir branches_only
for HX_SRC_ in "$HX_DIR_"/test/*.cpp "$HX_DIR_"/test/*.c; do
	HX_BASE_=$(basename "$HX_SRC_")
	HX_BASE_=${HX_BASE_%.*}
	for HX_GCDA_ in *"$HX_BASE_".gcda; do
		[ -e "$HX_GCDA_" ] && ln -s "../$HX_GCDA_" "branches_only/$HX_GCDA_"
		HX_GCNO_=${HX_GCDA_%.gcda}.gcno
		[ -e "$HX_GCNO_" ] && ln -s "../$HX_GCNO_" "branches_only/$HX_GCNO_"
	done
done

gcovr --gcov-executable "$HX_GCOV_" --gcov-object-directory branches_only              \
	--exclude-throw-branches --exclude-unreachable-branches --exclude-noncode-lines    \
	--exclude-lines-by-pattern .*hxassert.* --exclude-branches-by-pattern .*hxassert.* \
	--root "$HX_DIR_" --json coverage_test_branches.json . > /dev/null || HX_STATUS_=$?

# Line coverage. A script is required because the gcovr text output includes
# remarks about suppressed lines.
if [ "$HX_STATUS_" -ne 0 ]; then
python3 - coverage.json <<'EOF'
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
# This is load bearing.
python3 - coverage_test_branches.json <<'HX_EOF_' || HX_STATUS_=$?
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
HX_EOF_

if [ "$HX_STATUS_" -ne 0 ]; then
	echo "error: Missing coverage."
	exit 1
fi

# Launch Chrome if it is installed.
if [ -n "$HX_VERBOSE_" ] && which google-chrome; then
	set -o xtrace
	google-chrome "${HX_DEST_DIR_}coverage_details.html" >/dev/null 2>&1;
fi

# Make sure the script returns 0.
echo "🪓🪓🪓"
