// The MIT License (MIT)
// Copyright (c) 2025 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

#include <stdlib.h>
#include <string.h>

int main(void) {
    char* a = realloc(NULL, 6);
    if (!a) return 1;
    strcpy(a, "hello");

    // grow
    char* b = realloc(a, 13);
    if (!b) return 2;

    if (0 != memcmp(b, "hello", 6)) return 3;

    strcpy(b + 5, " world!");
    if (0 != memcmp(b, "hello world!", 13)) return 4;

    // shrink
    char* c = realloc(b, 6);
    if (!c) return 5;

    if (0 != memcmp(c, "hello ", 6)) return 6;
}
