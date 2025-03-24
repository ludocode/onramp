// The MIT License (MIT)
// Copyright (c) 2025 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

int main(void) {
    int x = 1;
    goto foo; // error, cannot cross defer
    defer x = 2;
foo:
    return x;
}
