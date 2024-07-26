// The MIT License (MIT)
// Copyright (c) 2023-2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

int main(void) {
    if (!1) {
        return 1;
    }
    if (!(char)2) {
        return 2;
    }
    if (!(char)0x100) {
        if (!0) {
            return 0;
        }
    }
    return 3;
}
