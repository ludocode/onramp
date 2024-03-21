// The MIT License (MIT)
// Copyright (c) 2023-2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

int main(void) {
    short x = 0xfff2345;
    if (x != 0x2345) {
        return 1;
    }
    return 0;
}
