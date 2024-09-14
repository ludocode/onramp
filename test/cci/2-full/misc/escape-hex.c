// The MIT License (MIT)
// Copyright (c) 2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

#include<stdio.h>
int main(void) {
    unsigned char y[] = "\x00\X01\x20\X90\xff";
    if (y[0] != 0) return 1;
    if (y[1] != 1) return 2;
    if (y[2] != 0x20) return 3;
    if (y[3] != 0x90) return 4;
    if (y[4] != 0xff) return 5;
    if (y[5] != 0) return 6;

}
