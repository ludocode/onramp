// The MIT License (MIT)
// Copyright (c) 2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

// This is allowed in C. It's called a tentative definition.
int x;
int x;

// This is also a tentative definition. The linkage must match.
static int y;
static int y;

int main(void) {
    if (x != 0) return 1;
    if (y != 0) return 2;
}
