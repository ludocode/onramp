// The MIT License (MIT)
// Copyright (c) 2023-2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

int main(void) {
    int x = 0;
    int y = 0;
    if (1) {
        x = 5;
    } else {
        y = 7;
    }
    if ((x != 5) | (y != 0)) {
        return 1;
    }

    x = 0;
    y = 0;
    if (0) {
        x = 5;
    } else {
        y = 7;
    }
    if ((x != 0) | (y != 7)) {
        return 1;
    }

    return 0;
}
