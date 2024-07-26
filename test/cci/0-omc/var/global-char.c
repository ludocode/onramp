// The MIT License (MIT)
// Copyright (c) 2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

// Make sure writing to this char doesn't overwrite the next function (a bug
// cci/2 had at one point)
static char x;

char foo(void) {
    return x;
}

int main(void) {
    x = 1;
    return !foo();
}
