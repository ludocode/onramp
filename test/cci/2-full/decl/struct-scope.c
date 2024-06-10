// The MIT License (MIT)
// Copyright (c) 2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

struct a {
    int x;
};

int main(void) {
    struct a a; // using global `struct a` in inner scope
    //TODO struct access not yet implemented
    //foo.x = 0;
    //return foo.x;
}
