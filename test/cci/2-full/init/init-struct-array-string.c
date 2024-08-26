// The MIT License (MIT)
// Copyright (c) 2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

#include <string.h>

struct foo {
    int x;
    char y[5];
    int z;
};

int main(void) {
    struct foo foo = {1, "Hello", 2};
    if (foo.x != 1) return 1;
    if (0 != memcmp(foo.y, "Hello", 5)) return 2;
    if (foo.z != 2) return 3;
}
