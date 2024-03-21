// The MIT License (MIT)
// Copyright (c) 2023-2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

int main(void) {
    int x = 5;
    if (x != 5 ? 4 - 3 : 6 - 6) {
        return 1;
    }
    if (x != 5 ? 4 - 4 : 1 - 0) {
        return 0;
    }
    return 1;
}
