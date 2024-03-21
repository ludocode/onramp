// The MIT License (MIT)
// Copyright (c) 2023-2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

int foo(int* x) {
    *x = 5;
    return 0;
}

int main(void) {
    int x;

    // only the last part of a conditional is returned
    if (1, 0, 1, 0) {
        return 1;
    }
    if (!(0, 1, 0, 1)) {
        return 1;
    }

    // the other parts are still evaluated and `,` is a sequence point
    if (foo(&x), x - 5) {
        return 1;
    }

    // comma has lower precedence than assignment
    x = 4, 7;
    if (x != 4) {
        return 1;
    }

    // comma is allowed inside the true branch of a conditional
    x = 1 ? 5, 3 : -1;
    if (x != 3) {
        return 1;
    }

    // comma has lower precedence than the outsides of a conditional
    if (1, 1 ? 1 : 1, 0) {
        return 1;
    }

    return 0;
}
