// The MIT License (MIT)
// Copyright (c) 2023-2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

int main(void) {

    // simple label test
    goto foo;
    return 1;
foo:

    // jumping into a loop
    int x = 0;
    goto bar;
    return 1;
    while (1) {
        break;
    bar:
        x = 1;
    }
    if (x != 1) {
        return 1;
    }

    // unbraced if statement with label
    if (0)
        baz: return 1;

    return 0;
}
