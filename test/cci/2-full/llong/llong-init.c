// The MIT License (MIT)
// Copyright (c) 2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

int main(void) {

    // make sure basic initializers are working properly.
    // we don't bother to test init from anything other than long long; those
    // would be tests of casting which are in llong-cast.c .

    long long a = 0x12345678abcdef12ll;
    if (a != 0x12345678abcdef12ll) return 1;

    unsigned long long b = 0xabcdefab12345678ull;
    if (b != 0xabcdefab12345678ull) return 2;

    long long c = 1ll;
    if (c != 1ll) return 3;

    struct {
        long long a;
        unsigned long long b;
    } d = {0x12345678abcdef12ll, 0xabcdefab12345678ull};
    if (d.a != 0x12345678abcdef12ll) return 4;
    if (d.b != 0xabcdefab12345678ull) return 5;

}
