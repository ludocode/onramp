// The MIT License (MIT)
// Copyright (c) 2025 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

#include <limits.h>
#include <stdarg.h>
#include <stdlib.h>
//#include <stdio.h>

void test(int exit_code, long long a, ...) {
    va_list args;
    va_start(args, a);

    long long b = va_arg(args, long long);
    long long c = va_arg(args, long long);

    /*
    printf("%lli\n", a);
    printf("%lli\n", b);
    printf("%lli\n", c);

    union {
        long long ll;
        char c[8];
    } u;
    u.ll = a;
    for (size_t i = 0; i < 8; ++i) {
        printf("%02hhx ",u.c[7-i]);
    }
    printf("\n");
    */

    if (a != b) {
        exit(exit_code);
    }
    if (a != c) {
        exit(exit_code + 1);
    }
    va_end(args);
}

int main(void) {
    test(10, 1ll, 1ll, 1ll);
    test(20, -1ll, -1ll, -1ll);
    test(30, (long long)INT_MAX, (long long)INT_MAX, (long long)INT_MAX);
    test(40, (long long)INT_MIN, (long long)INT_MIN, (long long)INT_MIN);
    test(50, LLONG_MAX, LLONG_MAX, LLONG_MAX);
    test(60, LLONG_MIN, LLONG_MIN, LLONG_MIN);
    test(70, (long long)INT_MAX + 1, (long long)INT_MAX + 1, (long long)INT_MAX + 1);
}
