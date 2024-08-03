// The MIT License (MIT)
// Copyright (c) 2023-2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

int foo(int a, int x[5], int b) {
    if (a != -5) {return 1;}
    if (x[0] != 0) {return 2;}
    if (x[1] != 1) {return 3;}
    if (x[2] != 2) {return 4;}
    if (x[3] != 3) {return 5;}
    if (x[4] != 4) {return 6;}
    if (b != -7) {return 7;}
    return 0;
}

int main(void) {
    int x[5];
    x[0] = 0;
    x[1] = 1;
    x[2] = 2;
    x[3] = 3;
    x[4] = 4;
    return foo(-5, x, -7);
}
