// The MIT License (MIT)
// Copyright (c) 2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

// declare outer struct
struct a;

int main(void) {

    // define inner struct; shadows outer struct
    struct a {
        int x;
    } foo;

    foo.x = 0;
    return foo.x;
}

// define outer struct. should be no conflict with inner struct.
struct a {
    char y;
};
