// The MIT License (MIT)
// Copyright (c) 2026 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

int main(void) {
    int A = 0;

    #define A return A
    #define B(x) x
    // This should be expanded to `return A;`. A should be expanded only once.
    B(A);

    return 1;
}
