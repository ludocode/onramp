// The MIT License (MIT)
// Copyright (c) 2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

int main(void) {
    extern int x;
    int x = 1; // ERROR, x redeclared in block scope
    return x;
}
