// The MIT License (MIT)
// Copyright (c) 2026 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

int main(void) {
    // Ensure arithmetic conversions are performed correctly in conditional
    // operator

    // widening of same sign
    if ((1 ? 2 : 3LL) != 2) return 1;
    if ((0 ? 4 : 5LL) != 5LL) return 2;
    if ((1 ? 6LL : 7) != 6LL) return 3;
    if ((0 ? 8LL : 9) != 9) return 4;

    // widening of opposing signs
    if ((1 ? 2u : 3LL) != 2u) return 5;
    if ((0 ? 4u : 5LL) != 5LL) return 6;
    if ((1 ? 6uLL : 7) != 6uLL) return 7;
    if ((0 ? 8uLL : 9) != 9) return 8;
    if ((1 ? 2 : 3LLu) != 2u) return 9;
    if ((0 ? 4 : 5LLu) != 5LL) return 10;
    if ((1 ? 6LL : 7u) != 6uLL) return 11;
    if ((0 ? 8LL : 9u) != 9) return 12;
}
