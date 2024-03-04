# Onramp C Standard Library -- Second Stage

This is the second stage libc. It implements just enough libc to bootstrap our remaining C compiler stages.

It is structured as an overlay over the [first stage libc](../0-oo). Most libc files from the first stage are kept as-is, and this stage mostly just adds header files that can be included in omC.

It does have a few additions however, such as a real `malloc()` and `free()` that reclaims memory. Files in `src/` (such as `stdlib_malloc.c`) replace files in the previous stage with the same name (such as `stdlib_malloc.oo`.) The build process for this stage libc compiles the files in `src/`, then combines the resulting object files with those from the previous stage into a static library.

Note: omC doesn't support unsigned so, when compiling code with the first stage compiler, `size_t` is signed! This libc is also used with the later stage compilers where `size_t` is properly unsigned. See [`include/__onramp/__size_t.h`](include/__onramp/__size_t.h).
