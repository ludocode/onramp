// The MIT License (MIT)
// Copyright (c) 2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

int main(void) {

    // test initializer cast
    _Bool x = 0x100;
    if (x != 1)
        return 1;

    // test assignment cast
    x = 0x100;
    if (x != 1)
        return 2;

    // test char truncation
    x = (char)0x200;
    if (x != 0)
        return 3;

    // test short truncation
    x = (short)0xfff30000;
    if (x != 0)
        return 4;

    return 0;
}
