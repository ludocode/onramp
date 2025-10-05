// The MIT License (MIT)
// Copyright (c) 2025 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

struct {
    char c[150000];
} a, b, c;

int main(void) {
    a = (b = c); // must not use stack space
}
