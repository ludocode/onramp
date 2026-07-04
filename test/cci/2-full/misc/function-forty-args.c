// The MIT License (MIT)
// Copyright (c) 2023-2026 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

// The frame offset for the 34th argument and later does not fit in a mix-type
// byte (8+(34-4)*4==128). This tests that we can pass more than 33 arguments.

int foo(int a1, int a2, int a3, int a4, int a5,
        int a6, int a7, int a8, int a9, int a10,
        int a11, int a12, int a13, int a14, int a15,
        int a16, int a17, int a18, int a19, int a20,
        int a21, int a22, int a23, int a24, int a25,
        int a26, int a27, int a28, int a29, int a30,
        int a31, int a32, int a33, int a34, int a35,
        int a36, int a37, int a38, int a39, int a40)
{
    if (a1 != 1) {return 1;}
    if (a2 != 2) {return 2;}
    if (a3 != 3) {return 3;}
    if (a4 != 4) {return 4;}
    if (a5 != 5) {return 5;}
    if (a6 != 6) {return 6;}
    if (a7 != 7) {return 7;}
    if (a8 != 8) {return 8;}
    if (a9 != 9) {return 9;}
    if (a10 != 10) {return 10;}
    if (a11 != 11) {return 11;}
    if (a12 != 12) {return 12;}
    if (a13 != 13) {return 13;}
    if (a14 != 14) {return 14;}
    if (a15 != 15) {return 15;}
    if (a16 != 16) {return 16;}
    if (a17 != 17) {return 17;}
    if (a18 != 18) {return 18;}
    if (a19 != 19) {return 19;}
    if (a20 != 20) {return 20;}
    if (a21 != 21) {return 21;}
    if (a22 != 22) {return 22;}
    if (a23 != 23) {return 23;}
    if (a24 != 24) {return 24;}
    if (a25 != 25) {return 25;}
    if (a26 != 26) {return 26;}
    if (a27 != 27) {return 27;}
    if (a28 != 28) {return 28;}
    if (a29 != 29) {return 29;}
    if (a30 != 30) {return 30;}
    if (a31 != 31) {return 31;}
    if (a32 != 32) {return 32;}
    if (a33 != 33) {return 33;}
    if (a34 != 34) {return 34;}
    if (a35 != 35) {return 35;}
    if (a36 != 36) {return 36;}
    if (a37 != 37) {return 37;}
    if (a38 != 38) {return 38;}
    if (a39 != 39) {return 39;}
    if (a40 != 40) {return 40;}
    return 0;
}

int main(void) {
    return foo(
            1, 2, 3, 4, 5,
            6, 7, 8, 9, 10,
            11, 12, 13, 14, 15,
            16, 17, 18, 19, 20,
            21, 22, 23, 24, 25,
            26, 27, 28, 29, 30,
            31, 32, 33, 34, 35,
            36, 37, 38, 39, 40);
}
