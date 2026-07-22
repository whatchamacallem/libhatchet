# libhatchet

Use the most recent [tagged release](https://github.com/whatchamacallem/libhatchet).
The `include` and `src` directories report 100% line and branch coverage, with a
few exemptions made.

## Overview

> People say that you should not micro-optimize. But if what you love is
> micro-optimization... that's what you should do. - Linus Torvalds

libhatchet is a fast-compiling, lightweight, bespoke C17/C++23 alternative to
the C++ standard library, designed for cross-compilation to resource-constrained
targets such as DSPs, FPGAs, ASICs and WebAssembly. When a modern toolchain is
unavailable it falls back to a C++11 compiler and C99 libraries, and it never
depends on the C++ standard library. If you have a low-level mindset, the
developer experience is better than with the standard library. Template
instantiation errors are easier to read, `hxassertmsg` formats your assert
message before setting a breakpoint for you, and there is nothing unnecessary to
step through in the debugger.

<img src="libhatchet.jpg" alt="banner" width="200" height="200" align="right"
hspace="20">

Expect very fast compile times with `ninja`, `ccache`, `clang` and a C++ module.

The implementation stays clean under every sensible warning flag and under the
GCC and Clang sanitizers, and asserts are used generously. The process heap is
unused except when initializing system allocators.

## Key Features

- **Library hardening and asserts** are controlled by `HX_HARDENING_MODE`, for
  example `-DHX_HARDENING_MODE=HX_HARDENING_MODE_STANDARD`. See
  `<hx/libhatchet.h>` for the kinds of asserts available. Null pointer checks
  only happen at the debug level of hardening.

  - `HX_HARDENING_MODE_NONE` omits library hardening and disables every assert.
  - `HX_HARDENING_MODE_STANDARD` hardens while saving space by omitting verbose
    output. Meets the requirements of the C++ standard.
  - `HX_HARDENING_MODE_VERBOSE` adds verbose messages and suits internal
    releases. Meets the requirements of the C++ standard.
  - `HX_HARDENING_MODE_DEBUG` provides comprehensive asserts and verbose output.

- **Performance Focus**: This is systems code. Everything should be optimized
  and cache-coherent without causing code bloat. Exceptions and RTTI are avoided
  for efficiency. If exceptions are enabled and one gets that far, the test
  driver and the console will catch it. `__restrict` is used extensively, and
  inlining attributes flatten the inner loops, as is standard practice for
  standard libraries.

- **Portability**: libhatchet runs on top of any old embedded C99 library. musl
  libc is recommended for embedded Linux and is widely packaged:
  <https://musl.libc.org/>. No other C++ runtime or C++ code is required.
  Threading uses pthreads or C11's `<threads.h>`, both widely implemented
  standards. See `.clang-tidy` for a discussion of linting rules and portability
  concerns. Every public symbol other than methods and fields starts with `HX`
  or `hx`, and every non-public symbol in the headers ends with an underscore. A
  subset of the Google Test macros is also optionally provided.

- **Containers**: A set of containers designed for environments where
  reallocation is not allowed. Reach for them in low-level work where the
  standard container libraries cause code bloat, memory fragmentation and poor
  cache coherence. If those are not your concerns, consider using additional
  libraries. No tree data structures will be provided. Exceptions are not
  supported for efficiency reasons, so `noexcept` is applied widely to defend
  against data loss from the unexpected use of exceptions.

  There are two array classes, and both support two allocation modes:

  | | compile time capacity > 0 | `hxallocator_dynamic_capacity` |
  | --- | --- | --- |
  | `hxarray` | Compile-time fixed size, inline storage | Variable initial size, non-resizable, non-reallocating heap storage [1] |
  | `hxvector` | Resizable, fixed capacity, inline storage | Resizable, variable initial capacity, non-reallocating heap storage |

  [1] Not provided by the standard.

  See the headers at [include/hx/](include/hx/) for the remaining containers.
  They generally use the same names as the standard.

- **Algorithms**: The implementation uses the `__restrict` keyword wherever
  appropriate. See `<hx/hxalgorithm.hpp>` for the standard algorithms and
  `<hx/hxsort.hpp>` for comparison based sorting and lookup. Prefer radix
  sorting with `<hx/hxradix_sort.hpp>` when you want Θ(n) sorting. Less common
  functions have been omitted to reduce compile time. Random access iterators
  are required for a lot of things, but the only relational operator used is
  `<`. This codebase tries not to give an AI rope to hang itself with. Showing
  inner-loop assembly to an AI is also advised.

- **Memory Management**: Fast and deterministic. Several allocation semantics
  are supported, which matters most when crashing from memory fragmentation is
  unacceptable. If you make a lot of temporary allocations, expect roughly 30%
  memory and 30% performance improvements overall from adding just a few RAII
  scopes. Complex applications can use an array of variable sized stacks. To use
  tools like heaptrack, disable the memory manager with
  `-DHX_USE_MEMORY_MANAGER=0`. Leak tracking is provided.

- **Testing Framework**: A non-allocating, lighter, debuggable reimplementation
  of the core Google Test functionality.

- **Fast builds**: Lightweight headers result in very fast builds when used with
   `ninja` and `ccache`. The module loads quickly with Clang and MSVC.

- **Pretty Printers**: GDB-compatible pretty printers let debuggers and most
  code editors display container contents in a human-readable format.

- **C99 Compatibility**: Logging, asserts and memory management are available in
  plain C99 through `<hx/libhatchet.h>`.

- **AI Friendly**: Anything with the same name as the standard generally works
  the same way as the standard, so an AI can apply its existing knowledge of
  standard C++ directly. It also already knows how to use the test macros when
  writing tests. Using tabs instead of spaces reduces token use.

- **Console**: An embedded command processor that binds C++ functions
  automatically using templates. Use it for interactive target debugging without
  recompilation, for config files, or for configuration from the command line.
  The syntax is just `verb [arg ...]`.

- **Profiling System**: Samples processor cycles to build a hierarchical
  timeline capture compatible with Chrome's `chrome://tracing` viewer. Navigate
  the capture with the W, A, S and D keys. Uncommon hardware may need one line
  of assembly.

- **Task Queue**: An unopinionated task queue with priorities and a worker pool.
  An execution graph is also available as a layer on top.

- **Smart Pointers**: Because the focus is non-dynamic allocation, shared and
  weak pointers are not provided. Use `hxhandle_table` for weak references
  instead. `hxptr` implements `std::unique_ptr`. `hxoptional` implements
  `std::optional`, giving the semantics of an optional temporary allocation with
  safe access and without the allocation itself. `hxref` is a non-owning pointer
  wrapper with the same safe access semantics.

- **64-bit Ready**: Designed for both 32-bit and 64-bit targets. `hxsize_t` is
  signed because that enables important optimizations. It is equivalent to
  `ptrdiff_t`, but could be made 32-bit on a 64-bit platform if desired.

- **constexpr Ready**: C++11 `constexpr` is used where possible. Asserts, the
  algorithms, `hxconstexpr_list`, `hxbitset` and `hxrandom` support `consteval`
  in C++23.

## Documentation

The documentation is at <https://whatchamacallem.github.io/libhatchet>.

Run `doxygen` with no arguments to generate `docs/index.html`. The markdown
source for the documentation is in the header files at `include/hx/` and reads
well as-is. The stylesheet was tested with Doxygen 1.15.0.

Read `<hx/hxsettings.h>`. A number of important things can be configured on the
compiler command line, such as whether the library is wrapped in a namespace.

## Tested Environments

Almost every reasonable GCC and Clang warning flag should be safe to enable, and
Clang-Tidy is in use. Every release is tested against glibc and musl on Ubuntu
26.04 LTS using g++-16 and clang-22. g++-10 and clang-11 are tested periodically
with C++11. The latest MSVC 2022 should work with most warnings enabled,
although the MSVC static analyzer is not being tested. Run `debian_packages.sh`
to install the packages the test scripts need on a Debian based distribution
such as Ubuntu.

The `pico` directory contains a port to the Raspberry Pi Pico 2. There is almost
nothing to do when porting because libc provides almost all of the device
abstraction required. Some devices require you to call the C++ global
constructors yourself before calling `main()`. Check the sample code that came
with your board.

The scripted builds exercise the following toolchains, language modes and
`HX_HARDENING_MODE` combinations:

| Script | Toolchain | Modes | Hardening | Notes |
| --- | --- | --- | --- | --- |
| `debugbuild.sh` | `clang` | C17, C++23 | 3 | 32-bit debug build with ccache and no exceptions/RTTI. |
| `testcmake.sh` | `cmake` | C17, C++23 | 3 | Uses the default compiler and the real Google Test and runs `hxtest` and Clang-Tidy. |
| `testcoverage.sh` | `gcc` | C99, C++23 | 3 | Builds with `--coverage`, enables `HX_TEST_ERROR_HANDLING=1` and emits `coverage_details.html`. |
| `testmatrix.sh` | `gcc`, `clang` | C99, C17, C++11, C++23 | 0-3 | Sweeps optimization levels and ASan/UBSan/TSan/MSan and sets `HX_USE_THREADS=1/11`. |
| `teststrip.sh` | `musl-gcc` | C17, C++11-23 | 0 | Size-focused static build with allocator/library stripping. |
| `testwasm.sh` | `emcc` | defaults | 3 | WebAssembly build with the memory manager disabled and pthreads enabled. Emscripten defaults are Clang-based C/C++. |

`testall.sh` runs all of the above and also enforces certain naming conventions.

`testmsvc.bat` tests 32 and 64-bit debug and release configurations on Windows
and automatically discovers the installed version of MSVC.

`debugbuild.sh` and `teststrip.sh` detect when C++23 is missing and fall back to
C++20 to test g++-10 and clang-11.

## Project Structure

### Top Level

The test scripts are at the top level for easy access. Meson and CMake scripts
are also available.

- 🗀 `.vscode` - The vscode configuration files.
- 🗀 `example` - Simple program showing usage.
- 🗀 `gdb` - GDB pretty printers, one per container, loaded by `.gdbinit`.
- 🗀 `include` - Add this directory to your include path.
  - 🗀 `hx` - The public `<hx/hx*>` headers, one class per header.
    - 🗀 `detail` - Internal implementation headers.
- 🗀 `src` - The runtime implementation. Add these files to your build.
- 🗀 `test` - Optional GoogleTest-style test suites.

### Core layer

- `libhatchet.h` is the entry point. It is also the header to use from C code.
  It provides the core macros, the assert family, memory management and feature
  detection, and it pulls in `hxsettings.h` and `hxmemory_manager.h`, which
  cannot be included directly.
- Choose an `HX_HARDENING_MODE` on the compile line. `hxassert` and
  `hxassertmsg` are active only at `DEBUG`, `hxassert_hard` at `STANDARD` and
  above, and `hxassert_always` in every mode. Null pointer checks only happen in
  `DEBUG`.
- Log through `hxlog`, `hxlog_warning`, `hxlog_release` and `hxlog_console`.
  They all route through `hxlog_handler`, which can be replaced.
- `hxsettings.h` handles compiler detection and polyfills and documents the
  default for every compile-line option. Set `HX_USE_NAMESPACE=<identifier>` to
  wrap the whole library in a namespace of your choosing.
- `hxmemory_manager.h` allocates by allocator ID. `hxsystem_allocator_heap` is
  the normal heap, `hxsystem_allocator_permanent` is never freed, and
  `hxsystem_allocator_stack_0 + n` selects a temporary stack that resets when
  the enclosing RAII `hxsystem_allocator_scope` closes. `hxmalloc_ext` takes an
  explicit ID, while plain `hxmalloc` uses the current scope.
- `hxutility.h` provides the metaprogramming basics: `hxmove`, `hxforward` and
  the type traits.
- `hxinitializer_list.hpp` either provides `std::initializer_list` or wraps the
  system version, depending on `HX_USE_LIBCXX`.

### Containers

Differences from the standard versions are listed.

- `hxallocator` is the static-or-dynamic capacity backing used by the
  containers. Prefer static capacities where allocation is a concern.
- `hxarray` is the fixed-size `std::array` analog. `hxvector` covers both
  `std::vector` and `std::inplace_vector`. Both have the algorithms as methods:
  `find`, `find_if`, `all_of`, `any_of`, `for_each`, `binary_search`, `sort`
  and `insertion_sort`. Both provide `get(index)` returning `hxnull` when out
  of range, `size_bytes`, `memcpy`/`memset` fills and construction and
  assignment from C-style arrays. `hxvector` adds `full`, Python-style
  `operator+=` append and concatenate, `generate_n`, Θ(1) `erase_unordered`
  and `erase_if_unordered`, and `make_heap`/`push_heap`/`pop_heap`/
  `erase_if_heap`, which replace `std::priority_queue`. An `hxvector` also
  works directly as an output iterator, replacing
  `std::back_insert_iterator`.
- `hxdeque` is a fixed-capacity ring buffer with a power-of-two capacity.
  Every operation, including `operator[]`, is Θ(1), and `full` is provided.
- `hxbitset` is a fixed-size bit set stored in `size_t` words with no heap use.
  Unlike `std::bitset` it exposes the underlying words via `data` and `bytes`
  and loads raw bytes with `load`. Shifts and bitwise operators are provided.
- `hxlist` is an intrusive doubly linked XOR list at half the size of a
  conventional one. `hxconstexpr_list` has the same interface but uses normal
  pointers so it works in constexpr code. Nodes are owned through a
  configurable deleter, and subclasses of the node type may be stored
  heterogeneously. `extract`, `pop_front` and `pop_back` return an owning
  `hxptr`, `release_all` abandons ownership, and `remove_if`, `splice` and
  `reverse` are provided.
- `hxflat_map` and `hxflat_set` are sorted parallel-array replacements for
  map/multimap and set/multiset. Expect O(log2(n)) lookups and O(n) insertions
  and removals. `find` returns a pointer to the mapped value or `hxnull`
  instead of an iterator, and elements are addressable by position with
  `operator[]` and `get`.
- `hxhash_table` is a fixed-size hash table with singly linked buckets embedded
  in the nodes. Additional node types are in `hxhash_table_nodes.hpp`.
  `find(key, previous)` walks duplicate keys without `equal_range`, `replace`
  swaps a node in with a single lookup, `extract` returns an owning `hxptr`,
  and `release_all` and `release_key` unlink nodes without deleting them.
  `load_factor` and `load_max` report bucket usage.
- `hxhandle_table` maps handles to pointers and `hxhandle_map` maps handles to
  values. Both use 64-bit generational handles, so use them when stale
  references need to be detected. Both provide `erase_if` and `extract`.
  `hxhandle_map` is a slot map keeping its values contiguous for iteration.
- `hxfree_list` is a fixed-capacity free-list allocator built on `hxallocator`.
  `allocate` and `try_allocate` construct a `T` and return an `hxptr` whose
  deleter returns the slot to the pool. `is_allocator` tests whether a pointer
  came from the pool.
- `hxoptional` implements `std::optional<T>`, `hxref` implements
  `std::optional<T&>`, which the standard does not provide before C++26, and
  `hxptr` is a unique owning pointer with a monadic `and_then` and a deleter
  that may decline deletion, e.g. `hxdo_not_delete`.
- Every keyed container and sort compares keys through the free functions
  `hxkey_equal`, `hxkey_less` and `hxkey_hash` in `hxkey.hpp`. Overload them for
  custom key types. The defaults require only `==` and `<`.
- Containers with dynamic capacity move and `hxswap` in Θ(1) without touching
  elements.

### Algorithms

Differences from the standard versions are listed.

- `hxalgorithm.hpp` provides searching and set utilities taking callables:
  `hxall_of`, `hxany_of`, `hxcount_if`, `hxexchange`, `hxfind_if`,
  `hxfor_each`, `hxmerge`, `hxminmax`, `hxset_difference`,
  `hxset_intersection`, `hxset_union` and `hxunique`. `hxmerge` and the set
  operations move-assign out of their inputs and accept an `hxvector`
  reference directly as the output iterator, appending to it without
  `std::back_inserter`, e.g. `hxmerge<const int*, hxvector<int>&>(...)`.
  `hxminmax` returns a result with `min` and `max` iterator fields in a
  single pass.
- `hxsort.hpp` provides the comparison sorts and lookup. `hxsort` is an
  introsort, and `hxinsertion_sort` and `hxheapsort` are also available.
  `hxbinary_search`, `hxlower_bound` and `hxupper_bound` are provided, and
  unlike `std::binary_search`, `hxbinary_search` returns an iterator to the
  first match instead of `bool`.
- Every comparison routes through the `hxkey_less` and `hxkey_equal` free
  functions or an explicit callable, so custom key types overload two free
  functions once instead of passing comparators everywhere.
- For scalar keys up to 32 bits, `hxradix_sort` sorts in Θ(n) time. Its
  implementation is in `src/hxradix_sort.cpp`. It sorts
  `hxradix_sort_key<key_t, value_t>` pairs and handles signed and `float`
  keys with bit manipulation. `hxradix_sort11` uses 11-bit digits for large
  workloads, and the temporary buffers come from a caller-selected memory
  manager allocator ID.
- `hxrandom` is a 64-bit LCG intended for generating test data. Beyond
  `operator()` it provides `u8`/`u16`/`u32`/`u64`, `f01`/`d01` in `[0..1)`,
  `range(base, size)` overloads that use a floating point multiply instead
  of integer modulo, `read` to fill a buffer with random bytes, and
  independently seeded streams. It works in `constexpr` code, so random test
  data can be built at compile time.

### Runtime Facilities

- `hxfile` is RAII file I/O. `HX_USE_FILE_IO` selects the backend: `1` is the
  libc backend in `src/hxfile_c.cpp` and `2` is the POSIX backend in
  `src/hxfile_posix.cpp`.
- `hxconsole` is a debug and remote command console that also parses config
  files such as `example.cfg`. Register bindings at file scope with
  `hxconsole_command()` and `hxconsole_variable()`, or use the `_named` variants
  to choose the name yourself.
- `hxprofiler` is a cycle-sampling scope profiler that emits `chrome://tracing`
  captures. Mark a scope with `hxprofile_scope("label")` and control capture
  with `hxprofiler_start()`, `hxprofiler_stop()` and `hxprofiler_log()`. It
  compiles away entirely unless `HX_USE_PROFILER=1`.
- `hxtask` and `hxtask_queue` form a worker-pool priority queue.
  `hxtask_dag_node` layers DAG dependencies on top.
- `hxthread` wraps pthreads or C11 `<threads.h>` with `hxmutex`,
  `hxcondition_variable`, `hxunique_lock`, `hxthread` and `hxthread_local`.
- `hxtest` is the GoogleTest-compatible subset that the test suite itself runs
  on. Set `HX_USE_GOOGLE_TEST=1` to run the same tests against the real Google
  Test.

## Not Provided

This project was started for the author's own personal use, and it tries to be
complete enough for ordinary C++ programmers. It predates a lot of similar
functionality in the standard. If you find something missing, odds are your
favorite AI already knows how to add it, or it was omitted because the C library
was deemed sufficient.

That said, some functionality of the C++ standard library is not worth
reimplementing here. If you need these things, use the standard library shipped
with your compiler.

- Atomics. The C version of `<stdatomic.h>` is incompatible with g++.
- The iterators library. This codebase intentionally deemphasizes iterators.
- The ranges library. This would be a large and pointless rewrite.
- The strings library. Strings are allocation intensive. See the `{fmt}`
  project.

## Other Projects

- **[musl libc](https://musl.libc.org/)** - The recommended C library for using
  libhatchet in a freestanding environment.
- **[{fmt}](https://fmt.dev)** - A micro-optimized version of `std::format`,
  with nice extras like console colors and a fast `printf`.

## License

© 2017-2026 Adrian Johnston. This project is licensed under the terms of the MIT
license found in the `LICENSE.md` file.

🪓🪓🪓
