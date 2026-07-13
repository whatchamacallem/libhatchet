// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

// Module for libhatchet. Requires C++20 or later. Use `import hx;` to import
// instead of individual `#include` directives. `<hx/libhatchet.h>`,
// `<hx/hxconsole.hpp>`, `<hx/hxprofiler.hpp>` and `<hx/hxtest.hpp>` may be
// included separately in the same translation unit in order to use their macros
// as long as `HX_USE_MACROS_WITH_MODULE=1`.

module;
#include <errno.h>
#include <fenv.h>
#include <limits.h>
#include <math.h>
#include <signal.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef __wasm__
#if __has_include(<x86intrin.h>)
#include <x86intrin.h>
#elif __has_include(<intrin.h>)
#include <intrin.h>
#endif
#endif

#if !defined HX_USE_THREADS || (HX_USE_THREADS)
#if __has_include(<threads.h>)
#include <threads.h>
#endif
#if __has_include(<pthread.h>)
#include <pthread.h>
#endif
#endif

// <stdio.h> is already included and HX_USE_FILE_IO defaults to 1.
#if defined HX_USE_FILE_IO && (HX_USE_FILE_IO) == 2
#include <fcntl.h>
#include <unistd.h>
#endif

// HX_USE_LIBCXX defaults to 1.
#if !defined HX_USE_LIBCXX || (HX_USE_LIBCXX)
#include <new>
#include <initializer_list>
#endif

#if defined HX_USE_GOOGLE_TEST && (HX_USE_GOOGLE_TEST)
#error HX_USE_GOOGLE_TEST is set and Google Test 1.15.2 does not compile as a module.
#endif

// Modules are not allowed to override globals. HX_PROVIDE_NEW_DELETE=-1 is used
// when compiling a module to prevent declaration.
#define HX_PROVIDE_NEW_DELETE -1

// Out of line declarations (.inl files) have to be outside the export block.
#ifdef HX_USE_NAMESPACE
#define HX_BEGIN_INL_ } } namespace HX_USE_NAMESPACE {
#define HX_END_INL_ } export { namespace HX_USE_NAMESPACE {
#else
#define HX_BEGIN_INL_ }
#define HX_END_INL_ export {
#endif

// hxmemory_manager.h and hxsettings.h are not meant to be included directly.
export module hx;
export {
#include "../include/hx/libhatchet.h"
#include "../include/hx/hxalgorithm.hpp"
#include "../include/hx/hxallocator.hpp"
#include "../include/hx/hxarray.hpp"
#include "../include/hx/hxvector.hpp"
#include "../include/hx/hxbitset.hpp"
#include "../include/hx/hxconsole.hpp"
#include "../include/hx/hxconstexpr_list.hpp"
#include "../include/hx/hxdeque.hpp"
#include "../include/hx/hxfile.hpp"
#include "../include/hx/hxfree_list.hpp"
#include "../include/hx/hxflat_map.hpp"
#include "../include/hx/hxflat_set.hpp"
#include "../include/hx/hxhandle_map.hpp"
#include "../include/hx/hxhandle_table.hpp"
#include "../include/hx/hxhash_table.hpp"
#include "../include/hx/hxhash_table_nodes.hpp"
#include "../include/hx/hxinitializer_list.hpp"
#include "../include/hx/hxkey.hpp"
#include "../include/hx/hxlist.hpp"
//#include "../include/hx/hxmemory_manager.h"
#include "../include/hx/hxoptional.hpp"
#include "../include/hx/hxprofiler.hpp"
#include "../include/hx/hxptr.hpp"
#include "../include/hx/hxradix_sort.hpp"
#include "../include/hx/hxref.hpp"
#include "../include/hx/hxrandom.hpp"
//#include "../include/hx/hxsettings.h"
#include "../include/hx/hxsort.hpp"
#include "../include/hx/hxtask.hpp"
#include "../include/hx/hxtask_dag_node.hpp"
#include "../include/hx/hxtask_queue.hpp"
#include "../include/hx/hxtest.hpp"
#include "../include/hx/hxthread.hpp"
#include "../include/hx/hxutility.h"
}

#include "./hxconsole.cpp"
#include "./hxfile_c.cpp"
#include "./hxfile_posix.cpp"
#include "./hxmemory_manager.cpp"
#include "./hxprofiler.cpp"
#include "./hxradix_sort.cpp"
#include "./hxsettings.cpp"
#include "./hxtask_queue.cpp"
#include "./hxtest.cpp"
#include "./hxutility.cpp"
#include "libhatchet.cpp"
