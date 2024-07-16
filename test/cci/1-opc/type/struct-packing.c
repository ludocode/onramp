// The MIT License (MIT)
// Copyright (c) 2023-2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

struct foo {
    char a0;
    int  a1;

    char b0;
    char b1;
    int  b2;

    char c0;
    char c1;
    char c2;
    int  c3;

    char d0;
    char d1;
    char d2;
    char d3;
    int  d4;

    short e0;
    int   e1;

    short f0;
    short f1;
    int   f2;

    char  g0;
    short g1;
    int   g2;

    char  h0;
    char  h1;
    short h2;
    int   h3;

    int   i0;
    int   i1;
};

int main(void) {
    struct foo foo;


    // make sure the padding is what we expect

    if (4 + (char*)&foo.a0 != (char*)&foo.a1) return 1;

    if (1 + (char*)&foo.b0 != (char*)&foo.b1) return 2;
    if (3 + (char*)&foo.b1 != (char*)&foo.b2) return 3;

    if (1 + (char*)&foo.c0 != (char*)&foo.c1) return 4;
    if (1 + (char*)&foo.c1 != (char*)&foo.c2) return 5;
    if (2 + (char*)&foo.c2 != (char*)&foo.c3) return 6;

    if (1 + (char*)&foo.d0 != (char*)&foo.d1) return 7;
    if (1 + (char*)&foo.d1 != (char*)&foo.d2) return 8;
    if (1 + (char*)&foo.d2 != (char*)&foo.d3) return 9;
    if (1 + (char*)&foo.d3 != (char*)&foo.d4) return 10;

    if (4 + (char*)&foo.e0 != (char*)&foo.e1) return 11;

    if (2 + (char*)&foo.f0 != (char*)&foo.f1) return 12;
    if (2 + (char*)&foo.f1 != (char*)&foo.f2) return 13;

    if (2 + (char*)&foo.g0 != (char*)&foo.g1) return 14;
    if (2 + (char*)&foo.g1 != (char*)&foo.g2) return 15;

    if (1 + (char*)&foo.h0 != (char*)&foo.h1) return 16;
    if (1 + (char*)&foo.h1 != (char*)&foo.h2) return 17;
    if (2 + (char*)&foo.h2 != (char*)&foo.h3) return 18;

    if (4 + (char*)&foo.i0 != (char*)&foo.i1) return 19;


    // make sure we can assign struct members without alignment errors

    foo.b0 = 1;
    foo.b1 = 2;
    foo.b2 = 3;

    if (*((char*)&foo.b0) != 1) return 20;
    if (*((char*)&foo.b0 + 1) != 2) return 21;
    if (*(int*)((char*)&foo.b0 + 4) != 3) return 22;

    return 0;
}
