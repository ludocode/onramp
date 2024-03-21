// The MIT License (MIT)
// Copyright (c) 2023-2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

int main(void) {
    if (2 == 3) {
        return 1;
    }
    if (4 == 3) {
        return 1;
    }
    if (3 == 3) {
        return 0;
    }
    return 1;
}
