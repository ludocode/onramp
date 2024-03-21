// The MIT License (MIT)
// Copyright (c) 2023-2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

int foo(void) {
    return 'f';
}

int bar(void) {
    return '\n';
}

int main(void) {
    if (foo() != 102) {
        return 1;
    }
    if (bar() == 0x0A) {
        return 0;
    }
    return 1;
}
