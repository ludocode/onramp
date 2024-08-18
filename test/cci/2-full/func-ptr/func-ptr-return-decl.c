// The MIT License (MIT)
// Copyright (c) 2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

#include <string.h>
#include <strings.h>
#include <stdbool.h>

// define a function that returns a function pointer
int (*compare(bool case_sensitive))(const char*, const char*) {
    return case_sensitive ? strcmp : strcasecmp;
}

// use a function that returns a function pointer
int main(void) {
    if (0 == compare(true)("Hello", "hello"))
        return 1;
    if (0 != compare(false)("Hello", "hello"))
        return 2;
    return 0;
}
