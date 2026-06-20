# libhatchet

## Agent Interaction

Always number separate items in any analysis so they are easy referenced.

Do not assume existing code or documentation is "intentionally so" and instead
consider rewrites, redesign and updating documentation as preferred to
maintaining existing design whenever that aligns with requests. Existing code
and documentation may have been recently written by an agent in an exploratory
mode, and therefore preserving it may actually hinder iterative design.

when asked to update documentation and tests assume existing bug free code is
correct when it conflicts with documentation or tests. If bugs or compile errors
are found outside of tests when asked to update tests then fix them first before
proceeding.

Do not add features or functionality that has not been explicitly requested. If
additional functionality is advisable present the user with a numbered list.

## Style Guide

This is a bespoke C17/C++20 alternative to the C++ standard library. Never use
the `std` namespace. The ranges library should not be implemented and do not go
looking for a `std::span` equivalent. Symbols starting with `hx` that have the
same name as symbols in the standard library are generally functionally
equivalent. E.g. use `hxforward` instead of `std::forward`. Standard
functionality is often available. Prefer `size_t` for sizes and array indexing.

Do not use C++ exceptions, RTTI or assume asserts are enabled. Check when adding
includes whether they are redundant and write them as `<stdio.h>` not
`<cstdio>`. Generously use `hxassertmsg` for debug asserts and `hxassert_harden`
for release asserts. Declare and define functions with no args as `void x(void)`
instead of `void x()`.

Read `.clang-tidy` when writing new code as it is in use and C-style casts are
not allowed. The rules are only checked by `testcmake.sh` and are not checked by
`vscode`. Declare local variables `const` when they are not modified. Prefer
`1000u` to `(size_t)1000`.

Prefer code that avoids unnecessary function calls or requires unnecessary
traversal of data structures in the debugger watch window. In particular avoid
writing simple one line helper functions. Except prefer delegating constructors
to repeating member initializers. Never implement hypothetical safety guarantees.
The best code is no code.

Do not use defensive programming or guard against mistakes. Unintended use cases
need to be identified and fixed instead of guarded against. Compile errors are
cheaper than debugging. Asserts are cheaper than debugging. Use `hxassertmsg` to
document preconditions and post conditions instead of null checks.

Wrap C-style implementation details in C++ classes with normal operators so that
C++ object models are used for interfaces. However, use hxarray with a static
capacity instead of large C-style arrays. Use template wrappers for type safety
while avoiding the associated code bloat.

Separate code onto individual lines when it helps step through expressions
individually in the debugger. Use references instead of pointers when a pointer
would never be null. Use tabs instead of spaces. Use K&R style.

Make any function or operator implementation that will not fit on one line in
100 chars an out of line implementation that goes after the class body.
Alphabetize all the functions, keep operators first. Place the opening brace of
a function body on the same line as the function signature. When creating a new
class do not use global operators and use member functions instead.

Use C-style headers and not the C++ wrappers around them. E.g. use `<math.h>`
not `<cmath>`. The goal is to be able to compile against libc alone without
using the C++ standard library at all.

## Naming

All symbols are `snake_case`. Except feature test macros and constants are
`SCREAMING_SNAKE_CASE`. Do not use abbreviated names except for iterators.

Classes, structs and functions begin with `hx` and not `hx_`. Template
parameters are `snake_case` and end with `_t_`. Use `struct` only in C code.
Function parameters and private fields do not begin with `hx` and end with an
underscore. Private fields begin with `m_`. Global variables start with `hxg_`
and static or anonymous namespace variables start with `hxs_`. Prefix calls to
the C standard library with `::` to indicate they are in the global namespace.
Use `src_` and `dst_` for source and destination iterators.

## Optimization

Prefer C-style implementation details that are cache coherent. Minimize branches
and memory indirection. Avoid division. Prefer iterating with pointers marked
`hxrestrict` instead of using array indices. Warn when working with tight loops
that will not be unrolled and suggest `hxattr_hot`. Use `size_t` to avoid
unnecessary 64-bit operations on 32-bit builds. Count CPU instructions required
for ARM M-profile and RISC-V bare metal processors and choose the cheapest
implementation. Pack classes and structures for minimum space use.

## Testing

