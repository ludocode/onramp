// The MIT License (MIT)
// Copyright (c) 2023-2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

int main(void) {
    if (~0xFFFFFFFF) {
        return 1;
    }
    if (~(char)0xff) {
        return 2;
    }
    if (~(char)0xfe != 1) {
        return 3;
    }
    if (~(char)0x0f != -16) {
        return 4;
    }
}
