// The MIT License (MIT)
// Copyright (c) 2023-2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

int main(void) {

    // actual bug from strndup() at one point
    char* s = strndup("./include/include-two.c", 9);
    puts(s);
    if (0 != strcmp(s, "./include")) {
        return 1;
    }
    free(s);

    return 0;
}
