# libhatchet

Please use the most recent [tagged release](https://github.com/whatchamacallem/libhatchet). It has been exhaustively tested.

## Overview

> People say that you should not micro-optimize. But if what you love is
> micro-optimization... that's what you should do. - Linus Torvalds

libhatchet is a fast compiling lightweight, bespoke C17/C++23 alternative to the
C++ standard library designed for cross-compilation to resource-constrained
targets like DSPs, FPGAs, ASICs or WebAssembly. It also falls back to requiring
only C99 libraries and a C++11 compiler, and deliberately avoids dependencies on
the C++ standard library. For those with a low-level mindset, the developer
experience is better than with the C++ standard library. For example, the
template instantiation errors are easier to read, and `hxassertmsg` will format
your assert messages before setting a breakpoint for you. There is nothing
unnecessary to step through in the debugger.

<img src="libhatchet.jpg" alt="banner" width="200" height="200" align="right" hspace="20">

Compile times are shockingly fast when using `ninja`+`ccache` without needing
precompiled headers. A C++20 module is also available in the src directory and
is recommended although modules load a little slower with GCC.

The implementation maintains compatibility with every sensible warning flag and
with sanitizers for both GCC and Clang. Of course, asserts are also widely used.
The implementation also avoids dynamic allocations except when initializing
system allocators.

## Key Features

- **Library hardening and asserts** are controlled via `HX_HARDENING_MODE`. E.g.
  usage is `-DHX_HARDENING_MODE=HX_HARDENING_MODE_STANDARD`. See
  `<hx/libhatchet.h>` for the different kinds of asserts available. Note: Checks
  against `null` are only provided at the debug level of hardening.

  - `HX_HARDENING_MODE_NONE`: Omits library hardening and disables all asserts.
  - `HX_HARDENING_MODE_STANDARD`: Provides hardening but saves space by omitting
    verbose output. Meets the requirements of the C++ standard.
  - `HX_HARDENING_MODE_VERBOSE`: Provides verbose messages, suitable for
    internal release. Meets the requirements of the C++ standard.
  - `HX_HARDENING_MODE_DEBUG`: Provides comprehensive asserts and verbose
    output.

- **Portability**: libhatchet can easily be made to run on top of any old
  embedded C99 library. musl libc is recommended for embedded Linux and is
  widely packaged: <https://musl.libc.org/>. No other C++ runtime or C++ code is
  required. pthreads or C99's `<thread.h>` may be used for threading, which are
  widely implemented standards. See `.clang-tidy` for a discussion of linting
  rules and portability concerns. All public symbols (aside from methods and
  fields) start with either `HX` or `hx`. All non-public symbols in the headers
  end with an underscore. A subset of the Google Test macros is also optionally
  provided.

- **Profiling System**: Uses processor cycle sampling to create a hierarchical
  timeline capture compatible with Chrome's `about://tracing` viewer (navigation
  uses W, A, S, and D keys). One line of assembly may be needed for uncommon
  hardware.

- **Memory Management**: Fast and deterministic. Supports various allocation
  semantics, particularly valuable for applications where crashing from memory
  fragmentation is unacceptable. If you have a lot of temporary allocations,
  this system reasonably offers 30% memory and 30% performance improvements
  overall with just a few RAII scopes added. Complex applications are supported
  using an array of variable sized stacks. Fancy tools like heaptrack can also
  be used by disabling the memory manager with `-DHX_USE_MEMORY_MANAGER=0`. Leak
  tracking is provided.

- **Console**: Provides an embedded command processor with automatic C++
  function binding using templates. Useful for interactive target debugging
  without recompilation and also provides support for config files or
  configuration via the command-line. Syntax is just `verb [arg ...]`.

- **Testing Framework**: Safer, lighter, debuggable reimplementation of core
  Google Test functionality.

- **Task Queue**: An unopinionated task queue with priorities and a worker pool.
  An execution graph is also available as a layer on top.

- **Containers**: Provides a set of containers designed for environments where
  reallocation is not allowed. This codebase is intended for low-level work
  where the standard container libraries cause code bloat, memory fragmentation,
  and poor cache coherence. If those are not your concerns, consider using
  additional libraries. No tree data structures will be provided. For efficiency
  reasons exceptions are not supported and therefore `noexcept` is widely
  applied to defend against data loss due to the unexpected use of exceptions.

  There are two array classes that both support two allocation modes:

  | | compile time capacity > 0 | `hxallocator_dynamic_capacity` |
  | --- | --- | --- |
  | `hxarray` | Compile-time fixed size, inline storage | Variable initial size, non-resizable, non-reallocating heap storage [1] |
  | `hxvector` | Resizable, fixed capacity, inline storage | Resizable, variable initial capacity, non-reallocating heap storage |

  [1] Not provided by the standard.

  See the headers at [include/hx/](include/hx/) for the remaining containers.
  They generally use the same names as the standard.

- **Pretty Printers**: Implements GDB-compatible pretty printers, enabling
  debuggers and most code editors to display container contents in a
  human-readable format.

- **Algorithms**: This implementation is designed to use the `__restrict`
  keyword wherever appropriate. See `<hx/hxalgorithm.hpp>` for standard
  algorithms and `<hx/hxsort.hpp>` for comparison based sorting and lookup.
  Radix sorting with `<hx/hxradix_sort.hpp>` is recommended for Θ(n) sorting.
  Less common functions have been omitted to reduce compile time. Random
  access iterators are required for a lot of things but the only relational
  operator used is `<`.

