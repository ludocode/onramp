// The MIT License (MIT)
// Copyright (c) 2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

#include <string.h>
#include <strings.h>
#include <stdbool.h>

int compare(bool case_sensitive, const char* left, const char* right) {
    // use ternary conditional on functions
    return (case_sensitive ? strcmp : strcasecmp)(left, right);
}

int main(void) {
    if (0 == compare(true, "Hello", "hello"))
        return 1;
    if (0 != compare(false, "Hello", "hello"))
        return 2;
    return 0;
}
