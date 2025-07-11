// The MIT License (MIT)
// Copyright (c) 2025 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

#include <string.h>

struct point {
    int x, y;
};

int main(void) {
    struct point p = {1, 2};

    // casting struct to qualified form of the same struct is valid
    if (((struct point)p).y != 2) return 1;
    if (((const struct point)p).y != 2) return 2;
    if (((volatile struct point)p).y != 2) return 3;
    if (((const volatile struct point)p).y != 2) return 4;

    // casting struct to void is valid
    (void)p;
    (void)(struct point)p;
    (void)(const struct point)p;

    const volatile struct point q = {3, 4};

    // casting cv-qualified struct to same struct with different qualifiers is valid
    if (((struct point)q).y != 4) return 5;
    if (((const struct point)q).y != 4) return 6;
    if (((volatile struct point)q).y != 4) return 7;
    if (((const volatile struct point)q).y != 4) return 8;
}
