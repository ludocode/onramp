// The MIT License (MIT)
// Copyright (c) 2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

static int x = 4;

int main(void) {
    // This is allowed as long as the static definition comes first. GCC and
    // Clang don't warn even under -Wpedantic but TinyCC does.
    extern int x;
    return 4 - x;
}
