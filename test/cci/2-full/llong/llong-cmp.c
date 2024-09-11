// The MIT License (MIT)
// Copyright (c) 2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

int main(void) {


    // unsigned

    // low
    if (1ull < 0ull) return 1;
    if (1ull <= 0ull) return 2;
    if (0ull > 1ull) return 3;
    if (0ull >= 1ull) return 4;

    // high, low equal
    if (0x100000000ull < 0ull) return 5;
    if (0x100000000ull <= 0ull) return 6;
    if (0ull > 0x100000000ull) return 7;
    if (0ull >= 0x100000000ull) return 8;

    // high, low greater
    if (0x100000000ull < 1ull) return 9;
    if (0x100000000ull <= 1ull) return 10;
    if (1ull > 0x100000000ull) return 11;
    if (1ull >= 0x100000000ull) return 12;

    // high, low less
    if (0x100000001ull < 0ull) return 13;
    if (0x100000001ull <= 0ull) return 14;
    if (0ull > 0x100000001ull) return 15;
    if (0ull >= 0x100000001ull) return 16;


    // signed, positive

    // low
    if (1ll < 0ll) return 17;
    if (1ll <= 0ll) return 18;
    if (0ll > 1ll) return 19;
    if (0ll >= 1ll) return 20;

    // high, low equal
    if (0x100000000ll < 0ll) return 21;
    if (0x100000000ll <= 0ll) return 22;
    if (0ll > 0x100000000ll) return 23;
    if (0ll >= 0x100000000ll) return 24;

    // high, low greater
    if (0x100000000ll < 1ll) return 25;
    if (0x100000000ll <= 1ll) return 26;
    if (1ll > 0x100000000ll) return 27;
    if (1ll >= 0x100000000ll) return 28;

    // high, low less
    if (0x100000001ll < 0ll) return 29;
    if (0x100000001ll <= 0ll) return 30;
    if (0ll > 0x100000001ll) return 31;
    if (0ll >= 0x100000001ll) return 32;


    // signed, negative

    // low
    if (-1ll > 0ll) return 33;
    if (-1ll >= 0ll) return 34;
    if (0ll < -1ll) return 35;
    if (0ll <= -1ll) return 36;

    // high, low equal
    if (-0x100000000ll > 0ll) return 37;
    if (-0x100000000ll >= 0ll) return 38;
    if (0ll < -0x100000000ll) return 39;
    if (0ll <= -0x100000000ll) return 40;

    // high, low greater
    if (-0x100000000ll > 1ll) return 41;
    if (-0x100000000ll >= 1ll) return 42;
    if (1ll < -0x100000000ll) return 43;
    if (1ll <= -0x100000000ll) return 44;

    // high, low less
    if (-0x100000001ll > 0ll) return 45;
    if (-0x100000001ll >= 0ll) return 46;
    if (0ll < -0x100000001ll) return 47;
    if (0ll <= -0x100000001ll) return 48;

}
