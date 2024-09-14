// The MIT License (MIT)
// Copyright (c) 2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

#include <limits.h>

int main(void) {
    unsigned char a = 0x77u;
    unsigned char x = 0;
    unsigned char b = 0xCCu;

    if (x++ != 0) {return 1;}
    if (x-- != 1) {return 2;}
    if (x-- != 0) {return 3;}
    if (x++ != UCHAR_MAX) {return 4;}
    if (x   != 0) {return 5;}

    // check that canaries weren't clobbered
    if (a != 0x77u) {return 6;}
    if (b != 0xCCu) {return 7;}
}
