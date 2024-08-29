// The MIT License (MIT)
// Copyright (c) 2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

int main(void) {
    // As long as nothing refers to this, we won't get a linker error due to it
    // not existing.
    extern int x;
}
