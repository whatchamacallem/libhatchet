# Release Notes

## v1.53.0 2026-10-27

```text
- Added `hxkey_three_way` and three-way comparison for non-integral keys.
- Replaced `bool multi` with `int traits` and added `hxflat_multimap`.
- Added `add_range`, `push_back` and `emplace_back` to the flat map and set.
- Added `hxpair`, used for `hxflat_map` values and initializer lists.
- Renamed `hxmemory_manager.h` to `hxslab_allocator.h` along with its symbols.
- Added `hxcopy_range`, `hxmove_range` and `hxforward_range` with `_if` forms.
- Added `hxprofiler_write` for raw binary profiler samples.
- Added Google Test style test filters accepting `*` and `:` separated lists.

The tested Linux environments have changed. The following are now tested starting
with fresh installs in a new container. `testmsvc.ps1` uses `wslc.exe` to drive the
tests. `testwasm.exe` is only run when emcc is installed and `testmsvc.ps1` does
not install it for bandwidth reasons.

| Distribution     |    GCC | Clang | 32-bit | 64-bit |
| ---------------- | -----: | ----: | -----: | -----: |
| Ubuntu 26.04 LTS |     15 |    21 |      x |      x |
| Debian 13        |     14 |    19 |      x |      x |
| Rocky Linux 10   | 14, 15 |    21 |        |      x |

```

## v1.52.1 2026-09-06

```text
- Removed `hxvector::erase_if_heap`.
- Removed `hxoptional<T>`. Use `hxexpected<T,bool>`.
- Added `hxexpected<T, E=bool>`. Implements `std::expected<T, E>`.
- Added `hxset_algorithms.hpp`.
- Renamed `hxhandle_map` to `hxslot_map`.
- Renamed `hxalgorithm.hpp` to `hxrange.hpp`.
- Moved `hxbinary_search`, `hxlower_bound`, `hxupper_bound` moved from `hxsort.hpp` into `hxrange.hpp`.
- Use of `end()` more consistent with the standard throughout.
- Experimental safe monadic operations on container and reference types in C++23.
- Obviously this release contains breaking API changes.
```

## v1.51.0 2026-08-10

```text
- Add QUICKSTART.md which shows how to link and run a simple example.
- Remove `get()` calls as they are obsoleted by library hardening.
- Use `std::initializer_list` in more places to simplify the test suites.
- Make the `hxallocator` force inline.
- Add `hxarray::empty`.
```

## v1.50.3 2026-07-22

```text
- Optimized console code size. Removed the use of virtual and cut code duplication.
- Optimized test script job control. `testall.sh` runs 2x faster.
- Test scripts handle flags more consistently.
- `listsymbols.sh` now uses straight python instead of a mix of commands.
- Drop `hxfile::scan` because it was bloated and dogy. Use the console or roll your own.
- Refactored test suite to use a test object that tallies and confirms operator usage.
  - Identified and fixed Named Return Value Optimization (NRVO) failures.
```

## v1.49.5 2026-07-15

```text
- Add `hxhandle_map`. Maps 64-bits to a dense array of T, allowing for high frequency cache coherent iteration.
- Fix a few names in the test suite.
- Rename the method `set_table_size_bits` to `set_size_bits`.
- Build scripts use  `HX_.*_` namespace. Legal compiler args do not have the trailing underscore.
- The README is detailed and lists differences with the standard.
- Optimized the size of hxtest.
- Multithreaded the build scripts.
```

## v1.47.4 2026-07-10

```text
- Adds `hxhandle_table` a 64-bit handle table optimized for 32-bit platforms as well.
- Coverage test requires 100% line and file coverage with exceptions for a few things like triggering breakpoints.
- Made deleters const correct to reflect that they may not be thread safe. E.g. `hxfree_list`.
- Added `hxhash_table::replace`.
- Compacted the test suite so that tests for the same function go in the same test.
- Made container inlining explicit. Innermost loops are mostly flattened. Confirmed -Os is largely unchanged.
```

## v1.46.0 2026-07-08

```text
- Added `hxref` which mostly implements the P2988R12  draft proposal for `std::optional<T&>`.
- Added missing functionality to `hxoptional`.
- Added safe "monadic" functions from `hxoptional` to `hxptr`.
- Extended `testcoverage.sh` to print an AI friendly and IDE clickable list of lines missing coverage when it fails.
```

