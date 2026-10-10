// The MIT License (MIT)
// Copyright (c) 2026 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

// TODO INFINITY is moved to <float.h> in C23
// TODO INFINITY and NAN are missing
//#include <math.h>

int main(void) {
    float a = 1.0f;
    if (a != a) return 81;
    if (!(a == a)) return 82;
    if (a < a) return 83;
    if (a > a) return 84;
    if (!(a <= a)) return 85;
    if (!(a >= a)) return 86;

    float b = 1.0f;
    if (a != b) return 1;
    if (!(a == b)) return 2;
    if (a < b) return 3;
    if (a > b) return 4;
    if (!(a <= b)) return 5;
    if (!(a >= b)) return 6;

    float c = 2.0f;
    if (a == c) return 11;
    if (!(a != c)) return 12;
    if (!(a < c)) return 13;
    if (a > c) return 14;
    if (!(a <= c)) return 15;
    if (a >= c) return 16;

    float z = 0.0f;
    if (z != z) return 21;
    if (!(z == z)) return 22;
    if (z < z) return 23;
    if (z > z) return 24;
    if (!(z <= z)) return 25;
    if (!(z >= z)) return 26;
    if (z == c) return 31;
    if (!(z != c)) return 32;
    if (!(z < c)) return 33;
    if (z > c) return 34;
    if (!(z <= c)) return 35;
    if (z >= c) return 36;

    float n = -1.0f;
    if (n == c) return 41;
    if (!(n != c)) return 42;
    if (!(n < c)) return 43;
    if (n > c) return 44;
    if (!(n <= c)) return 45;
    if (n >= c) return 46;

    if (c == n) return 51;
    if (!(c != n)) return 52;
    if (!(c > n)) return 53;
    if (c < n) return 54;
    if (!(c >= n)) return 55;
    if (c <= n) return 56;

    // TODO we don't have an INFINITY constant yet
    union {
        float f;
        unsigned u;
    } u;
    u.u = 0b0'11111111'00000000000000000000000u;
    float i = u.f;
    u.u = 0b1'11111111'00000000000000000000000u;
    float ii = u.f;

    if (i != i) return 71;
    if (!(i == i)) return 72;
    if (i < i) return 73;
    if (i > i) return 74;
    if (!(i <= i)) return 75;
    if (!(i >= i)) return 76;

    if (ii == i) return 91;
    if (!(ii != i)) return 92;
    if (!(ii < i)) return 93;
    if (ii > i) return 94;
    if (!(ii <= i)) return 95;
    if (ii >= i) return 96;

    if (ii == c) return 101;
    if (!(ii != c)) return 102;
    if (!(ii < c)) return 103;
    if (ii > c) return 104;
    if (!(ii <= c)) return 105;
    if (ii >= c) return 106;

    u.u = 0b0'11111111'10000000000000000000000u; // quiet nan
    float nn = u.f;

    if (nn == nn) return 111;
    if (!(nn != nn)) return 112;
    if (nn < nn) return 113;
    if (nn > nn) return 114;
    if (nn <= nn) return 115;
    if (nn >= nn) return 116;

    if (a == nn) return 131;
    if (!(a != nn)) return 132;
    if (a < nn) return 133;
    if (a > nn) return 134;
    if (a <= nn) return 135;
    if (a >= nn) return 136;

    if (nn == ii) return 141;
    if (!(nn != ii)) return 142;
    if (nn < ii) return 143;
    if (nn > ii) return 144;
    if (nn <= ii) return 145;
    if (nn >= ii) return 146;
}
