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

python3 - <<HX_EOF_ | c++filt
import subprocess
nm = subprocess.run(["nm", "--radix=d", "--print-size", "build/hxtest"],
	stdout=subprocess.PIPE, text=True, check=True).stdout

# Duplicate names for the same function are ignored. The C++ ABI requires them.
seen = set()
rows = []
for line in nm.splitlines():
	f = line.split()
	if len(f) == 4 and f[0] not in seen:
		seen.add(f[0])
		rows.append((f[1], f[2], f[3]))
rows.sort(reverse=True)

def pretty(size):
	n = float(size)
	for unit in ("", "K", "M", "G"):
		if n < 1024.0:
			break
		n /= 1024.0
	return f"{n:.0f}" if unit == "" else f"{n:.1f}{unit}"

def report(title, symbols):
	print(f"\n{'-' * 80}\n * {title}")
	for size, kind, name in symbols:
		print(f"{pretty(size):>6} {kind} {name}")

report("Largest ELF symbols...", rows[:200])

report("Non-test suite libhatchet symbols...",
	[r for r in rows if "hx" in r[2] and "test" not in r[2]])
HX_EOF_
