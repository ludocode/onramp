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
    if (a.x != 2) return 1;
    if (a.y != 3) return 2;

    struct {
        struct P p1, p2;
    } q = {foo(), foo()};
    if (q.p1.x != 2) return 3;
    if (q.p1.y != 3) return 4;
    if (q.p2.x != 2) return 5;
    if (q.p2.y != 3) return 6;

}
