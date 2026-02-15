// The MIT License (MIT)
// Copyright (c) 2025 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    char buf[128];

    #define TEST_FORMAT(ret, expected, ...) \
        memset(buf, 0xEE, sizeof(buf)); \
        snprintf(buf, sizeof(buf), __VA_ARGS__); \
        if (0 != strcmp(expected, buf)) { \
            printf("bad printf %i. expected \"%s\", got \"%s\"\n", ret, expected, buf); \
            return ret; \
        }

    // short
    TEST_FORMAT(1, "5", "%hi", 5);
    TEST_FORMAT(2, "56", "%hi", 56);
    TEST_FORMAT(3, "32767", "%hi", SHRT_MAX);

    // short truncation
    TEST_FORMAT(10, "-32768", "%hi", 32768);
    TEST_FORMAT(11, "-1", "%hi", USHRT_MAX);
    TEST_FORMAT(11, "-1", "%hi", INT_MAX);

    // char
    TEST_FORMAT(20, "5", "%hhi", 5);
    TEST_FORMAT(21, "56", "%hhi", 56);

    // char truncation
    TEST_FORMAT(30, "-1", "%hhi", 255);

    #ifdef __onramp__
    // long is the same as int (on Onramp)
    TEST_FORMAT(40, "2147483647 -2147483648", "%li %li", INT_MAX, INT_MIN);
    TEST_FORMAT(41, "-2147483648 2147483647", "%li %li", INT_MIN, INT_MAX);
    #endif

    // TODO this test works but it's disabled for now because this file runs
    // during bootstrapping but 64-bit formatting is not enabled in the
    // bootstrap libc (it only becomes available once the final libc is
    // rebuilt.) We need to figure out a way to run these tests only on the
    // final libc.
    #ifdef DISABLED
    // long long
    TEST_FORMAT(50, "2147483648", "%lli", (long long)INT_MAX + 1);
    #endif
}
