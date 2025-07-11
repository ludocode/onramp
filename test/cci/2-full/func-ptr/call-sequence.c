// The MIT License (MIT)
// Copyright (c) 2025 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

int foo(void) {
    return 4;
}

int main(void) {
    if (4 != (0, foo)()) return 1;
}
