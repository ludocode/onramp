// The MIT License (MIT)
// Copyright (c) 2023-2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

int main(void) {
    if (-5 != (0 - 5)) {
        return 1;
    }

    int x = -4;
    if (-x != 4) {
        return 1;
    }

    return 0;
}
