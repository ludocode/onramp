// The MIT License (MIT)
// Copyright (c) 2023-2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

int main(void) {
    char y;
    y = 5;
    if (y != 5) {
        return 1;
    }

    char x = -1;
    int w = x;
    if (w != -1) {
        return 1;
    }

    return 0;
}
