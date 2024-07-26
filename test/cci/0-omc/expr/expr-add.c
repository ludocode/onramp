// The MIT License (MIT)
// Copyright (c) 2023-2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

int main(void) {
    if ((-1 + -10) != -11) {
        return 1;
    }

    if ((5 + 3) != 8) {
        return 2;
    }

    // test promotion from char to int
    char x = -3;
    if ((x + -5) != -8) {
        return 3;
    }

    // test correct truncation of chars
    if (((char)0x101 + (char)0x102) != 3) {
        return 4;
    }
    if (((char)0xffffff03 + (char)0xeeeeee04) != 7) {
        return 5;
    }

    return 0;
}
