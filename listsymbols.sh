#!/bin/sh
# SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
# SPDX-License-Identifier: MIT
# This file is licensed under the terms of the LICENSE.md file.

export POSIXLY_CORRECT=1

trap 'trap "" INT; pkill -9 -P $$ 2>/dev/null; wait 2>/dev/null; exit 1' INT
set -eu

if [ ! -f build/hxtest ]; then
	echo "build/hxtest not found!"
	exit 2; # File not found.
fi

HX_USAGE_="usage: $0 [--verbose] [--] [destination-file]"

HX_VERBOSE_=0
HX_DEST_FILE_=
HX_OPT_END_=0
for HX_ARG_ in "$@"; do
	if [ "$HX_OPT_END_" = "0" ]; then
		case "$HX_ARG_" in
		"") continue ;;
		--)
			HX_OPT_END_=1; continue ;;
		--verbose)
			HX_VERBOSE_=1; continue ;;
		-*)
			echo "$HX_USAGE_" >&2
			exit 1 ;;
		esac
	fi

	if [ -n "$HX_DEST_FILE_" ]; then
		echo "$HX_USAGE_" >&2
		exit 1
	fi
	HX_DEST_FILE_="$HX_ARG_"
	HX_VERBOSE_=1
done

if [ -n "$HX_DEST_FILE_" ]; then
	echo "Writing $HX_DEST_FILE_..." >&2
	exec > "$HX_DEST_FILE_"
fi

python3 - <<HX_EOF_
import signal
import subprocess
import sys
from collections import defaultdict
signal.signal(signal.SIGPIPE, signal.SIG_DFL)

nm = subprocess.run(["nm", "--radix=d", "--print-size", "build/hxtest"],
	stdout=subprocess.PIPE, text=True, check=True).stdout

# Duplicate names for the same address are ignored. The C++ ABI requires them.
seen = set()
rows = []
for line in nm.splitlines():
	f = line.split()
	if len(f) == 4 and f[0] not in seen:
		seen.add(f[0])
		rows.append((int(f[1]), f[2], f[3]))
rows.sort(reverse=True)

all_sizes = [size for size, kind, name in rows]
grand_total = sum(all_sizes)

def pretty(size):
	n = float(size)
	for unit in ("", "K", "M", "G"):
		if n < 1024.0:
			break
		n /= 1024.0
	return f"{n:.0f}" if unit == "" else f"{n:.1f}{unit}"

def table(title, groups):
	print(f"{'-' * 80}\n * {title}")
	print(f"{'':24} {'count':>6} {'bytes':>8} {'%':>6} {'largest':>8} {'median':>8}")
	for label, sizes in sorted(groups.items(), key=lambda g: -sum(g[1])):
		total = sum(sizes)
		print(f"{label:24} {len(sizes):6} {pretty(total):>8}"
			f" {100.0 * total / grand_total:6.1f} {pretty(sizes[0]):>8}"
			f" {pretty(sizes[len(sizes) // 2]):>8}")
	print(f"{'total':24} {len(all_sizes):6} {pretty(grand_total):>8} {100.0:6.1f}"
		f" {pretty(all_sizes[0]):>8} {pretty(all_sizes[len(all_sizes) // 2]):>8}")

# Sizes are appended in descending order, keeping each group sorted.
kind_names = {"b": "bss", "d": "data", "g": "small data", "r": "rodata",
	"s": "small bss", "t": "code", "u": "unique", "v": "weak object",
	"w": "weak code"}
by_kind = defaultdict(list)
by_category = defaultdict(list)
for size, kind, name in rows:
	scope = "local" if kind.islower() else "global"
	by_kind[f"{kind} {kind_names.get(kind.lower(), '?')} {scope}"].append(size)
	category = "test suite" if "test" in name else "libhatchet" if "hx" in name else "other"
	by_category[category].append(size)

table("Symbol statistics by nm type...", by_kind)
table("Symbol statistics by category...", by_category)

if $HX_VERBOSE_:
	demangled = subprocess.run(["c++filt"],
		input="".join(name + "\n" for size, kind, name in rows),
		stdout=subprocess.PIPE, text=True, check=True).stdout.splitlines()
	is_tty = sys.stdout.isatty()
	printed = set()
	for (size, kind, name), readable in zip(rows, demangled):
		if readable in printed:
			continue
		printed.add(readable)
		if is_tty and "hx" in readable:
			color = "\033[94;40m" if "test" in readable else "\033[92;40m"
			print(f"{pretty(size):>8} {color}{readable}\033[0m")
		else:
			print(f"{pretty(size):>8} {readable}")
HX_EOF_
