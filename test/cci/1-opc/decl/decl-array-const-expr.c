// The MIT License (MIT)
// Copyright (c) 2023-2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

int main(void) {
    int x[3 + 8 * sizeof(short) / 3];
    if (sizeof(x) != 32) {
        return 1;
    }
    return 0;
}
