// The MIT License (MIT)
// Copyright (c) 2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

int main(void) {
    short x = 4;
    unsigned char* p = (unsigned char*)"abcdefgh";
    if (*p != 'a') return 1;

    p += x;
    if (*p != 'e') return 2;
    p += x = 5;
    p -= x = 1;
    if (*p != 0) return 3;

    x = 5;
    p -= x;
    if (*p != 'd') return 4;
    p -= x / 2;
    if (*p != 'b') return 5;
}
