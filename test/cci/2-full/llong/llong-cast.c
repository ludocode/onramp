// The MIT License (MIT)
// Copyright (c) 2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

int main(void) {

    /*
    // ull to unsigned
    unsigned a = 0x12345678abcdef12llu;
    if (a != 0xabcdef12u) return 1;
    unsigned b = 0x10llu;
    if (b != 0x10u) return 2;

    // ull to signed
    int c = 0x12345678abcdef12llu;
    if (c != 0xabcdef12) return 3;
    int d = 0x10llu;
    if (d != 0x10) return 4;

    // ll to signed
    int e = 0x12345678abcdef12ll;
    if (e != 0xabcdef12) return 5;
    int f = 0x12345678fffffffbll;
    if (f != -5) return 6;

    // ll to unsigned
    unsigned g = 0x12345678abcdef12ll;
    if (g != 0xabcdef12u) return 7;
    unsigned h = 0x12345678fffffffbll;
    if (h != (unsigned)-5) return 8;


    // ull to bool
    _Bool i = 0x2ull;
    if (i != 1) return 9;
    _Bool j = 0x8000000000000000ull;
    if (j != 1) return 10;

    // ll to bool
    _Bool k = 0x4ll;
    if (k != 1) return 11;
    _Bool l = 0xffffffff00000000ll;
    if (l != 1) return 12;
    */

    // bool to ll
    long long m = (_Bool)0;
    if (m != 0ll) return 13;
    /*
    long long n = (_Bool)2;
    if (n != 1ll) return 14;

    // bool to ull
    unsigned long long o = (_Bool)0;
    if (o != 0ull) return 15;
    unsigned long long p = (_Bool)2;
    if (p != 1ull) return 16;


    // unsigned to ll
    long long q = (unsigned)0xf2345678u;
    if (q != 0xf2345678ll) return 17;

    // signed to ll
    long long r = (signed)0xf2345678;
    if (r != 0xfffffffff2345678ll) return 18;

    // unsigned to ull
    unsigned long long s = (unsigned)0xf2345678u;
    if (s != 0xf2345678ll) return 19;

    // signed to ull
    unsigned long long t = (signed)0xf2345678;
    if (t != 0xfffffffff2345678ll) return 20;
    */

}
