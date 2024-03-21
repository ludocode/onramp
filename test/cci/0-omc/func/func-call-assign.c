// The MIT License (MIT)
// Copyright (c) 2023-2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

int foo(int x) {
    return 2 * x;
}

int bar(int x, int y) {
    return x + y;
}

int main(void) {
    int x = 0;
    int y = 0;

    // assignment expressions are allowed inside function arguments.
    if (foo(x = 4) != 8) {
        return 1;
    }
    if (x != 4) {
        return 1;
    }
    if (bar(x = 3, y = 7) != 10) {
        return 1;
    }
    if (x != 3) {
        return 1;
    }
    if (y != 7) {
        return 1;
    }

    return 0;
}
