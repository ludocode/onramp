# Onramp C Standard Library -- First Stage

This is the first stage of the Onramp libc. It serves as the libc for all of our Onramp programs until our omC compiler is bootstrapped.

This is in the bootstrap process as early as possible, immediately after getting label support, in order to avoid repeating these functions in each program. It is therefore written directly in Onramp object code.

Note that the individual files don't need to be compiled. They are already object files (which, in Onramp, are in plain text.) They are linked directly into the first stage `ar`, which is then used by this `build.sh` to assemble them into a `libc.oa` static library.

This libc provides a few common functions:

- process management:
    - `__start()`, the entry point, which initializes the libc
    - the calling of `main()` with command-line arguments
    - `_Exit()`
- some `<string.h>` functions:
    - `memcpy()`, `memmove()`, `memcmp()`
    - `strlen()`, `strcmp()`, `strcpy()`, `strchr()`
- some `<stdio.h>` functions:
    - `fopen()`, `fclose()`, `fread()`, `fwrite()` (unbuffered)
    - `fputs()`, `fputc()`, etc.
    - `fseek()`, `ftell()`
- some `<stdlib.h>` functions:
    - `malloc()` as a simple bump allocator with a no-op `free()`
    - `calloc()`, `strdup()`, `strndup()`
- extensions:
    - `__memdup()`
    - `__fatal()` to exit with an error message

We avoid implementing too much else. We aren't going to need it in the early stages, and we want to get at least opC bootstrapped before we really start filling out the libc.

Note: omC doesn't support unsigned so, when compiling code with the first stage compiler, `size_t` is signed! This libc is also used with the later stage compilers where `size_t` is properly unsigned. See [`include/__onramp/__size_t.h`](include/__onramp/__size_t.h).
