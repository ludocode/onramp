// The MIT License (MIT)
// Copyright (c) 2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

int main(void) {
    if (0ull == 0ull); else return 1;
    if (0ull != 0ull) return 2;

    if (0ull == 1ull) return 3;
    if (0ull != 1ull); else return 4;

    if (0x100000000ull == 0x100000000ull); else return 5;
    if (0x100000000ull != 0x100000000ull) return 6;

    if (0x100000000ull == 0x100000001ull) return 7;
    if (0x100000000ull != 0x100000001ull); else return 8;

    if (0xffffffffffffffffull == 0xffffffffffffffffull); else return 9;
    if (0xffffffffffffffffull != 0xffffffffffffffffull) return 10;
}
