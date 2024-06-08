// The MIT License (MIT)
// Copyright (c) 2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

#include <string.h>

int main(void) {
    if (0 != strcmp("Hello world!", "Hello" " " "world" "!")) {
        return 1;
    }
    return 0;
}
