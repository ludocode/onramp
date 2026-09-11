// The MIT License (MIT)
// Copyright (c) 2024-2026 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

//% SKIP

struct b {
    int x;
    int y;
};

struct z {
    int a;
    struct b b;
    int c;
};

int main(void) {
    // This basically the opposite of init-overrides-zeroing.c . The
    // initialization of b.x should not override the previous initialization
    // of b.y.
    struct z z = {.b.y = 5, .a = 1, 3};
    if (z.a != 1) return 1;
    if (z.b.x != 3) return 2;
    if (z.b.y != 5) return 3; // 5, not 0!
    if (z.c != 0) return 4;
}
