// The MIT License (MIT)
// Copyright (c) 2025 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

#include <stdbool.h>
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
    setenv("__ONRAMP_TEST_FRUIT", "apple", false);
    if (NULL == getenv("__ONRAMP_TEST_FRUIT")) return 1;
    if (0 != strcmp("apple", getenv("__ONRAMP_TEST_FRUIT"))) return 2;
    if (environ_count() != initial_count + 1) return 3;

    // overwrite it
    setenv("__ONRAMP_TEST_FRUIT", "banana", true);
    if (NULL == getenv("__ONRAMP_TEST_FRUIT")) return 4;
    if (0 != strcmp("banana", getenv("__ONRAMP_TEST_FRUIT"))) return 5;
    if (environ_count() != initial_count + 1) return 6;

    // don't overwrite it
    setenv("__ONRAMP_TEST_FRUIT", "cherry", false);
    if (NULL == getenv("__ONRAMP_TEST_FRUIT")) return 7;
    if (0 != strcmp("banana" /*not cherry!*/, getenv("__ONRAMP_TEST_FRUIT"))) return 8;
    if (environ_count() != initial_count + 1) return 9;

    // overwite it with editable buffers
    char key[] = "__ONRAMP_TEST_FRUIT";
    char value[] = "date";
    setenv(key, value, true);
    if (NULL == getenv("__ONRAMP_TEST_FRUIT")) return 10;
    if (0 != strcmp("date", getenv("__ONRAMP_TEST_FRUIT"))) return 11;
    if (environ_count() != initial_count + 1) return 12;

    // make sure it copied our buffers
    key[0] = 0;
    value[0] = 0;
    if (NULL == getenv("__ONRAMP_TEST_FRUIT")) return 13;
    if (0 != strcmp("date", getenv("__ONRAMP_TEST_FRUIT"))) return 14;
}
