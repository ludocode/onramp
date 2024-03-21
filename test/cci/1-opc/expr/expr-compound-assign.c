// The MIT License (MIT)
// Copyright (c) 2023-2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

int main(void) {
    int x = 4;

    // simple compound assignment addition
    x += 3;
    if (x != 7) {
        return 1;
    }

    // using the result of the assignment
    if ((x += 5) != 12) {
        return 1;
    }
    if (x != 12) {
        return 1;
    }

    // other operators
    x <<= 2;
    if (x != 48) {
        return 1;
    }
    x -= 17;
    if (x != 31) {
        return 1;
    }
    x &= 85;
    if (x != 21) {
        return 1;
    }
    x /= 3;
    if (x != 7) {
        return 1;
    }


    return 0;
}
