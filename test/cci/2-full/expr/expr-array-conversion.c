// The MIT License (MIT)
// Copyright (c) 2023-2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

int main(void) {
    char x[3];

    *x = 5;
    *(x + 1) = 7;
    x[2] = 11;

    if (sizeof(*x) != 1) return 1;
    if (sizeof(x) != 3) return 2;
    if (sizeof(&x) != sizeof(char*)) return 3;
    if (sizeof(x + 0) != sizeof(char*)) return 4;
    if (sizeof((char*)x) != sizeof(char*)) return 5;
    if (sizeof(*&x) != 3) return 6;
    if (sizeof(**&x) != 1) return 7;

    if (x != &x) return 11;
    if (&x != &x[0]) return 12;
    if (&x != x + 0) return 13;
    if (*&x != x + 0) return 14;
    if (*&x != x) return 15;
    if (*&x != &x) return 16;

    if (**&x != 5) return 21;
    if ((*&x)[1] != 7) return 22;
    if (*&x[2] != 11) return 23;
    if (*(char*)x != 5) return 24;
    if (*(char*)&x != 5) return 25;
}
