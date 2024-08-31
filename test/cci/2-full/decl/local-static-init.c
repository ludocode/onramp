// The MIT License (MIT)
// Copyright (c) 2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

static int foo(void) {
    static int x = 4;
    return ++x;
}

int main(void) {
    if (foo() != 5) return 1;
    if (foo() != 6) return 2;
    if (foo() != 7) return 3;
}
