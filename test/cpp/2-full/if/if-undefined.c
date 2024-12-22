// The MIT License (MIT)
// Copyright (c) 2023-2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

int main(void) {

    // Any identifier that isn't recognized as a macro is treated as a zero.
    #if FOO
    return 1;
    #endif

    // The zero should be usable in expressions.
    #if FOO || BAR
    return 1;
    #endif
    #if !FOO
    return 0;
    #endif

    return 1;
}
