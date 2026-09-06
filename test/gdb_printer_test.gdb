# SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
# SPDX-License-Identifier: MIT
# This file is licensed under the terms of the LICENSE.md file.
#
# GDB batch script to smoke-test the pretty printers against the real hxtest
# binary. Run from the build directory:
#
#   gdb -batch -x ../test/gdb_printer_test.gdb hxtest

source ../gdb/hxallocator_printer.py
source ../gdb/hxarray_printer.py
source ../gdb/hxbitset_printer.py
source ../gdb/hxconstexpr_list_printer.py
source ../gdb/hxdeque_printer.py
source ../gdb/hxexpected_printer.py
source ../gdb/hxflat_map_printer.py
source ../gdb/hxflat_set_printer.py
source ../gdb/hxhandle_table_printer.py
source ../gdb/hxhash_table_printer.py
source ../gdb/hxlist_printer.py
source ../gdb/hxptr_printer.py
source ../gdb/hxref_printer.py
source ../gdb/hxslot_map_printer.py
source ../gdb/hxvector_printer.py
source ../test/hxtest_object_printer.py

set hxallocator-columns 0
set print thread-events off

define hxprint
  pipe print $arg0 | cat >> gdb_printer_output.txt
end

break hxtest_gdb_break_hxarray_dynamic
commands
  up
  echo \n=== hxarray dynamic ===\n
  hxprint a
  echo === hxallocator dynamic ===\n
  hxprint *(hxallocator<hxtest_util::hxtest_object,0>*)&a
  continue
end

break hxtest_gdb_break_hxarray_static
commands
  up
  echo \n=== hxarray static ===\n
  hxprint a
  echo === hxallocator static ===\n
  hxprint *(hxallocator<hxtest_util::hxtest_object,4>*)&a
  continue
end

break hxtest_gdb_break_hxarray_unallocated
commands
  up
  echo \n=== hxarray unallocated ===\n
  hxprint a
  echo === hxallocator unallocated ===\n
  hxprint *(hxallocator<int,0>*)&a
  continue
end

break hxtest_gdb_break_hxbitset
commands
  up
  echo \n=== hxbitset ===\n
  hxprint a
  echo === hxbitset ===\n
  hxprint b
  echo === hxbitset multi-row ===\n
  hxprint c
  continue
end

break hxtest_gdb_break_hxconstexpr_list
commands
  up
  echo \n=== hxconstexpr_list ===\n
  hxprint list
  continue
end

break hxtest_gdb_break_hxconstexpr_list_empty
commands
  up
  echo \n=== hxconstexpr_list empty and multi-field ===\n
  hxprint a
  echo === hxconstexpr_list multi-field ===\n
  hxprint b
  continue
end

break hxtest_gdb_break_hxdeque_dynamic
commands
  up
  echo \n=== hxdeque dynamic ===\n
  hxprint a
  continue
end

break hxtest_gdb_break_hxdeque_static
commands
  up
  echo \n=== hxdeque static ===\n
  hxprint a
  echo === hxdeque unallocated ===\n
  hxprint b
  echo === hxdeque reserved empty ===\n
  hxprint c
  continue
end

break hxtest_gdb_break_hxexpected
commands
  up
  echo \n=== hxexpected ===\n
  hxprint a
  echo === hxexpected disengaged ===\n
  hxprint b
  echo === hxexpected truthy error ===\n
  hxprint c
  continue
end

break hxtest_gdb_break_hxflat_map_dynamic
commands
  up
  echo \n=== hxflat_map dynamic ===\n
  hxprint b
  echo === hxflat_map unallocated ===\n
  hxprint c
  echo === hxflat_map reserved empty ===\n
  hxprint d
  continue
end

break hxtest_gdb_break_hxflat_map_static
commands
  up
  echo \n=== hxflat_map static ===\n
  hxprint a
  continue
end

break hxtest_gdb_break_hxflat_set_dynamic
commands
  up
  echo \n=== hxflat_set dynamic ===\n
  hxprint a
  echo === hxflat_set unallocated ===\n
  hxprint b
  echo === hxflat_set reserved empty ===\n
  hxprint c
  continue
end

break hxtest_gdb_break_hxflat_set_static
commands
  up
  echo \n=== hxflat_set static ===\n
  hxprint a
  continue
end

break hxtest_gdb_break_hxhandle_table
commands
  up
  echo \n=== hxhandle_table ===\n
  hxprint a
  echo === hxhandle_table unallocated ===\n
  hxprint b
  continue
end

break hxtest_gdb_break_hxhash_table
commands
  up
  echo \n=== hxhash_table ===\n
  hxprint table
  continue
end

break hxtest_gdb_break_hxhash_table_edge_cases
commands
  up
  echo \n=== hxhash_table unallocated ===\n
  hxprint a
  echo === hxhash_table dynamic set ===\n
  hxprint b
  echo === hxhash_table static map ===\n
  hxprint c
  continue
end

break hxtest_gdb_break_hxlist
commands
  up
  echo \n=== hxlist ===\n
  hxprint list
  continue
end

break hxtest_gdb_break_hxlist_empty
commands
  up
  echo \n=== hxlist empty ===\n
  hxprint a
  echo === hxlist multi-field ===\n
  hxprint b
  continue
end

break hxtest_gdb_break_hxptr
commands
  up
  echo \n=== hxptr ===\n
  hxprint a
  echo === hxptr null ===\n
  hxprint b
  continue
end

break hxtest_gdb_break_hxref
commands
  up
  echo \n=== hxref ===\n
  hxprint a
  echo === hxref disengaged ===\n
  hxprint b
  continue
end

break hxtest_gdb_break_hxslot_map
commands
  up
  echo \n=== hxslot_map ===\n
  hxprint a
  echo === hxslot_map unallocated ===\n
  hxprint b
  continue
end

break hxtest_gdb_break_hxvector_dynamic
commands
  up
  echo \n=== hxvector dynamic ===\n
  hxprint a
  echo === hxvector unallocated ===\n
  hxprint b
  echo === hxvector reserved empty ===\n
  hxprint c
  continue
end

break hxtest_gdb_break_hxvector_static
commands
  up
  echo \n=== hxvector static ===\n
  hxprint a
  continue
end

run help runtests
quit
