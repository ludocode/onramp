// The MIT License (MIT)
// Copyright (c) 2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

int main(void) {
    struct foo;

    return 1 | sizeof(struct foo); // ERROR: `struct foo` is incomplete

    // Although we are completing foo within the function, the above sizeof
    // should not be allowed because it was incomplete at the point of the
    // sizeof. (This ensures the compiler actually checks at the point of the
    // sizeof rather than parsing the whole function and then checking at
    // codegen time.)
    struct foo {
        int x;
    };
}
