// The MIT License (MIT)
// Copyright (c) 2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

int main(void) {
    // sizeof cannot be used on function types. For some reason GCC and Clang
    // allow this and return 1 as the size of a function. They only warn about
    // this under -Wpointer-arith.
    return 1 | sizeof(main); // ERROR, cannot take the size of a function
}
