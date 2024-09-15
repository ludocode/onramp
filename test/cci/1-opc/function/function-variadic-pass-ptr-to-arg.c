// The MIT License (MIT)
// Copyright (c) 2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

#include <stdarg.h>

int x;

void bar(va_list* args) {
    x += va_arg(*args, int);
}

void vfoo(int ignored, va_list args) {
    (void)ignored;

    // This is the officially supported way to share a va_list with another
    // function. See C17 7.16.3 footnote 258.
    bar(&args);
    bar(&args);
    bar(&args);
}

void foo(int ignored, ...) {
    (void)ignored;

    va_list args;
    va_start(args, ignored);
    vfoo(ignored, args);
    va_end(args);
}

int main(void) {
    foo(0, 1, 2, 3);
    if (x != 6) return 1;
}
