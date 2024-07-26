// The MIT License (MIT)
// Copyright (c) 2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

#include <stdbool.h>

static bool x;

bool foo(void) {
    return x;
}

int main(void) {
    x = true;
    return !foo();
}
