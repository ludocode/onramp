// The MIT License (MIT)
// Copyright (c) 2023-2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

int main(void) {
    int x = 5;
    int y = 10;
    if ((x * 2) != y) {
        return 1;
    }

    int z = 15;
    if (z != (x + y)) {
        return 1;
    }

    int* px = &x;
    int** ppx = &px;
    int* pz = &z;
    if (px != &x) {
        return 1;
    }
    if (*ppx != &x) {
        return 1;
    }
    if (*pz != (3 * **ppx)) {
        return 1;
    }

    int X = *px;
    if (x != X) {
        return 1;
    }

    return 0;
}
