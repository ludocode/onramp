// The MIT License (MIT)
// Copyright (c) 2025 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

struct {
    char c[150000];
} a;

int main(void) {
    a; // must not use stack space
}
