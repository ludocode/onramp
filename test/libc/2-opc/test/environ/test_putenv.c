// The MIT License (MIT)
// Copyright (c) 2025 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

#include <stdlib.h>
#include <string.h>

extern char** environ;

static size_t environ_count(void) {
    for (size_t count = 0;; ++count) {
        if (environ[count] == NULL) {
            return count;
        }
    }
    __builtin_unreachable();
}

int main(void) {
    size_t initial_count = environ_count();

    // set a variable
    char* var = "__ONRAMP_TEST_FRUIT=apple";
    putenv(var);
    if (getenv("__ONRAMP_TEST_FRUIT") != strchr(var, '=') + 1) return 1;
    if (environ_count() != initial_count + 1) return 2;

    // overwrite it
    var = "__ONRAMP_TEST_FRUIT=banana";
    putenv(var);
    if (getenv("__ONRAMP_TEST_FRUIT") != strchr(var, '=') + 1) return 3;
    if (environ_count() != initial_count + 1) return 4;

    // overwite it with an editable buffer
    char str[] = "__ONRAMP_TEST_FRUIT=cherry\0                             ";
    putenv(str);
    if (getenv("__ONRAMP_TEST_FRUIT") != strchr(str, '=') + 1) return 5;
    if (environ_count() != initial_count + 1) return 6;

    // change it, making sure putenv() didn't copy our string
    strcpy(str, "__ONRAMP_TEST_VEGETABLE=carrot");
    if (getenv("__ONRAMP_TEST_FRUIT") != 0) return 7;
    if (getenv("__ONRAMP_TEST_VEGETABLE") != strchr(str, '=') + 1) return 8;
    if (environ_count() != initial_count + 1) return 9;
}
