// The MIT License (MIT)
// Copyright (c) 2026 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

//% FAIL

int a[5];
int b[] = {[-1] = 1}; // ERROR: designator with negative index
int c[5];

int main(void) {
    return 1;
}
