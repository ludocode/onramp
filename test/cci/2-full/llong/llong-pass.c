// The MIT License (MIT)
// Copyright (c) 2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

// Test of passing and returning llongs from functions. They are larger than a
// register so they are passed on the stack.

long long max(long long a, long long b) {
    return a > b ? a : b;
}

int main(void) {
    if (max(0ll, 1ll) != 1ll) {
        return 1;
    }
    if (max(9223372036854775807ll, 9223372036854775806ll) != 9223372036854775807ll) {
        return 1;
    }
    if (max(-9223372036854775807ll, -9223372036854775807ll-1) != -9223372036854775807ll) {
        return 1;
    }
    return 0;
}
