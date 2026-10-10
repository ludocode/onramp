// The MIT License (MIT)
// Copyright (c) 2026 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

int main(void) {

    // float to unsigned
    unsigned u = __builtin_bit_cast(unsigned, 1.0f);
    if (u != 0b0'01111111'00000000000000000000000) return 1;

    // unsigned to float
    float f = __builtin_bit_cast(float, 0b0'01111111'00000000000000000000000);
    if (f != 1.0f) return 2;

    // TODO ullong to double

}
