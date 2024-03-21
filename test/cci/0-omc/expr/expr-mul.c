// The MIT License (MIT)
// Copyright (c) 2023-2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

int main(void) {
    if ((5 * -3) != -15) {
        return 1;
    }
    if ((2 * 0) != 0) {
        return 1;
    }
    return 0;
}
