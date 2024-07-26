// The MIT License (MIT)
// Copyright (c) 2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

static int x;

int foo(void) {
    return x;
}

int main(void) {
    x = 1;
    return !foo();
}
