// The MIT License (MIT)
// Copyright (c) 2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

int main(void) {
    short x = 3;
    unsigned char* p = (unsigned char*)"a";
    p ^= x;
    p ^= x = 5;
    p ^= 6;
    if (*p != 'a') return 1;
}
