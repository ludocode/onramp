// The MIT License (MIT)
// Copyright (c) 2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

int x[3][5];

int main(void) {
    if (sizeof(x) != 15 * sizeof(int)) return 16;
    if (sizeof(x[1]) != 5 * sizeof(int)) return 17;

    x[0][0] = 1;
    x[0][1] = 2;
    x[0][2] = 3;
    x[0][3] = 4;
    x[0][4] = 5;
    x[1][0] = 6;
    x[1][1] = 7;
    x[1][2] = 8;
    x[1][3] = 9;
    x[1][4] = 10;
    x[2][0] = 11;
    x[2][1] = 12;
    x[2][2] = 13;
    x[2][3] = 14;
    x[2][4] = 15;

    int* y = (int*)x;
    if (y[0] != 1) return 1;
    if (y[1] != 2) return 2;
    if (y[2] != 3) return 3;
    if (y[3] != 4) return 4;
    if (y[4] != 5) return 5;
    if (y[5] != 6) return 6;
    if (y[6] != 7) return 7;
    if (y[7] != 8) return 8;
    if (y[8] != 9) return 9;
    if (y[9] != 10) return 10;
    if (y[10] != 11) return 11;
    if (y[11] != 12) return 12;
    if (y[12] != 13) return 13;
    if (y[13] != 14) return 14;
    if (y[14] != 15) return 15;
}
