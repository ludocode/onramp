// The MIT License (MIT)
// Copyright (c) 2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

struct a { // defines `struct a` at file scope
    int x;
};
struct a a; // ok

int main(void) {
    struct a; // declares unrelated `struct a` in an inner scope
    struct a a; // error: `struct a` isn't defined
}
