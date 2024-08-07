// The MIT License (MIT)
// Copyright (c) 2023-2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

// TODO temporarily using builtins directly until we switch over to stage 2 libc
//#include <stdarg.h>
#include <string.h>

int foo(int a, int b, ...) {
    if (a != -1) return 1;
    if (b != -2) return 2;
    __builtin_va_list args;
    __builtin_va_start(args, a);

    if (strcmp("hello", __builtin_va_arg(args, char*)) != 0) return 3;
    if (strcmp("goodbye", __builtin_va_arg(args, char*)) != 0) return 4;

    __builtin_va_end(args);
    return 0;
}

int main(void) {
    return foo(-1, -2, "hello", "goodbye");
}
