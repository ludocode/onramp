// The MIT License (MIT)
// Copyright (c) 2025 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

#include <string.h>

int main(void) {
    if (0 != strcmp("abcd", "abcd")) {
        return 1;
    }
    if (0 >= strcmp("abdd", "abcd")) {
        return 2;
    }
    if (0 <= strcmp("abbd", "abcd")) {
        return 3;
    }
    if (0 != strcmp("", "")) {
        return 4;
    }
    if (0 >= strcmp("a", "")) {
        return 5;
    }
    if (0 <= strcmp("", "b")) {
        return 6;
    }
    return 0;
}
