// The MIT License (MIT)
// Copyright (c) 2026 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

// A declaration is not executable, and control flow does not need to pass
// through it to be valid.

int x = 2;

int main(void) {
    switch (x) {
        int y;

        case 2:
            y = 2;
            return y - x;

        default:
            return 1;
    }
    return 2;
}