- **Performance Focus**: This is systems code. Everything should be optimized
  and cache-coherent without causing code bloat. This codebase avoids exceptions
  and RTTI for efficiency. Exceptions will be caught by the test driver and the
  console if they are enabled and make it that far.

- **C99 Compatibility**: Logging, asserts, and memory management are available
  in plain C99 via `<hx/libhatchet.h>`.

- **64-bit Ready**: Designed for both 32-bit and 64-bit targets.

- **Fast builds**: Blazing fast builds when used with `ccache` and no `.pch`.
  `ninja` + `ccache` is recommended when using `cmake`. Precompiled headers have
  surprisingly long load times and break `ccache`'s optimizations. One warning
  though, this is also accomplished by skipping less commonly used features.

- **AI Friendly**: Things with the same name as the standard generally work the
  same way as the standard. This means that AI is easily able to apply its
  existing knowledge of standard C++. It also already knows how to use the test
  macros when writing tests. Using tabs instead of spaces reduces token use.

- **constexpr ready**: C++11 `constexpr` are used where possible. Asserts,
  algorithms, `hxconstexpr_list`, `hxbitset` and `hxrandom` support `consteval`
  in C++23.

## Documentation

The documentation is at
<https://whatchamacallem.github.io/libhatchet>.

Running the command `doxygen` with no arguments will generate `docs/index.html`.
The markdown source for the documentation is in the header files at
`include/hx/` and is also readable as-is. The stylesheet was tested with Doxygen
1.15.0.

Reading `<hx/hxsettings.h>` is recommended. There are a number of important
things that can be configured on the compiler command line. E.g. whether or not
the library is wrapped in a namespace.

## Not Provided

This project was started for the author's own personal use, and it tries to be
complete enough for ordinary C++ programmers. It was started before a lot of
similar functionality was added to the standard. However, if you find something
missing, odds are your favorite AI already knows how to add it, or it was
omitted because the C library was deemed sufficient.

That said, some functionality of the C++ standard library is not worth
reimplementing here. If you need these things you are advised to use the
standard library shipped with your compiler.

- Atomics. The C version of `<stdatomic.h>` is incompatible with g++.
- Iterators Library. This codebase intentionally deemphasizes iterators.
- Ranges Library. This would be a large and pointless rewrite.
- Strings Library. These are allocation intensive. See the `{fmt}` project.

## Other Projects

- **[musl libc](https://musl.libc.org/)** - This is the recommended C library
  for use with libhatchet in a freestanding environment.
- **[{fmt}](https://fmt.dev)** - Has a micro-optimized version of `std::format`.
  Also has nice features like console colors and a fast `printf` too.

## Tested Environments

Almost every reasonable GCC and Clang warning flag should be safe to enable.
Clang-Tidy is also in use. The environment tested with every release is glibc
and musl on Ubuntu 26.04 LTS using g++-16 and clang-22. g++-10 and clang-11 are
periodically tested with C++11. The latest MSVC 2022 should be working with most
warnings enabled as well. Although the MSVC static analyzer is not being tested.
See `debian_packages.sh` to install the required packages for the test scripts
on a Debian based distribution (e.g. Ubuntu).

The `pico` directory contains a port to the Raspberry Pi Pico 2. Note how there
is almost nothing to do when porting as libc provides almost all of the device
abstraction required. Some devices may require you to explicitly call the C++
global constructors before calling `main()` yourself. Check the sample code that
came with your board.

The scripted builds exercise the following toolchains, language modes, and
`HX_HARDENING_MODE` combinations:

| Script | Toolchain | Language Modes | `HX_HARDENING_MODE` | Notes |
| --- | --- | --- | --- | --- |
| `debugbuild.sh` | `clang`/`clang++` | C17, C++23 | 0 | 32-bit debug build with ccache and no exceptions/RTTI. |
| `testcmake.sh` | `cmake` + default compiler | C17, C++23 | 0 (default) | Uses the real Google Test and runs `hxtest` and Clang-Tidy. |
| `testcoverage.sh` | `gcc`, `g++` + `--coverage` | C99, C++23 | 0 | Enables `HX_TEST_ERROR_HANDLING=1` and emits `coverage.html`. |
| `testmatrix.sh` | `gcc`, `clang` (ASan/UBSan) | C99, C17, C++11, C++23 | 0-3 | Sweeps optimization levels and sets `HX_USE_THREADS=0/1/11`. |
| `teststrip.sh` | `musl-gcc` (static) | C17, C++11/14/17/20 | 3 | Size-focused static build with allocator/library stripping. |
| `testwasm.sh` | `emcc` | Emscripten defaults (Clang-based C/C++) | 0 (default) | WebAssembly build with allocator disabled and single-thread mode. |

`testall.sh` runs all of the above and also enforces certain naming conventions.

`testmsvc.bat` tests 32 and 64-bit configurations of debug and release on
Windows. The currently installed version of MSVC should be automatically
discovered.

## Project Structure

The scripts are at the top level for easy access.

- 📁 `.vscode` - The vscode configuration files.
- 📁 `example` - Simple program showing usage.
- 📁 `gdb` - GDB scripts for the container classes.
- 📁 `include` - This is the directory to add to your include path.
  - 📁 `hx` - These are all the `<hx/hx*>` header files.
    - 📁 `detail` - These are internal header files.
- 📁 `src` - C/C++ files that have to be added to your build.
- 📁 `test` - An optional test suite.

## License

© 2017-2026 Adrian Johnston. This project is licensed under the terms of the MIT
license found in the `LICENSE.md` file.

🪓🪓🪓
