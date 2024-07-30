// The MIT License (MIT)
// Copyright (c) 2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

// `struct x` is visible only within the prototype of foo's argument, not
// within the prototype of foo, and therefore not within the definition of foo.
int foo(void (*x)(struct x {int x;})) {
    struct x y; // ERROR, `struct x` is not visible here
    return 2;
}

int main(void) {
    return 2 - foo(0);
}
