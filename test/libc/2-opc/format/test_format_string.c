// The MIT License (MIT)
// Copyright (c) 2026 Fraser Heavy Software
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

    // field width
    TEST_FORMAT(1, "|  abc|", "|%5s|", "abc");
    TEST_FORMAT(2, "|abc  |", "|%-5s|", "abc");
    TEST_FORMAT(3, "|  abc|", "|%*s|", 5, "abc");
    TEST_FORMAT(4, "|abc  |", "|%-*s|", 5, "abc");

    // precision
    TEST_FORMAT(11, "|abcde|", "|%.5s|", "abcdefghij");
    TEST_FORMAT(12, "|abcde|", "|%.*s|", 5, "abcdefghij");
    TEST_FORMAT(13, "|abc|", "|%.5s|", "abc");
    TEST_FORMAT(14, "|abc|", "|%.*s|", 5, "abc");

    // both
    TEST_FORMAT(21, "|  abc|", "|%5.3s|", "abcdefghij");
    TEST_FORMAT(22, "|abc  |", "|%-5.3s|", "abcdefghij");
    TEST_FORMAT(23, "|  abc|", "|%*.*s|", 5, 3, "abcdefghij");
    TEST_FORMAT(24, "|abc  |", "|%-*.*s|", 5, 3, "abcdefghij");
    TEST_FORMAT(25, "|  abc|", "|%*.*s|", 5, 4, "abc");
    TEST_FORMAT(26, "|abc  |", "|%-*.*s|", 5, 4, "abc");
}
