# SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
# SPDX-License-Identifier: MIT
# This file is licensed under the MIT license found in the LICENSE.md file.
#
# Project-local GDB init. Auto-loaded when gdb is started from this directory.
# If GDB refuses to load it, add one of the following to
# ~/.gdbinit or ~/.config/gdb/gdbinit:
#
#   add-auto-load-safe-path /path/to/libhatchet
#
# Or to allow all local .gdbinit files:
#
#   set auto-load safe-path /
#

set print pretty on
set print thread-events off
source gdb/hxarray_printer.py
source gdb/hxbitset_printer.py
source gdb/hxconst_list_printer.py
source gdb/hxdeque_printer.py
source gdb/hxflat_map_printer.py
source gdb/hxflat_set_printer.py
source gdb/hxhash_table_printer.py
source gdb/hxlist_printer.py
