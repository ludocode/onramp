# Onramp C Standard Library

This is the C standard library for programs running on the Onramp virtual machine. It is statically linked with programs, both in the Onramp toolchain and in the programs they compile. (There is no dynamic linking in Onramp.)

Unlike other components, the libc is built up incrementally. Each stage does not replace the previous stage entirely; rather, each stage adds and replaces only certain files from the previous stages. Any file that has the same name as a file from a previous stage replaces it, and any files not replaced are carried forward.

- [`0-oo`](0-oo/) is written directly in [object code](../../docs/object-code.md). It contains the entry point which calls `main()`, some basic `<string.h>` and `<stdio.h>` functions, and a simple `malloc()`.

- [`1-omc`](1-omc/) is written in [Onramp Minimal C (omC)](../../docs/minimal-c.md). It adds a few minor functions needed for later bootstrapping such as `strtol()` and `strrchr()`.

- [`2-opc`](2-opc/) is written in [Onramp Practical C (opC)](../../docs/practical-c.md). It implements most of our POSIX infrastructure, `printf()` and friends, and more. Most of the final libc is here.

- [`3-full`](3-full/) is written in modern C. It implements the remaining libc functions that require the full C compiler, in particular floating point math, `<math.h>` and related functions, a coalescing `malloc()`, and algorithms such as `qsort()`.



## File I/O

The first stage libc implements only the basic C `FILE*` APIs: `fopen()`, `fread()`, `puts()` and so on. These support primitive buffering, but otherwise call file system calls directly. The implementation is entirely in [`0-oo/src/stdio.oo`](0-oo/src/stdio.oo).

Starting from [libc/2](2-opc), the libc implements POSIX-style file I/O: `open()`, `read()` and so on. The implementation is pretty complicated compared to a typical libc because much of what we have to build would normally be provided by the kernel. We have to manage our own file descriptor table for example.

It's important to distinguish between a *file descriptor* and a *file description*. A file descriptor is represented by an `int` and refers to a file description. Multiple file descriptors can reference the same file description, in which case they share most state (such as the seek position in the underlying file.)

See the [POSIX spec](https://pubs.opengroup.org/onlinepubs/9799919799/) for definitions.

These are the types used in the Onramp libc:

- `fdn_t` -- An open file description. This includes the VM handle (if any), the file's type, the path to the file, the file description flags (but not file descriptor flags), and cached data (such as a directory entry.)

- `fdr_t` -- A file descriptor. This contains a retained reference to an open file description along with file descriptor flags (close-on-exec.) This is represented by an `int` which is a key to the `fdr_t*` in the file descriptor table.

Most of the above is implemented in [`2-opc/src/posixio.c`](2-opc/src/posixio.c). Additional code for directories is in [`3-full/src/dirent.c`](3-full/src/dirent.c).

The C `FILE*` API (`fopen()`, `fread()`, `printf()` and so on) is built on top of the POSIX APIs. A `FILE` contains a file descriptor and a read/write buffer.

The POSIX `DIR*` directory APIs (`opendir()`, `readdir()` and so on) are also built on top of the low-level POSIX code. A `__dirent()` function is used to read a directory entry from a file descriptor.



## Exiting a program

There are many ways to exit a program in C. This describes their implementation in Onramp.

TODO the below documentation is wrong and needs to be fixed, and libc/2 needs to be fixed as well. We are relaxing the rules so programs don't have to close open files, and `_Exit()` doesn't flush or close anything anymore, so it is now much closer to the real C rules.

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
