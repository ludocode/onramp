// The MIT License (MIT)
// Copyright (c) 2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

#include <string.h>

char x[] =
        "Hello"
        " "
        "world"
        "!";

int main(void) {
    if (sizeof(x) != 13) return 1;
    if (0 != strcmp(x, "Hello world!")) return 2;
}
