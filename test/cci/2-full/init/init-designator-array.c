// The MIT License (MIT)
// Copyright (c) 2026 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

int a[3] = {[0] = 1};
int c[] = {[2] = 3};
int d[] = {[1] = 4, 5, [0] = 6};

int main(void) {
    int b[3] = {[1] = 2};
    int e[] = {[0] = 7, [2] = 8, [1] = 9};

    if (sizeof(a) != sizeof(int) * 3) return 20;
    if (sizeof(b) != sizeof(int) * 3) return 21;
    if (sizeof(c) != sizeof(int) * 3) return 22;
    if (sizeof(d) != sizeof(int) * 3) return 23;
    if (sizeof(e) != sizeof(int) * 3) return 24;

    if (a[0] != 1) return 1;
    if (a[1] != 0) return 2;
    if (a[2] != 0) return 3;

    if (b[0] != 0) return 4;
    if (b[1] != 2) return 5;
    if (b[2] != 0) return 6;

    if (c[0] != 0) return 7;
    if (c[1] != 0) return 8;
    if (c[2] != 3) return 9;

    if (d[0] != 6) return 10;
    if (d[1] != 4) return 11;
    if (d[2] != 5) return 12;

    if (e[0] != 7) return 13;
    if (e[1] != 9) return 14;
    if (e[2] != 8) return 15;
}
