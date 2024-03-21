// The MIT License (MIT)
// Copyright (c) 2023-2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

#include <string.h>

int main(void) {
    if (strchr("+-&|<>", '*')) {
        return 1;
    }

    const char* foo = "foo";
    if (strchr(foo, 'f') != foo) {
        return 1;
    }
    if (strchr(foo, 'o') != (foo + 1)) {
        return 1;
    }

    // it's possible to search for the null byte at the end of the string
    if (strchr(foo, 0) != (foo + 3)) {
        return 1;
    }

    return 0;
}
