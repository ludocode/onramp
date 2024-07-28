// The MIT License (MIT)
// Copyright (c) 2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

int main(void) {

    // make sure while loop in statement expression doesn't clobber our
    // registers (a bug we had at one point)
    int x;
    x = 3 + ({
        int x = 1;
        while (x) {
            x = 0;
        }
        5;
    });

    if (x != 8)
        return 1;
    return 0;
}
