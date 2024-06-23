// The MIT License (MIT)
// Copyright (c) 2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

struct a {
    int x, y;
};

int main(void) {
    struct a a; // using global `struct a` in inner scope
    a.y = 0;
    a.x = 1;
    return a.y;
}
