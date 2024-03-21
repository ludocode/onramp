// The MIT License (MIT)
// Copyright (c) 2023-2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

int foo(void) {
    return -1;
}

int main(void) {
    char c = foo();
    if (c != -1) {
        return 1;
    }
    return 0;
}
