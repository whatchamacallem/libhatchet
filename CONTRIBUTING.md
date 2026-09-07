# libhatchet

## Everyone

"Tradition is not the worship of ashes. Tradition is the preservation of fire."

CONTRIBUTING.md is symbolically linked to by AGENTS.md and CLAUDE.md. It reads
like it was written for a robot because it was. Humans are invited to skim this
document before sending patches.

## AI Agents

Never assume existing code, documentation or tests are "intentionally so" and
instead consider redesign, rewriting, and updating as preferred to maintaining
existing design, code and documentation whenever that reduces complexity and
aligns with requests. Existing code, documentation and tests may have been
recently written in an entirely exploratory mode, and therefore preserving it
can hinder intended design and implementation.

When asked to update documentation or tests assume existing bug free code is
correct when it conflicts with documentation or tests. If bugs or compile errors
are found outside of tests when asked to update tests then fix the code being
tested first before proceeding to update the tests. All forward progress must be
correct. Do not enshrine bugs in tests or documentation.

Do not add features or functionality that has not been explicitly requested. If
additional functionality is advisable present the user with a numbered list.
Always number separate items in any analysis so they are easily referenced.

Do not create or modify git commits unless asked. Assume committed work
indicates intent instead of being in error, while uncommitted changes may still
be exploratory. Make a checklist for any multi-step task. E.g. when a change
touches two different places.

If the prompt is only an error message or warning assume it is a request to have
the error fixed.

Instead of tracing manually, the expected values for a number of `check_stats`
calls can be found in a single run. These must be monitored for optimal performance and
any changes root caused.

Do not add tokens to your context window that are not required for future
correct operation or to inform the user of actionable information. If an
assumption is being made that modifies the interpretation of the prompt then
that should be output. Otherwise, if you can understand existing context well
enough to analyze it correctly as is then any analysis should not be output.

When asked to test the build, e.g. by being sent the word "build" on its own,
run the following scripts in order. Try to preserve the intention of the
unstaged changes (and possibly earlier breaking commits) while fixing all errors
found.

```sh
debugbuild.sh --run
testcoverage.sh
teststrip.sh
testcmake.sh
```

When asked to "build all" then use `testall.sh` instead.

## Style Guide

"Add code and prose like you are adding weight to an airplane." Use K&R style.
Use size 4 tabs instead of spaces in all text files.

This is a bespoke C17/C++23 alternative to the C++ standard library. Never use
the `std` namespace. The ranges library should not be implemented and do not go
looking for a `std::span` equivalent. Symbols starting with `hx` that have the
same name as symbols in the standard library are generally functionally
equivalent. E.g. use `hxforward` instead of `std::forward`. Standard
functionality is often available. Prefer `hxsize_t` which is an alias for
`ptrdiff_t` for sizes and array indexing as it is advantageous to the optimizer.

Prefer methods that have already been added to a class over using the equivalent
in `<hx/hxalgortims.hpp>`.

Do not use C++ exceptions, RTTI or assume asserts are enabled. Check when adding
includes whether they are redundant and write them as `<stdio.h>` not
`<cstdio>`. Generously use `hxassertf` for debug asserts and `hxassert_hard`
for release asserts. Declare and define functions (except destructors) with no
args as `void x(void)` instead of `void x()`.

Read `.clang-tidy` when writing new code as it is in use and C-style casts are
not allowed. Prefer `1000u` to `(size_t)1000`. The rules are only checked by
`testcmake.sh` and are not checked by `vscode`. Declare local variables `const`
when they are not modified. Do not use `mutable` except for locking.

Do not add unrequested private helper methods to a class. However, prefer
delegating constructors over repeating field initializers. Do not write code
that requires unnecessary traversal of data structures in the debugger watch
window. Don't use `hxforward<...>(self)` unless it actually matters whether
`self` is an lvalue or rvalue. Use `&*x` not `x.operator->()`. `hxdetail_` is a
namespace that indicates a symbol is defined in a header with a name ending in
the word "detail." `hxutility.hpp` is not one of those headers. Do not forward
declare types solely for use in friends clauses. Place `const` overloads before
less qualified overloads and only document the const version.

Do not use defensive programming or guard against mistakes. Never implement
hypothetical safety guarantees. Unintended use cases need to be identified with
asserts and fixed instead of guarded against. Compile errors are cheaper than
debugging. Asserts are cheaper than debugging. Use `hxassertf` to document
preconditions and post conditions instead of null checks.

