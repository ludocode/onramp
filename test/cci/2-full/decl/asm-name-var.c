// The MIT License (MIT)
// Copyright (c) 2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

// We mark these as volatile because an optimizing compiler can assume these
// don't alias. (Obviously Onramp doesn't optimize on this but we want these
// test cases to work in general.)
volatile int foo;
extern volatile int bar __asm__("foo");

int main(void) {
    if (foo != 0) return 1;
    if (bar != 0) return 1;
    foo = 4;
    if (foo != 4) return 1;
    if (bar != 4) return 1;
    bar = 7;
    if (foo != 7) return 1;
    if (bar != 7) return 1;
}
