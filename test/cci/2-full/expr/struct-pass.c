// The MIT License (MIT)
// Copyright (c) 2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

#include <stdint.h>

struct P {
    int x, y;
};


// one arg

int test_s(struct P a) {
    return a.x + a.y;
}


// two args

int test_ss(struct P a, struct P b) {
    return a.x + a.y + b.x + b.y;
}

int test_us(uint32_t a, struct P b) {
    return a + b.x + b.y;
}

int test_su(struct P a, uint32_t b) {
    return a.x + a.y + b;
}


// three args

int test_sss(struct P a, struct P b, struct P c) {
    return a.x + a.y + b.x + b.y + c.x + c.y;
}

int test_sus(struct P a, uint32_t b, struct P c) {
    return a.x + a.y + b + c.x + c.y;
}

int test_usu(uint32_t a, struct P b, uint32_t c) {
    return a + b.x + b.y + c;
}


// seven args

int test_usuuusu(uint32_t a, struct P b, uint32_t c, uint32_t d,
        uint32_t e, struct P f, uint32_t g)
{
    return a + b.x + b.y + c + d + e + f.x + f.y + g;
}


int main(void) {
    struct P a = {2, 3};
    struct P b = {5, 7};

    if (test_s(a) != 5) return 1;

    if (test_ss(a, b) != 17) return 2;
    if (test_us(11, b) != 23) return 3;
    if (test_su(a, 13) != 18) return 4;

    if (test_sss(a, b, a) != 22) return 5;
    if (test_sus(a, -6, b) != 11) return 6;
    if (test_usu(5, a, 7) != 17) return 7;

    if (test_usuuusu(5, a, 7, 11, 13, b, 17) != 70) return 8;
}