Prefer wrapping C-style implementation details in C++ classes with normal
operators so that C++ object models are used for interfaces. However, use
`hxarray`/`hxvector` with a static capacity instead of large C-style arrays. Use
template wrappers for type safety while avoiding the associated code bloat.
Mismatched capacity must be allowed by templates wherever possible.

Separate code onto individual lines when it helps step through expressions
individually in the debugger. Use references instead of pointers when a pointer
would never be null.

Alphabetize all public class methods, keeping operators first. Order all
protected and private class methods and operators the same way in separate
blocks following the public one. Place the opening brace of a function body on
the same line as the function signature. When creating a new class do not use
global operators and use class methods instead.

When using the C library use C-style headers and not the C++ wrappers around
them. E.g. use `<math.h>` not `<cmath>`. The goal is to be able to compile
against libc alone without using the C++ standard library at all.

Do not make copies of parameters for no other reason than they are modified,
even when the name of the parameter is made incorrect by modification.
Specifically, do not gratuitously copy templated parameters as the cost of the
copy may be significant.

## Detail Files

Do not hide includes in the detail directory and place them in the corresponding
public header. If a class has a `detail/*.inl` file then it must be used for any
methods that do not fit on one line in 100 columns for that class.

## Naming

All symbols are `snake_case`. Except all shell variables, all feature test
macros and certain preprocessor symbols are `SCREAMING_SNAKE_CASE`. Do not use
abbreviated names. Always use `class` instead of `struct` except in C code and
template metaprogramming. Never use the word "member" and instead use "fields"
and "methods."

Classes, structs and functions begin with `hx` and not `hx_`. Template
parameters are `snake_case` and end with `_t_`. `using` statements may publish
template parameters and other types with an `_t` suffix (in violation of POSIX
chapter 2, section 2.2.2). Function parameters and private fields end with an
underscore and do not begin with `hx`. Private fields also begin with `m_`.
Const symbols start with `hxc_`, global symbols start with `hxg_` and static or
anonymous namespace symbols start with `hxs_`. Prefix calls to the C standard
library with `::` to indicate they are in the global namespace. Use `src_` and
`dst_` for source and destination iterators. Const, global and static prefixes
are not needed for symbols declared within a testcase.

Use scoped enums for private symbols and unscoped enums for public symbols.
Prefix all calls to methods in header files with `this->`. Use `hxnil` instead
of `std::null_ptr`.

## Optimization

Prefer C-style implementation details that are cache coherent. Minimize branches
and memory indirection. Avoid division. Prefer iterating with pointers marked
`hxrestrict` instead of using array indices. Warn when working with tight loops
that will not be unrolled and suggest `hxattr_hot`. Use `hxsize_t` and `size_t`
to avoid unnecessary 64-bit operations on 32-bit builds. Count CPU instructions
required for ARM M-profile and RISC-V bare metal processors and choose the
cheapest implementation. Pack classes and structures for minimum space use.

Always use `hxmove` instead of split placement-new/operator= patterns in shift
loops, since exceptions are disabled. Do not write ternary or other expressions
that would inhibit RVO or NRVO. Do not write extra code to defend against small
integer overflows in `size_t`, `ptrdiff_t` or `hxsize_t` calculations.

Approach all performance instrumentation with a rigorous continuous improvement
philosophy. If a testcase starts using more resources then that must be
diagnosed and defended instead of waved off as expected test-value drift.

## Testing

Use `debugbuild.sh --run` to test changes by default and not cmake. 🪓🪓🪓
output indicates success. When running tests execute `build/hxtest` with `build`
as the current directory. Consider all `.sh` files in the project except
`debian_packages.sh` safe to run at any time. On Windows, fall back to the VS
Code CMake win32 debug task. Use `testcoverage.sh` after writing new tests to
identify missing line coverage.

Tests that compile with a C++11 compiler against C99 libraries are required.
Support for `ILP32`, `LP64` and `LLP64` is required to pass tests. All
`HX_HARDENING_MODE` levels are required to pass tests. Do not use compiler
builtins as this code is intended to compile on any C++ compiler. All test
symbols that show up in the linker map must contain `hx` and `test`.

Do not write test suites until requested as the design may not be finalized. Do
not write redundant tests. Ignore spell checker errors. Use American English.

