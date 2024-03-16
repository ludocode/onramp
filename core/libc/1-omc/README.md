# Onramp C Standard Library -- Second Stage

This is the second stage libc.

The additions here are minimal. We implement the few extra things we need to make [cc](../../cc/) and [cci/1](../../cci/1-omc/) work. Without the `cc` driver bootstrapped yet, it's a pain to build C files so we only implement what we need.

We now have a real `malloc()` and `free()` that reclaims memory. This is especially important for our compiler which does a lot of string and type allocation.

This stage is structured as an overlay over the [first stage](../0-oo). Files in `src/` (such as `malloc.c`) replace files in the previous stage with the same name (such as `malloc.oo`.) The build process for this stage libc compiles the files in `src/`, then combines the resulting object files with those from the previous stage into a static library.

Note: omC doesn't support unsigned so, when compiling code with the first stage compiler, `size_t` is signed! This libc is also used with the later stage compilers where `size_t` is properly unsigned. See [`include/__onramp/__size_t.h`](include/__onramp/__size_t.h).
