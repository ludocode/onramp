// The MIT License (MIT)
// Copyright (c) 2026 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

typedef struct S {
    int x, y, z;
} S;

S a = {.x = 1};
S b = {.y = 2};
S c = {.z = 3};
S d = {.y = 4, 5, .x = 6};

int main(void) {
    S e = {.x = 7, .z = 8, .y = 9};

    if (a.x != 1) return 1;
    if (a.y != 0) return 2;
    if (a.z != 0) return 3;

    if (b.x != 0) return 4;
    if (b.y != 2) return 5;
    if (b.z != 0) return 6;

    if (c.x != 0) return 7;
    if (c.y != 0) return 8;
    if (c.z != 3) return 9;

    if (d.x != 6) return 10;
    if (d.y != 4) return 11;
    if (d.z != 5) return 12;

    if (e.x != 7) return 13;
    if (e.y != 9) return 14;
    if (e.z != 8) return 15;
}
