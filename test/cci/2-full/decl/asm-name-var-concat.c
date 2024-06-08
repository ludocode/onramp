// The MIT License (MIT)
// Copyright (c) 2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

// String literal concatenation is allowed in asm names.
int volatile test_test_test;
extern volatile int t __asm__(
        "test"
        "_"
        "test"
        "_"
        "test");

int main(void) {
    if (test_test_test != 0) return 1;
    if (t != 0) return 1;
    t = 4;
    if (test_test_test != 4) return 1;
    if (t != 4) return 1;
}
