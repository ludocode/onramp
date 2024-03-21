// The MIT License (MIT)
// Copyright (c) 2023-2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

#include "ctype.c"

#include "test.h"

static void test_ctype_isalnum() {
    test_assert(isalnum('A'));
    test_assert(isalnum('Z'));
    test_assert(isalnum('a'));
    test_assert(isalnum('z'));
    test_assert(isalnum('0'));
    test_assert(isalnum('1'));
    test_assert(isalnum('9'));
    test_assert(!isalnum('@'));
    test_assert(!isalnum('#'));
    test_assert(!isalnum('-'));
    //TODO
}

void test_ctype() {
    test_ctype_isalnum();
    //TODO
    puts("ctype tests pass");
}
