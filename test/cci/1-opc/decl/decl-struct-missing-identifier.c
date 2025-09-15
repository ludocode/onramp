// The MIT License (MIT)
// Copyright (c) 2025 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

//% FAIL cci
//% NON_STANDARD    // TODO only nonstd until we fix cci/2

struct foo {
    int x;
};

int main(void) {
    struct foo, b; // ERROR: not a forward struct declaration, missing identifier
    return 1;
}