Use `debugbuild.sh` to test changes by default and not cmake. 🪓🪓🪓 output
indicates success. When asked to run tests execute `build/hxtest` with `build`
as the current directory. Consider all `.sh` files in the project except
`ubuntu_packages.sh` safe to run at any time. On Windows, fall back to the VS
Code CMake win32 debug task.

Compiling tests with a C++11 compiler against C99 libraries is required. Support
for `ILP32`, `LP64` and `LLP64` are required to pass tests. All
`HX_HARDENING_MODE` levels are required to pass tests. Do not use compiler
builtins as this code is intended to compile on any C++ compiler.

Do not write test suites until requested as the design may not be finalized.
Every branch needs to be tested both ways with off by one tests and also do
white box testing that ensures the documentation is being followed. All test
symbols that show up in the linker map must contain `hx` and `test`.

Do not generate tests that are entirely redundant with other tests or do not
meaningfully test real code.  e.g. Do not compare the return value of a function
called on an object with the return value of the same function called on a
reference to the same object.

Ignore spell checker errors.

Never fix a failing test except when instructed to do so or those failures are a
direct consequence of changes you are making. Do not fix a failing test in a
manner that defeats the intent of the test except by removing it entirely.
Prompt the user with a list of failing tests when they are unrelated to your
work or the intent preserving fix is unclear.

Prefer `EXPECT_*` macros to `ASSERT_*` macros unless the failure looks like it
will cause memory corruption or other failures in subsequent tests.

## Debugging

Debug non-obvious test failures, asserts, and crashes with GDB. When debugging,
build with `debugbuild.sh` without `--headless`. Then run GDB in batch mode
passing `-x .gdbinit` explicitly to load pretty printers, with `-ex "run"` and
`-ex "bt"` to capture the backtrace, and `build/hxtest` as the target with no
args (do not use `--gtest_filter`). Both `hxassert()` and `hxbreakpoint()` will
raise `SIGTRAP` and can be added temporarily to stop execution at a specific
point. Do not look for a core dump on WSL as cores are discarded.

## Documentation

Update documentation independently only when making changes that obsolete it. Do
not add documentation describing reasons for making changes, e.g. instructions
given, issues resolved or bugs fixed. Remove trailing `_` from symbols in
doxygen comments and leave them in regular comments and follow existing style
otherwise. Do not use `;` or `-` in documentation unless it is part of a code
block or the doxygen formatting shown below. Documentation will be explicitly
requested when the design is final. Usage examples in documentation are not
expected to follow the preceding rules and are instead examples of code written
independently outside of the project. Put comments on preceding lines instead of
on the same line as code.

Describe only the expectation enforced by asserts in documentation instead of
explicitly describing the asserts themselves. If the assert just enforces a
routine invariant than do not document it at all.

Wrap all documentation except parameter documentation at 80 columns. Begin
function documentation by describing the return value on the stack if not
`void`. All parameters require parameter documentation. Use the following
Doxygen style:

```c++
/// `hxabs` - Returns the absolute value of `x` using a `<` comparison.
/// - `x` : The value to compute the absolute value of.
```

## Document Structure

Do not reorder major sections of code unless asked. This codebase does not put
translation unit local declarations and definitions close to where they are used
but instead places them near the top of the file to be immediately visible to
reviewers. E.g. at most one anonymous namespace at the top of a translation unit
should normally be needed.

All text files must end with `\n` or `\r\n`.

## Project Structure

Symbols that are included belong in the `include/hx` directory. Symbols that are
internal generally belong in the `include/hx/detail` directory and end with an
additional `_` if not already present.

All tests go in the `test` directory. Symbols in the `test` and `example`
directory never end with an `_` and this rule overrides the rules above in order
to show that internal symbols are not used when testing the APIs. This applies
to all symbols including local variables and function parameters.

Use file globs to discover files in the src directory instead of listing them.

## File Index

If the `tags` file generated by ctags doesn't exist, generate it:

```bash
ctags -R --fields=+n --languages=C,C++ -f tags include example src test
```

After modifying a source file, update only that file, where `file` is replaced
with the filename:

```bash
ctags -a --fields=+n --languages=C,C++ -f tags file
```

Always grep the `tags` file generated by ctags as shown below to find symbol
definitions, where `symbol` is replaced with the symbol name:

```bash
grep $'^symbol\t' tags
```