## v1.45.10 (v3.45.10) 2026-07-01

```text
- Tested with old and new compilers. The test scripts will need to be modified to work with older compilers.
  - GCC 10.2.1 and 16.0.1
  - Clang 11.0.1 and 22.1.2
  - MSVC 19.38.33145
  - emcc 5.0.3
- Substantially micro-optimized with Claude Fable 5.
- Add test coverage for the pretty printers.
- Silenced 3 -Wmaybe-uninitialized resulting from a gcc + optimizer + sanitizer bug and re-enabled the warning.
- Moved the debug build and release process to /tmp to reduce hd wear when that is RAM backed.
```

## v1.44.9 2026-06-27

```text
- Add type annotations to GDB scripts.
- Fix configuration of module.
- Add `HX_CYCLES_PER_SECOND` to configure CPU cycles per second.
- Test failure paths in the two `hxfile` implementations.
- Square up the docs further.
- Now checking for 100% line coverage in release process with 1% of code delimited with `GCOVR_EXCL_START`/`GCOVR_EXCL_STOP`.
- Add/test iterator type specifications and `hxforward` all non-relational callables in the codebase.
- Use branchless set operations.
```

## v1.43.13 2026-06-26

```text
- Make the memory manager more flexible and provide better diagnostics.
- Make the test scripts more flexible and less verbose when run as a batch.
- Add the commit SHA to the release tag message for extra verification.
- `archive.sh` now garbage collects and compacts `.git` before archiving it.
- Added `constexpr` everywhere as a tripwire to catch exceptions being thrown in exception unsafe code.
- Added `hxfreelist`. Provides a deleter following the same pattern as the standard library.
- All the containers can be passed a deleter at construction so they can reference the free list.
- Make `hxallocator` a private base class and wrap `data()` only as needed.
- Release tags contain `git describe` output stating the last tag, the number of patches in between and part of the SHA.
- Push release process now pushes docs to GitHub.
- Docs have been checked by AI.
```

## v1.43.14 2026-06-20

```text
- Make the memory manager more flexible and provide better diagnostics.
- Make the test scripts more flexible and less verbose when run as a batch.
- Add the commit SHA to the release tag message for extra verification.
- `archive.sh` now garbage collects and compacts `.git` before archiving it.
- Added `constexpr` everywhere as a tripwire to catch exceptions being thrown in exception unsafe code.
- Added `hxfreelist`. Provides a deleter following the same pattern as the standard library.
- All the containers can be passed a deleter at construction so they can reference the free list.
- Make `hxallocator` a private base class and wrap `data()` only as needed.
- Release tags contain `git describe` output stating the last tag, the number of patches in between and part of the SHA.
```

## v1.43.7 2026-06-17

```text
- Add `hxfree_list`. Provides a free list for the linked list and hash table classes.
- Extended the deleter syntax of those classes so nodes can be returned automatically.
- Now at 793 tests.
- Made temporary stacks dynamically allocated.
- Added reporting and testing of system allocator state.
- `archive.sh` garbage collects and repacks the repository.
- Made `testall.sh --headless` quiet enough for manual/AI review.
```

## v1.42.3 2026-06-15

```text
- Added `hxflat_map.hpp` and `hxflat_set.hpp`
- Make extensive use of `hxrestrict` everywhere possible. Avoids reloading fields.
- Switched from `size_t` to signed `hxsize_t` for container libraries to enable more optimizations.
- add constexpr to guard against use of expressions in code that does not support it.
- Clean up clang-tidy a bit and add more const.
- Add `hxif_constexpr` for backwards compatibility.
- Add ~200 more tests so that any operator/operation "mutation" in the code should cause a test to fail.
```

## v1.41.1 2026-06-13

```text
- Yet another AI correctness scan looking for trivia
- Split out `hxvector` and `hxarray`

  | | capacity > 0 | `hxallocator_dynamic_capacity` |
  | --- | --- | --- |
  | `hxarray` | Compile-time fixed size, inline storage | Variable initial size, non-resizable, non-reallocating heap storage [1] |
  | `hxvector` | Resizable, fixed capacity, inline storage | Resizable, variable initial capacity, non-reallocating heap storage |

  [1] Not provided by the standard.
```

## v1.40.3 2026-06-11

