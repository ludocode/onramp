// The MIT License (MIT)
// Copyright (c) 2023-2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

int main(void) {
    int x = 0;

    // for loop with declaration
    for (int y = 0; y < 5; ++y) {
        x += y;
    }
    if (x != 10) {
        return 1;
    }

    // for loop without declaration, comma operator in expression
    int y;
    for (y = 5, y = 3; y > -2; y = y - 1) {
        x = (x - 1);
    }
    if (x != 5) {
        return 2;
    }

    // for loop with false condition
    for (x = 1; 0; x = 10) {
        return 3;
    }
    if (x != 1) {
        return 4;
    }

    // infinite for loop
    x = 1;
    for (;;) {
        x = 2;
        break;
    }
    if (x != 2) {
        return 5;
    }

    // for loop with only condition
    x = 5;
    for (; x; ) --x;
    if (x != 0) {
        return 6;
    }

    // continue should run the increment
    for (x = 0; x < 5; ++x)
        continue;

    // for loop without braces should work normally
    x = 8;
    for (int i = 0; i < 2; ++i)
        x -= 4;
    if (x != 0)
        return 7;

    return 0;
}
