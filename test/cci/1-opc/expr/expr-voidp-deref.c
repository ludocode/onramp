// The MIT License (MIT)
// Copyright (c) 2025 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

//% FAIL cci
//% NON_STANDARD

int main(void) {
    char c[16];
    void* v = c;
    *v; // ERROR: dereferencing void not allowed (only a warning in most compilers)
    return 1;
}
