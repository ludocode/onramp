// The MIT License (MIT)
// Copyright (c) 2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

static int foo(void) {
    static int x;
    return ++x;
}

int main(void) {
    if (foo() != 1) return 1;
    if (foo() != 2) return 2;
    if (foo() != 3) return 3;
}
