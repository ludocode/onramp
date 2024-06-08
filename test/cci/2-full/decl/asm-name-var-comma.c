// The MIT License (MIT)
// Copyright (c) 2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

// asm names are attached to the declarator and can be used with multiple
// declarators in a declaration.
int volatile extern a __asm__("b"), b, c __asm__("b");
int volatile b;

int main(void) {
    if (a != 0) return 1;
    if (b != 0) return 1;
    if (c != 0) return 1;
    a = 4;
    if (a != 4) return 1;
    if (b != 4) return 1;
    if (c != 4) return 1;
}
