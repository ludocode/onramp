// The MIT License (MIT)
// Copyright (c) 2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

struct P {
    int x, y;
} foo(void) {
    struct P a = {2, 3};
    return a;
}

int main(void) {
    // overwrite some registers to make sure the below doesn't
    // work by coincidence
    5 * 3 + 4 * 7;

    struct P a = foo();
    if (a.x + a.y != 5) return 1;
}