```text
- Fix everything identified in a Claude Fable scan of the codebase.
- Backport the code to g++-10 and clang-11 compiling as C++11.
- Note: The test scripts do not automatically detect old compilers.
```

## v1.39.5 2026-06-01

```text
- Fix build for Ubuntu 26.04 LTS (Resolute Raccoon)
- Exhaustive combinatorial testing of feature flags.
- CMakeLists.txt now provides a library when it is not top level. Provides a test suite otherwise.
- Add a test for Raspberry Pi Pico 2.
- Add checks that HX_USE_* macros are not empty.
```

## v1.38.8 2026-05-28

```text
- Made `hxlist` use the [XOR linked list](https://en.wikipedia.org/wiki/XOR_linked_list) pattern.
- `hxconst_list` has an identical API and is available for use with `constexpr` at compile time.
- The list classes now return iterators when adding nodes.
- GDB pretty printers are available for both.
- Fix minor issues with Clang.
- Added `HX_USE_LOGGING`, `HX_USE_FILE_IO` and `HX_USE_CONSOLE`.
- Bring hardening in line with the standard: Remove hardening from list iteration and do not have hardened null checks.
- Make it possible to iterate backwards over a list from `--list.end()` to `--list.begin()`.
- Basic support for the Raspberry Pi Pico 2 is in.
```

## v1.37.2 2026-05-26

```text
- Added option to optionally put the library in a namespace. C API and macros unaffected.
- hxarray::erase_if_unordered is now optimized named correctly.
- Removed the global operators from `hxrandom` on policy to avoid them spamming compiler error messages.
```

## v1.36.3 2026-05-22

```text
- Add Clang 20.1.8 module support.
- Module support also tested with GCC 15.2.0.
```

## v1.36.2 2026-05-22

```text
- Remove `hxtask_dag_node`'s dependency of `<stdatomic.h>`.
- Atomics are not provided as GCC does not support using its C version of `<stdatomic.h>` when compiling C++.
```

## v1.36.1 2026-05-20

```text
- Adds and tests `releaseprocess.sh`
- Cleaned up and added color to the example program.
```

## v1.36.0 2026-05-20

```text
- libhatchet now Builds as a C++20 module. See testexample.sh for meson usage. The build tools are barely ready for C++ modules though.
- hxthread_local no longer allocates memory, reducing cache misses and simplifying leak tracking.
- Made it explicit whether `new` and `delete` are implemented with `HX_PROVIDE_NEW_DELETE`.
```

## v1.34.0 2026-05-15

```text
- Add `meson.build` which is tested with `testexample.sh`.
- Adds `includecost.sh` which provides LOC and timing comparisons with the standard libraries.
```

## v1.32.0 2026-05-10

```text
- General polish pass.
- Tighten up `hxoptional` a bit.
```

## v1.30.0 2026-05-09

```text
- Clean up hxsettings.h to make `HX_USE_*` consistent
- Add POSIX file I/O.
```

## v1.24.3 2026-05-08

```text
- Make library hardening resemble the standard library. (I had it 9 years earlier!)
- Add `hxtask_dag_node`. This is an extremely lightweight replacement for the execution library.
- Made `__cxa_guard_acquire` thread safe after all. Use `-fno-threadsafe-statics` to disable.
- msvc.bat added and tested.
```

## v1.22.0 2026-04-12

```text
This is an "orphaned" branch that drops the 7 year git history of this project. There was obsolete image data and it was getting bloated.

The old history is here:  https://github.com/whatchamacallem/hatchling-platform
```

## v3.40.0 2026-03-19

```text
This is the last release of the Hatchling Platform.

It contains a final usability pass on the container node base classes.

Hatchling Platform has been renamed libhatchet and moved to: https://github.com/whatchamacallem/libhatchet
```

## v3.38.0 2026-03-15

```text
Adds hxrbtree. Provides intrusive set, map, multiset and multimap.

Adds hxlist. Provides an intrusive linked list.
```

## v3.36.0 2026-03-14

```text
Significant additions are `hxbitset`, `hxdeque` and an example program.

The `CLAUDE.md` file is starting to mature however it still generates overly complex code.

`.clang-tidy` is being enforced everywhere now.

The `hxfile::read` now requires the size of the destination buffer in keeping with modern security practices.
```
