# Automating stop-and-inspect breakpoints for test/gdb_printer_test.gdb in VS Code

These instructions explain how to set breakpoints in the VS Code debug watch
window for all the breakpoints in `test/gdb_printer_test.gdb`.

This document is intended for an AI.

## Problem

`test/gdb_printer_test.gdb` sets GDB breakpoints on `hxtest_gdb_break_*`
marker functions with `commands` blocks that auto-`continue`, so it never
actually pauses for interactive inspection. To pause in VS Code instead, the
breakpoints have to exist as real VS Code breakpoints, not GDB commands.

## Key finding: setupCommands breakpoints do not work

Do NOT add `break <function>` lines to a `launch.json` `setupCommands` array
to try to pre-set breakpoints. This silently fails: GDB's MI protocol does
fire the stop event (confirmed with raw `gdb --interpreter=mi` testing), but
VS Code's debug UI (cpptools 1.33.8, MIEngine) does not track breakpoints
that were not created through its own breakpoint state, so the BREAKPOINTS
panel stays empty and the program just runs to completion. This matches
publicly reported cpptools bugs (e.g. vscode-cpptools#9504: "shows hit 1st
breakpoint in console logs but not in GUI").

There is also no `launch.json` schema field for pre-declared function
breakpoints. Breakpoints are pure UI/workspace state, not `launch.json` data.

## Where VS Code actually stores function breakpoints

For a normal local window: `state.vscdb` under
`~/.vscode-server/data/User/workspaceStorage/<hash>/` (Linux/WSL server side).

For a **VS Code Remote (WSL) window** specifically: that per-workspace
`state.vscdb` on the Linux side does NOT exist / is not the one read. The
real one is on the Windows client side, found by workspace folder URI match:

```bash
for d in /mnt/c/Users/<winuser>/AppData/Roaming/Code/User/workspaceStorage/*/; do
  grep -qi "<repo-folder-name>" "$d/workspace.json" 2>/dev/null && echo "$d" && cat "$d/workspace.json"
done
```

Look for a `workspace.json` whose `folder` is
`vscode-remote://wsl+<distro>/<path>` matching the actual remote path (e.g.
`vscode-remote://wsl%2Bubuntu-24.04/home/t/xz`) - there are usually many stale
entries from other clone locations, only one matches the live remote path.

That directory's `state.vscdb` is a SQLite DB with table `ItemTable(key,
value)`. Breakpoint-relevant keys:

- `debug.functionbreakpoint` -- JSON array of function breakpoints
- `debug.exceptionbreakpoint` -- sibling key, useful to see the general shape
- `debug.selectedconfigname` -- currently selected launch config name

## Schema (empirically confirmed, do not guess it)

```bash
python3 -c "
import sqlite3
con = sqlite3.connect('<path>/state.vscdb')
cur = con.cursor()
cur.execute(\"SELECT value FROM ItemTable WHERE key='debug.functionbreakpoint'\")
print(cur.fetchone()[0])
"
```

Confirmed shape (one entry per breakpoint):

```json
{"id": "<random-uuid-v4>", "enabled": true, "name": "<function-name>"}
```

`debug.functionbreakpoint` itself is a JSON array of these objects, stored as
a single string value (no pretty-printing, no trailing newline).

## Writing all breakpoints in one shot

Generate a fresh UUID per entry (do not reuse the human's one), build the
JSON array, and `UPDATE` the row (never `INSERT`, the key already exists):

```python
import sqlite3, json, uuid

names = [ ... ]  # the hxtest_gdb_break_* function names to stop at
entries = [{"id": str(uuid.uuid4()), "enabled": True, "name": n} for n in names]
value = json.dumps(entries, separators=(",", ":"))

con = sqlite3.connect("<path>/state.vscdb")
con.execute("UPDATE ItemTable SET value=? WHERE key='debug.functionbreakpoint'", (value,))
con.commit()
```

Before writing to the live file, copy it somewhere disposable first and
inspect that copy read-only -- `state.vscdb` backs the user's entire running
VS Code client (or the whole remote workspace), not just this task, and a
malformed write risks corrupting workbench state.

## Picking up the change

Now have a human run `Developer: Reload Window` from the command palette, or
fully restart VS Code.
