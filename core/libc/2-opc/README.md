# Onramp C Standard Library -- Third Stage

This is the third stage of the Onramp libc.

We aim to include most of the functionality in C23. It should be sufficient to compile a native compiler and other build tools, as well as typical command-line C programs. The code is designed for simplicity over performance, though we do make minor optimizations in various places.

Unlike the previous stages, this libc is designed to implement many POSIX calls. For example, POSIX I/O calls like `open()` and `read()` perform the Onramp system calls, and the C standard `fopen()` and `fread()` are an overlay over the POSIX calls.
