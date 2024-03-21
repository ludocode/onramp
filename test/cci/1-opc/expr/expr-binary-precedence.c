// The MIT License (MIT)
// Copyright (c) 2023-2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

int main(void) {

    // == has lower precedence than +
    if (0 == 2 + 1) {
        return 1;
    }

    // simple arithmetic with comparison in the middle
    if (5 * 3 + 10 / 2 != 30 * 2 / 3) {
        return 1;
    }

    return 0;
}
