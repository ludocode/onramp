// The MIT License (MIT)
// Copyright (c) 2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

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
    return z.b.y;
}
