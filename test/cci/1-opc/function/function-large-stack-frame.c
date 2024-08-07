// The MIT License (MIT)
// Copyright (c) 2023-2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

int foo(int x) {
    int foo[100]; // make this stack frame huge
    foo[0] = x + 1;  // make sure our arg hasn't been clobbered
    foo[99] = foo[0] + 2;
    return 6 - foo[99];
}

int main(void) {
    return foo(3);
}
