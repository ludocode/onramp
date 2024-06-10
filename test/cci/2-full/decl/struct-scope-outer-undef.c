// The MIT License (MIT)
// Copyright (c) 2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

struct a; // declares `struct a` at file scope

int main(void) {
    struct a { // defines unrelated `struct a` in an inner scope
        int x, y;
    };
    struct a a; // ok
}

struct a a; // error: `struct a` is not defined
