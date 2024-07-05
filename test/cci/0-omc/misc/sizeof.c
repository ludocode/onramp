// The MIT License (MIT)
// Copyright (c) 2023-2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

int main(void) {
    if (sizeof(int) != 4) {
        return 1;
    }
    if (sizeof(char) != 1) {
        return 1;
    }
    if (sizeof(char*) != 4) {
        return 1;
    }
    if (sizeof(void******) != 4) {
        return 1;
    }
    return 0;
}
