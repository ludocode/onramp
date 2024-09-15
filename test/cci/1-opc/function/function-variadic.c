// The MIT License (MIT)
// Copyright (c) 2023-2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

#include <stdarg.h>

int bar(va_list args) {
    if (va_arg(args, int) != 2) return 1;
    if (va_arg(args, int) != 3) return 2;
    if (va_arg(args, int) != 4) return 3;
    if (va_arg(args, int) != 5) return 4;
    return 0;
}

int foo(int a, ...) {
    if (a != 1) return 5;
    va_list args;
    va_start(args, a);

    va_list other;
    va_copy(other, args);
    int barval = bar(other);
    if (barval) return barval;

    if (va_arg(args, int) != 2) return 7;
    if (va_arg(args, int) != 3) return 8;
    if (va_arg(args, int) != 4) return 9;
    if (va_arg(args, int) != 5) return 10;

    va_end(args);
    return 0;
}

int main(void) {
    return foo(1, 2, 3, 4, 5);
}
