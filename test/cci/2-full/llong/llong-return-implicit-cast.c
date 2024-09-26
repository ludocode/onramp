// The MIT License (MIT)
// Copyright (c) 2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

long long foo(void) {
    return 5ll;
}

int main(void) {
    int x = foo(); // implicit shortening cast of return value
    if (5 != x) return 1;
}
