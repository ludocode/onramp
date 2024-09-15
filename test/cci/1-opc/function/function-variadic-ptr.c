// The MIT License (MIT)
// Copyright (c) 2023-2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

#include <stdarg.h>
#include <string.h>

int foo(int a, int b, ...) {
    if (a != -1) return 1;
    if (b != -2) return 2;
    va_list args;
    va_start(args, a);

    if (strcmp("hello", va_arg(args, char*)) != 0) return 3;
    if (strcmp("goodbye", va_arg(args, char*)) != 0) return 4;

    va_end(args);
    return 0;
}

int main(void) {
    return foo(-1, -2, "hello", "goodbye");
}
