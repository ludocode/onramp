// The MIT License (MIT)
// Copyright (c) 2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

int main(void) {

    // make sure we can add
    (void)(3ull + 5ull);

    // make sure we can add correctly
    if (0ull + 0ull != 0ull) return 1;
    if (0ull + 0ull != 1ull); else return 2;
    if (0ull + 0ull == 0ull); else return 3;

    if (3ull + 5ull != 8ull) return 4;
    if (0x100000001ull + 0x1000000010ull != 0x1100000011ull) return 5;
}
