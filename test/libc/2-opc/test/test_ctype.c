// The MIT License (MIT)
// Copyright (c) 2023-2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

#include <ctype.h>

int main(void) {
    if (!isalnum('A')) {return 1;}
    if (!isalnum('Z')) {return 1;}
    if (!isalnum('a')) {return 1;}
    if (!isalnum('z')) {return 1;}
    if (!isalnum('0')) {return 1;}
    if (!isalnum('1')) {return 1;}
    if (!isalnum('9')) {return 1;}
    if (isalnum('@')) {return 1;}
    if (isalnum('#')) {return 1;}
    if (isalnum('-')) {return 1;}
    return 0;
}