All tests go in the `test` directory and are GoogleTest-style tests, written to
kill off-by-one mutants. Enumerate the mutants an off-by-one introduces:
relational replacements (`<` vs `<=`, `>` vs `>=`, `==` vs `!=`), loop bound
shifts (`n` to `n-1` or `n+1`), index/pointer shifts (`i+1` to `i-1`, `&a[k]` to
`&a[k-1]` or `&a[k+1]`), and reversed increment direction. For each mutant,
choose the exact boundary input where it diverges from the original and write a
test using the appropriate assertion (`EXPECT_*`, `ASSERT_*`) that fails on the
mutant and passes on the original. Skip equivalent mutants and do not write
identical tests for them. However, testing for as many mutants as possible is
important. Test names are the only testcase documentation, add no other comments.

Prefer `EXPECT_*` macros to `ASSERT_*` macros unless the failure looks like it
will cause memory corruption or other failures in subsequent tests. Keep all
tests for the same function in the same `TEST` or `TEST_F` body. Do not use
helper functions or classes unless they are part of the functionality being
tested.

Do not fix a failing test in a manner that defeats the intent of the test except
by removing it entirely. Prompt the user with a list of failing tests when they
are unrelated to your work or the intent preserving fix is unclear.

100% line coverage is required by `testcoverage.sh`. Use `// GCOVR_EXCL_START`
and `// GCOVR_EXCL_STOP` to exclude uncallable lines. Exclusions should only be
required in test coverage. Deleting unused overloads is better than excluding
them.

Use `x_` for an arbitrary single parameter and `a_` and `b_` for arbitrary
double parameters. Arbitrary iterator args are named `it_`. The arbitrary
pointer name is `ptr_`. Do not use 42 in tests. Use a sequence starting with 31
for arbitrary values. The hashed value of `hxnull`/`hxnil` is also
`hxhash_t{31u}`.

## Debugging

Debug non-obvious test failures, asserts, and crashes with GDB. When debugging,
build with `debugbuild.sh` without `--run`. Then run GDB in batch mode passing
`-x .gdbinit` explicitly to load pretty printers, with `-ex "run"` and `-ex
"bt"` to capture the backtrace, and `build/hxtest` as the target (use
`--gtest_break_on_failure` and `--gtest_filter`). Use the `--cd` arg to specify
the `build` directory as the working directory to avoid polluting the unstaged
changes. Both `hxassert()` and `hxbreakpoint()` will raise `SIGTRAP` and can be
added temporarily to stop execution at a specific point. Automatic core dumps
are discarded on WSL, so do not look for one. If `ptrace` is not allowed then
explicitly generate a core dump to debug instead.

## Documentation

Update documentation independently only when making changes that obsolete it. Do
not add documentation describing reasons for making changes, e.g. instructions
given, issues resolved or bugs fixed. Documentation will be explicitly requested
when the design is final. Usage examples in documentation are not expected to
follow the preceding rules and are instead examples of code written
independently outside of the project. Put comments on preceding lines instead of
on the same line as code.

Describe only the expectation enforced by asserts in documentation instead of
explicitly describing the asserts themselves. If the assert just enforces a
routine invariant then do not document it at all.

Remove trailing `_` from symbols in doxygen comments and leave them in regular
comments and follow existing style otherwise. Do not use `;` or `-` in
documentation unless it is part of a code block or the doxygen formatting shown
below. Uses ASCII unless otherwise requested. Use `/// \cond HIDDEN` blocks
around all internal symbols.

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
should normally be needed to hold all local definitions.

Add section dividers e.g. `// ------` by request only. All text files must end
with a single `\n`.

## Project Structure

Public classes and symbols that are textually included or part of the module
interface are in `include/hx/*.{h,hpp}`. Inline implementation headers are in
`include/hx/detail/*.inl`. Internal symbols are in `include/hx/detail/*.hpp` and
normally end with an `_`. Files ending with `.h` contain both `C` and `C++`.

All tests go in the `test` directory. Symbols in the `test` and `example`
directory never end with an `_` and this rule overrides the rules above in order
to show that internal symbols are not used when testing the APIs. This applies
to all symbols including local variables and function parameters.

Use file globs to discover files in the `src` directory instead of listing them
in build files. Symbols in the `src` directory only end with `_` when required.
Function parameter names in `src` files do not end with `_` even when they do so
in headers.

The `hx` namespace is optional and should not be hardcoded in the GDB scripts.
`HX_NS_BEGIN_` and `HX_NS_END_` are used to wrap the code.

Update `src/hxmodule.cppm` when new files are added to `include`.

## File Index

If the `tags` file generated by ctags doesn't exist, and would be of use,
generate it:

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

© 2017-2026 Adrian Johnston. This project is licensed under the terms of the MIT
license found in the `LICENSE.md` file.
