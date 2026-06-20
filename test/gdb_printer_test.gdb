# SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
# SPDX-License-Identifier: MIT
# This file is licensed under the terms of the LICENSE.md file.
#
# GDB batch script to smoke-test the pretty printers against the real hxtest
# binary. Run from the build directory:
#
#   gdb -batch -x ../test/test_printers.gdb hxtest

source ../gdb/hxarray_printer.py
source ../gdb/hxbitset_printer.py
source ../gdb/hxconst_list_printer.py
source ../gdb/hxdeque_printer.py
source ../gdb/hxhash_table_printer.py
source ../gdb/hxlist_printer.py

set print thread-events off
set logging file gdb_printer_output.txt
set logging overwrite on
set logging redirect on
set logging enabled on

# ----- hxarray -----
break hxtest_gdb_break_hxarray
commands
  up
  echo \n=== hxarray ===\n
  print heap
  echo \n======\n
  continue
end

# ----- hxbitset -----
break hxtest_gdb_break_hxbitset
commands
  up
  echo \n=== hxbitset ===\n
  print src
  echo === hxbitset ===\n
  print dst
  echo \n======\n
  continue
end

# ----- hxconst_list -----
break hxtest_gdb_break_hxconst_list
commands
  up
  echo \n=== hxconst_list ===\n
  print list
  echo \n======\n
  continue
end

# ----- hxdeque -----
break hxtest_gdb_break_hxdeque
commands
  up
  echo \n=== hxdeque ===\n
  print d
  echo \n======\n
  continue
end

# ----- hxhash_table -----
break hxtest_gdb_break_hxhash_table
commands
  up
  echo \n=== hxhash_table ===\n
  print table
  echo \n======\n
  continue
end

# ----- hxlist -----
break hxtest_gdb_break_hxlist
commands
  up
  echo \n=== hxlist ===\n
  print list
  echo \n======\n
  continue
end

run help runtests

# This is required to return the error code from the process.
quit
