# Onramp C Standard Library -- Second Stage

This is the second stage libc.

The additions here are minimal. We implement the few extra things we need to make [cc](../../cc/) and [cci/1](../../cci/1-omc/) work. Without the `cc` driver bootstrapped yet, it's a pain to build C files so we only implement what we need.

Note: omC doesn't support unsigned so, when compiling code with the first stage compiler, `size_t` is signed! This libc is also used with the later stage compilers where `size_t` is properly unsigned. See [`include/__onramp/__size_t.h`](include/__onramp/__size_t.h).
