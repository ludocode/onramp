// The MIT License (MIT)
// Copyright (c) 2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

#include <stdarg.h>

int x;

void bar(va_list args) {
    x += va_arg(args, int);
}

void foo(int ignored, ...) {
    va_list args;
    va_start(args, ignored);

    // According to 7.16.3, since bar() calls va_arg() on the passed args, the
    // value of args is indeterminate after the call to bar(), and any
    // subsequent use must be preceded by a call to va_end(). So technically
    // this is not allowed, presumably because different implementations give
    // different results.
    //
    // On Linux, va_list is an array, so it is passed by reference. Thus each
    // call to bar() consumes the next argument, and the result is 6. On
    // Windows, va_list is an address, so it is passed by value. Thus each call
    // to bar() gets the same first argument and the result is 3.
    //
    // We have lots of options on what we could do in Onramp. We could choose
    // the Linux way or the Windows way; we could support both with a compiler
    // flag; we could insert code that discards the pointer in the parent when
    // passed so that attempting to re-use it crashes at runtime; and we could
    // detect many cases of re-use at compile time and issue a compiler error.
    //
    // TODO some of the above. In the meantime we do it the Windows way since
    // it's simplest to implement. Note that the standard officially recommends
    // passing a pointer to va_list to have a callee's changes reflected in the
    // caller.
    bar(args);
    bar(args);
    bar(args);

    va_end(args);
}

int main(void) {
    foo(0, 1, 2, 3);

    // TODO: on Linux, va_list is an array:
    if (x != 6) return 1;

    // TODO: on Windows, va_list is a value:
    if (x != 3) return 2;
}
