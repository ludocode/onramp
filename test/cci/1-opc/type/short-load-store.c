// The MIT License (MIT)
// Copyright (c) 2023-2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

short x[4];

int main(void) {
    x[0] = 0x1234;
    x[1] = 0x5678;
    x[2] = 0x147A;
    x[3] = 0x258B;

    if (x[0] != 0x1234) {
        return 1;
    }
    if (x[1] != 0x5678) {
        return 2;
    }
    if (x[2] != 0x147A) {
        return 3;
    }
    if (x[3] != 0x258B) {
        return 4;
    }

    int* y = (int*)x;
    if (y[0] != 0x56781234) {
        return 5;
    }
    if (y[1] != 0x258B147A) {
        return 6;
    }

    return 0;
}
