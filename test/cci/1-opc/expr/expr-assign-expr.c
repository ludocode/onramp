// The MIT License (MIT)
// Copyright (c) 2023-2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

int x;
int main(void) {
    x = 4 + 7;
    if (x != 11) {
        return 1;
    }

    int y = 4;
    x = y = 1;
    if (x != 1) {
        return 1;
    }
    if (y != 1) {
        return 1;
    }

    return 0;
}
