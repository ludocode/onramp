// The MIT License (MIT)
// Copyright (c) 2026 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

//% SKIP

typedef struct S {
    struct {
        int i;
        int j;
    } x;
    struct {
        struct {
            int l;
            int m;
            int n;
        } k;
    } y;
} S;

S s = {
    .x.j = 1,
    .y.k.m = 2,
    .y.k = {.l = 3}, // clears m!
    .x.i = 4, // doesn't clear j!
    .y.k.n = 5,
};

int main(void) {
    if (s.x.i != 4) return 1;
    if (s.x.j != 1) return 2;
    if (s.y.k.l != 3) return 3;
    if (s.y.k.m != 0) return 4; // 0, not 2!
    if (s.y.k.n != 5) return 5;
}
