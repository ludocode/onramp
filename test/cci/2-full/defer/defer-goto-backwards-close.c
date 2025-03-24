// The MIT License (MIT)
// Copyright (c) 2025 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

int main(void) {
    int x = 2;

    while (0) {
    foo:
        if (x != 5)
            return 1;
        return 0;
    }

    defer x += 3;
    goto foo;
    return 2;
}
