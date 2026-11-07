// The MIT License (MIT)
// Copyright (c) 2026 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

int main(void) {
    double d = 1.0;
    float f1 = (float)d;
    if (f1 != 1.0f) return 1;

    d = 2.0;
    float f2 = d;
    if (f2 != 2.0f) return 2;

    float f3 = 3.0;
    if (f3 != 3.0f) return 3;

    d = 4.0;
    int i4 = d;
    if (i4 != 4) return 4;

    d = 5.0;
    int i5 = d;
    if (i5 != 5) return 5;

    unsigned u6 = 6.0;
    if (u6 != 6) return 6;

    long long ll7 = 7.0;
    if (ll7 != 7ll) return 7;

    d = 8.0;
    unsigned long long ull8 = d;
    if (ull8 != 8llu) return 8;
}
