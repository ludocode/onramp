// The MIT License (MIT)
// Copyright (c) 2023-2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

#include <string.h>

int main(void) {
    // test converting `char[]` to `const char* const`
    // (cci/1 ignores `const` but this should still work. cci/2 checks this
    // `const` conversion.)
    const char* const p = "Hello!";
    if (0 != strcmp(p, "Hello!")) return 1;
}
