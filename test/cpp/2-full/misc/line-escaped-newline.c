// The MIT License (MIT)
// Copyright (c) 2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

int main(void) {
    if (__LINE__ != 6) return 1;

    // escaped newlines should not affect physical line count
    #define FOO \
    \

    if (__LINE__ != 12) return 1;

    // #line should change line number for following line
    #line 7
    if (__LINE__ != 7) return 1;

    // #line with escaped newline should continue until real line ending
    #line 19 \
    \

    if (__LINE__ != 19) return 1;
}
