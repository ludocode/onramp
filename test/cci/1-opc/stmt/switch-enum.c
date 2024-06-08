// The MIT License (MIT)
// Copyright (c) 2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

enum foo {
    A,
    B,
    C,
};

int main(void) {
    enum foo foo = A;

    switch (foo) {
        case A:
            break;
        case B:
        case C:
            return 1;
    }

    return 0;
}
