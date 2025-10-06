// The MIT License (MIT)
// Copyright (c) 2025 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

//% FAIL cci

int main(void) {
    char c[16];
    void* v = c;
    v[0] = 0; // ERROR: assigning dereferenced void not allowed
    return 1;
}
