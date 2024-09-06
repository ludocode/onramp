// The MIT License (MIT)
// Copyright (c) 2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

struct P {
    int x, y;
} foo(int x, int y) {
    struct P a = {x, y};
    return a;
}

struct Q {
    struct P p[8];
};

int main(void) {
    struct Q q = {
        1, 2,
        foo(3, 4),
        {5, 6,},
        foo(7, 8),
        9, {10},
        11, {},
        {},
        15,
    };
    if (q.p[0].x != 1) return 1;
    if (q.p[0].y != 2) return 2;
    if (q.p[1].x != 3) return 3;
    if (q.p[1].y != 4) return 4;
    if (q.p[2].x != 5) return 5;
    if (q.p[2].y != 6) return 6;
    if (q.p[3].x != 7) return 7;
    if (q.p[3].y != 8) return 8;
    if (q.p[4].x != 9) return 9;
    if (q.p[4].y != 10) return 10;
}
