# Onramp C Standard Library

This is the C standard library for programs running on the Onramp virtual machine. It is statically linked with programs, both in the Onramp toolchain and in the programs they compile. (There is no dynamic linking in Onramp.)

Unlike other components, the libc is built up incrementally. Each stage does not replace the previous stage entirely; rather, each stage adds and replaces only certain files from the previous stages. Any file that has the same name as a file from a previous stage replaces it, and any files not replaced are carried forward.

- [`0-oo`](0-oo/) is written directly in [object code](../../docs/object-code.md). It contains the entry point which calls `main()`, some basic `<string.h>` and `<stdio.h>` functions, and a trivial `malloc()` that never frees.

- [`1-omc`](1-omc/) is written in [Onramp Minimal C (omC)](../../docs/minimal-c.md). It replaces `malloc()` with a real allocator among other things.

- [`2-opc`](2-opc/) is written in [Onramp Practical C (opC)](../../docs/practical-c.md). It implements most of our POSIX infrastructure, buffered file I/O, a faster `malloc()`, `printf()` and friends, and more. Most of the final libc is here.

- [`3-full`](3-full/) is written in modern C. It implements the remaining libc functions that require the full C compiler, in particular floating point math, `<math.h>` and related functions.



## Exiting a program

There are many ways to exit a program in C. This describes their implementation in Onramp.

In Onramp, processes can run other processes, and all processes share a global set of file handles in the VM. All open files of a process must therefore be closed before exiting, otherwise they will be leaked. This is handled by the libc.

The Onramp libc implements two main ways of exiting a program: `_Exit()` and `abort()`. These two functions are implemented by all libc stages and used by all bootstrap stages of the toolchain.

- `_Exit()` is a normal exit. It flushes and closes open files and jumps to the exit address, which either returns to the parent Onramp process or exits the Onramp VM.

- `abort()` is an abnormal exit. It currently halts the entire VM.

In an abnormal exit, we assume a bug has been detected and memory corruption has potentially occured. Since there is no memory protection, our parent process memory may be corrupt as well. Even if it is not, it's probably not safe to close open files, and we can't return to our parent process without closing them. The safest thing to do is to take out the whole VM.

All other methods of exiting a program are mapped to one of the two mechanisms above in the final stage libc.

- `exit()` calls `atexit()` functions, then calls `__attribute__((destructor))` functions, then forwards to `_Exit()`.

- `quick_exit()` calls `at_quick_exit()` functions, then forwards to `_Exit()`.

- returning from `main()` calls `exit()`.

- `_exit()` (from POSIX) is an alias of `_Exit()`.

- `__builtin_trap()` and other such builtins call `abort()`.

- `assert()` failures call `abort()`. In earlier stages, `assert()` is a function. Once the final preprocessor is bootstrapped, it's a macro that can be eliminated under `NDEBUG`.

- `__fatal()` is an Onramp internal used to print an error message before forwarding to `_Exit(1)`.
