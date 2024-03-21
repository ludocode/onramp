// The MIT License (MIT)
// Copyright (c) 2023-2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

int main(void) {

    // truncating cast
    if ((char)0x123 != 0x23) {
        return 1;
    }

    // truncating cast with sign extension
    if ((char)0x2aa != 0xffffffaa) {
        return 1;
    }

    // casting l-values
    char x = 2;
    if (((int)x - 3) != -1) {
        return 1;
    }

    // truncating cast l-value
    int y = 0x123;
    if (0x23 != (char)y) {
        return 1;
    }

    return 0;
}
