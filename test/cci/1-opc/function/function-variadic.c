// The MIT License (MIT)
// Copyright (c) 2023-2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

// TODO temporarily using builtins directly until we switch over to stage 2 libc
//#include <stdarg.h>

int bar(__builtin_va_list args) {
    if (__builtin_va_arg(args, int) != 2) return 1;
    if (__builtin_va_arg(args, int) != 3) return 2;
    if (__builtin_va_arg(args, int) != 4) return 3;
    if (__builtin_va_arg(args, int) != 5) return 4;
    return 0;
}

int foo(int a, ...) {
    if (a != 1) return 5;
    __builtin_va_list args;
    __builtin_va_start(args, a);

    __builtin_va_list other;
    __builtin_va_copy(other, args);
    int barval = bar(other);
    if (barval) return barval;

    if (__builtin_va_arg(args, int) != 2) return 7;
    if (__builtin_va_arg(args, int) != 3) return 8;
    if (__builtin_va_arg(args, int) != 4) return 9;
    if (__builtin_va_arg(args, int) != 5) return 10;

    __builtin_va_end(args);
    return 0;
}

int main(void) {
    return foo(1, 2, 3, 4, 5);
}
