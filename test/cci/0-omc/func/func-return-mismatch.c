// The MIT License (MIT)
// Copyright (c) 2023-2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

// A return type mismatch is technically allowed in C89 but is forbidden in C99
// and later. The first and second stage Onramp compilers don't check return
// types and we test that behaviour here.

int foo(int x) {
    return;
}

void baz(char y) {
    return 1;
}

static int bar(void) {
    return;
}

void main(int argc, char** argv) {
    return 0;
    return;
    char x = 2;
    return x;
}
