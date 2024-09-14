// The MIT License (MIT)
// Copyright (c) 2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

#include <limits.h>

int main(void) {
    unsigned short a = 0x7777u;
    unsigned short x = 0;
    unsigned short b = 0xCCCCu;

    if (++x != 1) {return 1;}
    if (--x != 0) {return 2;}
    if (--x != USHRT_MAX) {return 3;}
    if (++x != 0) {return 4;}

    // check that canaries weren't clobbered
    if (a != 0x7777u) {return 5;}
    if (b != 0xCCCCu) {return 6;}
}
