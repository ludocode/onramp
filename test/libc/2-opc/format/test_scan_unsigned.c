// The MIT License (MIT)
// Copyright (c) 2025 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

void test_u(void) {
    unsigned u, u2;

    u = UINT_MAX;
    if (1 != sscanf("1234", "%u", &u)) exit(1);
    if (u != 1234) exit(2);

    u = UINT_MAX;
    u2 = UINT_MAX;
    if (2 != sscanf("abcdef -- -1234- zxcv:5678", "abcdef -- -%u- zxcv:%u", &u, &u2)) exit(3);
    if (u != 1234) exit(4);
    if (u2 != 5678) exit(5);
}

void test_llu(void) {
    // TODO test works but disabled, need to fix bootstrapping test problem
    #ifdef DISABLED
    unsigned long long llu, llu2, llu3;

    llu = ULLONG_MAX;
    if (1 != sscanf("1234", "%llu", &llu)) exit(51);
    if (llu != 1234) exit(52);

    llu = ULLONG_MAX;
    if (1 != sscanf("12345678901234567", "%llu", &llu)) exit(53);
    if (llu != 12345678901234567llu) exit(54);

    llu = ULLONG_MAX;
    llu2 = ULLONG_MAX;
    llu3 = ULLONG_MAX;
    if (3 != sscanf(":999999999999999999-- \n abcdef999555111222777444\v\v\v5", ":%llu--\tabcdef%llu %llu", &llu, &llu2, &llu3)) exit(55);
    if (llu != 999999999999999999llu) exit(56);
    if (llu2 != 999555111222777444llu) exit(57);
    if (llu3 != 5) exit(58);
    #endif
}

void test_x(void) {
    // TODO test works but disabled, need to fix bootstrapping test problem
    #ifdef DISABLED
    unsigned u;
    unsigned long long llu;

    u = UINT_MAX;
    llu = ULLONG_MAX;
    if (2 != sscanf("abcdef01-:0x012345678ABCdEf", " %X - : %llp ", &u, &llu)) exit(101);
    if (u != 0xabcdef01) exit(102);
    if (llu != 0x012345678abcdefllu) exit(103);
    #endif
}

int main(void) {
    test_u();
    test_llu();
    test_x();
}
