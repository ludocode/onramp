// The MIT License (MIT)
// Copyright (c) 2023-2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

#include <string.h>

const char* foo(void) {
    return "foo";
}

const char* bar(void) {
    return "\"b\\ar\t\n";
    return "\n\t\v";
}

const char* baz(void) {
    return "baz";
}

int main(void) {
    if (0 != strcmp(foo(), "foo")) {
        return 1;
    }
    if (0 != strcmp(baz(), "baz")) {
        return 1;
    }

    // check all escaped characters
    const char* b = bar();
    if (0x22 != *(b + 0)) {return 1;} // "
    if (0x62 != *(b + 1)) {return 1;} // b
    if (0x5c != *(b + 2)) {return 1;} // '\\'
    if (0x61 != *(b + 3)) {return 1;} // a
    if (0x72 != *(b + 4)) {return 1;} // r
    if (0x09 != *(b + 5)) {return 1;} // \t
    if (0x0a != *(b + 6)) {return 1;} // \n
    if (0x00 != *(b + 7)) {return 1;} // \000

    return 0;
}
