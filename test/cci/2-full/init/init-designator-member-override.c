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
    // overriding a previous explicit initializer with an implicit
    // zero-initialization as part of a short nested initializer
    struct z z = {.b.y = 5, .b = {3}}; // WARNING: -Winitializer-overrides
    if (z.a != 0) return 1;
    if (z.b.x != 3) return 2;
    if (z.b.y != 0) return 3; // 0, not 5!
    if (z.c != 0) return 4;
}
