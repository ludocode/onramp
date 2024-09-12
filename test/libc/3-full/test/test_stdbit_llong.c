// The MIT License (MIT)
// Copyright (c) 2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

#include <stdbit.h>

#include <stdlib.h>

static void test_stdc_leading_zerosull(void) {
    if (stdc_leading_zerosull(0) != 64) exit(1);
    if (stdc_leading_zerosull(1) != 63) exit(2);
    if (stdc_leading_zerosull(2) != 62) exit(3);
    if (stdc_leading_zerosull(3) != 62) exit(4);
    if (stdc_leading_zerosull(4) != 61) exit(5);
    if (stdc_leading_zerosull(0xffffffffffffffff) != 0) exit(6);
    if (stdc_leading_zerosull(0x100000001u) != 31) exit(7);
    if (stdc_leading_zerosull(0x7fffffffffffffff) != 1) exit(8);
    if (stdc_leading_zerosull(0x7000000000000000) != 1) exit(9);
}

int main(void) {
    test_stdc_leading_zerosull();
}
