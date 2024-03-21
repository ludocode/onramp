// The MIT License (MIT)
// Copyright (c) 2023-2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

int main(void) {
    // a bug building ld/1
    int offset = 12;
    if ((offset < -0x8000) | (offset > 0xFFFF)) {
        return 1;
    }
    return 0;
}
