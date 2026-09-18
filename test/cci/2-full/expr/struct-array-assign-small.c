// The MIT License (MIT)
// Copyright (c) 2026 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

// This struct has a size of 3 bytes which is smaller than a register. In
// Onramp, this struct is still passed indirectly.
struct s {
    char x;
    char y;
    char z;
} a, b, c, d;

static struct s return_s(struct s s) {
    return s;
}

int main(void) {
    a.x = 5;
    a.y = 6;
    a.z = 7;

    // assignment of a small struct by value
    b = a;
    if (b.x != 5) return 1;
    if (b.y != 6) return 2;
    if (b.z != 7) return 3;

    // assignment of a small struct by dereferencing pointer
    (&c)[0] = (&a)[0];
    if (c.x != 5) return 4;
    if (c.y != 6) return 5;
    if (c.z != 7) return 6;

    // passing through function argument and return value
    d = return_s(a);
    if (d.x != 5) return 7;
    if (d.y != 6) return 8;
    if (d.z != 7) return 9;
}
