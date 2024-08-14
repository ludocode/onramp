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
    struct z z = {1, 2, 4, 8};
    return 15 - z.a - z.b.x - z.b.y - z.c;
}
