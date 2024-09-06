// The MIT License (MIT)
// Copyright (c) 2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

int main(void) {

    // Is a trailing comma allowed on a scalar initializer? The C17 grammar
    // suggests it is. GCC, Clang and MSVC accept; TinyCC and chibicc reject.
    // Onramp matches the behaviour of the popular compilers.

    int x = {4,};
    if (x != 4) return 1;

    struct {
       int x, y;
    } a = {7, {9,},};
    if (a.x != 7) return 2;
    if (a.y != 9) return 3;

}
