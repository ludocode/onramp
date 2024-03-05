# Onramp C Standard Library

This is the C standard library for programs running on the Onramp virtual machine. It is statically linked with programs, both in the Onramp toolchain and in the programs they compile. (There is no dynamic linking in Onramp.)

Unlike other components, the libc is built up incrementally. Each stage does not replace the previous stage entirely; rather, each stage adds and replaces only certain files from the previous stages. Any file that has the same name as a file from a previous stage replaces it, and any files not replaced are carried forward.

- [`0-oo`](0-oo/) is written directly in [object code](../../docs/object-code.md). It contains the entry point which calls `main()`, some basic `<string.h>` and `<stdio.h>` functions, and a trivial `malloc()` that never frees.

- [`1-omc`](1-omc/) is written in [Onramp Minimal C (omC)](../../docs/minimal-c.md). It replaces `malloc()` with a real allocator among other things.

- [`2-opc`](2-opc/) is written in [Onramp Practical C (opC)](../../docs/practical-c.md). It implements most of our POSIX infrastructure, buffered file I/O, a faster `malloc()`, `printf()` and friends, and more. Most of the final libc is here.

- [`3-full`](3-full/) is written in modern C. It implements the remaining libc functions that require the full C compiler, in particular floating point math, `<math.h>` and related functions.
