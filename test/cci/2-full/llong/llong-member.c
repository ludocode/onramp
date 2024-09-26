// The MIT License (MIT)
// Copyright (c) 2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

struct a {
    long long b;
} a;

long long x(void) {
    return a.b;
}

int main(void) {
    if (x() != 0) return 1;

    a.b = 4;
    if (4 - x() != 0) return 2;
}
