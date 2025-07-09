// The MIT License (MIT)
// Copyright (c) 2025 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

int
// The lexer should parse an angle-bracketed include as a string even if there
// is whitespace between `#` and `include`.
#   include <include/include-hello.h>
;

int main(void) {
    return hello;
}
