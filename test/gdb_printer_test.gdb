# SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
# SPDX-License-Identifier: MIT
# This file is licensed under the terms of the LICENSE.md file.
#
# GDB batch script to smoke-test the pretty printers against the real hxtest
# binary. Run from the build directory:
#
#   gdb -batch -x ../test/gdb_printer_test.gdb hxtest

source ../gdb/hxarray_printer.py
source ../gdb/hxbitset_printer.py
source ../gdb/hxconstexpr_list_printer.py
source ../gdb/hxdeque_printer.py
source ../gdb/hxflat_map_printer.py
source ../gdb/hxflat_set_printer.py
source ../gdb/hxhash_table_printer.py
source ../gdb/hxlist_printer.py
source ../gdb/hxvector_printer.py

set print thread-events off
set logging file gdb_printer_output.txt
set logging overwrite on
set logging redirect on
set logging enabled on

break hxtest_gdb_break_hxarray_dynamic
commands
  up
  echo \n=== hxarray dynamic ===\n
  print a
  continue
end

break hxtest_gdb_break_hxarray_static
commands
  up
  echo \n=== hxarray static ===\n
  print a
  continue
end

break hxtest_gdb_break_hxbitset
commands
  up
  echo \n=== hxbitset ===\n
  print src
  echo === hxbitset ===\n
  print dst
  continue
end

break hxtest_gdb_break_hxconstexpr_list
commands
  up
  echo \n=== hxconstexpr_list ===\n
  print list
  continue
end

break hxtest_gdb_break_hxdeque_dynamic
commands
  up
  echo \n=== hxdeque dynamic ===\n
  print d
  continue
end

break hxtest_gdb_break_hxdeque_static
commands
  up
  echo \n=== hxdeque static ===\n
  print d
  continue
end

break hxtest_gdb_break_hxflat_map_dynamic
commands
  up
  echo \n=== hxflat_map dynamic ===\n
  print md
  continue
end

break hxtest_gdb_break_hxflat_map_static
commands
  up
  echo \n=== hxflat_map static ===\n
  print ms
  continue
end

break hxtest_gdb_break_hxflat_set_dynamic
commands
  up
  echo \n=== hxflat_set dynamic ===\n
  print s
  continue
end

break hxtest_gdb_break_hxflat_set_static
commands
  up
  echo \n=== hxflat_set static ===\n
  print s
  continue
end

break hxtest_gdb_break_hxhash_table
commands
  up
  echo \n=== hxhash_table ===\n
  print table
  continue
end

break hxtest_gdb_break_hxlist
commands
  up
  echo \n=== hxlist ===\n
  print list
  continue
end

break hxtest_gdb_break_hxvector_dynamic
commands
  up
  echo \n=== hxvector dynamic ===\n
  print objs
  continue
end

break hxtest_gdb_break_hxvector_static
commands
  up
  echo \n=== hxvector static ===\n
  print heap
  continue
end

run help runtests

quit
