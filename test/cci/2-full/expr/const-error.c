// The MIT License (MIT)
// Copyright (c) 2023-2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

#include <string.h>

int main(void) {
    const char* p = "Hello!";
    char* q = p; // ERROR: assignment removes const (-Wdiscarded-qualifiers)
}
