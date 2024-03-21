// The MIT License (MIT)
// Copyright (c) 2023-2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

#include <string.h>

int main(void) {
    if (strlen("Hello") != 5) {
        return 1;
    }
    if (strlen("") != 0) {
        return 1;
    }
    if (strlen("a b c d e f") != 11) {
        return 1;
    }
    return 0;
}
