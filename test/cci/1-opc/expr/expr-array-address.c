// The MIT License (MIT)
// Copyright (c) 2023-2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

int main(void) {
    char x[1];
    *x = 4;
    if (x != &x) {
        return 1;
    }
    if (*x != 4) {
        return 1;
    }
    if (*&x != 4) {
        return 1;
    }
    return 0;
}
