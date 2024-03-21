// The MIT License (MIT)
// Copyright (c) 2023-2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

int main(void) {
    signed char sc = -3;
    signed short ss = -0x333;
    signed int si = -0x33333;
    unsigned char uc = 3;
    unsigned short us = 0x333;
    unsigned int ui = 0x33333;

    // We test every permutation of casts.

    if ((signed char)si != -0x33) {return 1;}
    if ((signed char)ui != 0x33) {return 1;}
    if ((signed char)ss != -0x33) {return 1;}
    if ((signed char)us != 0x33) {return 1;}
    if ((signed char)sc != -3) {return 1;}
    if ((signed char)uc != 3) {return 1;}

    if ((unsigned char)si != 0xCD) {return 1;}
    if ((unsigned char)ui != 0x33) {return 1;}
    if ((unsigned char)ss != 0xCD) {return 1;}
    if ((unsigned char)us != 0x33) {return 1;}
    if ((unsigned char)sc != 0xFD) {return 1;}
    if ((unsigned char)uc != 3) {return 1;}

    if ((signed short)si != -0x3333) {return 1;}
    if ((signed short)ui != 0x3333) {return 1;}
    if ((signed short)ss != -0x333) {return 1;}
    if ((signed short)us != 0x333) {return 1;}
    if ((signed short)sc != -3) {return 1;}
    if ((signed short)uc != 3) {return 1;}

    if ((unsigned short)si != 0xCCCD) {return 1;}
    if ((unsigned short)ui != 0x3333) {return 1;}
    if ((unsigned short)ss != 0xFCCD) {return 1;}
    if ((unsigned short)us != 0x333) {return 1;}
    if ((unsigned short)sc != 0xFFFD) {return 1;}
    if ((unsigned short)uc != 3) {return 1;}

    if ((signed int)si != -0x33333) {return 1;}
    if ((signed int)ui != 0x33333) {return 1;}
    if ((signed int)ss != -0x333) {return 1;}
    if ((signed int)us != 0x333) {return 1;}
    if ((signed int)sc != -3) {return 1;}
    if ((signed int)uc != 3) {return 1;}

    if ((unsigned int)si != 0xFFFCCCCD) {return 1;}
    if ((unsigned int)ui != 0x33333) {return 1;}
    if ((unsigned int)ss != 0xFFFFFCCD) {return 1;}
    if ((unsigned int)us != 0x333) {return 1;}
    if ((unsigned int)sc != 0xFFFFFFFD) {return 1;}
    if ((unsigned int)uc != 3) {return 1;}

    return 0;
}
