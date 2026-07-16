// The MIT License (MIT)
// Copyright (c) 2026 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

// A declaration is not executable, and control flow does not need to pass
// through it to be valid.

int main(void) {
    while (0) {
        int y;
    foo:
        y = 2;
        return y - 2;
    }
    goto foo;
    return 1;
}
