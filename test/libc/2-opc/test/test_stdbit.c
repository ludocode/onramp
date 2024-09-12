// The MIT License (MIT)
// Copyright (c) 2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

#include <stdbit.h>

#include <stdlib.h>

static void test_stdc_leading_zeros(void) {
    if (stdc_leading_zerosui(0) != 32) exit(1);
    if (stdc_leading_zerosui(1) != 31) exit(2);
    if (stdc_leading_zerosui(2) != 30) exit(3);
    if (stdc_leading_zerosui(3) != 30) exit(4);
    if (stdc_leading_zerosui(4) != 29) exit(5);
    if (stdc_leading_zerosui(0xffffffff) != 0) exit(6);
    if (stdc_leading_zerosui(0x80000000) != 0) exit(7);
    if (stdc_leading_zerosui(0x7fffffff) != 1) exit(8);
    if (stdc_leading_zerosui(0x70000000) != 1) exit(9);

    if (stdc_leading_zerosus((unsigned short)0) != 16) exit(1);
    if (stdc_leading_zerosus((unsigned short)1) != 15) exit(1);
    if (stdc_leading_zerosus((unsigned short)0x7777) != 1) exit(1);
    if (stdc_leading_zerosus((unsigned short)0xffff) != 0) exit(1);

    if (stdc_leading_zerosuc((unsigned char)0) != 8) exit(1);
    if (stdc_leading_zerosuc((unsigned char)1) != 7) exit(1);
    if (stdc_leading_zerosuc((unsigned char)0x70) != 1) exit(1);
    if (stdc_leading_zerosuc((unsigned char)0xff) != 0) exit(1);
}

int main(void) {
    test_stdc_leading_zeros();
}
