// The MIT License (MIT)
// Copyright (c) 2023-2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

// TODO temporarily using builtins directly until we switch over to stage 2 libc
//#include <stdarg.h>

int bar(__builtin_va_list args) {
    if (__builtin_va_arg(args, int) != 2) {return 1;}
    if (__builtin_va_arg(args, int) != 3) {return 1;}
    if (__builtin_va_arg(args, int) != 4) {return 1;}
    if (__builtin_va_arg(args, int) != 5) {return 1;}
    return 0;
}

int foo(int a, ...) {
    if (a != 1) {return 1;}
    __builtin_va_list args;
    __builtin_va_start(args, a);

    __builtin_va_list other;
    __builtin_va_copy(other, args);
    if (bar(other)) {return 1;}

    if (__builtin_va_arg(args, int) != 2) {return 1;}
    if (__builtin_va_arg(args, int) != 3) {return 1;}
    if (__builtin_va_arg(args, int) != 4) {return 1;}
    if (__builtin_va_arg(args, int) != 5) {return 1;}

    __builtin_va_end(args);
    return 0;
}

int main(void) {
    return foo(1, 2, 3, 4, 5);
}
