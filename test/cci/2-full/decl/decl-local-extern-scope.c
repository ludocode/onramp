// The MIT License (MIT)
// Copyright (c) 2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

static int x = 3;
extern int x;

int main(void) {
    int x = 7;
    {
        extern int x; // refers to x at file scope
        return 3 - x;
    }
}
