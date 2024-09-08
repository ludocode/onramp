// The MIT License (MIT)
// Copyright (c) 2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

int main(void) {
    int a = 0'7'5'5; // unlike 0x/0b, digit separator is allowed after 0 octal prefix
    if (a != 493) return 1;

    int b = 0b1'1'1'1;
    if (b != 15) return 2;

    int c = 0xf'f'f'f;
    if (c != 65535) return 3;

    int d = 1'000'000'000;
    if (d != 1000000000) return 4;
}
