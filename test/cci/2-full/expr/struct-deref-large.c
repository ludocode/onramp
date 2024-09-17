// The MIT License (MIT)
// Copyright (c) 2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

#include <stddef.h>
#include <stdio.h>

struct S {
    int x[33];

    // member offset is more than 128 from start of
    // struct; doesn't fit in a mix-type byte
    int y;
    int z;
} a = {{1},2,3};

int main(void) {
    if (a.x[0] != 1) return 1;
    if (a.x[1] != 0) return 2;
    if (a.y != 2) return 3;
    if (a.z != 3) return 4;

    struct S* pa = &a;

    if (pa->x[0] != 1) return 5;
    if (pa->x[1] != 0) return 6;
    if (pa->y != 2) return 7;
    if (pa->z != 3) return 8;

    if (*&pa->x[0] != 1) return 9;
    if (*&pa->x[1] != 0) return 10;
    if (*&pa->y != 2) return 11;
    if (*&pa->z != 3) return 12;
}
