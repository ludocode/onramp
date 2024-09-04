// The MIT License (MIT)
// Copyright (c) 2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

int foo(void) {
    return 5;
}

int main(void) {
    int (*f)(void) = foo;

    // call f as a function pointer
    if (5 != f()) return 1;

    // dereference f and call it as a function
    if (5 != (*f)()) return 2;
}
