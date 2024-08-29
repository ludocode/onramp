// The MIT License (MIT)
// Copyright (c) 2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

int main(void) {
    {
        extern int x;
    }
    return x; // ERROR: x not visible here
}

int x = 1;
