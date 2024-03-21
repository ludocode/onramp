// The MIT License (MIT)
// Copyright (c) 2023-2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

int main(void) {
    char x = -5;
    if ((0?3:x) != -5) {
        return 1;
    }
    return 0;
}
