// The MIT License (MIT)
// Copyright (c) 2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

// This is allowed in C. It's called a tentative definition.
int x;
int x;

int main(void) {
    if (x != 0) return 1;
}