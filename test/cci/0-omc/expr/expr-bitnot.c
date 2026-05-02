// The MIT License (MIT)
// Copyright (c) 2023-2026 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

int main(void) {
    if (~0xFFFFFFFF) {
        return 1;
    }
    if (~(unsigned char)0xff != 0xFFFFFF00) {
        return 2;
    }
    if (~(unsigned char)0xfe != 0xFFFFFF01) {
        return 3;
    }
    if (~0x0f != -16) {
        return 4;
    }
}
