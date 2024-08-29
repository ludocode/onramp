// The MIT License (MIT)
// Copyright (c) 2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

int x; // tentative definition of x
static int x; // ERROR: x redeclared with different linkage

int main(void) {
    return 1;
}
