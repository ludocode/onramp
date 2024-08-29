// The MIT License (MIT)
// Copyright (c) 2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

int main(void) {
    extern int x;
    return 4 - x;
}

static int x = 4; // ERROR: static declaration follows non-static declaration
