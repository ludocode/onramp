// The MIT License (MIT)
// Copyright (c) 2023-2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

int main(void) {
    int x[2];
    x[0] = 5;
    x[1] = 7;

    int* p = x;
    if (*p++ != 5) return 1;
    if (*p-- != 7) return 2;
    if (*++p != 7) return 3;
    if (*--p != 5) return 4;
}
